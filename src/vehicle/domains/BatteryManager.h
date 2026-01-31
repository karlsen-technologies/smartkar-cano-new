#pragma once

#include <Arduino.h>
#include "../IDomain.h"
#include "../VehicleTypes.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declarations
class VehicleManager;
class BatteryControlChannel;
class ChargingProfileManager;
class WakeController;

/**
 * BatteryManager - Unified battery domain manager
 * 
 * Encapsulates ALL battery data sources:
 * - Standard CAN frames (11-bit): 0x5CA, 0x59E, 0x483
 * - BAP channel callbacks: plug state, charge state
 * 
 * Implements new domain-based architecture:
 * - Owns complete battery state
 * - Subscribes to BatteryControlChannel callbacks
 * - Provides clean public API
 * - Fast CAN frame processing (<1-2ms)
 * 
 * Data Source Priority:
 * 1. BAP (most detailed and reliable)
 * 2. Standard CAN (real-time broadcast)
 * 3. Computed (derived values)
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex
 * - loop() called from main loop (Core 1)
 * - State access is thread-safe via VehicleManager mutex
 */
class BatteryManager : public IDomain {
public:
    /**
     * Battery state structure
     * Combines data from CAN and BAP sources
     */
    struct State {
        // === From Standard CAN ===
        
        // BMS_07 (0x5CA) - Energy and charging
        float energyWh = 0.0f;
        float maxEnergyWh = 0.0f;
        bool chargingActive = false;      // CAN broadcast
        bool balancingActive = false;
        unsigned long energyUpdate = 0;
        unsigned long balancingUpdate = 0;
        
        // BMS_06 (0x59E) - Temperature
        float temperature = 0.0f;
        unsigned long tempUpdate = 0;
        
        // Motor_Hybrid_06 (0x483) - Power
        float powerKw = 0.0f;
        unsigned long powerUpdate = 0;
        
        // === From BAP (via BatteryControlChannel) ===
        
        // Plug state (function 0x10)
        PlugState plugState;
        DataSource plugStateSource = DataSource::NONE;
        unsigned long plugStateUpdate = 0;
        
        // Charge state (function 0x11)
        float soc = 0.0f;                 // BAP provides more accurate SOC
        DataSource socSource = DataSource::NONE;
        uint8_t chargingMode = 0;         // BAP detailed charging mode
        uint8_t chargingStatus = 0;       // BAP detailed charging status
        uint8_t chargingAmps = 0;
        uint8_t targetSoc = 0;
        uint8_t remainingTimeMin = 0;
        bool charging = false;             // Unified (BAP overrides CAN)
        DataSource chargingSource = DataSource::NONE;
        unsigned long chargingUpdate = 0;
        unsigned long socUpdate = 0;
        
        // === Computed ===
        
        /**
         * Calculate energy percentage (current/max)
         */
        float energyPercent() const {
            if (maxEnergyWh > 0) {
                return (energyWh / maxEnergyWh) * 100.0f;
            }
            return 0.0f;
        }
        
        /**
         * Check if state is valid (has received data)
         */
        bool isValid() const {
            return energyUpdate > 0 || socUpdate > 0;
        }
    };

    /**
     * Construct the battery manager.
     * @param vehicleManager Pointer to vehicle manager for sending commands
     */
    explicit BatteryManager(VehicleManager* vehicleManager);

    // =========================================================================
    // IDomain interface implementation
    // =========================================================================
    
    const char* getName() const override { return "BatteryManager"; }
    bool setup() override;
    void loop() override;
    void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    void onWakeComplete() override;
    bool isBusy() const override;

    // =========================================================================
    // Public API (for external consumers)
    // =========================================================================
    
    /**
     * Get complete battery state (read-only).
     * Thread-safe: called from main loop with mutex held by caller.
     */
    const State& getState() const { return state; }
    
    /**
     * Get plug state (connection, lock, supply).
     */
    const PlugState& getPlugState() const { return state.plugState; }
    
    /**
     * Get charge state information.
     */
    float getSoc() const { return state.soc; }
    bool isCharging() const { return state.charging; }
    bool isPlugged() const { return state.plugState.isPlugged(); }
    
    /**
     * Get energy information.
     */
    float getEnergyWh() const { return state.energyWh; }
    float getMaxEnergyWh() const { return state.maxEnergyWh; }
    float getEnergyPercent() const { return state.energyPercent(); }
    
    /**
     * Get power information.
     */
    float getPowerKw() const { return state.powerKw; }
    float getTemperature() const { return state.temperature; }
    
