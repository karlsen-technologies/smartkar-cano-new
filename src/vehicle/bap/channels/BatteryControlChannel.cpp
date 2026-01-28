#include "BatteryControlChannel.h"
#include "../../VehicleManager.h"
#include "../../../core/CommandRouter.h"
#include <ArduinoJson.h>

using namespace BapProtocol;

BatteryControlChannel::BatteryControlChannel(VehicleState& state, VehicleManager* mgr)
    : state(state)
    , manager(mgr)
{
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
    
    PlugState& plug = state.plug;
    plug.lockSetup = decoded.lockSetup;
    plug.lockState = decoded.lockState;
    plug.supplyState = static_cast<uint8_t>(decoded.supplyState);
    plug.plugState = static_cast<uint8_t>(decoded.plugState);
    plug.lastUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryControlChannel::processChargeState(const uint8_t* payload, uint8_t len) {
    if (len < 2) {
        decodeErrors++;
        return;
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
    
    ChargeStateData decoded = decodeChargeState(payload, len);
    
    BatteryState& battery = state.battery;
    
    // Update unified SOC field (BAP source - takes priority over CAN)
    battery.soc = decoded.socPercent;
    battery.socSource = DataSource::BAP;
    battery.socUpdate = millis();
    
    // Update unified charging field (BAP source - more detailed than CAN)
    battery.charging = (decoded.chargeMode != ChargeMode::OFF && 
                        decoded.chargeMode != ChargeMode::INIT &&
                        decoded.chargeStatus == ChargeStatus::RUNNING);
    battery.chargingSource = DataSource::BAP;
    battery.chargingUpdate = millis();
    
    // Store detailed charging info
    battery.chargingMode = static_cast<uint8_t>(decoded.chargeMode);
    battery.chargingStatus = static_cast<uint8_t>(decoded.chargeStatus);
    battery.chargingAmps = decoded.chargingAmps;
    battery.targetSoc = decoded.targetSoc;
    battery.remainingTimeMin = decoded.remainingTimeMin;
    battery.chargingDetailsUpdate = millis();
}

void BatteryControlChannel::processClimateState(const uint8_t* payload, uint8_t len) {
    if (len < 1) {
        decodeErrors++;
        return;
    }
    
    ClimateStateData decoded = decodeClimateState(payload, len);
    
    ClimateState& climate = state.climate;
    
    // Update unified climate fields (BAP source - provides detailed mode info)
    climate.climateActive = decoded.climateActive;
    climate.climateActiveSource = DataSource::BAP;
    climate.heating = decoded.heating;
    climate.cooling = decoded.cooling;
    climate.ventilation = decoded.ventilation;
    climate.autoDefrost = decoded.autoDefrost;
    climate.climateTimeMin = decoded.climateTimeMin;
    climate.climateActiveUpdate = millis();
    
    // Update inside temperature if climate is active (BAP priority)
    if (decoded.climateActive) {
        climate.insideTemp = decoded.currentTempC;
        climate.insideTempSource = DataSource::BAP;
        climate.insideTempUpdate = millis();
    }
    
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

uint8_t BatteryControlChannel::buildOperationModeStart(uint8_t* dest) {
    // Execute Profile 0 (the "immediately" profile)
    // Short BAP message to OPERATION_MODE (0x18)
    // Payload: [0x00, 0x01] = immediately active
    uint8_t payload[2] = {
        0x00,  // Flags
        0x01   // immediately = true
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_ID, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t BatteryControlChannel::buildOperationModeStop(uint8_t* dest) {
    // Stop Profile 0 execution
    // Short BAP message to OPERATION_MODE (0x18)
    // Payload: [0x00, 0x00] = immediately off
    uint8_t payload[2] = {
        0x00,  // Flags
        0x00   // immediately = false (stop)
    };
    
    return encodeShortMessage(dest, OpCode::SET_GET, DEVICE_ID, 
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t BatteryControlChannel::buildProfileConfig(uint8_t* startFrame, uint8_t* contFrame, uint8_t operationMode) {
    // Configure Profile 0 with the desired operation mode
    // This is a LONG BAP message to PROFILES_ARRAY (0x19) using mode 6 update
    
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
    encodeLongStart(startFrame, OpCode::SET_GET, DEVICE_ID,
                    Function::PROFILES_ARRAY, 6, payload, 0);
    
    // Encode continuation frame with remaining payload bytes
    uint8_t contPayload[7] = {
        operationMode,  // This is where the operation mode actually appears
        0x00,           // Padding
        0x20,           // Unknown flag (from traces)
        0x00, 0x00, 0x00, 0x00  // Padding
    };
    encodeLongContinuation(contFrame, contPayload, 7, 0, 0);
    
    return 8;  // Each frame is 8 bytes
}

uint8_t BatteryControlChannel::buildClimateStart(uint8_t* dest, float tempCelsius) {
    // For climate start, we just need to trigger the operation mode
    // The profile should already be configured, or we configure it for climate
    // 
    // Temperature is stored in the profile configuration, not in the start command.
    // To set temperature, you need to update the profile's temperature field.
    (void)tempCelsius;  // Temperature requires separate profile temperature update
    
    return buildOperationModeStart(dest);
}

uint8_t BatteryControlChannel::buildClimateStop(uint8_t* dest) {
    // Stop by sending operation mode stop
    return buildOperationModeStop(dest);
}

uint8_t BatteryControlChannel::buildChargeStart(uint8_t* dest) {
    // Same as climate - trigger operation mode
    // Profile should be configured for charging mode
    return buildOperationModeStart(dest);
}

uint8_t BatteryControlChannel::buildChargeStop(uint8_t* dest) {
    // Stop by sending operation mode stop
    return buildOperationModeStop(dest);
}

uint8_t BatteryControlChannel::buildChargingAndClimateStart(uint8_t* dest) {
    // For combined charging+climate, use operation mode start
    // The operation mode is determined by whether battery power is allowed
    // ProfileOperation::CHARGING_AND_CLIMATE (0x03) - both charging and climate, climate from external power only
    // ProfileOperation::CHARGING_ALLOW_CLIMATE_BATTERY (0x05) - both, climate can use battery
    
    // Note: This just triggers the operation. Profile configuration with temperature/SOC
    // would need to be sent separately via buildProfileConfig() if we want to set those.
    // For now, we're using the "immediately" profile which should already be configured.
    
    return buildOperationModeStart(dest);
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

bool BatteryControlChannel::startClimate(float tempCelsius, bool allowBattery) {
    // Check if busy
    if (isBusy()) {
        Serial.println("[BatteryControl] Busy - command rejected");
        return false;
    }
    
    // Queue command
    pendingCommand = PendingCommand::START_CLIMATE;
    pendingTempCelsius = tempCelsius;
    pendingAllowBattery = allowBattery;
    
    setCommandState(CommandState::REQUESTING_WAKE);
    commandsQueued++;
    
    Serial.printf("[BatteryControl] Queued: Start climate %.1f°C (battery=%s)\r\n",
                 tempCelsius, allowBattery ? "yes" : "no");
    return true;
}

bool BatteryControlChannel::stopClimate() {
    // Check if busy
    if (isBusy()) {
        Serial.println("[BatteryControl] Busy - command rejected");
        return false;
    }
    
    // Queue command
    pendingCommand = PendingCommand::STOP_CLIMATE;
    
    setCommandState(CommandState::REQUESTING_WAKE);
    commandsQueued++;
    
    Serial.println("[BatteryControl] Queued: Stop climate");
    return true;
}

bool BatteryControlChannel::startCharging(uint8_t targetSoc, uint8_t maxCurrent) {
    // Check if busy
    if (isBusy()) {
        Serial.println("[BatteryControl] Busy - command rejected");
        return false;
    }
    
    // Queue command
    pendingCommand = PendingCommand::START_CHARGING;
    pendingTargetSoc = targetSoc;
    pendingMaxCurrent = maxCurrent;
    
    setCommandState(CommandState::REQUESTING_WAKE);
    commandsQueued++;
    
    Serial.printf("[BatteryControl] Queued: Start charging (SOC=%d%%, current=%dA)\r\n",
                 targetSoc, maxCurrent);
    return true;
}

bool BatteryControlChannel::stopCharging() {
    // Check if busy
    if (isBusy()) {
        Serial.println("[BatteryControl] Busy - command rejected");
        return false;
    }
    
    // Queue command
    pendingCommand = PendingCommand::STOP_CHARGING;
    
    setCommandState(CommandState::REQUESTING_WAKE);
    commandsQueued++;
    
    Serial.println("[BatteryControl] Queued: Stop charging");
    return true;
}

bool BatteryControlChannel::startChargingAndClimate(float tempCelsius, uint8_t targetSoc, 
                                                     uint8_t maxCurrent, bool allowBattery) {
    // Check if busy
    if (isBusy()) {
        Serial.println("[BatteryControl] Busy - command rejected");
        return false;
    }
    
    // Queue command
    pendingCommand = PendingCommand::START_CHARGING_AND_CLIMATE;
    pendingTempCelsius = tempCelsius;
    pendingTargetSoc = targetSoc;
    pendingMaxCurrent = maxCurrent;
    pendingAllowBattery = allowBattery;
    
    setCommandState(CommandState::REQUESTING_WAKE);
    commandsQueued++;
    
    Serial.printf("[BatteryControl] Queued: Start charging+climate %.1f°C, SOC=%d%%, current=%dA, battery=%s\r\n",
                 tempCelsius, targetSoc, maxCurrent, allowBattery ? "yes" : "no");
    return true;
}

// =============================================================================
// Command State Machine
// =============================================================================

void BatteryControlChannel::loop() {
    updateCommandStateMachine();
}

const char* BatteryControlChannel::getCommandStateName() const {
    switch (commandState) {
        case CommandState::IDLE: return "IDLE";
        case CommandState::REQUESTING_WAKE: return "REQUESTING_WAKE";
        case CommandState::WAITING_FOR_WAKE: return "WAITING_FOR_WAKE";
        case CommandState::SENDING_COMMAND: return "SENDING_COMMAND";
        case CommandState::DONE: return "DONE";
        case CommandState::FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

void BatteryControlChannel::updateCommandStateMachine() {
    unsigned long now = millis();
    unsigned long elapsed = now - commandStateStartTime;
    
    switch (commandState) {
        case CommandState::IDLE:
            // Nothing to do
            break;
            
        case CommandState::REQUESTING_WAKE:
            // Request vehicle wake
            if (manager->requestWake()) {
                Serial.println("[BatteryControl] Wake requested");
                setCommandState(CommandState::WAITING_FOR_WAKE);
            } else {
                Serial.println("[BatteryControl] Wake request failed");
                setCommandState(CommandState::FAILED);
                commandsFailed++;
                
                // Send failure event via singleton
                emitCommandEvent("commandFailed", "wake_request_failed");
                
                pendingCommand = PendingCommand::NONE;
            }
            break;
            
        case CommandState::WAITING_FOR_WAKE:
            // Check if vehicle awake
            if (manager->isAwake()) {
                Serial.printf("[BatteryControl] Vehicle awake after %lums\r\n", elapsed);
                setCommandState(CommandState::SENDING_COMMAND);
            }
            // Check for timeout
            else if (elapsed > COMMAND_WAKE_TIMEOUT) {
                Serial.println("[BatteryControl] Wake timeout");
                setCommandState(CommandState::FAILED);
                commandsFailed++;
                
                // Send failure event via singleton
                emitCommandEvent("commandFailed", "wake_timeout");
                
                pendingCommand = PendingCommand::NONE;
            }
            break;
            
        case CommandState::SENDING_COMMAND:
            // Execute the pending command
            if (executePendingCommand()) {
                Serial.println("[BatteryControl] Command sent successfully");
                setCommandState(CommandState::DONE);
                commandsCompleted++;
            } else {
                Serial.println("[BatteryControl] Command send failed");
                setCommandState(CommandState::FAILED);
                commandsFailed++;
                
                // Send failure event via singleton
                emitCommandEvent("commandFailed", "send_failed");
            }
            pendingCommand = PendingCommand::NONE;
            break;
            
        case CommandState::DONE:
            // Send success event via singleton
            emitCommandEvent("commandCompleted", nullptr);
            
            // Reset to IDLE on next loop
            setCommandState(CommandState::IDLE);
            break;
            
        case CommandState::FAILED:
            // Reset to IDLE on next loop
            setCommandState(CommandState::IDLE);
            break;
    }
}

bool BatteryControlChannel::executePendingCommand() {
    uint8_t frame[8];
    
    switch (pendingCommand) {
        case PendingCommand::START_CLIMATE:
            buildClimateStart(frame, pendingTempCelsius);
            Serial.printf("[BatteryControl] Sending: Start climate %.1f°C\r\n", pendingTempCelsius);
            return sendBapFrame(frame, 8);
            
        case PendingCommand::STOP_CLIMATE:
            buildClimateStop(frame);
            Serial.println("[BatteryControl] Sending: Stop climate");
            return sendBapFrame(frame, 8);
            
        case PendingCommand::START_CHARGING:
            buildChargeStart(frame);
            Serial.printf("[BatteryControl] Sending: Start charging (SOC=%d%%)\r\n", pendingTargetSoc);
            return sendBapFrame(frame, 8);
            
        case PendingCommand::STOP_CHARGING:
            buildChargeStop(frame);
            Serial.println("[BatteryControl] Sending: Stop charging");
            return sendBapFrame(frame, 8);
            
        case PendingCommand::START_CHARGING_AND_CLIMATE:
            buildChargingAndClimateStart(frame);
            Serial.printf("[BatteryControl] Sending: Start charging+climate %.1f°C, SOC=%d%%\r\n", 
                         pendingTempCelsius, pendingTargetSoc);
            return sendBapFrame(frame, 8);
            
        case PendingCommand::NONE:
        default:
            Serial.println("[BatteryControl] No pending command to execute");
            return false;
    }
}

void BatteryControlChannel::setCommandState(CommandState newState) {
    if (commandState != newState) {
        Serial.printf("[BatteryControl] Command: %s -> %s\r\n",
                     getCommandStateName(),
                     [this, newState]() {
                         CommandState temp = commandState;
                         commandState = newState;
                         const char* name = getCommandStateName();
                         commandState = temp;
                         return name;
                     }());
        commandState = newState;
        commandStateStartTime = millis();
    }
}

const char* BatteryControlChannel::getPendingCommandName() const {
    switch (pendingCommand) {
        case PendingCommand::START_CLIMATE: return "startClimate";
        case PendingCommand::STOP_CLIMATE: return "stopClimate";
        case PendingCommand::START_CHARGING: return "startCharging";
        case PendingCommand::STOP_CHARGING: return "stopCharging";
        case PendingCommand::START_CHARGING_AND_CLIMATE: return "startChargingAndClimate";
        case PendingCommand::NONE:
        default: return "none";
    }
}

void BatteryControlChannel::emitCommandEvent(const char* eventName, const char* reason) {
    CommandRouter* router = CommandRouter::getInstance();
    if (!router) return;
    
    // Build event details
    JsonDocument doc;
    JsonObject details = doc.to<JsonObject>();
    details["command"] = getPendingCommandName();
    if (reason) {
        details["reason"] = reason;
    }
    
    // Send event
    router->sendEvent("vehicle", eventName, &details);
}
