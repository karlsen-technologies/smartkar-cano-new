#include "BatteryDomain.h"
#include "../VehicleManager.h"

BatteryDomain::BatteryDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool BatteryDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_BMS_01 ||
           canId == CAN_ID_BMS_10 ||
           canId == CAN_ID_BMS_07 ||
           canId == CAN_ID_BMS_06 ||
           canId == CAN_ID_DCDC_01;
}

bool BatteryDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        // Battery CAN messages are 8 bytes
        return false;
    }
    
    switch (canId) {
        case CAN_ID_BMS_01:
            processBMS01(data);
            return true;
            
        case CAN_ID_BMS_10:
            processBMS10(data);
            return true;
            
        case CAN_ID_BMS_07:
            processBMS07(data);
            return true;
            
        case CAN_ID_BMS_06:
            processBMS06(data);
            return true;
            
        case CAN_ID_DCDC_01:
            processDCDC01(data);
            return true;
            
        default:
            return false;
    }
}

void BatteryDomain::processBMS01(const uint8_t* data) {
    // BMS_01 (0x191) - Core battery data
    BroadcastDecoder::BMS01Data decoded = BroadcastDecoder::decodeBMS01(data);
    
    BatteryState& battery = vehicleState.battery;
    battery.current = decoded.current;
    battery.voltage = decoded.voltage;
    battery.socHiRes = decoded.socHiRes;
    battery.bms01Update = millis();
    
    // Debug log on significant changes (every ~10 seconds or when SOC changes)
    static unsigned long lastLog = 0;
    static float lastSoc = 0;
    
    if (millis() - lastLog > 10000 || abs(battery.socHiRes - lastSoc) > 0.5) {
        Serial.printf("[BatteryDomain] BMS_01: V=%.1fV I=%.1fA SOC=%.1f%% P=%.1fkW\r\n",
            battery.voltage, battery.current, battery.socHiRes, powerKw());
        lastLog = millis();
        lastSoc = battery.socHiRes;
    }
}

void BatteryDomain::processBMS10(const uint8_t* data) {
    // BMS_10 (0x509) - Usable SOC and energy content
    BroadcastDecoder::BMS10Data decoded = BroadcastDecoder::decodeBMS10(data);
    
    BatteryState& battery = vehicleState.battery;
    battery.energyWh = decoded.energyWh;
    battery.maxEnergyWh = decoded.maxEnergyWh;
    battery.usableSoc = decoded.usableSoc;
    battery.bms10Update = millis();
    
    // Debug log occasionally
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 60000) {  // Every 60 seconds
        Serial.printf("[BatteryDomain] BMS_10: usable=%.1f%% energy=%.0fWh/%.0fWh\r\n",
            battery.usableSoc, battery.energyWh, battery.maxEnergyWh);
        lastLog = millis();
    }
}

void BatteryDomain::processBMS07(const uint8_t* data) {
    // BMS_07 (0x5CA) - Charging status
    BroadcastDecoder::BMS07Data decoded = BroadcastDecoder::decodeBMS07(data);
    
    BatteryState& battery = vehicleState.battery;
    bool wasCharging = battery.chargingActive;
    
    battery.chargingActive = decoded.chargingActive;
    battery.balancingActive = decoded.balancingActive;
    battery.bms07Update = millis();
    
    // Log charging state changes
    if (decoded.chargingActive != wasCharging) {
        Serial.printf("[BatteryDomain] Charging: %s\r\n", 
            decoded.chargingActive ? "STARTED" : "STOPPED");
    }
    
    // Log balancing status occasionally
    static unsigned long lastLog = 0;
    if (decoded.balancingActive && millis() - lastLog > 60000) {
        Serial.println("[BatteryDomain] Cell balancing active");
        lastLog = millis();
    }
}

void BatteryDomain::processBMS06(const uint8_t* data) {
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

void BatteryDomain::processDCDC01(const uint8_t* data) {
    // DCDC_01 (0x2AE) - DC-DC converter
    BroadcastDecoder::DCDC01Data decoded = BroadcastDecoder::decodeDCDC01(data);
    
    BatteryState& battery = vehicleState.battery;
    battery.dcdc12v = decoded.lvVoltage;
    battery.dcdcCurrent = decoded.lvCurrent;
    battery.dcdcUpdate = millis();
    
    // Log occasionally
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 60000) {  // Every 60 seconds
        Serial.printf("[BatteryDomain] DCDC: 12V=%.1fV HV=%.1fV I=%.1fA\r\n",
            decoded.lvVoltage, decoded.hvVoltage, decoded.lvCurrent);
        lastLog = millis();
    }
}
