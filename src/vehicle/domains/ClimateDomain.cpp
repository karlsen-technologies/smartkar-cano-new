#include "ClimateDomain.h"
#include "../VehicleManager.h"

ClimateDomain::ClimateDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool ClimateDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_KLIMA_03;
}

bool ClimateDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        return false;
    }
    
    switch (canId) {
        case CAN_ID_KLIMA_03:
            processKlima03(data);
            return true;
            
        default:
            return false;
    }
}

void ClimateDomain::processKlima03(const uint8_t* data) {
    // Klima_03 (0x66E) - Climate status
    BroadcastDecoder::KlimaData decoded = BroadcastDecoder::decodeKlima03(data);
    
    ClimateState& climate = vehicleState.climate;
    bool wasHeating = climate.standbyHeatingActive;
    bool wasVent = climate.standbyVentActive;
    
    climate.insideTemp = decoded.insideTemp;
    climate.standbyHeatingActive = decoded.standbyHeatingActive;
    climate.standbyVentActive = decoded.standbyVentActive;
    climate.klimaUpdate = millis();
    
    // Log changes to standby climate
    if (decoded.standbyHeatingActive != wasHeating) {
        Serial.printf("[ClimateDomain] Standby heating: %s\r\n", 
            decoded.standbyHeatingActive ? "ON" : "OFF");
    }
    if (decoded.standbyVentActive != wasVent) {
        Serial.printf("[ClimateDomain] Standby ventilation: %s\r\n", 
            decoded.standbyVentActive ? "ON" : "OFF");
    }
    
    // Log temperature occasionally
    static unsigned long lastLog = 0;
    static float lastTemp = 0;
    
    if (millis() - lastLog > 60000 || abs(climate.insideTemp - lastTemp) > 1.0) {
        Serial.printf("[ClimateDomain] Inside temp: %.1f°C\r\n", climate.insideTemp);
        lastLog = millis();
        lastTemp = climate.insideTemp;
    }
}
