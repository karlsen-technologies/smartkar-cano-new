#include "DriveManager.h"
#include "../VehicleManager.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store drive state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR DriveManager::State rtcDriveState = {};

// =============================================================================
// Constructor
// =============================================================================

DriveManager::DriveManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , state(rtcDriveState) {  // Initialize reference to RTC memory
}

bool DriveManager::setup() {
    Serial.println("[DriveManager] Initializing...");
    
    Serial.println("[DriveManager] Initialized:");
    Serial.println("[DriveManager]   - CAN IDs: 0x3C0 (Klemmen_Status_01), 0x0FD (ESP_21), 0x6B2 (Diagnose_01)");
    Serial.println("[DriveManager]   - Data: ignition, speed, odometer, vehicle time");
    Serial.println("[DriveManager]   - Read-only domain (no commands)");
    
    return true;
}

void DriveManager::loop() {
    // No periodic tasks
}

void DriveManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    switch (canId) {
        case CAN_ID_IGNITION:
            if (dlc >= 4) processIgnition(data);  // 0x3C0 is only 4 bytes
            break;
        case CAN_ID_SPEED:
            if (dlc >= 8) processSpeed(data);
            break;
        case CAN_ID_DIAGNOSE:
            if (dlc >= 8) processDiagnose(data);
            break;
    }
}

void DriveManager::onWakeComplete() {}
bool DriveManager::isBusy() const { return false; }

void DriveManager::processIgnition(const uint8_t* data) {
    ignitionCount++;
    auto decoded = BroadcastDecoder::decodeIgnition(data);
    
    state.keyInserted = decoded.keyInserted;
    state.ignitionOn = decoded.ignitionOn;
    state.startRequested = decoded.startRequested;
    state.ignitionUpdate = millis();
    
    if (decoded.startRequested) state.ignition = IgnitionState::START;
    else if (decoded.ignitionOn) state.ignition = IgnitionState::ON;
    else if (decoded.keyInserted) state.ignition = IgnitionState::ACCESSORY;
    else state.ignition = IgnitionState::OFF;
}

void DriveManager::processSpeed(const uint8_t* data) {
    speedCount++;
    state.speedKmh = BroadcastDecoder::decodeSpeed(data);
    state.speedUpdate = millis();
}

void DriveManager::processDiagnose(const uint8_t* data) {
    diagnoseCount++;
    auto decoded = BroadcastDecoder::decodeDiagnose(data);
    
    state.odometerKm = decoded.odometerKm;
    state.odometerUpdate = millis();
    state.year = decoded.year;
    state.month = decoded.month;
    state.day = decoded.day;
    state.hour = decoded.hour;
    state.minute = decoded.minute;
    state.second = decoded.second;
    state.timeUpdate = millis();
}
