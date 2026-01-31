#include "BodyManager.h"
#include "../VehicleManager.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store body state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR BodyManager::State rtcBodyState = {};

// =============================================================================
// Constructor
// =============================================================================

BodyManager::BodyManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , state(rtcBodyState) {  // Initialize reference to RTC memory
}

bool BodyManager::setup() {
    Serial.println("[BodyManager] Initializing...");
    
    if (!vehicleManager) {
        Serial.println("[BodyManager] ERROR: No VehicleManager!");
        return false;
    }
    
    Serial.println("[BodyManager] Initialized:");
    Serial.println("[BodyManager]   - CAN IDs: 0x3D0 (TSG_FT_01), 0x3D1 (TSG_BT_01), 0x583 (ZV_02)");
    Serial.println("[BodyManager]   - Commands: horn, flash, lock, unlock, panic (TM_01)");
    Serial.println("[BodyManager]   - Data source: CAN only");
    
    return true;
}

void BodyManager::loop() {
    // No periodic tasks needed for now
    // All processing happens in CAN frame callbacks
}

void BodyManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // Called from CAN task (Core 0) with mutex held
    // MUST be fast (<1-2ms)
    
    if (dlc < 8) {
        return;  // Need full frame
    }
    
    switch (canId) {
        case CAN_ID_DRIVER_DOOR:
            processDriverDoor(data);
            break;
            
        case CAN_ID_PASSENGER_DOOR:
            processPassengerDoor(data);
            break;
            
        case CAN_ID_LOCK_STATUS:
            processLockStatus(data);
            break;
            
        default:
            // Not our frame
            break;
    }
}

void BodyManager::onWakeComplete() {
    // No special action needed after wake
}

bool BodyManager::isBusy() const {
    // Body commands are one-shot (no state machine)
    return false;
}

// =============================================================================
// CAN Frame Processors
// =============================================================================

void BodyManager::processDriverDoor(const uint8_t* data) {
    // TSG_FT_01 (0x3D0) - Driver door status
    driverDoorCount++;
    
    auto decoded = BroadcastDecoder::decodeDriverDoor(data);
    
    state.driverDoor.open = decoded.doorOpen;
    state.driverDoor.locked = decoded.doorLocked;
    state.driverDoor.windowPosition = decoded.windowPos;
    state.driverDoor.lastUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BodyManager::processPassengerDoor(const uint8_t* data) {
    // TSG_BT_01 (0x3D1) - Passenger door status
    passengerDoorCount++;
    
    auto decoded = BroadcastDecoder::decodePassengerDoor(data);
    
    state.passengerDoor.open = decoded.doorOpen;
    state.passengerDoor.locked = decoded.doorLocked;
    state.passengerDoor.windowPosition = decoded.windowPos;
    state.passengerDoor.lastUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BodyManager::processLockStatus(const uint8_t* data) {
    // ZV_02 (0x583) - Central locking status
    lockStatusCount++;
    
    auto decoded = BroadcastDecoder::decodeLockStatus(data);
    
    // Store raw bytes for debugging
    state.zv02_byte2 = decoded.byte2;
    state.zv02_byte7 = decoded.byte7;
    
    // Update lock state
    state.centralLock = decoded.isLocked ? LockState::LOCKED : LockState::UNLOCKED;
    state.centralLockUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// Command Interface
// =============================================================================

bool BodyManager::sendTm01Command(Tm01Commands::Command cmd) {
    if (!vehicleManager) {
        Serial.println("[BodyManager] ERROR: No VehicleManager!");
        return false;
    }
    
    uint8_t data[8];
    Tm01Commands::buildCommand(cmd, data);
    
    Serial.printf("[BodyManager] Sending %s command\r\n", Tm01Commands::getCommandName(cmd));
    
    return vehicleManager->sendCanFrame(Tm01Commands::getCanId(), data, 8, false);
}

bool BodyManager::horn() {
    return sendTm01Command(Tm01Commands::Command::HORN);
}

bool BodyManager::flash() {
    return sendTm01Command(Tm01Commands::Command::FLASH);
}

bool BodyManager::lock() {
    return sendTm01Command(Tm01Commands::Command::LOCK);
}

bool BodyManager::unlock() {
    return sendTm01Command(Tm01Commands::Command::UNLOCK);
}

bool BodyManager::panic() {
    return sendTm01Command(Tm01Commands::Command::PANIC);
}
