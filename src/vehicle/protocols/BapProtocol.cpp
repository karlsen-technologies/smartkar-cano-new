#include "BapProtocol.h"
#include "../VehicleManager.h"

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
