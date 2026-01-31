#pragma once

#include <Arduino.h>
#include "../IDomain.h"
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"
#include "../protocols/Tm01Commands.h"

// Forward declaration
class VehicleManager;

/**
 * BodyManager - Unified body domain manager
 * 
 * Encapsulates ALL body control data sources:
 * - Standard CAN frames (11-bit): 0x3D0 (TSG_FT_01), 0x3D1 (TSG_BT_01), 0x583 (ZV_02)
 * 
 * Implements new domain-based architecture:
 * - Owns complete body state (doors, locks, windows, trunk)
 * - Provides clean public API
 * - Fast CAN frame processing (<1-2ms)
 * - Command interface for horn, flash, lock/unlock
 * 
 * Data Source: CAN only (no BAP equivalent for body control)
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex
 * - loop() called from main loop (Core 1)
 * - State access is thread-safe via VehicleManager mutex
 */
class BodyManager : public IDomain {
public:
    /**
     * Body state structure
     * Contains door, lock, window, and trunk status
     */
    struct State {
        // === Central Lock Status (ZV_02 0x583) ===
        LockState centralLock = LockState::UNKNOWN;
        unsigned long centralLockUpdate = 0;
        
        // === Individual Doors ===
        DoorState driverDoor;       // Front left (TSG_FT_01 0x3D0)
        DoorState passengerDoor;    // Front right (TSG_BT_01 0x3D1)
        DoorState rearLeftDoor;     // Rear left (future)
        DoorState rearRightDoor;    // Rear right (future)
        
        // === Trunk/Hatch ===
        bool trunkOpen = false;
        unsigned long trunkUpdate = 0;
        
        // === Raw Debug Data ===
        uint8_t zv02_byte2 = 0;     // For debugging lock state parsing
        uint8_t zv02_byte7 = 0;
        
        // === Helper Methods ===
        
        bool isLocked() const {
            return centralLock == LockState::LOCKED;
        }
        
        bool isUnlocked() const {
            return centralLock == LockState::UNLOCKED;
        }
        
        bool anyDoorOpen() const {
            return driverDoor.open || passengerDoor.open || 
                   rearLeftDoor.open || rearRightDoor.open || trunkOpen;
        }
        
        bool isValid() const {
            return centralLockUpdate > 0 || driverDoor.lastUpdate > 0;
        }
    };

    /**
     * Construct the body manager.
     * @param vehicleManager Pointer to vehicle manager for sending commands
     */
    explicit BodyManager(VehicleManager* vehicleManager);

    // =========================================================================
    // IDomain interface implementation
    // =========================================================================
    
    const char* getName() const override { return "BodyManager"; }
    bool setup() override;
    void loop() override;
    void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    void onWakeComplete() override;
    bool isBusy() const override;

    // =========================================================================
    // Public API (for external consumers)
    // =========================================================================
    
    /**
     * Get complete body state (read-only).
     * Thread-safe: called from main loop with mutex held by caller.
     */
    const State& getState() const { return state; }
    
    /**
     * Get lock status.
     */
    bool isLocked() const { return state.isLocked(); }
    bool isUnlocked() const { return state.isUnlocked(); }
    LockState getLockState() const { return state.centralLock; }
    
    /**
     * Get door status.
     */
    const DoorState& getDriverDoor() const { return state.driverDoor; }
    const DoorState& getPassengerDoor() const { return state.passengerDoor; }
    bool anyDoorOpen() const { return state.anyDoorOpen(); }
    bool isTrunkOpen() const { return state.trunkOpen; }
    
    // =========================================================================
    // Command Interface (sends CAN frames via VehicleManager)
    // =========================================================================
    
    /**
     * Send horn command (non-blocking).
     * @return true if command sent, false if failed
     */
    bool horn();
    
    /**
     * Send flash lights command (non-blocking).
     * @return true if command sent, false if failed
     */
    bool flash();
    
    /**
     * Send lock doors command (non-blocking).
     * @return true if command sent, false if failed
     */
    bool lock();
    
    /**
     * Send unlock doors command (non-blocking).
     * @return true if command sent, false if failed
     */
    bool unlock();
    
    /**
     * Send panic (horn + flash) command (non-blocking).
     * @return true if command sent, false if failed
     */
    bool panic();
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& driverDoor, uint32_t& passengerDoor, uint32_t& lockStatus) const {
        driverDoor = driverDoorCount;
        passengerDoor = passengerDoorCount;
        lockStatus = lockStatusCount;
    }

private:
    VehicleManager* vehicleManager;
    
    // Domain state (reference to RTC memory - survives deep sleep)
    State& state;
    
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_DRIVER_DOOR = 0x3D0;      // TSG_FT_01
    static constexpr uint32_t CAN_ID_PASSENGER_DOOR = 0x3D1;   // TSG_BT_01
    static constexpr uint32_t CAN_ID_LOCK_STATUS = 0x583;      // ZV_02
    
    // Statistics
    volatile uint32_t driverDoorCount = 0;
    volatile uint32_t passengerDoorCount = 0;
    volatile uint32_t lockStatusCount = 0;
    
    // CAN frame processors
    void processDriverDoor(const uint8_t* data);
    void processPassengerDoor(const uint8_t* data);
    void processLockStatus(const uint8_t* data);
    
    // Command helper
    bool sendTm01Command(Tm01Commands::Command cmd);
};
