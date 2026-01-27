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
    
    // Feed frame to assembler - it handles short/long message abstraction
    BapMessage msg;
    if (frameAssembler.processFrame(data, dlc, msg)) {
        // Complete message ready - handle it
        handleBapMessage(msg);
        return true;
    }
    
    // Not a complete message yet (waiting for more frames for long msg)
    // Still return true since we processed a valid BAP frame
    return frameAssembler.isAssemblingLongMessage();
}

// =============================================================================
// Message handling
// =============================================================================

void BapDomain::handleBapMessage(const BapMessage& msg) {
    // Only process response opcodes (we don't care about requests from other modules)
    // OpCodes 0x03-0x07 are responses/indications
    if (msg.opcode < OpCode::HEARTBEAT) {
        // Silently ignore request opcodes - no logging from CAN task
        ignoredRequests++;
        return;
    }
    
    // Route to appropriate handler based on function ID
    switch (msg.functionId) {
        case Function::PLUG_STATE:
            processPlugState(msg.payload, msg.payloadLen);
            plugFrames++;
            break;
            
        case Function::CHARGE_STATE:
            processChargeState(msg.payload, msg.payloadLen);
            chargeFrames++;
            break;
            
        case Function::CLIMATE_STATE:
            processClimateState(msg.payload, msg.payloadLen);
            climateFrames++;
            break;
            
        default:
            // Silently count unhandled functions - no logging from CAN task
            otherFrames++;
            break;
    }
}

void BapDomain::processPlugState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        decodeErrors++;
        return;
    }
    
    PlugStateData decoded = decodePlugState(payload, len);
    
    BapPlugState& plug = state.bapPlug;
    plug.lockSetup = decoded.lockSetup;
    plug.lockState = decoded.lockState;
    plug.supplyState = static_cast<uint8_t>(decoded.supplyState);
    plug.plugState = static_cast<uint8_t>(decoded.plugState);
    plug.lastUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BapDomain::processChargeState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        decodeErrors++;
        return;
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
    
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
}

void BapDomain::processClimateState(const uint8_t* payload, uint8_t len) {
    if (len < 1) {
        decodeErrors++;
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
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
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
