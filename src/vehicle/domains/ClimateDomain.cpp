#include "ClimateDomain.h"
#include "../VehicleManager.h"

ClimateDomain::ClimateDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool ClimateDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_KLIMA_03 ||
           canId == CAN_ID_KLIMA_SENSOR_02;
}

bool ClimateDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        return false;
    }
    
    switch (canId) {
        case CAN_ID_KLIMA_03:
            processKlima03(data);
            return true;
            
        case CAN_ID_KLIMA_SENSOR_02:
            processKlimaSensor02(data);
            return true;
            
        default:
            return false;
    }
}

void ClimateDomain::processKlima03(const uint8_t* data) {
    // Klima_03 (0x66E) - Climate status and inside temperature
    BroadcastDecoder::KlimaData decoded = BroadcastDecoder::decodeKlima03(data);
    
    ClimateState& climate = vehicleState.climate;
    
    // Update inside temperature (CAN source - only if BAP hasn't updated it recently)
    // BAP takes priority when climate is actively controlled
    if (climate.insideTempSource != DataSource::BAP || 
        (millis() - climate.insideTempUpdate) > 5000) {
        climate.insideTemp = decoded.insideTemp;
        climate.insideTempSource = DataSource::CAN_STD;
        climate.insideTempUpdate = millis();
    }
    
    // Update standby modes (CAN only - not controlled via BAP)
    climate.standbyHeatingActive = decoded.standbyHeatingActive;
    climate.standbyVentActive = decoded.standbyVentActive;
    climate.standbyUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void ClimateDomain::processKlimaSensor02(const uint8_t* data) {
    // Klima_Sensor_02 (0x5E1) - Outside temperature
    // BCM1_Aussen_Temp_ungef: Byte 0, scale 0.5, offset -50
    uint8_t rawTemp = data[0];
    float outsideTemp = rawTemp * 0.5f - 50.0f;
    
    ClimateState& climate = vehicleState.climate;
    
    climate.outsideTemp = outsideTemp;
    climate.outsideTempUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}
