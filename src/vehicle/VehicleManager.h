#pragma once

#include <Arduino.h>
#include "VehicleState.h"
#include "domains/BodyDomain.h"
#include "domains/BatteryDomain.h"
#include "domains/DriveDomain.h"
#include "domains/ClimateDomain.h"

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
 * - (Future) BatteryDomain: SOC, voltage, current, charging
 * - (Future) DriveDomain: ignition, speed, odometer
 * - (Future) ClimateDomain: temperatures, climate control
 * - (Future) GpsDomain: CAN-based GPS data
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
    
    // Configuration
    bool verbose = false;
    
    // Statistics
    unsigned long lastLogTime = 0;
    static constexpr unsigned long LOG_INTERVAL = 30000;  // Log stats every 30s
    
    void logStatistics();
};
