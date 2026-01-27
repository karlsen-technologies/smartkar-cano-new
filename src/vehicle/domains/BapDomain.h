#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BapProtocol.h"

// Forward declaration
class VehicleManager;

/**
 * BapDomain - BAP Protocol Handler
 * 
 * Handles BAP (Bedien- und Anzeigeprotokoll) messages for Battery Control.
 * 
 * Architecture:
 * - PASSIVELY listens to RX channel (0x17332510) to track state changes
 *   made by ANY module (infotainment, gateway, phone app, etc.)
 * - ACTIVELY sends commands on TX channel (0x17332501) when we need to act
 * 
 * This allows us to know the current state of charging/climate even if
 * another module initiated the action.
 */
class BapDomain {
public:
    BapDomain(VehicleState& state, VehicleManager* mgr);
    
    // =========================================================================
    // Frame handling
    // =========================================================================
    
    /**
     * Check if this domain handles a given CAN ID
     * We only handle the RX channel (responses) for passive monitoring
     */
    static bool handlesCanId(uint32_t canId);
    
    /**
     * Get list of CAN IDs this domain handles
     */
    static void getHandledIds(uint32_t* ids, uint8_t& count, uint8_t maxCount);
    
    /**
     * Process an incoming CAN frame (from RX channel)
     * Returns true if frame was processed
     */
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    // =========================================================================
    // State accessors
    // =========================================================================
    
    const BapPlugState& getPlugState() const { return state.bapPlug; }
    const BapChargeState& getChargeState() const { return state.bapCharge; }
    const BapClimateState& getClimateState() const { return state.bapClimate; }
    
    // =========================================================================
    // Command methods (send on TX channel)
    // =========================================================================
    
    /**
     * Request current plug state
     */
    bool requestPlugState();
    
    /**
     * Request current charge state
     */
    bool requestChargeState();
    
    /**
     * Request current climate state
     */
    bool requestClimateState();
    
    /**
     * Start climate control
     * @param tempCelsius Target temperature (15.5 - 30.0)
     */
    bool startClimate(float tempCelsius = 21.0f);
    
    /**
     * Stop climate control
     */
    bool stopClimate();
    
    /**
     * Start charging
     */
    bool startCharging();
    
    /**
     * Stop charging
     */
    bool stopCharging();
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    void getFrameCounts(uint32_t& plugCount, uint32_t& chargeCount, 
                        uint32_t& climateCount) const {
        plugCount = plugFrames;
        chargeCount = chargeFrames;
        climateCount = climateFrames;
    }
    
private:
    VehicleState& state;
    VehicleManager* manager;
    
    // Frame counters for statistics
    uint32_t plugFrames = 0;
    uint32_t chargeFrames = 0;
    uint32_t climateFrames = 0;
    uint32_t otherFrames = 0;
    
    // Long message reassembly buffer
    // Key: (messageIndex << 8) | functionId
    static constexpr size_t MAX_LONG_MSG_SIZE = 64;
    uint8_t longMsgBuffer[MAX_LONG_MSG_SIZE];
    uint8_t longMsgLength = 0;
    uint8_t longMsgExpectedLength = 0;
    uint8_t longMsgFunctionId = 0;
    uint8_t longMsgMessageIndex = 0;
    bool longMsgInProgress = false;
    
    // =========================================================================
    // Internal methods
    // =========================================================================
    
    /**
     * Handle a complete BAP message (short or reassembled long)
     */
    void handleBapMessage(uint8_t functionId, uint8_t opcode, 
                          const uint8_t* payload, uint8_t payloadLen);
    
    /**
     * Process PlugState response (function 0x10)
     */
    void processPlugState(const uint8_t* payload, uint8_t len);
    
    /**
     * Process ChargeState response (function 0x11)
     */
    void processChargeState(const uint8_t* payload, uint8_t len);
    
    /**
     * Process ClimateState response (function 0x12)
     */
    void processClimateState(const uint8_t* payload, uint8_t len);
    
    /**
     * Send a BAP command frame
     */
    bool sendBapFrame(const uint8_t* data, uint8_t len);
};
