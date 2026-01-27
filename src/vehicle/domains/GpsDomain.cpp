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
    
    gps.latitude = decoded.latitude;
    gps.longitude = decoded.longitude;
    gps.satellites = decoded.satellites;
    gps.fixType = decoded.fixType;
    gps.positionUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void GpsDomain::processNavData02(const uint8_t* data) {
    // NavData_02 (0x485) - Altitude, UTC, Satellites
    BroadcastDecoder::NavData02Data decoded = BroadcastDecoder::decodeNavData02(data);
    
    CanGpsState& gps = vehicleState.gps;
    
    gps.altitude = decoded.altitude;
    gps.utcTime = decoded.utcTime;
    gps.satsInUse = decoded.satsInUse;
    gps.satsInView = decoded.satsInView;
    gps.accuracy = decoded.accuracy;
    gps.altitudeUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void GpsDomain::processNavData01(const uint8_t* data) {
    // NavData_01 (0x484) - Heading and DOP values
    BroadcastDecoder::NavData01Data decoded = BroadcastDecoder::decodeNavData01(data);
    
    CanGpsState& gps = vehicleState.gps;
    
    gps.heading = decoded.heading;
    gps.hdop = decoded.hdop;
    gps.vdop = decoded.vdop;
    gps.pdop = decoded.pdop;
    gps.gpsInit = decoded.gpsInit;
    gps.headingUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}
