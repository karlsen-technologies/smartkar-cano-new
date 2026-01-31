#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "VehicleState.h"
#include "bap/channels/BatteryControlChannel.h"
#include "ChargingProfileManager.h"
#include "../core/IModule.h"  // For ActivityCallback
#include "services/ActivityTracker.h"
#include "services/WakeController.h"

// Domain-based architecture
#include "domains/BatteryManager.h"
#include "domains/ClimateManager.h"
#include "domains/BodyManager.h"
#include "domains/DriveManager.h"
#include "domains/GpsManager.h"
#include "domains/RangeManager.h"

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
 * - Managed by WakeController service
 * - States: ASLEEP, WAKE_REQUESTED, WAKING, AWAKE
 * - Non-blocking state transitions in loop()
 */
class VehicleManager {
public:
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
     * Get the new BatteryManager (domain-based architecture).
     * NOTE: Running in parallel with old BatteryDomain for testing.
     */
    BatteryManager* battery() { return &batteryManager; }
    
    /**
     * Get the new ClimateManager (domain-based architecture).
     * NOTE: Running in parallel with old ClimateDomain for testing.
     */
    ClimateManager* climate() { return &climateManager; }
    
    /**
     * Get the new BodyManager (domain-based architecture).
     * NOTE: Running in parallel with old BodyDomain for testing.
     */
    BodyManager* body() { return &bodyManager; }
    
    /**
     * Get the new DriveManager (domain-based architecture).
     * NOTE: Running in parallel with old DriveDomain for testing.
     */
    DriveManager* drive() { return &driveManager; }
    
    /**
     * Get the new GpsManager (domain-based architecture).
     * NOTE: Running in parallel with old GpsDomain for testing.
     */
    GpsManager* gps() { return &gpsManager; }
    
    /**
     * Get the new RangeManager (domain-based architecture).
     * NOTE: Running in parallel with old RangeDomain for testing.
     */
    RangeManager* range() { return &rangeManager; }
    
    /**
     * Request vehicle wake (non-blocking).
     * Sets WAKE_REQUESTED state. State machine in loop() will handle wake sequence.
     * @return true if wake requested, false if already waking/awake
     */
    bool requestWake() {
        return wakeController.requestWake();
    }
    
    /**
     * Check if vehicle is awake and ready for commands.
     * @return true if in AWAKE state
     */
    bool isAwake() const {
        return wakeController.isAwake();
    }
    
    /**
     * Get current wake state.
     */
    WakeController::WakeState getWakeState() const {
        return wakeController.getState();
    }
    
    /**
     * Get wake state as string (for logging).
     */
    const char* getWakeStateName() const {
        return wakeController.getStateName();
    }
    
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
    
    // Domain managers (NEW ARCHITECTURE - Phase 2-4 complete)
    BatteryManager batteryManager;
    ClimateManager climateManager;
    BodyManager bodyManager;
    DriveManager driveManager;
    GpsManager gpsManager;
    RangeManager rangeManager;
    
    // BAP infrastructure
    BatteryControlChannel batteryControlChannel;
    
    // Charging profile manager (high-level API for charging/climate)
    ChargingProfileManager profileManager;
    
    // Services
    ActivityTracker activityTracker;
    WakeController wakeController;
    
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
};
