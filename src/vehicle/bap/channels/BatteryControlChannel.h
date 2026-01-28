#pragma once

#include <Arduino.h>
#include "../BapChannel.h"
#include "../../VehicleState.h"
#include "../../protocols/BapProtocol.h"

// Forward declaration
class VehicleManager;

/**
 * BatteryControlChannel - BAP Channel for Battery Control (Device 0x25)
 * 
 * Handles all Battery Control functionality:
 * - Plug state (connection, lock, power supply)
 * - Charge state (SOC, charging status, mode)
 * - Climate state (heating/cooling status, temperature)
 * - Command sending (start/stop climate, start/stop charging)
 * 
 * Architecture:
 * ```
 * CAN Frame → BapChannelRouter → BatteryControlChannel
 *                                      ↓
 *                                 VehicleState (updates)
 * ```
 * 
 * This channel passively listens to RX channel (0x17332510) to track state
 * changes made by ANY module (infotainment, gateway, phone app), and actively
 * sends commands on TX channel (0x17332501) when needed.
 */
class BatteryControlChannel : public BapChannel {
public:
    // Device ID and CAN IDs
    static constexpr uint8_t DEVICE_ID = 0x25;
    static constexpr uint32_t CAN_ID_TX = 0x17332501;  // Commands TO battery control
    static constexpr uint32_t CAN_ID_RX = 0x17332510;  // Responses FROM battery control
    
    // Function IDs (Battery Control - Device 0x25)
    struct Function {
        static constexpr uint8_t GET_ALL_PROPERTIES = 0x01;
        static constexpr uint8_t BAP_CONFIG = 0x02;
        static constexpr uint8_t FUNCTION_LIST = 0x03;
        static constexpr uint8_t HEARTBEAT_CONFIG = 0x04;
        static constexpr uint8_t FSG_SETUP = 0x0E;
        static constexpr uint8_t FSG_OPERATION_STATE = 0x0F;
        static constexpr uint8_t PLUG_STATE = 0x10;          // Plug connection and lock
        static constexpr uint8_t CHARGE_STATE = 0x11;        // SOC, charging status
        static constexpr uint8_t CLIMATE_STATE = 0x12;       // Climate active, temps
        static constexpr uint8_t START_STOP_CHARGE = 0x14;   // Start/stop charging
        static constexpr uint8_t START_STOP_CLIMATE = 0x15;  // Start/stop climate
        static constexpr uint8_t OPERATION_MODE = 0x18;      // Climate operation mode
        static constexpr uint8_t PROFILES_ARRAY = 0x19;      // Charging/climate profiles
        static constexpr uint8_t POWER_PROVIDERS = 0x1A;     // Power provider info
    };
    
    // Charge/Climate State Enums
    enum class ChargeMode : uint8_t {
        OFF = 0x0,
        AC = 0x1,
        DC = 0x2,
        CONDITIONING = 0x3,
        AC_AND_CONDITIONING = 0x4,
        DC_AND_CONDITIONING = 0x5,
        INIT = 0xF
    };
    
    enum class ChargeStatus : uint8_t {
        INIT = 0x0,
        IDLE = 0x1,
        RUNNING = 0x2,
        CONSERVATION = 0x3,
        ABORTED_TEMP_LOW = 0x4,
        ABORTED_DEVICE_ERROR = 0x5,
        ABORTED_NO_POWER = 0x6,
        ABORTED_NOT_IN_PARK = 0x7,
        COMPLETED = 0x8,
        NO_ERROR = 0x9
    };
    
    enum class PlugStatus : uint8_t {
        UNPLUGGED = 0x0,
        PLUGGED = 0x1,
        INIT = 0xF
    };
    
    enum class SupplyStatus : uint8_t {
        INACTIVE = 0x0,
        ACTIVE = 0x1,
        CHARGE_STATION_CONNECTED = 0x2,
        INIT = 0xF
    };
    
    // Decoded data structures
    struct PlugStateData {
        uint8_t lockSetup = 0;
        uint8_t lockState = 0;
        SupplyStatus supplyState = SupplyStatus::INIT;
        PlugStatus plugState = PlugStatus::INIT;
        
        bool isPlugged() const { return plugState == PlugStatus::PLUGGED; }
        bool hasSupply() const { 
            return supplyState == SupplyStatus::ACTIVE || 
                   supplyState == SupplyStatus::CHARGE_STATION_CONNECTED; 
        }
    };
    
