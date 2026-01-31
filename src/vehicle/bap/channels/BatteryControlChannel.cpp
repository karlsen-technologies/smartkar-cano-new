#include "BatteryControlChannel.h"
#include "../../VehicleManager.h"
#include "../../../core/CommandRouter.h"
#include "../../../core/CommandStateManager.h"
#include <ArduinoJson.h>

using namespace BapProtocol;

BatteryControlChannel::BatteryControlChannel(VehicleManager* mgr)
    : manager(mgr)
{
}

// =============================================================================
// Frame Processing (for new domain-based architecture)
// =============================================================================

bool BatteryControlChannel::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // This method is called directly from VehicleManager in the new architecture
    // (bypassing BapChannelRouter)
    
    if (canId != CAN_ID_RX) {
        return false;  // Not our channel
    }
    
    // Assemble multi-frame BAP message
    BapProtocol::BapMessage msg;
    if (frameAssembler.processFrame(data, dlc, msg)) {
        // Complete message assembled - process it
        return processMessage(msg);
    }
    
    // Frame accepted but message not complete yet (waiting for continuation frames)
    return true;
}

// =============================================================================
// BapChannel interface implementation
// =============================================================================

bool BatteryControlChannel::handlesCanId(uint32_t canId) const {
    // We only passively listen to the RX channel (responses from Battery Control)
    return canId == CAN_ID_RX;
}

bool BatteryControlChannel::processMessage(const BapProtocol::BapMessage& msg) {
    // Only process response opcodes (we don't care about requests from other modules)
    // OpCodes 0x03-0x07 are responses/indications
    if (msg.opcode < OpCode::HEARTBEAT) {
        // Silently ignore request opcodes - no logging from CAN task
        ignoredRequests++;
        return false;
    }
    
    // Route to appropriate handler based on function ID
    switch (msg.functionId) {
        case Function::PLUG_STATE:
            processPlugState(msg.payload, msg.payloadLen);
            plugFrames++;
            return true;
            
        case Function::CHARGE_STATE:
            processChargeState(msg.payload, msg.payloadLen);
            chargeFrames++;
            return true;
            
        case Function::CLIMATE_STATE:
            processClimateState(msg.payload, msg.payloadLen);
            climateFrames++;
            return true;
            
        case Function::PROFILES_ARRAY:
            // Only process STATUS responses (OpCode 0x04)
            // Profile arrays are sent as STATUS in response to GET requests
            if (msg.opcode == OpCode::STATUS) {
                // Forward to ChargingProfileManager for parsing
                if (manager) {
                    manager->profiles().processProfilesArray(msg.payload, msg.payloadLen);
                }
                profileFrames++;
                return true;
            } else {
                // Ignore non-STATUS opcodes (HeartbeatStatus, Error, etc.)
                otherFrames++;
                return false;
            }
            
        case Function::OPERATION_MODE:
            // NEW (Phase 1): Forward OPERATION_MODE responses to ChargingProfileManager
            // Handles HeartbeatStatus (0x03), Status (0x04), and Error (0x07) responses
            // for profile execution tracking
            if (msg.opcode == OpCode::HEARTBEAT || 
                msg.opcode == OpCode::STATUS || 
                msg.opcode == OpCode::ERROR) {
                if (manager) {
                    manager->profiles().processOperationModeResponse(msg);
                }
                otherFrames++;
                return true;
            } else {
                // Ignore request opcodes
                otherFrames++;
                return false;
            }
            
        default:
            // Silently count unhandled functions - no logging from CAN task
            otherFrames++;
            return false;
    }
}

// =============================================================================
// State processing
// =============================================================================

void BatteryControlChannel::processPlugState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        decodeErrors++;
        return;
    }
    
    PlugStateData decoded = decodePlugState(payload, len);
    
    // Build PlugState structure to pass to callbacks
    PlugState plugData;
    plugData.lockSetup = decoded.lockSetup;
    plugData.lockState = decoded.lockState;
    plugData.supplyState = static_cast<uint8_t>(decoded.supplyState);
    plugData.plugState = static_cast<uint8_t>(decoded.plugState);
    plugData.lastUpdate = millis();
    
    // Notify subscribers (pass by const reference)
    notifyPlugStateCallbacks(plugData);
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryControlChannel::processChargeState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        decodeErrors++;
        return;
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
    
    ChargeStateData decoded = decodeChargeState(payload, len);
    
    // Build BatteryState structure to pass to callbacks
    BatteryState batteryData;
    
    // Update unified SOC field (BAP source - takes priority over CAN)
    batteryData.soc = decoded.socPercent;
    batteryData.socSource = DataSource::BAP;
    batteryData.socUpdate = millis();
    
    // Update unified charging field (BAP source - more detailed than CAN)
    batteryData.charging = (decoded.chargeMode != ChargeMode::OFF && 
                        decoded.chargeMode != ChargeMode::INIT &&
                        decoded.chargeStatus == ChargeStatus::RUNNING);
    batteryData.chargingSource = DataSource::BAP;
    batteryData.chargingUpdate = millis();
    
    // Store detailed charging info
    batteryData.chargingMode = static_cast<uint8_t>(decoded.chargeMode);
    batteryData.chargingStatus = static_cast<uint8_t>(decoded.chargeStatus);
    batteryData.chargingAmps = decoded.chargingAmps;
    batteryData.targetSoc = decoded.targetSoc;
    batteryData.remainingTimeMin = decoded.remainingTimeMin;
    batteryData.chargingDetailsUpdate = millis();
    
    // Notify subscribers (new architecture)
    notifyChargeStateCallbacks(batteryData);
}

