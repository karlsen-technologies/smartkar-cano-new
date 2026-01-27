#pragma once

#include <Arduino.h>
#include "driver/twai.h"
#include "../core/IModule.h"
#include <functional>

/**
 * CAN bus state machine states
 */
enum class CanState {
    OFF,            // CAN controller is off
    STARTING,       // CAN controller is starting
    RUNNING,        // CAN controller is running and receiving
    BUS_OFF,        // Bus-off state (too many errors)
    CAN_ERROR       // Error state
};

/**
 * CAN bus speed presets
 */
enum class CanSpeed {
    CAN_100KBPS,
    CAN_125KBPS,
    CAN_250KBPS,
    CAN_500KBPS,    // Common for OBD-II
    CAN_1MBPS
};

/**
 * Callback type for received CAN frames.
 * Parameters: canId, data, dlc, extended
 */
using CanFrameCallback = std::function<void(uint32_t, const uint8_t*, uint8_t, bool)>;

/**
 * CanManager - ESP32 TWAI (CAN) bus control module
 * 
 * Responsibilities:
 * - Initialize and configure the ESP32 TWAI peripheral
 * - Receive CAN bus messages
 * - Log/debug CAN traffic to console
 * - Future: Parse specific vehicle messages
 * 
 * Hardware:
 * - Uses ESP32's built-in TWAI (CAN) controller
 * - Connected to external CAN transceiver (e.g., SN65HVD230)
 * - TX: GPIO 36
 * - RX: GPIO 35
 */
class CanManager : public IModule {
public:
    CanManager();
    ~CanManager();

    // IModule interface
    bool setup() override;
    void loop() override;
    void prepareForSleep() override;
    bool isBusy() override;
    bool isReady() override;
    const char* getName() override { return "CAN"; }

    /**
     * Set the activity callback for reporting activity to DeviceController.
     */
    void setActivityCallback(ActivityCallback callback) { activityCallback = callback; }

    /**
     * Set the frame callback for routing frames to VehicleManager.
     * Called for each received CAN frame with (canId, data, dlc, extended).
     */
    void setFrameCallback(CanFrameCallback callback) { frameCallback = callback; }

    /**
     * Get current CAN state.
     */
    CanState getState() { return state; }

    /**
     * Check if CAN bus is running and receiving.
     */
    bool isRunning() { return state == CanState::RUNNING; }

    /**
     * Start the CAN controller.
     * @return true if started successfully
     */
    bool start();

    /**
     * Stop the CAN controller.
     * @return true if stopped successfully
     */
    bool stop();

    /**
     * Set CAN bus speed.
     * Must be called before start() or will require restart.
     * @param speed The desired CAN bus speed
     */
    void setSpeed(CanSpeed speed) { canSpeed = speed; }

    /**
     * Enable/disable verbose message logging.
     * @param enabled true to log all received messages
     */
    void setVerbose(bool enabled) { verbose = enabled; }

    /**
     * Get message receive count since start.
     */
    uint32_t getMessageCount() { return messageCount; }

    /**
     * Get error count since start.
     */
    uint32_t getErrorCount() { return errorCount; }

private:
    ActivityCallback activityCallback = nullptr;
    CanFrameCallback frameCallback = nullptr;

    CanState state = CanState::OFF;
    CanState previousState = CanState::OFF;
    
    // Configuration
    CanSpeed canSpeed = CanSpeed::CAN_500KBPS;  // Default to OBD-II speed
    bool verbose = false;  // Log all received messages (disabled - too noisy)

    // Statistics
    uint32_t messageCount = 0;
    uint32_t errorCount = 0;

    // Timing
    unsigned long stateEntryTime = 0;
    unsigned long lastStatusCheck = 0;
    unsigned long lastTestMessage = 0;

    // Pin configuration
    // CAN TX on GPIO47
    // CAN RX on GPIO21 (RTC-capable for deep sleep wake)
    // Transceiver enable: hardwired to 3.3V
    static const gpio_num_t CAN_TX_PIN = GPIO_NUM_47;
    static const gpio_num_t CAN_RX_PIN = GPIO_NUM_21;

    // State machine helpers
    void setState(CanState newState);
    unsigned long timeInState();

    // CAN operations
    bool installDriver();
    void uninstallDriver();
    void processReceivedMessages();
    void checkBusStatus();
    void logMessage(const twai_message_t& message);
    void sendTestMessage();

    // Timing constants
    static const unsigned long STATUS_CHECK_INTERVAL = 5000;  // Check bus status every 5s
    static const unsigned long TEST_MESSAGE_INTERVAL = 1000;  // Send test message every 1s
};
