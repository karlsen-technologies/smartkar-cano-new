#include "VehicleManager.h"
#include "../modules/CanManager.h"

VehicleManager::VehicleManager(CanManager* canMgr)
    : canManager(canMgr)
    , bodyDomain(state, this)
    , batteryDomain(state, this)
    , driveDomain(state, this)
    , climateDomain(state, this)
{
}

bool VehicleManager::setup() {
    Serial.println("[VehicleManager] Initializing vehicle domains...");
    
    // Initialize state
    state = VehicleState();
    
    Serial.println("[VehicleManager] Domains initialized:");
    Serial.println("[VehicleManager]   - BodyDomain (0x3D0, 0x3D1, 0x583)");
    Serial.println("[VehicleManager]   - BatteryDomain (0x191, 0x509, 0x5CA, 0x59E, 0x2AE)");
    Serial.println("[VehicleManager]   - DriveDomain (0x3C0, 0x0FD, 0x6B2)");
    Serial.println("[VehicleManager]   - ClimateDomain (0x66E)");
    
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
    
    // Battery domain (SOC, voltage, current, charging)
    if (!processed && batteryDomain.handlesCanId(canId)) {
        processed = batteryDomain.processFrame(canId, data, dlc);
    }
    
    // Drive domain (ignition, speed, odometer)
    if (!processed && driveDomain.handlesCanId(canId)) {
        processed = driveDomain.processFrame(canId, data, dlc);
    }
    
    // Climate domain (temperatures)
    if (!processed && climateDomain.handlesCanId(canId)) {
        processed = climateDomain.processFrame(canId, data, dlc);
    }
    
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
    
    // Battery status
    const BatteryState& batt = state.battery;
    if (batt.bms01Update > 0) {
        Serial.printf("[VehicleManager] Battery: %.1f%% %.1fV %.1fA (%.1fkW)\r\n",
            batt.socHiRes, batt.voltage, batt.current, batt.power() / 1000.0f);
    }
    if (batt.bms07Update > 0 && batt.chargingActive) {
        Serial.println("[VehicleManager] Charging: ACTIVE");
    }
    if (batt.tempUpdate > 0) {
        Serial.printf("[VehicleManager] Battery temp: %.1f°C\r\n", batt.temperature);
    }
    
    // Drive status
    const DriveState& drv = state.drive;
    if (drv.ignitionUpdate > 0) {
        const char* ignStr;
        switch (drv.ignition) {
            case IgnitionState::OFF: ignStr = "OFF"; break;
            case IgnitionState::ACCESSORY: ignStr = "ACCESSORY"; break;
            case IgnitionState::ON: ignStr = "ON"; break;
            case IgnitionState::START: ignStr = "START"; break;
            default: ignStr = "UNKNOWN"; break;
        }
        Serial.printf("[VehicleManager] Ignition: %s, Speed: %.1f km/h\r\n", ignStr, drv.speedKmh);
    }
    if (drv.odometerUpdate > 0) {
        Serial.printf("[VehicleManager] Odometer: %lu km\r\n", drv.odometerKm);
    }
    
    // Climate status
    const ClimateState& clim = state.climate;
    if (clim.klimaUpdate > 0) {
        Serial.printf("[VehicleManager] Inside temp: %.1f°C\r\n", clim.insideTemp);
        if (clim.standbyHeatingActive) {
            Serial.println("[VehicleManager] Standby heating: ON");
        }
    }
    
    Serial.println("[VehicleManager] ======================");
}
