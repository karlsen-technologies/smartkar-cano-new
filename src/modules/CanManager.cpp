#include "CanManager.h"

CanManager::CanManager() {
}

CanManager::~CanManager() {
    if (state != CanState::OFF) {
        stop();
    }
}

bool CanManager::setup() {
    Serial.println("[CAN] Setting up CAN manager");
    
    // Transceiver enable is hardwired to 3.3V
    
    Serial.println("[CAN] Setup complete (not started)");
    return true;
}

void CanManager::loop() {
    switch (state) {
        case CanState::OFF:
            // Nothing to do
            break;

        case CanState::STARTING:
            // Check if driver is ready
            if (timeInState() > 100) {
                setState(CanState::RUNNING);
                Serial.println("[CAN] Controller running, listening for messages...");
            }
            break;

        case CanState::RUNNING:
            // Process any received messages
            processReceivedMessages();
            
            // Periodically check bus status
            if (millis() - lastStatusCheck > STATUS_CHECK_INTERVAL) {
                checkBusStatus();
                lastStatusCheck = millis();
            }
            break;

        case CanState::BUS_OFF:
            // Try to recover after a delay
            if (timeInState() > 1000) {
                Serial.println("[CAN] Attempting recovery from bus-off...");
                twai_initiate_recovery();
                setState(CanState::STARTING);
            }
            break;

        case CanState::CAN_ERROR:
            // Try to restart after error
            if (timeInState() > 5000) {
                Serial.println("[CAN] Attempting restart after error...");
                stop();
                delay(100);
                start();
            }
            break;
    }
}

void CanManager::prepareForSleep() {
    Serial.println("[CAN] Preparing for sleep");
    stop();
}

bool CanManager::isBusy() {
    return state == CanState::STARTING;
}

bool CanManager::isReady() {
    return state == CanState::RUNNING;
}

bool CanManager::start() {
    if (state != CanState::OFF) {
        Serial.println("[CAN] Already started");
        return true;
    }

    Serial.println("[CAN] Starting CAN controller...");

    if (!installDriver()) {
        Serial.println("[CAN] Failed to install driver");
        setState(CanState::CAN_ERROR);
        return false;
    }

    // Start the TWAI driver
    esp_err_t result = twai_start();
    if (result != ESP_OK) {
        Serial.printf("[CAN] Failed to start driver: %s\r\n", esp_err_to_name(result));
        uninstallDriver();
        setState(CanState::CAN_ERROR);
        return false;
    }

    // Reset statistics
    messageCount = 0;
    errorCount = 0;
    lastStatusCheck = millis();
    lastTestMessage = millis();

    setState(CanState::STARTING);
    Serial.println("[CAN] Controller started");
    return true;
}

bool CanManager::stop() {
    if (state == CanState::OFF) {
        return true;
    }

    Serial.println("[CAN] Stopping CAN controller...");

    // Stop and uninstall the driver
    twai_stop();
    uninstallDriver();

    setState(CanState::OFF);
    Serial.printf("[CAN] Stopped. Messages: %lu, Errors: %lu\r\n", messageCount, errorCount);
    return true;
}

// Private methods

void CanManager::setState(CanState newState) {
    if (state != newState) {
        Serial.printf("[CAN] State: %d -> %d\r\n", (int)state, (int)newState);
        previousState = state;
        state = newState;
        stateEntryTime = millis();
    }
}

unsigned long CanManager::timeInState() {
    return millis() - stateEntryTime;
}

bool CanManager::installDriver() {
    // Get timing config based on speed setting
    twai_timing_config_t timingConfig;
    
    switch (canSpeed) {
        case CanSpeed::CAN_100KBPS:
            timingConfig = TWAI_TIMING_CONFIG_100KBITS();
            Serial.println("[CAN] Speed: 100 kbps");
            break;
        case CanSpeed::CAN_125KBPS:
            timingConfig = TWAI_TIMING_CONFIG_125KBITS();
            Serial.println("[CAN] Speed: 125 kbps");
            break;
        case CanSpeed::CAN_250KBPS:
            timingConfig = TWAI_TIMING_CONFIG_250KBITS();
            Serial.println("[CAN] Speed: 250 kbps");
            break;
        case CanSpeed::CAN_500KBPS:
            timingConfig = TWAI_TIMING_CONFIG_500KBITS();
            Serial.println("[CAN] Speed: 500 kbps");
            break;
        case CanSpeed::CAN_1MBPS:
            timingConfig = TWAI_TIMING_CONFIG_1MBITS();
            Serial.println("[CAN] Speed: 1 Mbps");
            break;
        default:
            timingConfig = TWAI_TIMING_CONFIG_500KBITS();
            Serial.println("[CAN] Speed: 500 kbps (default)");
            break;
    }

    // General configuration - normal mode for TX capability (debugging)
    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    
    // Accept all messages (no filtering for now)
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    Serial.printf("[CAN] Installing driver (TX: GPIO%d, RX: GPIO%d)\r\n", CAN_TX_PIN, CAN_RX_PIN);

    esp_err_t result = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
    if (result != ESP_OK) {
        Serial.printf("[CAN] Failed to install driver: %s\r\n", esp_err_to_name(result));
        return false;
    }

    return true;
}

