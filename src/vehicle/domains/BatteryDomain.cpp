#include "BatteryDomain.h"
#include "../VehicleManager.h"

BatteryDomain::BatteryDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool BatteryDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_BMS_07 ||
           canId == CAN_ID_BMS_06 ||
           canId == CAN_ID_MOTOR_HYBRID_06;
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
            
        case CAN_ID_MOTOR_HYBRID_06:
            processMotorHybrid06(data);
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
    
    // Update unified charging field (CAN source)
    battery.charging = decoded.chargingActive;
    battery.chargingSource = DataSource::CAN_STD;
    battery.chargingUpdate = millis();
    
    battery.balancingActive = decoded.balancingActive;
    battery.energyWh = decoded.energyWh;
    battery.maxEnergyWh = decoded.maxEnergyWh;
    battery.energyUpdate = millis();
    battery.balancingUpdate = millis();
    
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

void BatteryDomain::processMotorHybrid06(const uint8_t* data) {
    motorHybrid06Count++;
    // Motor_Hybrid_06 (0x483) - Power meter for charging/climate
    BroadcastDecoder::MotorHybrid06Data decoded = BroadcastDecoder::decodeMotorHybrid06(data);
    
    BatteryState& battery = vehicleState.battery;
    battery.powerKw = decoded.powerKw;
    battery.powerUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}
