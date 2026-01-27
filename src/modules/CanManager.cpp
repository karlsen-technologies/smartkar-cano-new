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
    // Main loop only handles state transitions and status checks
    // Actual RX processing happens in dedicated task on Core 0
    
    switch (state) {
        case CanState::OFF:
            // Nothing to do
            break;

        case CanState::STARTING:
            // Check if driver is ready
            if (timeInState() > 100) {
                setState(CanState::RUNNING);
                Serial.println("[CAN] Controller running, CAN task processing on Core 0");
            }
            break;

        case CanState::RUNNING:
            // Periodically check bus status (from main loop - less critical)
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
    lastRxMissedCount = 0;
    lastStatusCheck = millis();
    lastTestMessage = millis();
    
    // Start the dedicated CAN receive task on Core 0
    taskRunning = true;
    BaseType_t taskResult = xTaskCreatePinnedToCore(
        canTaskEntry,           // Task function
        "CAN_RX",              // Task name
        CAN_TASK_STACK_SIZE,   // Stack size
        this,                  // Parameter (this pointer)
        CAN_TASK_PRIORITY,     // Priority (higher than normal)
        &canTaskHandle,        // Task handle
        CAN_TASK_CORE          // Core 0 (separate from main loop on Core 1)
    );
    
    if (taskResult != pdPASS) {
        Serial.println("[CAN] Failed to create CAN task!");
        twai_stop();
        uninstallDriver();
        setState(CanState::CAN_ERROR);
        return false;
    }
    
    Serial.printf("[CAN] CAN task started on Core %d with priority %d\r\n", 
        CAN_TASK_CORE, CAN_TASK_PRIORITY);

    setState(CanState::STARTING);
    Serial.println("[CAN] Controller started");
    return true;
}

bool CanManager::stop() {
    if (state == CanState::OFF) {
        return true;
    }

    Serial.println("[CAN] Stopping CAN controller...");
    
    // Stop the CAN task first
    if (canTaskHandle != nullptr) {
        taskRunning = false;
        // Give the task time to exit gracefully
        vTaskDelay(pdMS_TO_TICKS(50));
        // Delete the task if it's still running
        vTaskDelete(canTaskHandle);
        canTaskHandle = nullptr;
        Serial.println("[CAN] CAN task stopped");
    }

    // Stop and uninstall the driver
    twai_stop();
    uninstallDriver();

    setState(CanState::OFF);
    Serial.printf("[CAN] Stopped. Messages: %lu, Errors: %lu, Missed: %lu\r\n", 
        messageCount, errorCount, lastRxMissedCount);
    return true;
}

// =============================================================================
// CAN Task (runs on Core 0)
// =============================================================================

void CanManager::canTaskEntry(void* param) {
    CanManager* self = static_cast<CanManager*>(param);
    self->canTaskLoop();
}

void CanManager::canTaskLoop() {
    Serial.printf("[CAN] Task running on Core %d\r\n", xPortGetCoreID());
    
    twai_message_t message;
    
    while (taskRunning) {
        // Block waiting for a message with short timeout
        // This is more efficient than polling and yields to other tasks
        esp_err_t result = twai_receive(&message, pdMS_TO_TICKS(10));
        
        if (result == ESP_OK) {
            messageCount++;
            
            // Process the message immediately
            if (frameCallback) {
                frameCallback(
                    message.identifier,
                    message.data,
                    message.data_length_code,
                    message.extd
                );
            }
            
            // Report activity
            if (activityCallback) {
                activityCallback();
            }
            
            // After receiving one message, drain any others in the queue
            // Use non-blocking receive to get all pending messages
            while (twai_receive(&message, 0) == ESP_OK) {
                messageCount++;
                
                if (frameCallback) {
                    frameCallback(
                        message.identifier,
                        message.data,
                        message.data_length_code,
                        message.extd
                    );
                }
                
                if (activityCallback) {
                    activityCallback();
                }
            }
        }
        // ESP_ERR_TIMEOUT is normal - just means no message in the timeout period
    }
    
    Serial.println("[CAN] Task exiting");
    vTaskDelete(NULL);
}

// =============================================================================
// Private methods
// =============================================================================

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

    // General configuration - normal mode for TX capability
    // Increase RX queue to handle bursts while task is processing
    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    generalConfig.rx_queue_len = 32;  // Moderate queue - task drains quickly
    generalConfig.tx_queue_len = 8;   // Keep TX small, we don't send much
    
    // Accept all messages - hardware filtering is limited for our use case
    // (we need many different standard IDs across a wide range)
    // Software filtering happens in VehicleManager
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
    // This is now only used as fallback - main processing in canTaskLoop()
    twai_message_t message;
    
    while (twai_receive(&message, 0) == ESP_OK) {
        messageCount++;
        
        if (verbose) {
            logMessage(message);
        }
        
        if (frameCallback) {
            frameCallback(
                message.identifier,
                message.data,
                message.data_length_code,
                message.extd
            );
        }
        
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

        // Log missed messages (RX buffer overflow)
        if (status.rx_missed_count > lastRxMissedCount) {
            uint32_t newMissed = status.rx_missed_count - lastRxMissedCount;
            Serial.printf("[CAN] WARNING: %lu messages missed (total: %lu) - RX buffer overflow!\r\n",
                newMissed, status.rx_missed_count);
            errorCount += newMissed;
            lastRxMissedCount = status.rx_missed_count;
        }
        
        // Track TX failures
        if (status.tx_failed_count > 0) {
            errorCount += status.tx_failed_count;
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