    struct ChargeStateData {
        ChargeMode chargeMode = ChargeMode::INIT;
        ChargeStatus chargeStatus = ChargeStatus::INIT;
        uint8_t socPercent = 0;
        uint8_t remainingTimeMin = 0;
        uint8_t currentRange = 0;
        uint8_t rangeUnit = 0;
        uint8_t chargingAmps = 0;
        uint8_t batteryClimateState = 0;
        uint8_t startReason = 0;
        uint8_t targetSoc = 0;
        
        bool isCharging() const { 
            return chargeMode != ChargeMode::OFF && chargeMode != ChargeMode::INIT &&
                   chargeStatus == ChargeStatus::RUNNING; 
        }
        bool isAcCharging() const { 
            return chargeMode == ChargeMode::AC || chargeMode == ChargeMode::AC_AND_CONDITIONING; 
        }
        bool isDcCharging() const { 
            return chargeMode == ChargeMode::DC || chargeMode == ChargeMode::DC_AND_CONDITIONING; 
        }
    };
    
    struct ClimateStateData {
        bool climateActive = false;
        bool autoDefrost = false;
        bool heating = false;
        bool cooling = false;
        bool ventilation = false;
        bool fuelBasedHeating = false;
        float currentTempC = 0.0f;
        uint8_t tempUnit = 0;
        uint16_t climateTimeMin = 0;
        uint8_t climateState = 0;
        
        bool isActive() const { return climateActive; }
    };
    
    // Profile operation modes
    struct ProfileOperation {
        static constexpr uint8_t CHARGING = 0x01;
        static constexpr uint8_t CLIMATE = 0x02;
        static constexpr uint8_t CHARGING_AND_CLIMATE = 0x03;
        static constexpr uint8_t CHARGING_ALLOW_CLIMATE_BATTERY = 0x05;
        static constexpr uint8_t CLIMATE_ALLOW_BATTERY = 0x06;
    };
    
    // =========================================================================
    // Construction
    // =========================================================================
    
    BatteryControlChannel(VehicleState& state, VehicleManager* mgr);
    
    // =========================================================================
    // BapChannel interface implementation
    // =========================================================================
    
    uint8_t getDeviceId() const override { return DEVICE_ID; }
    uint32_t getTxCanId() const override { return CAN_ID_TX; }
    uint32_t getRxCanId() const override { return CAN_ID_RX; }
    bool handlesCanId(uint32_t canId) const override;
    const char* getName() const override { return "BatteryControl"; }
    bool processMessage(const BapProtocol::BapMessage& msg) override;
    
    // =========================================================================
    // State accessors
    // =========================================================================
    
    const BapPlugState& getPlugState() const { return state.bapPlug; }
    const BapChargeState& getChargeState() const { return state.bapCharge; }
    const BapClimateState& getClimateState() const { return state.bapClimate; }
    
    // =========================================================================
    // Command State Machine
    // =========================================================================
    
    enum class CommandState {
        IDLE,              // No pending commands
        REQUESTING_WAKE,   // Requested vehicle wake
        WAITING_FOR_WAKE,  // Polling for vehicle awake
        SENDING_COMMAND,   // Sending BAP frames
        DONE,              // Command sent successfully
        FAILED             // Command failed (wake timeout, send error)
    };
    
    /**
     * Process command state machine (called from VehicleManager::loop())
     */
    void loop();
    
    /**
     * Check if command is in progress
     */
    bool isBusy() const { return commandState != CommandState::IDLE; }
    
    /**
     * Get current command state
     */
    CommandState getCommandState() const { return commandState; }
    
    /**
     * Get command state as string (for logging)
     */
    const char* getCommandStateName() const;
    
    // =========================================================================
    // Command methods (non-blocking - set flags, processed in loop())
    // =========================================================================
    
    bool requestPlugState();
    bool requestChargeState();
    bool requestClimateState();
    
    /**
     * Start climate control (non-blocking).
     * Sets flag, processed in loop() when vehicle awake.
     * @return true if queued, false if busy with another command
     */
    bool startClimate(float tempCelsius = 21.0f, bool allowBattery = false);
    
    /**
     * Stop climate control (non-blocking).
     * @return true if queued, false if busy
     */
    bool stopClimate();
    
    /**
     * Start charging (non-blocking).
     * @param targetSoc Target state of charge percentage (0-100)
     * @param maxCurrent Maximum charging current in amps
     * @return true if queued, false if busy
     */
    bool startCharging(uint8_t targetSoc = 80, uint8_t maxCurrent = 32);
    
