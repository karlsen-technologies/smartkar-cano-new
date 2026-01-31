#pragma once

#include <Arduino.h>
#include "../IDomain.h"
#include "../VehicleTypes.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * DriveManager - Unified drive domain manager
 * 
 * Encapsulates ALL drive data sources:
 * - Standard CAN frames (11-bit): 0x3C0 (Klemmen_Status_01), 0x0FD (ESP_21), 0x6B2 (Diagnose_01)
 * 
 * Implements new domain-based architecture:
 * - Owns complete drive state (ignition, speed, odometer, vehicle time)
 * - Provides clean public API
 * - Fast CAN frame processing (<1-2ms)
 * - Read-only domain (no commands)
 * 
 * Data Source: CAN only (no BAP equivalent for drive data)
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex
 * - loop() called from main loop (Core 1)
 * - State access is thread-safe via VehicleManager mutex
 */
class DriveManager : public IDomain {
public:
    /**
     * Drive state structure
     * Contains ignition, speed, odometer, and vehicle time
     */
    struct State {
        // === Ignition Status (Klemmen_Status_01 0x3C0) ===
        IgnitionState ignition = IgnitionState::OFF;
        bool keyInserted = false;
        bool ignitionOn = false;
        bool startRequested = false;
        unsigned long ignitionUpdate = 0;
        
        // === Speed (ESP_21 0x0FD) ===
        float speedKmh = 0.0f;
        unsigned long speedUpdate = 0;
        
        // === Odometer & Time (Diagnose_01 0x6B2) ===
        uint32_t odometerKm = 0;
        unsigned long odometerUpdate = 0;
        
        uint16_t year = 0;
        uint8_t month = 0;
        uint8_t day = 0;
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
        unsigned long timeUpdate = 0;
        
        // === Helper Methods ===
        
        bool isOn() const {
            return ignition == IgnitionState::ON || ignition == IgnitionState::START;
        }
        
        bool isMoving() const {
            return speedKmh > 1.0f;
        }
        
        bool isValid() const {
            return ignitionUpdate > 0 || speedUpdate > 0;
        }
    };

    /**
     * Construct the drive manager.
     * @param vehicleManager Pointer to vehicle manager (not used for read-only domain)
     */
    explicit DriveManager(VehicleManager* vehicleManager);

    // =========================================================================
    // IDomain interface implementation
    // =========================================================================
    
    const char* getName() const override { return "DriveManager"; }
    bool setup() override;
    void loop() override;
    void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    void onWakeComplete() override;
    bool isBusy() const override;

    // =========================================================================
    // Public API (for external consumers)
    // =========================================================================
    
    /**
     * Get complete drive state (read-only).
     * Thread-safe: called from main loop with mutex held by caller.
     */
    const State& getState() const { return state; }
    
    /**
     * Get ignition status.
     */
    IgnitionState getIgnitionState() const { return state.ignition; }
    bool isOn() const { return state.isOn(); }
    bool isKeyInserted() const { return state.keyInserted; }
    
    /**
     * Get speed information.
     */
    float getSpeedKmh() const { return state.speedKmh; }
    bool isMoving() const { return state.isMoving(); }
    
    /**
     * Get odometer.
     */
    uint32_t getOdometerKm() const { return state.odometerKm; }
    
    /**
     * Get vehicle time.
     */
    uint16_t getYear() const { return state.year; }
    uint8_t getMonth() const { return state.month; }
    uint8_t getDay() const { return state.day; }
    uint8_t getHour() const { return state.hour; }
    uint8_t getMinute() const { return state.minute; }
    uint8_t getSecond() const { return state.second; }
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& ignition, uint32_t& speed, uint32_t& diagnose) const {
        ignition = ignitionCount;
        speed = speedCount;
        diagnose = diagnoseCount;
    }

private:
    VehicleManager* vehicleManager;
    
    // Domain state (reference to RTC memory - survives deep sleep)
    State& state;
    
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_IGNITION = 0x3C0;    // Klemmen_Status_01
    static constexpr uint32_t CAN_ID_SPEED = 0x0FD;       // ESP_21
    static constexpr uint32_t CAN_ID_DIAGNOSE = 0x6B2;    // Diagnose_01
    
    // Statistics
    volatile uint32_t ignitionCount = 0;
    volatile uint32_t speedCount = 0;
    volatile uint32_t diagnoseCount = 0;
    
    // CAN frame processors
    void processIgnition(const uint8_t* data);
    void processSpeed(const uint8_t* data);
    void processDiagnose(const uint8_t* data);
};
