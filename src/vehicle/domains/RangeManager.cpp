#include "RangeManager.h"
#include "../VehicleManager.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store range state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR RangeManager::State rtcRangeState = {};

// =============================================================================
// Constructor
// =============================================================================

RangeManager::RangeManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , state(rtcRangeState) {  // Initialize reference to RTC memory
}

bool RangeManager::setup() {
    Serial.println("[RangeManager] Initializing...");
    Serial.println("[RangeManager] Initialized:");
    Serial.println("[RangeManager]   - CAN IDs: 0x5F5 (Reichweite_01), 0x5F7 (Reichweite_02)");
    Serial.println("[RangeManager]   - Data: total/electric/display range, consumption, tendency");
    Serial.println("[RangeManager]   - Read-only domain (no commands)");
    return true;
}

void RangeManager::loop() {}

void RangeManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) return;
    
    switch (canId) {
        case CAN_ID_REICHWEITE_01:
            processReichweite01(data);
            break;
        case CAN_ID_REICHWEITE_02:
            processReichweite02(data);
            break;
    }
}

void RangeManager::onWakeComplete() {}
bool RangeManager::isBusy() const { return false; }

void RangeManager::processReichweite01(const uint8_t* data) {
    reichweite01Count++;
    auto decoded = BroadcastDecoder::decodeReichweite01(data);
    
    // Skip invalid values (2045-2047)
    if (decoded.totalRange < State::INVALID_RANGE) {
        state.totalRangeKm = decoded.totalRange;
    }
    if (decoded.electricRange < State::INVALID_RANGE) {
        state.electricRangeKm = decoded.electricRange;
    }
    
    state.consumptionKwh100km = decoded.consumption;
    state.rangeUpdate = millis();
}

void RangeManager::processReichweite02(const uint8_t* data) {
    reichweite02Count++;
    auto decoded = BroadcastDecoder::decodeReichweite02(data);
    
    // Skip invalid values
    if (decoded.displayTotalRange < State::INVALID_RANGE) {
        state.displayRangeKm = decoded.displayTotalRange;
    }
    
    state.tendency = static_cast<RangeTendency>(decoded.tendency);
    state.reserveWarning = decoded.reserveWarning;
    state.displayUpdate = millis();
}