    /**
     * Stop charging (non-blocking).
     * @return true if queued, false if busy
     */
    bool stopCharging();
    
    /**
     * Start combined charging and climate (non-blocking).
     * @param tempCelsius Target temperature in Celsius
     * @param targetSoc Target state of charge percentage (0-100)
     * @param maxCurrent Maximum charging current in amps
     * @param allowBattery Allow climate to use battery power
     * @return true if queued, false if busy
     */
    bool startChargingAndClimate(float tempCelsius = 21.0f, uint8_t targetSoc = 80, 
                                  uint8_t maxCurrent = 32, bool allowBattery = false);
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& plugCount, uint32_t& chargeCount, 
                        uint32_t& climateCount) const {
        plugCount = plugFrames;
        chargeCount = chargeFrames;
        climateCount = climateFrames;
    }
    
    void getCommandStats(uint32_t& queued, uint32_t& completed, uint32_t& failed) const {
        queued = commandsQueued;
        completed = commandsCompleted;
        failed = commandsFailed;
    }
    
private:
    VehicleState& state;
    VehicleManager* manager;
    
    // Command state machine
    CommandState commandState = CommandState::IDLE;
    unsigned long commandStateStartTime = 0;
    
    // Pending command data (only ONE can be active at a time)
    enum class PendingCommand {
        NONE,
        START_CLIMATE,
        STOP_CLIMATE,
        START_CHARGING,
        STOP_CHARGING,
        START_CHARGING_AND_CLIMATE
    };
    
    PendingCommand pendingCommand = PendingCommand::NONE;
    
    // Command parameters
    float pendingTempCelsius = 21.0f;
    bool pendingAllowBattery = false;
    uint8_t pendingTargetSoc = 80;
    uint8_t pendingMaxCurrent = 32;
    
    // Command timing
    static constexpr unsigned long COMMAND_WAKE_TIMEOUT = 15000;  // 15s max wait for wake
    
    // Statistics
    volatile uint32_t plugFrames = 0;
    volatile uint32_t chargeFrames = 0;
    volatile uint32_t climateFrames = 0;
    volatile uint32_t otherFrames = 0;
    volatile uint32_t ignoredRequests = 0;
    volatile uint32_t decodeErrors = 0;
    
    uint32_t commandsQueued = 0;
    uint32_t commandsCompleted = 0;
    uint32_t commandsFailed = 0;
    
    // =========================================================================
    // Internal methods
    // =========================================================================
    
    /**
     * Update command state machine (called from loop())
     */
    void updateCommandStateMachine();
    
    /**
     * Execute the pending command (send BAP frames)
     */
    bool executePendingCommand();
    
    /**
     * Set command state with logging
     */
    void setCommandState(CommandState newState);
    
    /**
     * Get pending command name as string (for events)
     */
    const char* getPendingCommandName() const;
    
    /**
     * Emit command event via CommandRouter singleton
     * @param eventName Event name (e.g., "commandCompleted", "commandFailed")
     * @param reason Failure reason (nullptr for success)
     */
    void emitCommandEvent(const char* eventName, const char* reason);
    
    void processPlugState(const uint8_t* payload, uint8_t len);
    void processChargeState(const uint8_t* payload, uint8_t len);
    void processClimateState(const uint8_t* payload, uint8_t len);
    
    PlugStateData decodePlugState(const uint8_t* payload, uint8_t len);
    ChargeStateData decodeChargeState(const uint8_t* payload, uint8_t len);
    ClimateStateData decodeClimateState(const uint8_t* payload, uint8_t len);
    
    uint8_t buildGetRequest(uint8_t* dest, uint8_t functionId);
    uint8_t buildClimateStart(uint8_t* dest, float tempCelsius);
    uint8_t buildClimateStop(uint8_t* dest);
    uint8_t buildChargeStart(uint8_t* dest);
    uint8_t buildChargeStop(uint8_t* dest);
    uint8_t buildChargingAndClimateStart(uint8_t* dest);
    uint8_t buildOperationModeStart(uint8_t* dest);
    uint8_t buildOperationModeStop(uint8_t* dest);
    uint8_t buildProfileConfig(uint8_t* startFrame, uint8_t* contFrame, uint8_t operationMode);
    
    bool sendBapFrame(const uint8_t* data, uint8_t len);
};
