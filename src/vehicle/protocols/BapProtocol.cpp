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
        // Byte 0: (opcode << 4) | (device_id >> 2)
        // Byte 1: (device_id << 6) | function_id
        header.opcode = (firstByte >> 4) & 0x07;
        header.deviceId = ((firstByte & 0x0F) << 2) | ((data[1] >> 6) & 0x03);
        header.functionId = data[1] & 0x3F;
        header.totalLength = (dlc > 2) ? (dlc - 2) : 0;
    }
    else if (!header.isContinuation) {
        // Long start frame
        // Byte 0: 0x80 | (message_index << 2)
        // Byte 1: total_length
        // Byte 2: (opcode << 4) | (device_id >> 2)
        // Byte 3: (device_id << 6) | function_id
        if (dlc < 4) {
            return header;  // Invalid
        }
        header.messageIndex = (firstByte >> 2) & 0x0F;
        header.totalLength = data[1];
        header.opcode = (data[2] >> 4) & 0x07;
        header.deviceId = ((data[2] & 0x0F) << 2) | ((data[3] >> 6) & 0x03);
        header.functionId = data[3] & 0x3F;
    }
    else {
        // Long continuation frame
        // Byte 0: 0xC0 | (message_index << 2)
        // Bytes 1-7: payload continuation
        header.messageIndex = (firstByte >> 2) & 0x0F;
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
                        const uint8_t* payload, uint8_t messageIndex) {
    // Byte 0: 0x80 | (message_index << 2)
    dest[0] = 0x80 | ((messageIndex & 0x0F) << 2);
    
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
                                uint8_t payloadLen, uint8_t messageIndex) {
    // Byte 0: 0xC0 | (message_index << 2)
    dest[0] = 0xC0 | ((messageIndex & 0x0F) << 2);
    
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

uint8_t buildClimateStart(uint8_t* dest, float tempCelsius) {
    // Climate start uses profiles - need to configure profile first
    // For immediate start, we use OperationMode function (0x18)
    
    // Temperature encoding: (temp_celsius - 10) * 10
    // Valid range: 15.5 - 30.0°C
    if (tempCelsius < 15.5f) tempCelsius = 15.5f;
    if (tempCelsius > 30.0f) tempCelsius = 30.0f;
    
    // Build SetGet for OperationMode with "immediately" flag
    // Payload: [flags, mode_byte]
    // mode_byte bit 0 = immediately active
    uint8_t payload[2] = {
        0x00,  // Flags
        0x01   // Mode: immediately = true
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_BATTERY_CONTROL, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t buildClimateStop(uint8_t* dest) {
    // Stop climate by clearing the "immediately" flag
    uint8_t payload[2] = {
        0x00,  // Flags
        0x00   // Mode: all timers off
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_BATTERY_CONTROL, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t buildChargeStart(uint8_t* dest) {
    // Charging is typically controlled via profiles
    // For immediate start, configure profile with CHARGE flag
    uint8_t payload[2] = {
        0x00,  // Flags
        0x01   // Mode: immediately = true (charging enabled in profile)
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_BATTERY_CONTROL, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t buildChargeStop(uint8_t* dest) {
    // Stop charging by clearing operation mode
    uint8_t payload[2] = {
        0x00,  // Flags  
        0x00   // Mode: all off
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_BATTERY_CONTROL, 
                              Function::OPERATION_MODE, payload, 2);
}

} // namespace BapProtocol
