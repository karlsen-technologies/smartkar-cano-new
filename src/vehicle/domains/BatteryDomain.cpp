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
    bool wasCharging = battery.chargingActive;
    
    battery.chargingActive = decoded.chargingActive;
    battery.balancingActive = decoded.balancingActive;
    battery.energyWh = decoded.energyWh;
    battery.maxEnergyWh = decoded.maxEnergyWh;
    battery.bms07Update = millis();
    
    // Log charging state changes
    if (decoded.chargingActive != wasCharging) {
        Serial.printf("[BatteryDomain] Charging: %s\r\n", 
            decoded.chargingActive ? "STARTED" : "STOPPED");
    }
    
    // Log energy occasionally
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 60000) {
        Serial.printf("[BatteryDomain] Energy: %.0f/%.0f Wh (%.0f%%)\r\n",
            battery.energyWh, battery.maxEnergyWh, energyPercent());
        lastLog = millis();
    }
}

void BatteryDomain::processBMS06(const uint8_t* data) {
    bms06Count++;
    // BMS_06 (0x59E) - Battery temperature
    float temp = BroadcastDecoder::decodeBMS06Temperature(data);
    
    BatteryState& battery = vehicleState.battery;
    battery.temperature = temp;
    battery.tempUpdate = millis();
    
    // Log occasionally or on significant change
    static unsigned long lastLog = 0;
    static float lastTemp = 0;
    
    if (millis() - lastLog > 60000 || abs(temp - lastTemp) > 2.0) {
        Serial.printf("[BatteryDomain] Battery temp: %.1f°C\r\n", temp);
        lastLog = millis();
        lastTemp = temp;
    }
}
