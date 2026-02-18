#include "WakeController.h"
#include "../../modules/CanManager.h"
#include "../IDomain.h"
#include <driver/twai.h>

WakeController::WakeController(CanManager* canMgr)
    : canManager(canMgr) {
}

bool WakeController::setup() {
    Serial.println("[WakeController] Initialized");
    Serial.println("[WakeController]   - Wake sequence: 0x17330301 + 0x1B000067");
    Serial.println("[WakeController]   - Keep-alive: 0x5A7 (500ms interval)");
    Serial.println("[WakeController]   - Timeout: 5 minutes");
    return true;
}

void WakeController::loop(bool vehicleHasCanActivity) {
    unsigned long now = millis();
    unsigned long elapsed = now - wakeStateStartTime;

    // Clear initialization flag after first loop iteration
    if (canInitializing) {
        canInitializing = false;
        Serial.println("[WakeController] Initialization complete, starting activity tracking");
    }

    // Update keep-alive management
    if (keepAliveActive) {
        // Stop keep-alive after timeout (no commands for 5 minutes)
        if (now - lastCommandActivity > KEEPALIVE_TIMEOUT) {
            Serial.println("[WakeController] Keep-alive timeout (5 min since last command)");
            stopKeepAlive();
            // Don't force ASLEEP - let vehicle naturally go to sleep
        }

        // Send keep-alive frame every 500ms while active
        if (now - lastKeepAlive >= KEEPALIVE_INTERVAL) {
            sendKeepAliveFrame();
            lastKeepAlive = now;
        }
    }

    // Process state transitions
    switch (wakeState) {
    case WakeState::ASLEEP:
        // Check if vehicle woke up naturally (door open, key fob, etc.)
        if (vehicleHasCanActivity) {
            Serial.println("[WakeController] Vehicle woke up naturally (CAN activity detected)");
            Serial.println("[WakeController] Keep-alive NOT started (will start when command executes)");
            setWakeState(WakeState::AWAKE);
            // Don't start keep-alive - only start when a command is executed
            // This allows vehicle to go back to sleep naturally if no commands are active
        }
        break;

    case WakeState::WAKE_REQUESTED:
        // Send wake sequence
        Serial.println("[WakeController] Initiating wake sequence...");

        // Send wake frame
        if (!sendWakeFrame()) {
            Serial.println("[WakeController] Failed to send wake frame");
            setWakeState(WakeState::ASLEEP);
            wakeFailed++;
            break;
        }

        // Start keep-alive immediately
        startKeepAlive();

        // Small delay before BAP init
        delay(100);

        // Send BAP init
        if (!sendBapInitFrame()) {
            Serial.println("[WakeController] Failed to send BAP init frame");
            stopKeepAlive();
            setWakeState(WakeState::ASLEEP);
            wakeFailed++;
            break;
        }

        // Transition to WAKING
        setWakeState(WakeState::WAKING);
        break;

    case WakeState::WAKING:
        // Check if vehicle has woken up (CAN activity)
        if (vehicleHasCanActivity) {
            // Wait for BAP initialization
            if (elapsed >= BAP_INIT_WAIT) {
                Serial.printf("[WakeController] Vehicle awake after %lums\r\n", elapsed);
                setWakeState(WakeState::AWAKE);
            }
        }
        // Check for wake timeout
        else if (elapsed > WAKE_TIMEOUT) {
            Serial.println("[WakeController] Wake timeout - no CAN activity");
            stopKeepAlive();
            setWakeState(WakeState::ASLEEP);
            wakeFailed++;
        }
        break;

    case WakeState::AWAKE:
        // Vehicle went to sleep naturally (no CAN activity)
        if (!vehicleHasCanActivity) {
            Serial.println("[WakeController] Vehicle went to sleep (no CAN activity)");
            stopKeepAlive();
            setWakeState(WakeState::ASLEEP);
        }
        break;
    }
}

