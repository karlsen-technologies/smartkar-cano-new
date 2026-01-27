#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"
#include "../protocols/Tm01Commands.h"

// Forward declaration
class VehicleManager;

/**
 * BodyDomain - Handles door, lock, window, and trunk status
 * 
 * Listens to:
 *   0x3D0 (TSG_FT_01) - Driver door module
 *   0x3D1 (TSG_BT_01) - Passenger door module  
 *   0x583 (ZV_02)     - Central locking status
 * 
 * Commands:
 *   TM_01 (0x5A7) - Horn, flash, lock, unlock via Tm01Commands
 */
class BodyDomain {
public:
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_DRIVER_DOOR = 0x3D0;      // TSG_FT_01
    static constexpr uint32_t CAN_ID_PASSENGER_DOOR = 0x3D1;   // TSG_BT_01
    static constexpr uint32_t CAN_ID_LOCK_STATUS = 0x583;      // ZV_02
    
    /**
     * Construct the body domain.
     * @param state Reference to shared vehicle state
     * @param manager Pointer to vehicle manager (for sending commands)
     */
    BodyDomain(VehicleState& state, VehicleManager* manager = nullptr);
    
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
    // Commands
    // =========================================================================
    
    /**
     * Send horn command.
     * @return true if command was queued
     */
    bool horn();
    
    /**
     * Send flash lights command.
     * @return true if command was queued
     */
    bool flash();
    
    /**
     * Send lock doors command.
     * @return true if command was queued
     */
    bool lock();
    
    /**
     * Send unlock doors command.
     * @return true if command was queued
     */
    bool unlock();
    
    /**
     * Send panic (horn + flash) command.
     * @return true if command was queued
     */
    bool panic();
    
    // =========================================================================
    // Status getters (convenience methods)
    // =========================================================================
    
    bool isLocked() const { return vehicleState.body.isLocked(); }
    bool isUnlocked() const { return vehicleState.body.isUnlocked(); }
    bool anyDoorOpen() const { return vehicleState.body.anyDoorOpen(); }
    bool isTrunkOpen() const { return vehicleState.body.trunkOpen; }
    
    const DoorState& driverDoor() const { return vehicleState.body.driverDoor; }
    const DoorState& passengerDoor() const { return vehicleState.body.passengerDoor; }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;
    
    // Process specific message types
    void processDriverDoor(const uint8_t* data);
    void processPassengerDoor(const uint8_t* data);
    void processLockStatus(const uint8_t* data);
    
    // Send a TM_01 command
    bool sendTm01Command(Tm01Commands::Command cmd);
};
