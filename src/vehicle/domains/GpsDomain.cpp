#include "GpsDomain.h"
#include "../VehicleManager.h"

GpsDomain::GpsDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool GpsDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_NAV_POS_01 ||
           canId == CAN_ID_NAV_DATA_02 ||
           canId == CAN_ID_NAV_DATA_01;
}

bool GpsDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        return false;
    }
    
    switch (canId) {
        case CAN_ID_NAV_POS_01:
            processNavPos01(data);
            navPos01Frames++;
            return true;
            
        case CAN_ID_NAV_DATA_02:
            processNavData02(data);
            navData02Frames++;
            return true;
            
        case CAN_ID_NAV_DATA_01:
            processNavData01(data);
            navData01Frames++;
            return true;
            
        default:
            return false;
    }
}

void GpsDomain::processNavPos01(const uint8_t* data) {
    // NavPos_01 (0x486) - GPS Position
    BroadcastDecoder::NavPosData decoded = BroadcastDecoder::decodeNavPos01(data);
    
    CanGpsState& gps = vehicleState.gps;
    
    // Check for significant position change (> 0.0001 degrees ~ 11 meters)
    bool positionChanged = (abs(decoded.latitude - gps.latitude) > 0.0001 ||
                           abs(decoded.longitude - gps.longitude) > 0.0001);
    bool fixChanged = (decoded.fixType != gps.fixType);
    
    gps.latitude = decoded.latitude;
    gps.longitude = decoded.longitude;
    gps.satellites = decoded.satellites;
    gps.fixType = decoded.fixType;
    gps.positionUpdate = millis();
    
    // Log on fix change or significant movement
    if (fixChanged) {
        Serial.printf("[GpsDomain] Fix: %s (%d sats)\r\n", 
            gps.fixTypeStr(), decoded.satellites);
    }
    if (positionChanged && decoded.fixType >= 2) {
        Serial.printf("[GpsDomain] Position: %.6f, %.6f\r\n", 
            decoded.latitude, decoded.longitude);
    }
}

void GpsDomain::processNavData02(const uint8_t* data) {
    // NavData_02 (0x485) - Altitude, UTC, Satellites
    BroadcastDecoder::NavData02Data decoded = BroadcastDecoder::decodeNavData02(data);
    
    CanGpsState& gps = vehicleState.gps;
    
    // Check for significant altitude change (> 5 meters)
    bool altChanged = (abs(decoded.altitude - gps.altitude) > 5.0f);
    
    gps.altitude = decoded.altitude;
    gps.utcTime = decoded.utcTime;
    gps.satsInUse = decoded.satsInUse;
    gps.satsInView = decoded.satsInView;
    gps.accuracy = decoded.accuracy;
    gps.altitudeUpdate = millis();
    
    // Log on significant altitude change
    if (altChanged && gps.hasFix()) {
        Serial.printf("[GpsDomain] Altitude: %.0fm (accuracy: %dm)\r\n", 
            decoded.altitude, decoded.accuracy);
    }
}

void GpsDomain::processNavData01(const uint8_t* data) {
    // NavData_01 (0x484) - Heading and DOP values
    BroadcastDecoder::NavData01Data decoded = BroadcastDecoder::decodeNavData01(data);
    
    CanGpsState& gps = vehicleState.gps;
    
    // Check for significant heading change (> 5 degrees)
    bool headingChanged = (abs(decoded.heading - gps.heading) > 5.0f);
    bool initChanged = (decoded.gpsInit != gps.gpsInit);
    
    gps.heading = decoded.heading;
    gps.hdop = decoded.hdop;
    gps.vdop = decoded.vdop;
    gps.pdop = decoded.pdop;
    gps.gpsInit = decoded.gpsInit;
    gps.headingUpdate = millis();
    
    // Log on GPS initialization change
    if (initChanged) {
        Serial.printf("[GpsDomain] GPS initialized: %s\r\n", 
            decoded.gpsInit ? "YES" : "NO");
    }
    
    // Log significant heading change when moving
    if (headingChanged && gps.hasFix()) {
        Serial.printf("[GpsDomain] Heading: %.1f° (HDOP: %.1f)\r\n", 
            decoded.heading, decoded.hdop);
    }
}
