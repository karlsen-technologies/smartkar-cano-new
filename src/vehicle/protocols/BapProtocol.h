#pragma once

#include <Arduino.h>

/**
 * BAP (Bedien- und Anzeigeprotokoll) Protocol Handler
 * 
 * VW's proprietary two-way protocol over CAN for control modules.
 * Used primarily for Battery Control (charging, climate, plug state).
 * 
 * Architecture:
 * - Multiple modules (OCU, Infotainment, Gateway) share the BAP channels
 * - We primarily LISTEN to responses (0x17332510) to track state
 * - We SEND commands (0x17332501) only when we need to act
 * 
 * Message Types:
 * - Short: ≤6 byte payload, single frame
 * - Long: >6 byte payload, multi-frame with start + continuation
 */

namespace BapProtocol {

// =============================================================================
// CAN IDs
// =============================================================================

// Battery Control channel (Device 0x25)
constexpr uint32_t CAN_ID_BATTERY_TX = 0x17332501;  // Commands TO battery control
constexpr uint32_t CAN_ID_BATTERY_RX = 0x17332510;  // Responses FROM battery control

// Wake/Init (for reference, not primary focus)
constexpr uint32_t CAN_ID_WAKE = 0x17330301;
constexpr uint32_t CAN_ID_BAP_INIT = 0x1B000067;

// =============================================================================
// Device IDs
// =============================================================================

constexpr uint8_t DEVICE_BATTERY_CONTROL = 0x25;
constexpr uint8_t DEVICE_DOOR_LOCKING = 0x0D;
constexpr uint8_t DEVICE_ENI = 0x37;

// =============================================================================
// Function IDs (Battery Control - Device 0x25)
// =============================================================================

namespace Function {
    constexpr uint8_t GET_ALL_PROPERTIES = 0x01;
    constexpr uint8_t BAP_CONFIG = 0x02;
    constexpr uint8_t FUNCTION_LIST = 0x03;
    constexpr uint8_t HEARTBEAT_CONFIG = 0x04;
    constexpr uint8_t FSG_SETUP = 0x0E;
    constexpr uint8_t FSG_OPERATION_STATE = 0x0F;
    constexpr uint8_t PLUG_STATE = 0x10;          // Plug connection and lock
    constexpr uint8_t CHARGE_STATE = 0x11;        // SOC, charging status
    constexpr uint8_t CLIMATE_STATE = 0x12;       // Climate active, temps
    constexpr uint8_t START_STOP_CHARGE = 0x14;   // Start/stop charging
    constexpr uint8_t START_STOP_CLIMATE = 0x15;  // Start/stop climate
    constexpr uint8_t OPERATION_MODE = 0x18;      // Climate operation mode
    constexpr uint8_t PROFILES_ARRAY = 0x19;      // Charging/climate profiles
    constexpr uint8_t POWER_PROVIDERS = 0x1A;     // Power provider info
}

// =============================================================================
// OpCodes
// =============================================================================

namespace OpCode {
    // Request opcodes (for sending)
    constexpr uint8_t RESET = 0x00;
    constexpr uint8_t GET = 0x01;           // Request current state
    constexpr uint8_t SET_GET = 0x02;       // Set value and get result
    
    // Response opcodes (what we receive)
    constexpr uint8_t HEARTBEAT = 0x03;     // Periodic status update
    constexpr uint8_t PROCESSING = 0x04;    // Command acknowledged, working
    constexpr uint8_t INDICATION = 0x05;    // Unsolicited state change
    constexpr uint8_t ACK = 0x06;           // Simple acknowledgment
    constexpr uint8_t ERROR = 0x07;         // Command failed
}

// =============================================================================
// Charge/Climate State Enums
// =============================================================================

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

// =============================================================================
// Decoded Message Structure
// =============================================================================

struct BapHeader {
    uint8_t opcode = 0;
    uint8_t deviceId = 0;
    uint8_t functionId = 0;
    bool isLong = false;
    bool isContinuation = false;
    uint8_t group = 0;        // Bits 5-4 of control byte (0-3), identifies concurrent message stream
    uint8_t index = 0;        // Bits 3-0 of control byte (0-15), sequence counter within group
    uint8_t totalLength = 0;  // For long messages: payload length (not including BAP header)
    
