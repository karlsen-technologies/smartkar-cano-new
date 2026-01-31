#pragma once

#include <Arduino.h>
#include "../IDomain.h"
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declarations
class VehicleManager;
class BatteryControlChannel;

/**
 * ClimateManager - Unified climate domain manager
 * 
 * Encapsulates ALL climate data sources:
 * - Standard CAN frames (11-bit): 0x66E (Klima_03), 0x5E1 (Klima_Sensor_02)
 * - BAP channel callbacks: climate state (function 0x12)
 * 
 * Implements new domain-based architecture:
 * - Owns complete climate state
 * - Subscribes to BatteryControlChannel callbacks (SHARED with BatteryManager)
 * - Provides clean public API
 * - Fast CAN frame processing (<1-2ms)
 * 
 * Data Source Priority:
 * 1. BAP (most detailed and reliable for active climate)
 * 2. Standard CAN (passive temperature monitoring)
 * 3. CAN climate flags are UNRELIABLE - BAP only for active status
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex
 * - loop() called from main loop (Core 1)
 * - State access is thread-safe via VehicleManager mutex
 */
class ClimateManager : public IDomain {
public:
    /**
     * Climate state structure
     * Combines data from CAN and BAP sources
     */
    struct State {
        // === From Standard CAN ===
        
        // Klima_03 (0x66E) - Inside temperature
        float insideTemp = 0.0f;
        DataSource insideTempSource = DataSource::NONE;
        unsigned long insideTempUpdate = 0;
        
        // Klima_Sensor_02 (0x5E1) - Outside temperature
        float outsideTemp = 0.0f;
        unsigned long outsideTempUpdate = 0;
        
        // === From BAP (via BatteryControlChannel function 0x12) ===
        
        // Climate control state (BAP-only - CAN flags unreliable)
        bool climateActive = false;
        DataSource climateActiveSource = DataSource::NONE;
        bool heating = false;
        bool cooling = false;
        bool ventilation = false;
        bool autoDefrost = false;
        uint16_t climateTimeMin = 0;       // Remaining time in minutes
        unsigned long climateActiveUpdate = 0;
        
        // === Computed ===
        
        /**
         * Check if climate control is actively running
         */
        bool isActive() const {
            return climateActive;
        }
        
        /**
         * Check if state is valid (has received data)
         */
        bool isValid() const {
            return insideTempUpdate > 0 || outsideTempUpdate > 0;
        }
    };

    /**
     * Construct the climate manager.
     * @param vehicleManager Pointer to vehicle manager for sending commands
     */
    explicit ClimateManager(VehicleManager* vehicleManager);

    // =========================================================================
    // IDomain interface implementation
    // =========================================================================
    
    const char* getName() const override { return "ClimateManager"; }
    bool setup() override;
    void loop() override;
    void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    void onWakeComplete() override;
    bool isBusy() const override;

    // =========================================================================
    // Public API (for external consumers)
    // =========================================================================
    
    /**
     * Get complete climate state (read-only).
     * Thread-safe: called from main loop with mutex held by caller.
     */
    const State& getState() const { return state; }
    
    /**
     * Get temperature information.
     */
    float getInsideTemp() const { return state.insideTemp; }
    float getOutsideTemp() const { return state.outsideTemp; }
    
    /**
     * Get climate control status (BAP-only).
     */
    bool isActive() const { return state.climateActive; }
    bool isHeating() const { return state.heating; }
    bool isCooling() const { return state.cooling; }
    bool isVentilating() const { return state.ventilation; }
    uint16_t getRemainingTimeMin() const { return state.climateTimeMin; }
    
    // =========================================================================
    // Command Interface (delegates to BatteryControlChannel)
    // =========================================================================
    
    /**
     * Request current climate state from vehicle (non-blocking).
     * @return true if command queued, false if busy
     */
    bool requestState();
    
    /**
     * Start climate control (non-blocking).
     * @param commandId Command ID for tracking
     * @param tempCelsius Target temperature in Celsius (default 21.0)
     * @param allowBattery Allow using battery power (default false - only when plugged)
     * @return true if command queued, false if busy
     */
    bool startClimate(int commandId, float tempCelsius = 21.0f, bool allowBattery = false);
    
    /**
     * Stop climate control (non-blocking).
     * @param commandId Command ID for tracking
     * @return true if command queued, false if busy
     */
    bool stopClimate(int commandId);
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& klima03, uint32_t& klimaSensor02) const {
        klima03 = klima03Count;
        klimaSensor02 = klimaSensor02Count;
    }
    
    uint32_t getCallbackCount() const {
        return climateCallbackCount;
    }

private:
    VehicleManager* vehicleManager;
    BatteryControlChannel* bapChannel;  // Reference to BAP channel (set in setup)
    
    // Domain state (owned by this manager)
    State state;
    
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_KLIMA_03 = 0x66E;         // Inside temp, climate status
    static constexpr uint32_t CAN_ID_KLIMA_SENSOR_02 = 0x5E1;  // Outside temp
    
    // Statistics
    volatile uint32_t klima03Count = 0;
    volatile uint32_t klimaSensor02Count = 0;
    volatile uint32_t climateCallbackCount = 0;
    
    // CAN frame processors
    void processKlima03(const uint8_t* data);
    void processKlimaSensor02(const uint8_t* data);
    
    // BAP callback handler (registered in setup)
    void onClimateStateUpdate(const ClimateState& climate);
};
