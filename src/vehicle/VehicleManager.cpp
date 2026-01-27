#include "VehicleManager.h"
#include "../modules/CanManager.h"

VehicleManager::VehicleManager(CanManager* canMgr)
    : canManager(canMgr)
    , bodyDomain(state, this)
{
}

bool VehicleManager::setup() {
    Serial.println("[VehicleManager] Initializing vehicle domains...");
    
    // Initialize state
    state = VehicleState();
    
    Serial.println("[VehicleManager] Domains initialized:");
    Serial.println("[VehicleManager]   - BodyDomain (0x3D0, 0x3D1, 0x583)");
    
    return true;
}

void VehicleManager::loop() {
    // Periodic statistics logging
    if (millis() - lastLogTime > LOG_INTERVAL) {
        logStatistics();
        lastLogTime = millis();
    }
}

void VehicleManager::onCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended) {
    // Mark activity
    state.markCanActivity();
    
    // Skip extended frames for now (used by BAP protocol)
    if (extended) {
        // TODO: Route to BAP handler when implemented
        return;
    }
    
    // Route to domains based on CAN ID
    bool processed = false;
    
    // Body domain (doors, locks, windows)
    if (bodyDomain.handlesCanId(canId)) {
        processed = bodyDomain.processFrame(canId, data, dlc);
    }
    
    // TODO: Add more domains here
    // if (batteryDomain.handlesCanId(canId)) { ... }
    // if (driveDomain.handlesCanId(canId)) { ... }
    
    // Verbose logging of unprocessed frames
    if (verbose && !processed) {
        // Only log every Nth unprocessed frame to avoid spam
        static uint32_t unprocessedCount = 0;
        if (++unprocessedCount % 100 == 1) {
            Serial.printf("[VehicleManager] Unhandled ID: 0x%03lX\r\n", canId);
        }
    }
}

bool VehicleManager::sendCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended) {
    if (!canManager) {
        Serial.println("[VehicleManager] No CAN manager - cannot send");
        return false;
    }
    
    if (!canManager->isRunning()) {
        Serial.println("[VehicleManager] CAN not running - cannot send");
        return false;
    }
    
    // Build TWAI message
    twai_message_t message;
    message.identifier = canId;
    message.extd = extended ? 1 : 0;
    message.rtr = 0;
    message.data_length_code = dlc;
    memcpy(message.data, data, dlc);
    
    // Send with 100ms timeout
    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(100));
    
    if (result == ESP_OK) {
        Serial.printf("[VehicleManager] TX: ID:0x%03lX [%d]", canId, dlc);
        for (int i = 0; i < dlc; i++) {
            Serial.printf(" %02X", data[i]);
        }
        Serial.println();
        return true;
    } else if (result == ESP_ERR_TIMEOUT) {
        Serial.println("[VehicleManager] TX timeout - no ACK");
        return false;
    } else {
        Serial.printf("[VehicleManager] TX failed: %s\r\n", esp_err_to_name(result));
        return false;
    }
}

void VehicleManager::logStatistics() {
    Serial.println("[VehicleManager] === Vehicle Status ===");
    Serial.printf("[VehicleManager] CAN frames: %lu\r\n", state.canFrameCount);
    Serial.printf("[VehicleManager] Vehicle awake: %s\r\n", state.isAwake() ? "YES" : "NO");
    
    // Body status
    const BodyState& body = state.body;
    Serial.printf("[VehicleManager] Lock: %s\r\n", 
        body.centralLock == LockState::LOCKED ? "LOCKED" :
        body.centralLock == LockState::UNLOCKED ? "UNLOCKED" : "UNKNOWN");
    
    if (body.driverDoor.lastUpdate > 0) {
        Serial.printf("[VehicleManager] Driver door: %s, window: %.0f%%\r\n",
            body.driverDoor.open ? "OPEN" : "closed",
            body.driverDoor.windowPercent());
    }
    
    if (body.passengerDoor.lastUpdate > 0) {
        Serial.printf("[VehicleManager] Passenger door: %s, window: %.0f%%\r\n",
            body.passengerDoor.open ? "OPEN" : "closed",
            body.passengerDoor.windowPercent());
    }
    
    Serial.println("[VehicleManager] ======================");
}
