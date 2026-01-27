#include "BapDomain.h"
#include "../VehicleManager.h"

using namespace BapProtocol;

BapDomain::BapDomain(VehicleState& state, VehicleManager* mgr)
    : state(state)
    , manager(mgr)
{
}

// =============================================================================
// Frame handling
// =============================================================================

bool BapDomain::handlesCanId(uint32_t canId) {
    // We only passively listen to the RX channel (responses from Battery Control)
    return canId == CAN_ID_BATTERY_RX;
}

void BapDomain::getHandledIds(uint32_t* ids, uint8_t& count, uint8_t maxCount) {
    count = 0;
    if (maxCount >= 1) {
        ids[count++] = CAN_ID_BATTERY_RX;
    }
}

bool BapDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (canId != CAN_ID_BATTERY_RX) {
        return false;
    }
    
    if (dlc < 2) {
        return false;
    }
    
    // Decode BAP header
    BapHeader header = decodeHeader(data, dlc);
    
    if (header.isLong && header.isContinuation) {
        // Continuation frame for long message
        if (!longMsgInProgress) {
            // Unexpected continuation - ignore
            return false;
        }
        
        // Append payload to buffer
        const uint8_t* payload = getContinuationPayload(data);
        uint8_t payloadLen = dlc - 1;
        
        if (longMsgLength + payloadLen <= MAX_LONG_MSG_SIZE) {
            memcpy(longMsgBuffer + longMsgLength, payload, payloadLen);
            longMsgLength += payloadLen;
        }
        
        // Check if complete
        if (longMsgLength >= longMsgExpectedLength) {
            // Message complete - process it
            handleBapMessage(longMsgFunctionId, header.opcode, 
                           longMsgBuffer, longMsgExpectedLength);
            longMsgInProgress = false;
        }
        
        return true;
    }
    else if (header.isLong && !header.isContinuation) {
        // Long message start frame
        longMsgInProgress = true;
        longMsgMessageIndex = header.messageIndex;
        longMsgFunctionId = header.functionId;
        longMsgExpectedLength = header.totalLength - 2;  // Subtract 2-byte header
        longMsgLength = 0;
        
        // Copy first 4 bytes of payload
        const uint8_t* payload = getLongStartPayload(data);
        uint8_t payloadLen = dlc - 4;
        
        if (payloadLen > 0 && payloadLen <= MAX_LONG_MSG_SIZE) {
            memcpy(longMsgBuffer, payload, payloadLen);
            longMsgLength = payloadLen;
        }
        
        // Check if this short long message is already complete
        if (longMsgLength >= longMsgExpectedLength) {
            handleBapMessage(longMsgFunctionId, header.opcode,
                           longMsgBuffer, longMsgExpectedLength);
            longMsgInProgress = false;
        }
        
        return true;
    }
    else {
        // Short message - process immediately
        const uint8_t* payload = getShortPayload(data);
        uint8_t payloadLen = getShortPayloadLength(dlc);
        
        handleBapMessage(header.functionId, header.opcode, payload, payloadLen);
        return true;
    }
}

// =============================================================================
// Message handling
// =============================================================================

void BapDomain::handleBapMessage(uint8_t functionId, uint8_t opcode, 
                                  const uint8_t* payload, uint8_t payloadLen) {
    // Only process response opcodes (we don't care about requests from other modules)
    // OpCodes 0x03-0x07 are responses/indications
    if (opcode < OpCode::HEARTBEAT) {
        return;  // This is a request, not a response
    }
    
    // Route to appropriate handler based on function ID
    switch (functionId) {
        case Function::PLUG_STATE:
            processPlugState(payload, payloadLen);
            plugFrames++;
            break;
            
        case Function::CHARGE_STATE:
            processChargeState(payload, payloadLen);
            chargeFrames++;
            break;
            
        case Function::CLIMATE_STATE:
            processClimateState(payload, payloadLen);
            climateFrames++;
            break;
            
        default:
            otherFrames++;
            break;
    }
}

