#pragma once

#include <Arduino.h>
#include <cstdint>

// Forward declaration
class CanManager;
class IDomain;

/**
 * WakeController - Manages vehicle wake/sleep state machine
 * 
 * Extracted from VehicleManager to separate concerns.
 * Handles:
 * - Wake sequence (wake frame + BAP init)
 * - Keep-alive heartbeat management
 * - CAN activity tracking
 * - Wake timeout handling
 * 
 * Wake State Machine:
 * ASLEEP -> WAKE_REQUESTED -> WAKING -> AWAKE
 *    ^                                      |
 *    |______ (no CAN activity) ___________|
 * 
 * Keep-Alive Management:
 * - Starts when vehicle wakes up
 * - Sends heartbeat every 500ms
 * - Stops after 5 minutes of no command activity
 * - Prevents vehicle from going to sleep during operations
 */
class WakeController {
public:
    /**
     * Wake state machine states
     */
    enum class WakeState {
        ASLEEP,          // Vehicle CAN bus inactive
        WAKE_REQUESTED,  // Command requested wake, need to start sequence
        WAKING,          // Wake frames sent, waiting for CAN activity
        AWAKE            // Vehicle responding, ready for commands
    };

    /**
     * Construct the wake controller.
     * @param canManager Pointer to CAN manager for sending frames
     */
    explicit WakeController(CanManager* canManager);

    /**
     * Initialize the wake controller.
     * @return true if initialization succeeded
     */
    bool setup();

    /**
     * Process periodic tasks (state machine, keep-alive).
     * Called from main loop.
     * @param vehicleHasCanActivity true if vehicle CAN bus is active
     */
    void loop(bool vehicleHasCanActivity);

    /**
     * Request vehicle wake (non-blocking).
     * @param requester Optional domain requesting wake (for tracking)
     * @return true if wake requested, false if already waking/awake
     */
    bool requestWake(IDomain* requester = nullptr);

    /**
     * Notify controller of CAN activity (called on every frame).
     */
    void onCanActivity();

    /**
     * Check if vehicle is awake and ready for commands.
     * @return true if in AWAKE state
     */
    bool isAwake() const { return wakeState == WakeState::AWAKE; }

    /**
     * Get current wake state.
     */
    WakeState getState() const { return wakeState; }

    /**
     * Get wake state as string (for logging).
     */
    const char* getStateName() const;

    /**
     * Update last command activity time (extends keep-alive timeout).
     * Call this whenever a command is initiated to keep vehicle awake.
     */
    void notifyCommandActivity();

    /**
     * Get statistics for diagnostics.
     */
    void getStats(uint32_t& attempts, uint32_t& keepAlivesSent, uint32_t& failed) const {
        attempts = wakeAttempts;
        keepAlivesSent = this->keepAlivesSent;
        failed = wakeFailed;
    }

private:
    CanManager* canManager;

    // Wake state machine
    WakeState wakeState = WakeState::ASLEEP;
    unsigned long wakeStateStartTime = 0;  // When current state was entered
    bool canInitializing = true;            // Ignore CAN activity during first loop

    // Keep-alive management
    bool keepAliveActive = false;
    unsigned long lastKeepAlive = 0;
    unsigned long lastCommandActivity = 0;  // Last time a command was initiated

    // Wake statistics
    uint32_t wakeAttempts = 0;
    uint32_t keepAlivesSent = 0;
    uint32_t wakeFailed = 0;

    // Pending wake requesters (for debugging)
    IDomain* pendingRequester = nullptr;

    // Wake timing constants
    static constexpr unsigned long KEEPALIVE_INTERVAL = 500;      // Send keep-alive every 500ms
    static constexpr unsigned long KEEPALIVE_TIMEOUT = 300000;    // Stop after 5 minutes of inactivity
    static constexpr unsigned long BAP_INIT_WAIT = 2000;          // Wait 2s after wake for BAP init
    static constexpr unsigned long WAKE_TIMEOUT = 10000;          // Give up if no CAN activity after 10s

    // Wake CAN IDs
    static constexpr uint32_t CAN_ID_WAKE = 0x17330301;      // Wake frame (extended)
    static constexpr uint32_t CAN_ID_BAP_INIT = 0x1B000067;  // BAP initialization (extended)
    static constexpr uint32_t CAN_ID_KEEPALIVE = 0x5A7;      // Keep-alive heartbeat (standard)

    /**
     * Send wake frame to vehicle.
     */
    bool sendWakeFrame();

    /**
     * Send BAP initialization frame.
     */
    bool sendBapInitFrame();

    /**
     * Send keep-alive frame.
     */
    bool sendKeepAliveFrame();

    /**
     * Send a CAN frame.
     */
    bool sendCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended);

    /**
     * Start keep-alive heartbeat.
     */
    void startKeepAlive();

    /**
     * Stop keep-alive heartbeat.
     */
    void stopKeepAlive();

    /**
     * Change wake state with logging.
     */
    void setWakeState(WakeState newState);
};