void BatteryControlChannel::processClimateState(const uint8_t* payload, uint8_t len) {
    if (len < 1) {
        decodeErrors++;
        return;
    }
    
    ClimateStateData decoded = decodeClimateState(payload, len);
    
    // Build ClimateState structure
    ClimateState climateData;
    
    // Update unified climate fields (BAP source - provides detailed mode info)
    climateData.climateActive = decoded.climateActive;
    climateData.climateActiveSource = DataSource::BAP;
    climateData.heating = decoded.heating;
    climateData.cooling = decoded.cooling;
    climateData.ventilation = decoded.ventilation;
    climateData.autoDefrost = decoded.autoDefrost;
    climateData.climateTimeMin = decoded.climateTimeMin;
    climateData.climateActiveUpdate = millis();
    
    // Update inside temperature if climate is active (BAP priority)
    if (decoded.climateActive) {
        climateData.insideTemp = decoded.currentTempC;
        climateData.insideTempSource = DataSource::BAP;
        climateData.insideTempUpdate = millis();
    }
    
    // Notify subscribers (new architecture)
    notifyClimateStateCallbacks(climateData);
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// Payload decoders (device-specific)
// =============================================================================

BatteryControlChannel::PlugStateData BatteryControlChannel::decodePlugState(const uint8_t* payload, uint8_t len) {
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

BatteryControlChannel::ChargeStateData BatteryControlChannel::decodeChargeState(const uint8_t* payload, uint8_t len) {
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

BatteryControlChannel::ClimateStateData BatteryControlChannel::decodeClimateState(const uint8_t* payload, uint8_t len) {
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
// Command builders (device-specific)
// =============================================================================

uint8_t BatteryControlChannel::buildGetRequest(uint8_t* dest, uint8_t functionId) {
    return encodeShortMessage(dest, OpCode::GET, DEVICE_ID, functionId);
}

// =============================================================================
// Command methods
// =============================================================================

bool BatteryControlChannel::sendBapFrame(const uint8_t* data, uint8_t len) {
    if (!manager) {
        Serial.println("[BatteryControl] No manager - cannot send");
        return false;
    }
    
    // Send as extended frame to TX channel
    return manager->sendCanFrame(CAN_ID_TX, data, len, true);
}

bool BatteryControlChannel::requestPlugState() {
    uint8_t frame[8];
    buildGetRequest(frame, Function::PLUG_STATE);
    
    Serial.println("[BatteryControl] Requesting PlugState...");
    return sendBapFrame(frame, 8);
}

bool BatteryControlChannel::requestChargeState() {
    uint8_t frame[8];
    buildGetRequest(frame, Function::CHARGE_STATE);
    
    Serial.println("[BatteryControl] Requesting ChargeState...");
    return sendBapFrame(frame, 8);
}

bool BatteryControlChannel::requestClimateState() {
    uint8_t frame[8];
    buildGetRequest(frame, Function::CLIMATE_STATE);
    
    Serial.println("[BatteryControl] Requesting ClimateState...");
    return sendBapFrame(frame, 8);
}

// =============================================================================
// Callback Notifications (for new domain-based architecture)
// =============================================================================

void BatteryControlChannel::notifyPlugStateCallbacks(const PlugState& plugData) {
    // Called from CAN thread - keep FAST (just data copying)
    if (plugStateCallbacks.empty()) return;
    
    for (const auto& callback : plugStateCallbacks) {
        if (callback) {
            callback(plugData);
        }
    }
}

void BatteryControlChannel::notifyChargeStateCallbacks(const BatteryState& batteryData) {
    // Called from CAN thread - keep FAST (just data copying)
    if (chargeStateCallbacks.empty()) return;
    
    for (const auto& callback : chargeStateCallbacks) {
        if (callback) {
            callback(batteryData);
        }
    }
}

void BatteryControlChannel::notifyClimateStateCallbacks(const ClimateState& climateData) {
    // Called from CAN thread - keep FAST (just data copying)
    if (climateStateCallbacks.empty()) return;
    
    for (const auto& callback : climateStateCallbacks) {
        if (callback) {
            callback(climateData);
        }
    }
}
