#include "BapProtocol.h"

namespace BapProtocol {

// =============================================================================
// Decoding Functions
// =============================================================================

BapHeader decodeHeader(const uint8_t* data, uint8_t dlc) {
    BapHeader header;
    
    if (dlc < 2) {
        return header;  // Invalid - too short
    }
    
    uint8_t firstByte = data[0];
    header.isLong = (firstByte & 0x80) != 0;
    header.isContinuation = (firstByte & 0x40) != 0;
    
    if (!header.isLong) {
        // Short message format
        // Byte 0: [0][OpCode:3][DeviceID_hi:4]
        // Byte 1: [DeviceID_lo:2][FunctionID:6]
        header.opcode = (firstByte >> 4) & 0x07;
        header.deviceId = ((firstByte & 0x0F) << 2) | ((data[1] >> 6) & 0x03);
        header.functionId = data[1] & 0x3F;
        header.totalLength = (dlc > 2) ? (dlc - 2) : 0;
    }
    else if (!header.isContinuation) {
        // Long start frame
        // Byte 0: [1][0][Group:2][Index:4]  -- Index always 0 for start
        // Byte 1: total_length
        // Byte 2: [0][OpCode:3][DeviceID_hi:4]
        // Byte 3: [DeviceID_lo:2][FunctionID:6]
        if (dlc < 4) {
            return header;  // Invalid
        }
        header.group = (firstByte >> 4) & 0x03;   // Bits 5-4
        header.index = firstByte & 0x0F;          // Bits 3-0 (always 0 for start)
        header.totalLength = data[1];
        header.opcode = (data[2] >> 4) & 0x07;
        header.deviceId = ((data[2] & 0x0F) << 2) | ((data[3] >> 6) & 0x03);
        header.functionId = data[3] & 0x3F;
    }
    else {
        // Long continuation frame
        // Byte 0: [1][1][Group:2][Index:4]
        // Bytes 1-7: payload continuation
        header.group = (firstByte >> 4) & 0x03;   // Bits 5-4
        header.index = firstByte & 0x0F;          // Bits 3-0
        // opcode, deviceId, functionId stay 0 for continuation frames
    }
    
    return header;
}

// =============================================================================
// Encoding Functions
// =============================================================================

uint8_t encodeHeader(uint8_t* dest, uint8_t opcode, uint8_t deviceId, uint8_t functionId) {
    // Byte 0: (opcode << 4) | (device_id >> 2)
    // Byte 1: (device_id << 6) | function_id
    dest[0] = ((opcode & 0x07) << 4) | ((deviceId >> 2) & 0x0F);
    dest[1] = ((deviceId & 0x03) << 6) | (functionId & 0x3F);
    return 2;
}

uint8_t encodeShortMessage(uint8_t* dest, uint8_t opcode, uint8_t deviceId, 
                           uint8_t functionId, const uint8_t* payload, 
                           uint8_t payloadLen) {
    // Encode header
    encodeHeader(dest, opcode, deviceId, functionId);
    
    // Copy payload (max 6 bytes)
    uint8_t copyLen = (payloadLen > 6) ? 6 : payloadLen;
    if (payload && copyLen > 0) {
        memcpy(dest + 2, payload, copyLen);
    }
    
    // Pad to 8 bytes with zeros
    for (uint8_t i = 2 + copyLen; i < 8; i++) {
        dest[i] = 0x00;
    }
    
    return 8;
}

uint8_t encodeLongStart(uint8_t* dest, uint8_t opcode, uint8_t deviceId,
                        uint8_t functionId, uint8_t totalPayloadLen,
                        const uint8_t* payload, uint8_t group) {
    // Byte 0: Long=1, Cont=0, Group in bits 5-4, Index=0 for start frame
    // Format: [1][0][GG][0000] = 0x80 | (group << 4)
    dest[0] = 0x80 | ((group & 0x03) << 4);
    
    // Byte 1: total length (includes 2-byte header)
    dest[1] = totalPayloadLen + 2;
    
    // Bytes 2-3: BAP header
    encodeHeader(dest + 2, opcode, deviceId, functionId);
    
    // Bytes 4-7: first 4 bytes of payload
    uint8_t copyLen = (totalPayloadLen > 4) ? 4 : totalPayloadLen;
    if (payload && copyLen > 0) {
        memcpy(dest + 4, payload, copyLen);
    }
    
    // Pad remaining bytes
    for (uint8_t i = 4 + copyLen; i < 8; i++) {
        dest[i] = 0x00;
    }
    
    return 8;
}

uint8_t encodeLongContinuation(uint8_t* dest, const uint8_t* payload, 
                                uint8_t payloadLen, uint8_t group, uint8_t index) {
    // Byte 0: Long=1, Cont=1, Group in bits 5-4, Index in bits 3-0
    // Format: [1][1][GG][IIII] = 0xC0 | (group << 4) | index
    dest[0] = 0xC0 | ((group & 0x03) << 4) | (index & 0x0F);
    
    // Bytes 1-7: next 7 bytes of payload
    uint8_t copyLen = (payloadLen > 7) ? 7 : payloadLen;
    if (payload && copyLen > 0) {
        memcpy(dest + 1, payload, copyLen);
    }
    
    // Pad remaining bytes
    for (uint8_t i = 1 + copyLen; i < 8; i++) {
        dest[i] = 0x00;
    }
    
    return 8;
}

// =============================================================================
// Payload Decoders
// =============================================================================

PlugStateData decodePlugState(const uint8_t* payload, uint8_t len) {
    PlugStateData data;
    
    if (len < 2) {
        return data;
    }
    
    // Byte 0: (lock_setup << 4) | lock_state
    data.lockSetup = (payload[0] >> 4) & 0x0F;
    data.lockState = payload[0] & 0x0F;
    
    // Byte 1: (supply_state << 4) | plug_state
    data.supplyState = static_cast<SupplyStatus>((payload[1] >> 4) & 0x0F);
    data.plugState = static_cast<PlugStatus>(payload[1] & 0x0F);
    
    return data;
}

ChargeStateData decodeChargeState(const uint8_t* payload, uint8_t len) {
    ChargeStateData data;
    
    if (len < 2) {
        return data;
    }
    
    // Byte 0: (charge_mode << 4) | charge_state
    data.chargeMode = static_cast<ChargeMode>((payload[0] >> 4) & 0x0F);
    data.chargeStatus = static_cast<ChargeStatus>(payload[0] & 0x0F);
    
    // Byte 1: SOC percent
    data.socPercent = payload[1];
    
    // Optional fields
    if (len >= 3) {
        data.remainingTimeMin = payload[2];
    }
    if (len >= 4) {
        data.currentRange = payload[3];
    }
    if (len >= 5) {
        data.rangeUnit = payload[4];
    }
    if (len >= 6) {
        data.chargingAmps = payload[5];
    }
    if (len >= 7) {
        data.batteryClimateState = (payload[6] >> 4) & 0x0F;
    }
    if (len >= 9) {
        data.startReason = (payload[8] >> 4) & 0x0F;
        data.targetSoc = payload[8] & 0x0F;
    }
    
    return data;
}

ClimateStateData decodeClimateState(const uint8_t* payload, uint8_t len) {
    ClimateStateData data;
    
    if (len < 1) {
        return data;
    }
    
    // Byte 0: mode flags
    uint8_t mode = payload[0];
    data.climateActive = (mode & 0x01) != 0;
    data.autoDefrost = (mode & 0x02) != 0;
    data.heating = (mode & 0x04) != 0;
    data.cooling = (mode & 0x08) != 0;
    data.ventilation = (mode & 0x10) != 0;
    data.fuelBasedHeating = (mode & 0x20) != 0;
    
    // Byte 1: current temperature (raw)
    // Formula: (raw + 100) / 10
    if (len >= 2) {
        data.currentTempC = (payload[1] + 100) / 10.0f;
    }
    
    // Byte 2: temperature unit
    if (len >= 3) {
        data.tempUnit = payload[2];
    }
    
    // Bytes 3-4: climate time remaining (little endian)
    if (len >= 5) {
        data.climateTimeMin = payload[3] | (payload[4] << 8);
    }
    
    // Byte 5: climate state
    if (len >= 6) {
        data.climateState = (payload[5] >> 4) & 0x0F;
    }
    
    return data;
}

// =============================================================================
// Command Builders
// =============================================================================

/**
 * Profile 0 Operation Modes (for PROFILES_ARRAY function 0x19)
 * These define what Profile 0 should do when activated.
 * 
 * From reverse engineering (discord thread):
 * - 0x01 = Charging only
 * - 0x02 = Climate only
 * - 0x03 = Charging and climate
 * - 0x05 = Charging and allow climate on battery
 * - 0x06 = Climate and allow on battery
 */
namespace ProfileOperation {
    constexpr uint8_t CHARGING = 0x01;
    constexpr uint8_t CLIMATE = 0x02;
    constexpr uint8_t CHARGING_AND_CLIMATE = 0x03;
    constexpr uint8_t CHARGING_ALLOW_CLIMATE_BATTERY = 0x05;
    constexpr uint8_t CLIMATE_ALLOW_BATTERY = 0x06;
}

uint8_t buildOperationModeStart(uint8_t* dest) {
    // Execute Profile 0 (the "immediately" profile)
    // Short BAP message to OPERATION_MODE (0x18)
    // Payload: [0x00, 0x01] = immediately active
    // 
    // From trace: 29,58,00,01
    // Decoded: opcode=2 (SET_GET), device=0x25, function=0x18, payload=[0x00, 0x01]
    uint8_t payload[2] = {
        0x00,  // Flags
        0x01   // immediately = true
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_BATTERY_CONTROL, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t buildOperationModeStop(uint8_t* dest) {
    // Stop Profile 0 execution
    // Short BAP message to OPERATION_MODE (0x18)
    // Payload: [0x00, 0x00] = immediately off
    //
    // From trace: 29,58,00,00
    uint8_t payload[2] = {
        0x00,  // Flags
        0x00   // immediately = false (stop)
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_BATTERY_CONTROL, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t buildProfileConfig(uint8_t* startFrame, uint8_t* contFrame, uint8_t operationMode) {
    // Configure Profile 0 with the desired operation mode
    // This is a LONG BAP message to PROFILES_ARRAY (0x19) using mode 6 update
    //
    // From trace analysis:
    // Start:  80,08,29,59,22,06,00,01
    //   - 80 = Long message start, group 0
    //   - 08 = Total payload length (6 bytes after BAP header)
    //   - 29,59 = BAP header: opcode=2 (SET_GET), device=0x25, function=0x19
    //   - 22,06,00,01 = First 4 bytes of payload
    //
    // Cont:   C0,06,00,20,00,00,00,00
    //   - C0 = Long continuation, group 0, index 0
    //   - 06,00,20,00,00,00,00 = Remaining payload bytes
    //   - Byte at position 0 (0x06) is the operation mode!
    //
    // The payload structure (mode 6 partial update of profile array):
    //   [0x22, profile_index, 0x00, 0x01, operation_mode, 0x00, 0x20, 0x00, ...]
    //   - 0x22 = Update mode (mode 6 = partial property update)
    //   - profile_index = 0x06 for profile 0 (encoded as 0x06)
    //   - operation_mode = what the profile should do
    
    // Payload for profile 0 configuration (total 6 bytes)
    uint8_t payload[6] = {
        0x22,           // Update mode marker
        0x06,           // Profile index (0 = immediately profile, encoded)
        0x00,           // Padding
        0x01,           // Unknown (always 0x01 in traces)
        operationMode,  // The operation mode (climate, charging, etc.)
        0x00            // Padding
    };
    
    // Encode long message start frame
    // Note: totalPayloadLen = 6 (just the payload, not including BAP header)
    encodeLongStart(startFrame, OpCode::SET_GET, DEVICE_BATTERY_CONTROL,
                    Function::PROFILES_ARRAY, 6, payload, 0);
    
    // Encode continuation frame with remaining payload bytes
    // After start frame takes first 4 bytes of payload, we have 2 more + padding
    uint8_t contPayload[7] = {
        operationMode,  // This is where the operation mode actually appears
        0x00,           // Padding
        0x20,           // Unknown flag (from traces)
        0x00, 0x00, 0x00, 0x00  // Padding
    };
    encodeLongContinuation(contFrame, contPayload, 7, 0, 0);
    
    return 8;  // Each frame is 8 bytes
}

uint8_t buildClimateStart(uint8_t* dest, float tempCelsius) {
    // For climate start, we just need to trigger the operation mode
    // The profile should already be configured, or we configure it for climate
    // 
    // NOTE: For full implementation, you would:
    // 1. Send buildProfileConfig() with CLIMATE_ALLOW_BATTERY mode
    // 2. Wait for ACK
    // 3. Send buildOperationModeStart()
    //
    // For now, we just send the operation mode start command.
    // The VW OCU typically pre-configures profile 0 before triggering.
    //
    // Temperature is stored in the profile configuration, not in the start command.
    // To set temperature, you need to update the profile's temperature field.
    (void)tempCelsius;  // Temperature requires separate profile temperature update
    
    return buildOperationModeStart(dest);
}

uint8_t buildClimateStop(uint8_t* dest) {
    // Stop by sending operation mode stop
    return buildOperationModeStop(dest);
}

uint8_t buildChargeStart(uint8_t* dest) {
    // Same as climate - trigger operation mode
    // Profile should be configured for charging mode
    return buildOperationModeStart(dest);
}

uint8_t buildChargeStop(uint8_t* dest) {
    // Stop by sending operation mode stop
    return buildOperationModeStop(dest);
}

// =============================================================================
// BAP Frame Assembler Implementation (Backward Search Approach)
// =============================================================================

bool BapFrameAssembler::processFrame(const uint8_t* data, uint8_t dlc, BapMessage& outMsg) {
    if (dlc < 2) {
        return false;
    }
    
    // Decode the frame header
    BapHeader header = decodeHeader(data, dlc);
    
    if (!header.isLong) {
        // Short message - complete in single frame
        outMsg.opcode = header.opcode;
        outMsg.deviceId = header.deviceId;
        outMsg.functionId = header.functionId;
        outMsg.payloadLen = getShortPayloadLength(dlc);
        
        if (outMsg.payloadLen > 0) {
            memcpy(outMsg.payload, getShortPayload(data), outMsg.payloadLen);
        }
        
        shortMessagesDecoded++;
        return true;
    }
    
    // Long message handling
    if (!header.isContinuation) {
        // Long message START frame
        longStartFrames++;
        
        // totalLength (byte 1) is the payload length directly (does NOT include BAP header)
        uint8_t payloadLength = header.totalLength;
        
        // Add new pending message
        int8_t idx = addPendingMessage(header.group, header.opcode, 
                                        header.deviceId, header.functionId, payloadLength);
        if (idx < 0) {
            pendingOverflows++;
            return false;  // No room for more pending messages
        }
        
        PendingMessage& pm = pending[idx];
        
        // Copy first chunk of payload (bytes 4-7, up to 4 bytes)
        uint8_t firstChunkLen = (dlc > 4) ? (dlc - 4) : 0;
        if (firstChunkLen > 0 && pm.assembledLength + firstChunkLen <= MAX_PAYLOAD_SIZE) {
            memcpy(pm.buffer + pm.assembledLength, data + 4, firstChunkLen);
            pm.assembledLength += firstChunkLen;
        }
        
        // Check if message is complete (small long message that fits in start frame)
        if (pm.assembledLength >= pm.expectedLength) {
            outMsg.opcode = pm.opcode;
            outMsg.deviceId = pm.deviceId;
            outMsg.functionId = pm.functionId;
            outMsg.payloadLen = pm.expectedLength;
            memcpy(outMsg.payload, pm.buffer, pm.expectedLength);
            
            removePendingMessage(idx);
            longMessagesDecoded++;
            return true;
        }
        
        return false;  // Waiting for continuation frames
    }
    else {
        // Long message CONTINUATION frame
        longContFrames++;
        
        // Search for pending message matching group and expecting this index
        int8_t idx = findPendingMessage(header.group, header.index);
        if (idx < 0) {
            continuationErrors++;
            return false;  // No matching start frame found
        }
        
        PendingMessage& pm = pending[idx];
        
        // Append payload (bytes 1-7, up to 7 bytes)
        uint8_t chunkLen = (dlc > 1) ? (dlc - 1) : 0;
        
        // Limit to only copy what's needed to reach expectedLength
        uint8_t remaining = pm.expectedLength - pm.assembledLength;
        if (chunkLen > remaining) {
            chunkLen = remaining;
        }
        
        if (chunkLen > 0 && pm.assembledLength + chunkLen <= MAX_PAYLOAD_SIZE) {
            memcpy(pm.buffer + pm.assembledLength, data + 1, chunkLen);
            pm.assembledLength += chunkLen;
        }
        
        // Increment expected index for next continuation (wraps at 16)
        pm.nextExpectedIndex = (pm.nextExpectedIndex + 1) & 0x0F;
        
        // Check if message is complete
        if (pm.assembledLength >= pm.expectedLength) {
            outMsg.opcode = pm.opcode;
            outMsg.deviceId = pm.deviceId;
            outMsg.functionId = pm.functionId;
            outMsg.payloadLen = pm.expectedLength;
            memcpy(outMsg.payload, pm.buffer, pm.expectedLength);
            
            removePendingMessage(idx);
            longMessagesDecoded++;
            return true;
        }
        
        return false;  // Waiting for more continuation frames
    }
}

int8_t BapFrameAssembler::findPendingMessage(uint8_t group, uint8_t index) {
    // Search backwards through ALL slots to find one matching group
    // and expecting this specific continuation index
    for (int8_t i = MAX_PENDING_MESSAGES - 1; i >= 0; i--) {
        if (pending[i].active && 
            pending[i].group == group &&
            pending[i].nextExpectedIndex == index &&
            pending[i].assembledLength < pending[i].expectedLength) {
            return i;
        }
    }
    return -1;  // Not found
}

int8_t BapFrameAssembler::addPendingMessage(uint8_t group, uint8_t opcode,
                                             uint8_t deviceId, uint8_t functionId,
                                             uint8_t expectedLength) {
    // Find first inactive slot (reuse freed slots)
    int8_t idx = -1;
    for (uint8_t i = 0; i < MAX_PENDING_MESSAGES; i++) {
        if (!pending[i].active) {
            idx = i;
            break;
        }
    }
    
    if (idx < 0) {
        return -1;  // All slots full
    }
    
    pending[idx].active = true;
    pending[idx].group = group;
    pending[idx].nextExpectedIndex = 0;  // First continuation has index 0
    pending[idx].opcode = opcode;
    pending[idx].deviceId = deviceId;
    pending[idx].functionId = functionId;
    pending[idx].expectedLength = expectedLength;
    pending[idx].assembledLength = 0;
    
    // Count active entries
    pendingCount = 0;
    for (uint8_t i = 0; i < MAX_PENDING_MESSAGES; i++) {
        if (pending[i].active) {
            pendingCount++;
        }
    }
    
    // Track high water mark
    if (pendingCount > maxPendingCount) {
        maxPendingCount = pendingCount;
    }
    
    return idx;
}

void BapFrameAssembler::removePendingMessage(uint8_t index) {
    if (index >= MAX_PENDING_MESSAGES) {
        return;
    }
    
    // Just mark as inactive - slot can be reused by addPendingMessage
    pending[index].active = false;
    pending[index].assembledLength = 0;
    
    // Count actual active entries
    pendingCount = 0;
    for (uint8_t i = 0; i < MAX_PENDING_MESSAGES; i++) {
        if (pending[i].active) {
            pendingCount++;
        }
    }
}

void BapFrameAssembler::reset() {
    for (uint8_t i = 0; i < MAX_PENDING_MESSAGES; i++) {
        pending[i].active = false;
        pending[i].assembledLength = 0;
    }
    pendingCount = 0;
}

bool BapFrameAssembler::isAssemblingLongMessage() const {
    return pendingCount > 0;
}

} // namespace BapProtocol