    bool isValid() const { return deviceId != 0 || functionId != 0; }
    bool isResponse() const { return opcode >= OpCode::HEARTBEAT; }
    bool isError() const { return opcode == OpCode::ERROR; }
};

// =============================================================================
// Decoding Functions
// =============================================================================

/**
 * Decode BAP header from CAN frame data
 * 
 * Short message format:
 *   Byte 0: [0][OpCode:3][DeviceID_hi:4]
 *   Byte 1: [DeviceID_lo:2][FunctionID:6]
 *   Bytes 2-7: Payload
 * 
 * Long message control byte format:
 *   Byte 0: [Long:1][Cont:1][Group:2][Index:4]
 *           Bit 7: 1 = Long message
 *           Bit 6: 0 = Start frame, 1 = Continuation frame
 *           Bits 5-4: Group (0-3), allows 4 concurrent message streams
 *           Bits 3-0: Index (0-15), sequence counter within group
 * 
 * Long start frame:
 *   Byte 0: [1][0][Group:2][0000]  -- Index always 0 for start
 *   Byte 1: total_length (payload bytes, not including BAP header)
 *   Byte 2: [0][OpCode:3][DeviceID_hi:4]
 *   Byte 3: [DeviceID_lo:2][FunctionID:6]
 *   Bytes 4-7: First 4 bytes of payload
 * 
 * Long continuation frame:
 *   Byte 0: [1][1][Group:2][Index:4]  -- Index increments each frame, wraps at 16
 *   Bytes 1-7: Next 7 bytes of payload
 */
BapHeader decodeHeader(const uint8_t* data, uint8_t dlc);

/**
 * Get payload from a short message (bytes 2-7)
 */
inline const uint8_t* getShortPayload(const uint8_t* data) {
    return data + 2;
}

/**
 * Get payload length from a short message
 */
inline uint8_t getShortPayloadLength(uint8_t dlc) {
    return (dlc > 2) ? (dlc - 2) : 0;
}

/**
 * Get payload from a long start frame (bytes 4-7)
 */
inline const uint8_t* getLongStartPayload(const uint8_t* data) {
    return data + 4;
}

/**
 * Get payload from a continuation frame (bytes 1-7)
 */
inline const uint8_t* getContinuationPayload(const uint8_t* data) {
    return data + 1;
}

// =============================================================================
// Encoding Functions
// =============================================================================

/**
 * Encode BAP header bytes
 * Returns: number of bytes written (always 2)
 */
uint8_t encodeHeader(uint8_t* dest, uint8_t opcode, uint8_t deviceId, uint8_t functionId);

/**
 * Encode a short BAP message (≤6 byte payload)
 * Returns: total frame length (always 8, padded with zeros)
 */
uint8_t encodeShortMessage(uint8_t* dest, uint8_t opcode, uint8_t deviceId, 
                           uint8_t functionId, const uint8_t* payload = nullptr, 
                           uint8_t payloadLen = 0);

/**
 * Encode a long BAP message start frame
 * @param group Message group (0-3) for interleaved streams
 * Returns: frame length (always 8)
 */
uint8_t encodeLongStart(uint8_t* dest, uint8_t opcode, uint8_t deviceId,
                        uint8_t functionId, uint8_t totalPayloadLen,
                        const uint8_t* payload, uint8_t group = 0);

/**
 * Encode a long BAP message continuation frame
 * @param group Message group (0-3) for interleaved streams
 * @param index Sequence counter (0-15), increments each continuation
 * Returns: frame length
 */
uint8_t encodeLongContinuation(uint8_t* dest, const uint8_t* payload, 
                                uint8_t payloadLen, uint8_t group = 0, uint8_t index = 0);

// =============================================================================
// Payload Decoders (for specific functions)
// =============================================================================

struct PlugStateData {
    uint8_t lockSetup = 0;
    uint8_t lockState = 0;
    SupplyStatus supplyState = SupplyStatus::INIT;
    PlugStatus plugState = PlugStatus::INIT;
    
    bool isPlugged() const { return plugState == PlugStatus::PLUGGED; }
    bool hasSupply() const { return supplyState == SupplyStatus::ACTIVE || 
                                   supplyState == SupplyStatus::CHARGE_STATION_CONNECTED; }
};

struct ChargeStateData {
    ChargeMode chargeMode = ChargeMode::INIT;
    ChargeStatus chargeStatus = ChargeStatus::INIT;
    uint8_t socPercent = 0;           // 0-100%
    uint8_t remainingTimeMin = 0;     // Minutes to full
    uint8_t currentRange = 0;         // Current range value
    uint8_t rangeUnit = 0;            // Range unit
    uint8_t chargingAmps = 0;         // Charging current
    uint8_t batteryClimateState = 0;  // Battery conditioning
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

/**
 * Decode PlugState payload (function 0x10)
 */
PlugStateData decodePlugState(const uint8_t* payload, uint8_t len);

/**
 * Decode ChargeState payload (function 0x11)
 */
ChargeStateData decodeChargeState(const uint8_t* payload, uint8_t len);

/**
 * Decode ClimateState payload (function 0x12)
 */
ClimateStateData decodeClimateState(const uint8_t* payload, uint8_t len);

// =============================================================================
// Command Builders
// =============================================================================

/**
 * Build a Get request for a function
 */
inline uint8_t buildGetRequest(uint8_t* dest, uint8_t functionId) {
    return encodeShortMessage(dest, OpCode::GET, DEVICE_BATTERY_CONTROL, functionId);
}

/**
 * Build climate start command
 * @param temp Temperature in Celsius (15.5 - 30.0)
 */
uint8_t buildClimateStart(uint8_t* dest, float tempCelsius);

/**
 * Build climate stop command
 */
uint8_t buildClimateStop(uint8_t* dest);

/**
 * Build charge start command
 */
uint8_t buildChargeStart(uint8_t* dest);

/**
 * Build charge stop command
 */
uint8_t buildChargeStop(uint8_t* dest);

// =============================================================================
// BAP Frame Assembler - Abstracts short/long message handling
// =============================================================================

/**
 * Complete BAP message (after reassembly if needed)
 */
struct BapMessage {
    uint8_t opcode = 0;
    uint8_t deviceId = 0;
    uint8_t functionId = 0;
    uint8_t payloadLen = 0;   // Moved BEFORE payload to prevent overflow corruption
    uint8_t payload[128];     // Reassembled payload - must match MAX_PAYLOAD_SIZE
    
