#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * DriveDomain - Handles ignition, speed, odometer, and vehicle time
 * 
 * Listens to:
 *   0x3C0 (Klemmen_Status_01) - Ignition/terminal status
 *   0x0FD (ESP_21)            - Vehicle speed
 *   0x6B2 (Diagnose_01)       - Odometer and vehicle time
 * 
 * No commands - this is a read-only domain.
 */
class DriveDomain {
public:
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_IGNITION = 0x3C0;    // Klemmen_Status_01
    static constexpr uint32_t CAN_ID_SPEED = 0x0FD;       // ESP_21
    static constexpr uint32_t CAN_ID_DIAGNOSE = 0x6B2;    // Diagnose_01
    
    /**
     * Construct the drive domain.
     * @param state Reference to shared vehicle state
     * @param manager Pointer to vehicle manager (not used, no commands)
     */
    DriveDomain(VehicleState& state, VehicleManager* manager = nullptr);
    
    /**
     * Check if this domain handles a CAN ID.
     */
    bool handlesCanId(uint32_t canId) const;
    
    /**
     * Process a CAN frame. Called by VehicleManager when a relevant frame arrives.
     * @param canId The CAN ID
     * @param data The frame data (8 bytes)
     * @param dlc Data length code
     * @return true if frame was processed
     */
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    // =========================================================================
    // Status getters (convenience methods)
    // =========================================================================
    
    // Ignition state
    IgnitionState ignitionState() const { return vehicleState.drive.ignition; }
    bool isOn() const { return vehicleState.drive.isOn(); }
    bool keyInserted() const { return vehicleState.drive.keyInserted; }
    
    // Speed
    float speedKmh() const { return vehicleState.drive.speedKmh; }
    bool isMoving() const { return vehicleState.drive.isMoving(); }
    
    // Odometer
    uint32_t odometerKm() const { return vehicleState.drive.odometerKm; }
    
    // Vehicle time
    uint16_t year() const { return vehicleState.drive.year; }
    uint8_t month() const { return vehicleState.drive.month; }
    uint8_t day() const { return vehicleState.drive.day; }
    uint8_t hour() const { return vehicleState.drive.hour; }
    uint8_t minute() const { return vehicleState.drive.minute; }
    uint8_t second() const { return vehicleState.drive.second; }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;  // Not used currently, but kept for consistency
    
    // Process specific message types
    void processIgnition(const uint8_t* data);
    void processSpeed(const uint8_t* data);
    void processDiagnose(const uint8_t* data);
};