bool WakeController::requestWake() {
    // If already waking or awake, don't restart
    if (wakeState == WakeState::WAKING || wakeState == WakeState::AWAKE) {
        Serial.printf("[WakeController] Wake already in progress/complete (state=%s)\r\n",
                      getStateName());
        return true;
    }

    // Request wake
    Serial.println("[WakeController] Wake requested");
    
    setWakeState(WakeState::WAKE_REQUESTED);
    notifyCommandActivity();
    wakeAttempts++;

    return true;
}

void WakeController::ensureAwake() {
    // Update activity timestamp and start keep-alive if already awake
    notifyCommandActivity();
    
    // If asleep, initiate wake sequence
    if (wakeState == WakeState::ASLEEP) {
        requestWake();
    }
}

void WakeController::onCanActivity() {
    // Called on every CAN frame - lightweight tracking only
    // Don't do any processing here, just note activity occurred
}

void WakeController::notifyCommandActivity() {
    lastCommandActivity = millis();
    
    // If vehicle is awake, ensure keep-alive is active
    if (wakeState == WakeState::AWAKE && !keepAliveActive) {
        startKeepAlive();
    }
}

const char* WakeController::getStateName() const {
    switch (wakeState) {
    case WakeState::ASLEEP:
        return "ASLEEP";
    case WakeState::WAKE_REQUESTED:
        return "WAKE_REQUESTED";
    case WakeState::WAKING:
        return "WAKING";
    case WakeState::AWAKE:
        return "AWAKE";
    default:
        return "UNKNOWN";
    }
}

bool WakeController::sendWakeFrame() {
    uint8_t wakeData[4] = {0x40, 0x00, 0x01, 0x1F};
    Serial.println("[WakeController] Sending wake frame (0x17330301)");
    return sendCanFrame(CAN_ID_WAKE, wakeData, 4, true);
}

bool WakeController::sendBapInitFrame() {
    uint8_t bapInitData[8] = {0x67, 0x10, 0x41, 0x84, 0x14, 0x00, 0x00, 0x00};
    Serial.println("[WakeController] Sending BAP init frame (0x1B000067)");
    return sendCanFrame(CAN_ID_BAP_INIT, bapInitData, 8, true);
}

bool WakeController::sendKeepAliveFrame() {
    uint8_t keepAliveData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (sendCanFrame(CAN_ID_KEEPALIVE, keepAliveData, 8, false)) {
        keepAlivesSent++;
        return true;
    }

    Serial.println("[WakeController] Failed to send keep-alive frame");
    return false;
}

bool WakeController::sendCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended) {
    if (!canManager) {
        Serial.println("[WakeController] No CAN manager - cannot send");
        return false;
    }

    if (!canManager->isRunning()) {
        Serial.println("[WakeController] CAN not running - cannot send");
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
        return true;
    } else if (result == ESP_ERR_TIMEOUT) {
        Serial.println("[WakeController] TX timeout - no ACK");
        return false;
    } else {
        Serial.printf("[WakeController] TX failed: %s\r\n", esp_err_to_name(result));
        return false;
    }
}

void WakeController::startKeepAlive() {
    if (!keepAliveActive) {
        Serial.println("[WakeController] Starting keep-alive (500ms interval)");
        keepAliveActive = true;
        lastKeepAlive = millis();
        lastCommandActivity = millis();

        // Send first keep-alive immediately
        sendKeepAliveFrame();
    }
}

void WakeController::stopKeepAlive() {
    if (keepAliveActive) {
        Serial.println("[WakeController] Stopping keep-alive");
        keepAliveActive = false;
    }
}

void WakeController::setWakeState(WakeState newState) {
    if (wakeState != newState) {
        Serial.printf("[WakeController] Wake: %s -> %s\r\n",
                      getStateName(),
                      [this, newState]() {
                          WakeState temp = wakeState;
                          wakeState = newState;
                          const char* name = getStateName();
                          wakeState = temp;
                          return name;
                      }());
        wakeState = newState;
        wakeStateStartTime = millis();
    }
}
