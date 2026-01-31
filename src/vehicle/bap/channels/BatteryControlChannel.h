#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>
#include "../BapChannel.h"
#include "../../VehicleTypes.h"
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
    
    BatteryControlChannel(VehicleManager* mgr);
    
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
    // Callback Registration (for new domain-based architecture)
    // =========================================================================
    
    /**
     * Callback types for domain subscribers.
     * Domains register these callbacks to be notified when state updates occur.
     * Callbacks are called from CAN thread (Core 0) - keep them FAST!
     */
    using PlugStateCallback = std::function<void(const PlugState&)>;
    using ChargeStateCallback = std::function<void(const BatteryState&)>;  // Pass full battery state for charge info
    using ClimateStateCallback = std::function<void(const ClimateState&)>;
    
    /**
     * Register callback for plug state updates (function 0x10).
     * Called whenever plug state changes (plug/unplug, lock/unlock, supply changes).
     * @param callback Function to call with new plug state
     */
    void onPlugState(PlugStateCallback callback) {
        plugStateCallbacks.push_back(callback);
    }
    
    /**
     * Register callback for charge state updates (function 0x11).
     * Called whenever charging state changes (SOC, current, status, mode).
     * @param callback Function to call with updated battery state
     */
    void onChargeState(ChargeStateCallback callback) {
        chargeStateCallbacks.push_back(callback);
    }
    
    /**
     * Register callback for climate state updates (function 0x12).
     * Called whenever climate state changes (on/off, temp, heating/cooling).
     * @param callback Function to call with updated climate state
     */
    void onClimateState(ClimateStateCallback callback) {
        climateStateCallbacks.push_back(callback);
    }
    
    /**
     * Process a CAN frame directly (for new architecture).
     * Alternative to using BapChannelRouter - allows domain managers to
     * route directly to this channel from VehicleManager.
     * @param canId CAN ID (must match RX ID)
     * @param data Frame data
     * @param dlc Data length code
     * @return true if frame was processed
     */
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    // =========================================================================
    // State Request Methods
    // =========================================================================
    
    bool requestPlugState();
    bool requestChargeState();
    bool requestClimateState();
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& plugCount, uint32_t& chargeCount, 
                        uint32_t& climateCount, uint32_t& profileCount) const {
        plugCount = plugFrames;
        chargeCount = chargeFrames;
        climateCount = climateFrames;
        profileCount = profileFrames;
    }
    
private:
    VehicleManager* manager;
    
    // BAP frame assembly
    BapProtocol::BapFrameAssembler frameAssembler;
    
    // Callback subscribers (for new domain-based architecture)
    std::vector<PlugStateCallback> plugStateCallbacks;
    std::vector<ChargeStateCallback> chargeStateCallbacks;
    std::vector<ClimateStateCallback> climateStateCallbacks;
    
    // Statistics
    volatile uint32_t plugFrames = 0;
    volatile uint32_t chargeFrames = 0;
    volatile uint32_t climateFrames = 0;
    volatile uint32_t profileFrames = 0;
    volatile uint32_t otherFrames = 0;
    volatile uint32_t ignoredRequests = 0;
    volatile uint32_t decodeErrors = 0;
    
    // =========================================================================
    // Internal methods
    // =========================================================================
    
    /**
     * Notify all registered callbacks (for new domain-based architecture).
     * Called after state is updated. Keep FAST - just data copying.
     */
    void notifyPlugStateCallbacks(const PlugState& plugData);
    void notifyChargeStateCallbacks(const BatteryState& batteryData);
    void notifyClimateStateCallbacks(const ClimateState& climateData);
    
    void processPlugState(const uint8_t* payload, uint8_t len);
    void processChargeState(const uint8_t* payload, uint8_t len);
    void processClimateState(const uint8_t* payload, uint8_t len);
    
    PlugStateData decodePlugState(const uint8_t* payload, uint8_t len);
    ChargeStateData decodeChargeState(const uint8_t* payload, uint8_t len);
    ClimateStateData decodeClimateState(const uint8_t* payload, uint8_t len);
    
    uint8_t buildGetRequest(uint8_t* dest, uint8_t functionId);
    
    bool sendBapFrame(const uint8_t* data, uint8_t len);
};