    // =========================================================================
    // Command Interface (NEW - Phase 2: Uses domain state machine)
    // =========================================================================
    
    /**
     * Start charging with domain orchestration (non-blocking).
     * Uses profile manager and wake controller for intelligent command execution.
     * 
     * Workflow:
     * 1. Validate charging parameters (business logic)
     * 2. Request wake if needed
     * 3. Update profile 0 if needed (SOC/current changed)
     * 4. Execute profile 0
     * 5. Monitor response and call callback
     * 
     * @param commandId Command ID for tracking
     * @param targetSoc Target state of charge (0-100)
     * @param maxCurrent Maximum charging current in amps
     * @return true if command queued, false if already busy
     */
    bool startCharging(uint8_t commandId, uint8_t targetSoc = 80, uint8_t maxCurrent = 32);
    
    /**
     * Stop charging (non-blocking).
     * Stops profile 0 execution immediately.
     * 
     * @param commandId Command ID for tracking
     * @return true if command queued, false if busy
     */
    bool stopCharging(uint8_t commandId);
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& bms07, uint32_t& bms06, uint32_t& motorHybrid06) const {
        bms07 = bms07Count;
        bms06 = bms06Count;
        motorHybrid06 = motorHybrid06Count;
    }
    
    void getCallbackCounts(uint32_t& plugCallbacks, uint32_t& chargeCallbacks) const {
        plugCallbacks = plugCallbackCount;
        chargeCallbacks = chargeCallbackCount;
    }

private:
    VehicleManager* vehicleManager;
    BatteryControlChannel* bapChannel;  // Reference to BAP channel (set in setup)
    ChargingProfileManager* profileManager;  // Reference to profile manager (set in setup)
    WakeController* wakeController;  // Reference to wake controller (set in setup)
    
    // Domain state (reference to RTC memory - survives deep sleep)
    State& state;
    
    // =========================================================================
    // Command State Machine (NEW - Phase 2)
    // =========================================================================
    
    /**
     * Command state machine states
     */
    enum class CommandState {
        IDLE,                   // No command in progress
        REQUESTING_WAKE,        // Waiting for vehicle wake
        UPDATING_PROFILE,       // Profile update in progress
        EXECUTING_COMMAND,      // Profile execution in progress
        DONE,                   // Command complete
        FAILED                  // Command failed
    };
    
    /**
     * Pending command type
     */
    enum class PendingCommandType {
        NONE,
        START_CHARGING,
        STOP_CHARGING
    };
    
    // Command state machine state
    CommandState cmdState = CommandState::IDLE;
    unsigned long cmdStateStartTime = 0;  // When current state was entered
    
    // Pending command data
    PendingCommandType pendingCmdType = PendingCommandType::NONE;
    int pendingCommandId = -1;
    uint8_t pendingTargetSoc = 80;
    uint8_t pendingMaxCurrent = 32;
    
    // Command state machine methods
    void updateCommandStateMachine();
    void setCommandState(CommandState newState);
    bool validateChargingParams(uint8_t targetSoc, uint8_t maxCurrent);
    bool needsProfileUpdate(uint8_t targetSoc, uint8_t maxCurrent);
    void completeCommand();
    void failCommand(const char* reason);
    
    // Timeout constants
    static constexpr unsigned long WAKE_TIMEOUT = 10000;      // 10 seconds
    static constexpr unsigned long PROFILE_UPDATE_TIMEOUT = 30000;  // 30 seconds
    static constexpr unsigned long EXECUTION_TIMEOUT = 30000;  // 30 seconds
    
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_BMS_07 = 0x5CA;           // Charging status, energy
    static constexpr uint32_t CAN_ID_BMS_06 = 0x59E;           // Battery temperature
    static constexpr uint32_t CAN_ID_MOTOR_HYBRID_06 = 0x483;  // Power meter (kW)
    
    // Statistics
    volatile uint32_t bms07Count = 0;
    volatile uint32_t bms06Count = 0;
    volatile uint32_t motorHybrid06Count = 0;
    volatile uint32_t plugCallbackCount = 0;
    volatile uint32_t chargeCallbackCount = 0;
    
    // CAN frame processors
    void processBMS07(const uint8_t* data);
    void processBMS06(const uint8_t* data);
    void processMotorHybrid06(const uint8_t* data);
    
    // BAP callback handlers (registered in setup)
    void onPlugStateUpdate(const PlugState& plug);
    void onChargeStateUpdate(const BatteryState& battery);
};
