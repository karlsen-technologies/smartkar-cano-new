#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "VehicleState.h"
#include "domains/BodyDomain.h"
#include "domains/BatteryDomain.h"
#include "domains/DriveDomain.h"
#include "domains/ClimateDomain.h"
#include "domains/GpsDomain.h"
#include "domains/RangeDomain.h"
#include "bap/BapChannelRouter.h"
#include "bap/channels/BatteryControlChannel.h"
#include "ChargingProfileManager.h"
#include "../core/IModule.h"  // For ActivityCallback

// Enable this to track unique CAN IDs seen (adds overhead to CAN task)
// #define DEBUG_CAN_IDS  // DISABLED - no longer needed

// Forward declaration
class CanManager;

/**
 * VehicleManager - Orchestrates vehicle CAN communication
 * 
 * Responsibilities:
 * - Routes incoming CAN frames to appropriate domains
 * - Provides interface for sending CAN commands
 * - Manages vehicle wake/sleep state
 * - Holds the shared VehicleState (thread-safe via mutex)
 * 
 * Thread Safety:
 * - CAN frames are processed from CAN task on Core 0
 * - State is read from main loop on Core 1
 * - Mutex protects all state access
 * 
 * Domains:
 * - BodyDomain: doors, locks, windows, horn/flash
 * - BatteryDomain: charging status, energy, temperature
 * - DriveDomain: ignition, speed, odometer
 * - ClimateDomain: temperatures, climate control
 * - GpsDomain: CAN-based GPS data from infotainment
 * - RangeDomain: range estimation from cluster
 * 
 * BAP Architecture:
 * - BapChannelRouter: routes BAP messages to channels
 * - BatteryControlChannel: BAP protocol for Battery Control (Device 0x25)
 * 
 * Wake State Machine:
 * - Integrated wake/keep-alive management
 * - States: ASLEEP, WAKE_REQUESTED, WAKING, AWAKE
 * - Non-blocking state transitions in loop()
 */
class VehicleManager {
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
     * Construct the vehicle manager.
     * @param canManager Pointer to the CAN manager for TX/RX
     */
    explicit VehicleManager(CanManager* canManager);
    
    ~VehicleManager();
    
    /**
     * Initialize all domains.
     * @return true if initialization succeeded
     */
    bool setup();
    
    /**
     * Process periodic tasks (called from main loop).
     */
    void loop();
    
    /**
     * Process an incoming CAN frame. Called by CanManager from CAN task on Core 0.
     * Thread-safe: acquires mutex internally.
     * @param canId The CAN identifier
     * @param data Frame data (8 bytes max)
     * @param dlc Data length code
     * @param extended true if extended (29-bit) ID
     */
    void onCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended);
    
    /**
     * Send a CAN frame.
     * @param canId The CAN identifier
     * @param data Frame data
     * @param dlc Data length code (1-8)
     * @param extended true for extended (29-bit) ID
     * @return true if frame was queued for transmission
     */
    bool sendCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended = false);
    
    // =========================================================================
    // State access (thread-safe)
    // =========================================================================
    
    /**
     * Get a copy of the vehicle state (thread-safe).
     * Use this when you need multiple fields consistently.
     */
    VehicleState getStateCopy();
    
    /**
     * Check if vehicle CAN bus is active (thread-safe).
     */
    bool isVehicleAwake();
    
    /**
     * Get count of CAN frames processed (thread-safe).
     */
    uint32_t getFrameCount();
    
    // =========================================================================
    // Domain access (for command sending - use from main loop only)
    // =========================================================================
    
    /**
     * Get the Battery Control channel (charging/climate control via BAP protocol).
     */
    BatteryControlChannel& batteryControl() { return batteryControlChannel; }
    
    /**
     * Get the charging profile manager (high-level charging/climate API).
     */
    ChargingProfileManager& profiles() { return profileManager; }
    
    /**
     * Request vehicle wake (non-blocking).
     * Sets WAKE_REQUESTED state. State machine in loop() will handle wake sequence.
     * @return true if wake requested, false if already waking/awake
     */
    bool requestWake();
    
    /**
     * Check if vehicle is awake and ready for commands.
     * @return true if in AWAKE state
     */
    bool isAwake() const;
    
    /**
     * Get current wake state.
     */
    WakeState getWakeState() const { return wakeState; }
    
    /**
     * Get wake state as string (for logging).
     */
    const char* getWakeStateName() const;
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    /**
     * Enable/disable verbose logging of all CAN frames.
     */
    void setVerbose(bool enabled) { verbose = enabled; }
    
private:
    CanManager* canManager;
    
    // Vehicle state (written by CAN task on Core 0, read by main loop on Core 1)
    // Protected by mutex for thread-safe access across cores
    VehicleState state;
    SemaphoreHandle_t stateMutex = nullptr;  // FreeRTOS mutex for thread-safe state access
    
    // Domains
    BodyDomain bodyDomain;
    BatteryDomain batteryDomain;
    DriveDomain driveDomain;
    ClimateDomain climateDomain;
    GpsDomain gpsDomain;
    RangeDomain rangeDomain;
    
    // BAP infrastructure
    BapChannelRouter bapRouter;
    BatteryControlChannel batteryControlChannel;
    
    // Charging profile manager (high-level API for charging/climate)
    ChargingProfileManager profileManager;
    
    // Wake state machine
    WakeState wakeState = WakeState::ASLEEP;
    unsigned long wakeStateStartTime = 0;  // When current state was entered
    
    // Keep-alive management
    bool keepAliveActive = false;
    unsigned long lastKeepAlive = 0;
    unsigned long lastWakeActivity = 0;  // Last time a command requested wake
    
    // Wake statistics
    uint32_t wakeAttempts = 0;
    uint32_t keepAlivesSent = 0;
    uint32_t wakeFailed = 0;
    
    // Wake timing constants
    static constexpr unsigned long KEEPALIVE_INTERVAL = 500;      // Send keep-alive every 500ms
    static constexpr unsigned long KEEPALIVE_TIMEOUT = 300000;    // Stop after 5 minutes of inactivity
    static constexpr unsigned long BAP_INIT_WAIT = 2000;          // Wait 2s after wake for BAP init
    static constexpr unsigned long WAKE_TIMEOUT = 10000;          // Give up if no CAN activity after 10s
    
    // Wake CAN IDs
    static constexpr uint32_t CAN_ID_WAKE = 0x17330301;      // Wake frame (extended)
    static constexpr uint32_t CAN_ID_BAP_INIT = 0x1B000067;  // BAP initialization (extended)
    static constexpr uint32_t CAN_ID_KEEPALIVE = 0x5A7;      // Keep-alive heartbeat (standard)
    
    // Configuration
    bool verbose = false;
    
    // Statistics (accessed from CAN task - use volatile)
    unsigned long lastLogTime = 0;
    static constexpr unsigned long LOG_INTERVAL = 10000;  // Log stats every 10s
    
    // Frame counters per domain (for debugging)
    volatile uint32_t bodyFrames = 0;
    volatile uint32_t batteryFrames = 0;
    volatile uint32_t driveFrames = 0;
    volatile uint32_t climateFrames = 0;
    volatile uint32_t gpsFrames = 0;
    volatile uint32_t rangeFrames = 0;
    volatile uint32_t bapFrames = 0;
    volatile uint32_t unhandledFrames = 0;
    
    /**
     * Log statistics to serial (called periodically).
     */
    void logStatistics();
    
    /**
     * Update wake state machine (called from loop()).
     */
    void updateWakeStateMachine();
    
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