void BapDomain::processPlugState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        return;
    }
    
    PlugStateData decoded = decodePlugState(payload, len);
    
    BapPlugState& plug = state.bapPlug;
    plug.lockSetup = decoded.lockSetup;
    plug.lockState = decoded.lockState;
    plug.supplyState = static_cast<uint8_t>(decoded.supplyState);
    plug.plugState = static_cast<uint8_t>(decoded.plugState);
    plug.lastUpdate = millis();
    
    Serial.printf("[BapDomain] PlugState: %s, supply=%s\r\n",
        plug.plugStateStr(), plug.hasSupply() ? "yes" : "no");
}

void BapDomain::processChargeState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        return;
    }
    
    ChargeStateData decoded = decodeChargeState(payload, len);
    
    BapChargeState& charge = state.bapCharge;
    charge.chargeMode = static_cast<BapChargeMode>(static_cast<uint8_t>(decoded.chargeMode));
    charge.chargeStatus = static_cast<BapChargeStatus>(static_cast<uint8_t>(decoded.chargeStatus));
    charge.socPercent = decoded.socPercent;
    charge.remainingTimeMin = decoded.remainingTimeMin;
    charge.currentRange = decoded.currentRange;
    charge.chargingAmps = decoded.chargingAmps;
    charge.targetSoc = decoded.targetSoc;
    charge.lastUpdate = millis();
    
    Serial.printf("[BapDomain] ChargeState: SOC=%d%%, mode=%s, status=%s\r\n",
        charge.socPercent, charge.chargeModeStr(), charge.chargeStatusStr());
}

void BapDomain::processClimateState(const uint8_t* payload, uint8_t len) {
    if (len < 1) {
        return;
    }
    
    ClimateStateData decoded = decodeClimateState(payload, len);
    
    BapClimateState& climate = state.bapClimate;
    climate.climateActive = decoded.climateActive;
    climate.autoDefrost = decoded.autoDefrost;
    climate.heating = decoded.heating;
    climate.cooling = decoded.cooling;
    climate.ventilation = decoded.ventilation;
    climate.currentTempC = decoded.currentTempC;
    climate.climateTimeMin = decoded.climateTimeMin;
    climate.climateState = decoded.climateState;
    climate.lastUpdate = millis();
    
    Serial.printf("[BapDomain] ClimateState: active=%s, temp=%.1f°C, heat=%d cool=%d\r\n",
        climate.climateActive ? "YES" : "no", climate.currentTempC,
        climate.heating, climate.cooling);
}

// =============================================================================
// Command methods
// =============================================================================

bool BapDomain::sendBapFrame(const uint8_t* data, uint8_t len) {
    if (!manager) {
        Serial.println("[BapDomain] No manager - cannot send");
        return false;
    }
    
    // Send as extended frame to TX channel
    return manager->sendCanFrame(CAN_ID_BATTERY_TX, data, len, true);
}

bool BapDomain::requestPlugState() {
    uint8_t frame[8];
    buildGetRequest(frame, Function::PLUG_STATE);
    
    Serial.println("[BapDomain] Requesting PlugState...");
    return sendBapFrame(frame, 8);
}

bool BapDomain::requestChargeState() {
    uint8_t frame[8];
    buildGetRequest(frame, Function::CHARGE_STATE);
    
    Serial.println("[BapDomain] Requesting ChargeState...");
    return sendBapFrame(frame, 8);
}

bool BapDomain::requestClimateState() {
    uint8_t frame[8];
    buildGetRequest(frame, Function::CLIMATE_STATE);
    
    Serial.println("[BapDomain] Requesting ClimateState...");
    return sendBapFrame(frame, 8);
}

bool BapDomain::startClimate(float tempCelsius) {
    uint8_t frame[8];
    buildClimateStart(frame, tempCelsius);
    
    Serial.printf("[BapDomain] Starting climate at %.1f°C...\r\n", tempCelsius);
    return sendBapFrame(frame, 8);
}

bool BapDomain::stopClimate() {
    uint8_t frame[8];
    buildClimateStop(frame);
    
    Serial.println("[BapDomain] Stopping climate...");
    return sendBapFrame(frame, 8);
}

bool BapDomain::startCharging() {
    uint8_t frame[8];
    buildChargeStart(frame);
    
    Serial.println("[BapDomain] Starting charging...");
    return sendBapFrame(frame, 8);
}

bool BapDomain::stopCharging() {
    uint8_t frame[8];
    buildChargeStop(frame);
    
    Serial.println("[BapDomain] Stopping charging...");
    return sendBapFrame(frame, 8);
}