void CanManager::uninstallDriver() {
    twai_driver_uninstall();
}

void CanManager::processReceivedMessages() {
    twai_message_t message;
    
    // Process all available messages (non-blocking)
    while (twai_receive(&message, 0) == ESP_OK) {
        messageCount++;
        
        if (verbose) {
            logMessage(message);
        }
        
        // Notify VehicleManager of received frame
        if (frameCallback) {
            frameCallback(
                message.identifier,
                message.data,
                message.data_length_code,
                message.extd
            );
        }
        
        // Report activity (CAN traffic = vehicle is active)
        if (activityCallback) {
            activityCallback();
        }
    }
}

void CanManager::checkBusStatus() {
    twai_status_info_t status;
    
    if (twai_get_status_info(&status) == ESP_OK) {
        // Check for bus-off condition
        if (status.state == TWAI_STATE_BUS_OFF) {
            Serial.println("[CAN] Bus-off detected!");
            errorCount++;
            setState(CanState::BUS_OFF);
            return;
        }
        
        // Check if driver stopped (after recovery or error)
        if (status.state == TWAI_STATE_STOPPED) {
            Serial.println("[CAN] Driver stopped - restarting...");
            esp_err_t result = twai_start();
            if (result == ESP_OK) {
                Serial.println("[CAN] Driver restarted successfully");
            } else {
                Serial.printf("[CAN] Failed to restart: %s\r\n", esp_err_to_name(result));
                setState(CanState::CAN_ERROR);
            }
            return;
        }

        // Track errors
        if (status.rx_missed_count > 0 || status.tx_failed_count > 0) {
            errorCount += status.rx_missed_count + status.tx_failed_count;
        }
    }
}

void CanManager::logMessage(const twai_message_t& message) {
    // Format: [CAN] ID:0x123 [8] 01 02 03 04 05 06 07 08
    
    if (message.extd) {
        // Extended ID (29-bit)
        Serial.printf("[CAN] ID:0x%08lX [%d]", message.identifier, message.data_length_code);
    } else {
        // Standard ID (11-bit)
        Serial.printf("[CAN] ID:0x%03lX [%d]", message.identifier, message.data_length_code);
    }
    
    // Print data bytes
    for (int i = 0; i < message.data_length_code; i++) {
        Serial.printf(" %02X", message.data[i]);
    }
    
    // Add RTR indicator if applicable
    if (message.rtr) {
        Serial.print(" (RTR)");
    }
    
    Serial.println();
}

void CanManager::sendTestMessage() {
    static uint8_t counter = 0;
    
    // Check if we're still in a valid state to transmit
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        if (status.state == TWAI_STATE_BUS_OFF) {
            Serial.println("[CAN] Cannot TX - bus off, initiating recovery...");
            twai_initiate_recovery();
            return;
        }
        if (status.state == TWAI_STATE_RECOVERING) {
            Serial.println("[CAN] Cannot TX - recovering from bus-off...");
            return;
        }
        if (status.state != TWAI_STATE_RUNNING) {
            Serial.printf("[CAN] Cannot TX - state is %d (not running)\r\n", status.state);
            return;
        }
    }
    
    twai_message_t message;
    message.identifier = 0x123;  // Test ID
    message.extd = 0;            // Standard frame (11-bit ID)
    message.rtr = 0;             // Not a remote frame
    message.data_length_code = 8;
    message.data[0] = 0xDE;
    message.data[1] = 0xAD;
    message.data[2] = 0xBE;
    message.data[3] = 0xEF;
    message.data[4] = counter++;
    message.data[5] = (millis() >> 16) & 0xFF;
    message.data[6] = (millis() >> 8) & 0xFF;
    message.data[7] = millis() & 0xFF;
    
    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(100));
    
    if (result == ESP_OK) {
        Serial.printf("[CAN] TX: ID:0x%03lX [%d] DE AD BE EF %02X ...\r\n", 
            message.identifier, message.data_length_code, message.data[4]);
    } else if (result == ESP_ERR_TIMEOUT) {
        Serial.println("[CAN] TX timeout - no ACK received (is another node connected?)");
    } else {
        Serial.printf("[CAN] TX failed: %s\r\n", esp_err_to_name(result));
    }
}
