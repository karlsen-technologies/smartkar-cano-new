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
// CAN IDs (Generic BAP protocol)
// =============================================================================

// Wake/Init (for reference, not primary focus)
constexpr uint32_t CAN_ID_WAKE = 0x17330301;
constexpr uint32_t CAN_ID_BAP_INIT = 0x1B000067;

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
// Generic Command Builders
// =============================================================================

/**
 * Build a GET request for any device/function
 * @param deviceId Target device ID (e.g., 0x25 for Battery Control)
 * @param functionId Function to query
 */
inline uint8_t buildGetRequest(uint8_t* dest, uint8_t deviceId, uint8_t functionId) {
    return encodeShortMessage(dest, OpCode::GET, deviceId, functionId);
}

/**
 * Build a SET_GET request for any device/function
 * @param deviceId Target device ID
 * @param functionId Function to invoke
 * @param payload Optional payload data
 * @param payloadLen Length of payload
 */
inline uint8_t buildSetGetRequest(uint8_t* dest, uint8_t deviceId, uint8_t functionId,
                                   const uint8_t* payload = nullptr, uint8_t payloadLen = 0) {
    return encodeShortMessage(dest, OpCode::SET_GET, deviceId, functionId, payload, payloadLen);
}

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