    bool isValid() const { return payloadLen > 0 || (deviceId != 0 && functionId != 0); }
    bool isResponse() const { return opcode >= OpCode::HEARTBEAT; }
    bool isError() const { return opcode == OpCode::ERROR; }
};

/**
 * BAP Frame Assembler
 * 
 * Handles the complexity of short vs long messages, providing a clean
 * interface that delivers complete messages regardless of framing.
 * 
 * Usage:
 *   BapFrameAssembler assembler;
 *   
 *   // For each CAN frame received:
 *   BapMessage msg;
 *   if (assembler.processFrame(data, dlc, msg)) {
 *       // Complete message ready - handle it
 *       handleMessage(msg.functionId, msg.opcode, msg.payload, msg.payloadLen);
 *   }
 *   // If returns false, either invalid frame or waiting for more data (long msg)
 * 
 * Implementation note:
 *   Uses backward search approach (like VW BAP Analyser) to handle interleaved
 *   long messages. Multiple incomplete messages can coexist, and continuations
 *   search backwards to find their matching start frame.
 */
class BapFrameAssembler {
public:
    static constexpr size_t MAX_PAYLOAD_SIZE = 128;    // Max payload buffer size
    static constexpr size_t MAX_PENDING_MESSAGES = 16; // Max concurrent incomplete long messages
    
    /**
     * Process a CAN frame and attempt to produce a complete BAP message.
     * 
     * @param data Raw CAN frame data
     * @param dlc Data length code
     * @param outMsg Output: complete message if returns true
     * @return true if a complete message is ready, false if waiting for more frames
     * 
     * NOTE: This runs on CAN task (Core 0) - NO Serial output allowed!
     * Use the counter variables below for debugging from main loop.
     */
    bool processFrame(const uint8_t* data, uint8_t dlc, BapMessage& outMsg);
    
    /**
     * Reset assembler state (e.g., after timeout or error)
     */
    void reset();
    
    /**
     * Check if we're in the middle of assembling any long message
     */
    bool isAssemblingLongMessage() const;
    
    /**
     * Get number of pending incomplete messages
     */
    uint8_t getPendingCount() const { return pendingCount; }
    
    // Statistics counters (read from main loop for logging)
    volatile uint32_t shortMessagesDecoded = 0;
    volatile uint32_t longMessagesDecoded = 0;
    volatile uint32_t continuationErrors = 0;   // Continuation without matching start
    volatile uint32_t longStartFrames = 0;      // Long message start frames seen
    volatile uint32_t longContFrames = 0;       // Continuation frames seen
    volatile uint32_t pendingOverflows = 0;     // Start frames dropped due to full pending list
    volatile uint32_t staleReplacements = 0;    // Stale pending entries replaced by new starts
    volatile uint8_t maxPendingCount = 0;       // High water mark for pending count
    
    // Debug: capture state of first incomplete message for analysis
    volatile uint8_t dbgFirstIncompleteIdx = 0xFF;      // Index of first incomplete (0xFF = none)
    volatile uint8_t dbgFirstIncompleteExpected = 0;    // Expected length
    volatile uint8_t dbgFirstIncompleteAssembled = 0;   // Assembled length
    
private:
    // Pending incomplete long message
    struct PendingMessage {
        bool active = false;
        uint8_t group = 0;              // Message group (0-3), from bits 5-4 of control byte
        uint8_t nextExpectedIndex = 0;  // Next continuation index expected (0-15)
        uint8_t opcode = 0;
        uint8_t deviceId = 0;
        uint8_t functionId = 0;
        uint8_t expectedLength = 0;     // Total payload length expected
        uint8_t assembledLength = 0;    // Bytes assembled so far
        uint8_t buffer[MAX_PAYLOAD_SIZE];
    };
    
    PendingMessage pending[MAX_PENDING_MESSAGES];
    uint8_t pendingCount = 0;
    
    /**
     * Find a pending message by group and expected index.
     * Returns index or -1 if not found.
     */
    int8_t findPendingMessage(uint8_t group, uint8_t index);
    
    /**
     * Add a new pending message. Returns index or -1 if full.
     */
    int8_t addPendingMessage(uint8_t group, uint8_t opcode, uint8_t deviceId,
                             uint8_t functionId, uint8_t expectedLength);
    
    /**
     * Remove a pending message by index (shifts remaining entries)
     */
    void removePendingMessage(uint8_t index);
};

} // namespace BapProtocol
