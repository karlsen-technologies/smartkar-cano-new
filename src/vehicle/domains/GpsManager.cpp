#include "GpsManager.h"
#include "../VehicleManager.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store GPS state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR GpsManager::State rtcGpsState = {};

// =============================================================================
// Constructor
// =============================================================================

GpsManager::GpsManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , state(rtcGpsState) {  // Initialize reference to RTC memory
}

bool GpsManager::setup() {
    Serial.println("[GpsManager] Initializing...");
    Serial.println("[GpsManager] Initialized:");
    Serial.println("[GpsManager]   - CAN IDs: 0x486 (NavPos_01), 0x485 (NavData_02), 0x484 (NavData_01)");
    Serial.println("[GpsManager]   - Data: position, altitude, satellites, heading, DOP");
    Serial.println("[GpsManager]   - Read-only domain (no commands)");
    return true;
}

void GpsManager::loop() {}

void GpsManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) return;
    
    switch (canId) {
        case CAN_ID_NAV_POS_01:
            processNavPos01(data);
            break;
        case CAN_ID_NAV_DATA_02:
            processNavData02(data);
            break;
        case CAN_ID_NAV_DATA_01:
            processNavData01(data);
            break;
    }
}

void GpsManager::onWakeComplete() {}
bool GpsManager::isBusy() const { return false; }

void GpsManager::processNavPos01(const uint8_t* data) {
    navPos01Count++;
    auto decoded = BroadcastDecoder::decodeNavPos01(data);
    state.latitude = decoded.latitude;
    state.longitude = decoded.longitude;
    state.satellites = decoded.satellites;
    state.fixType = decoded.fixType;
    state.positionUpdate = millis();
}

void GpsManager::processNavData02(const uint8_t* data) {
    navData02Count++;
    auto decoded = BroadcastDecoder::decodeNavData02(data);
    state.altitude = decoded.altitude;
    state.utcTime = decoded.utcTime;
    state.satsInUse = decoded.satsInUse;
    state.satsInView = decoded.satsInView;
    state.accuracy = decoded.accuracy;
    state.altitudeUpdate = millis();
}

void GpsManager::processNavData01(const uint8_t* data) {
    navData01Count++;
    auto decoded = BroadcastDecoder::decodeNavData01(data);
    state.heading = decoded.heading;
    state.hdop = decoded.hdop;
    state.vdop = decoded.vdop;
    state.pdop = decoded.pdop;
    state.gpsInit = decoded.gpsInit;
    state.headingUpdate = millis();
}
