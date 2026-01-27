#pragma once

#include <Arduino.h>
#include "VehicleState.h"
#include "domains/BodyDomain.h"
#include "domains/BatteryDomain.h"
#include "domains/DriveDomain.h"
#include "domains/ClimateDomain.h"
#include "domains/GpsDomain.h"
#include "domains/RangeDomain.h"
#include "domains/BapDomain.h"

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
 * - BapDomain: BAP protocol for charging/climate control
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
     * Get the BAP domain (charging/climate control via BAP protocol).
     */
    BapDomain& bap() { return bapDomain; }
    
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
    // No mutex - CAN task has priority, main loop reads may see stale data
    VehicleState state;
    
    // Domains
    BodyDomain bodyDomain;
    BatteryDomain batteryDomain;
    DriveDomain driveDomain;
    ClimateDomain climateDomain;
    GpsDomain gpsDomain;
    RangeDomain rangeDomain;
    BapDomain bapDomain;
    
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
    
    void logStatistics();
};
