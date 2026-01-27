#include "BatteryDomain.h"
#include "../VehicleManager.h"

BatteryDomain::BatteryDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool BatteryDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_BMS_07 ||
           canId == CAN_ID_BMS_06;
}

bool BatteryDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        return false;
    }
    
    switch (canId) {
        case CAN_ID_BMS_07:
            processBMS07(data);
            return true;
            
        case CAN_ID_BMS_06:
            processBMS06(data);
            return true;
            
        default:
            return false;
    }
}

void BatteryDomain::processBMS07(const uint8_t* data) {
    bms07Count++;
    // BMS_07 (0x5CA) - Charging status and energy content
    BroadcastDecoder::BMS07Data decoded = BroadcastDecoder::decodeBMS07(data);
    
    BatteryState& battery = vehicleState.battery;
    
    battery.chargingActive = decoded.chargingActive;
    battery.balancingActive = decoded.balancingActive;
    battery.energyWh = decoded.energyWh;
    battery.maxEnergyWh = decoded.maxEnergyWh;
    battery.bms07Update = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryDomain::processBMS06(const uint8_t* data) {
    bms06Count++;
    // BMS_06 (0x59E) - Battery temperature
    float temp = BroadcastDecoder::decodeBMS06Temperature(data);
    
    BatteryState& battery = vehicleState.battery;
    battery.temperature = temp;
    battery.tempUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}
