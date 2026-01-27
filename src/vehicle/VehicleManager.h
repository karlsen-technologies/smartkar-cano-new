#pragma once

#include <Arduino.h>
#include "VehicleState.h"
#include "domains/BodyDomain.h"
#include "domains/BatteryDomain.h"
#include "domains/DriveDomain.h"
#include "domains/ClimateDomain.h"
#include "domains/GpsDomain.h"
#include "domains/RangeDomain.h"

// Forward declaration
class CanManager;

/**
 * VehicleManager - Orchestrates vehicle CAN communication
 * 
 * Responsibilities:
 * - Routes incoming CAN frames to appropriate domains
 * - Provides interface for sending CAN commands
 * - Manages vehicle wake/sleep state
 * - Holds the shared VehicleState
 * 
 * Domains:
 * - BodyDomain: doors, locks, windows, horn/flash
 * - BatteryDomain: charging status, energy, temperature
 * - DriveDomain: ignition, speed, odometer
 * - ClimateDomain: temperatures, climate control
 * - GpsDomain: CAN-based GPS data from infotainment
 */
class VehicleManager {
public:
    /**
     * Construct the vehicle manager.
     * @param canManager Pointer to the CAN manager for TX/RX
     */
    explicit VehicleManager(CanManager* canManager);
    
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
     * Process an incoming CAN frame. Called by CanManager.
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
    // State access
    // =========================================================================
    
    /**
     * Get read-only access to vehicle state.
     */
    const VehicleState& getState() const { return state; }
    
    /**
     * Get mutable access to vehicle state (for domains).
     */
    VehicleState& getStateMutable() { return state; }
    
    // =========================================================================
    // Domain access
    // =========================================================================
    
    /**
     * Get the body domain (doors, locks, windows).
     */
    BodyDomain& body() { return bodyDomain; }
    
    /**
     * Get the battery domain (SOC, voltage, current, charging).
     */
    BatteryDomain& battery() { return batteryDomain; }
    
    /**
     * Get the drive domain (ignition, speed, odometer).
     */
    DriveDomain& drive() { return driveDomain; }
    
    /**
     * Get the climate domain (temperatures).
     */
    ClimateDomain& climate() { return climateDomain; }
    
    /**
     * Get the GPS domain (CAN-based GPS from infotainment).
     */
    GpsDomain& gps() { return gpsDomain; }
    
    /**
     * Get the range domain (range estimation from cluster).
     */
    RangeDomain& range() { return rangeDomain; }
    
    // =========================================================================
    // Status
    // =========================================================================
    
    /**
     * Check if vehicle CAN bus is active (receiving frames).
     */
    bool isVehicleAwake() const { return state.isAwake(); }
    
    /**
     * Get count of CAN frames processed.
     */
    uint32_t getFrameCount() const { return state.canFrameCount; }
    
    /**
     * Enable/disable verbose logging of all CAN frames.
     */
    void setVerbose(bool enabled) { verbose = enabled; }
    
private:
    CanManager* canManager;
    
    // Vehicle state (shared by all domains)
    VehicleState state;
    
    // Domains
    BodyDomain bodyDomain;
    BatteryDomain batteryDomain;
    DriveDomain driveDomain;
    ClimateDomain climateDomain;
    GpsDomain gpsDomain;
    RangeDomain rangeDomain;
    
    // Configuration
    bool verbose = false;
    
    // Statistics
    unsigned long lastLogTime = 0;
    static constexpr unsigned long LOG_INTERVAL = 10000;  // Log stats every 10s (for testing)
    
    // Frame counters per domain (for debugging)
    uint32_t bodyFrames = 0;
    uint32_t batteryFrames = 0;
    uint32_t driveFrames = 0;
    uint32_t climateFrames = 0;
    uint32_t gpsFrames = 0;
    uint32_t rangeFrames = 0;
    uint32_t unhandledFrames = 0;
    
    // Track unique CAN IDs seen (for debugging)
    static constexpr size_t MAX_TRACKED_IDS = 64;
    uint32_t seenCanIds[MAX_TRACKED_IDS] = {0};
    uint32_t seenIdCounts[MAX_TRACKED_IDS] = {0};
    uint8_t seenIdDlcs[MAX_TRACKED_IDS] = {0};  // Track DLC of each ID
    size_t numSeenIds = 0;
    
    void trackCanId(uint32_t canId, uint8_t dlc);
    void logStatistics();
};
