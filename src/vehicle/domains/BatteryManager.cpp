#include "BatteryManager.h"
#include "../VehicleManager.h"
#include "../bap/channels/BatteryControlChannel.h"
#include "../ChargingProfileManager.h"
#include "../services/WakeController.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store battery state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR BatteryManager::State rtcBatteryState = {};

// =============================================================================
// Constructor
// =============================================================================

BatteryManager::BatteryManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , bapChannel(nullptr)
    , profileManager(nullptr)
    , wakeController(nullptr)
    , state(rtcBatteryState) {  // Initialize reference to RTC memory
}

bool BatteryManager::setup() {
    Serial.println("[BatteryManager] Initializing...");
    
    if (!vehicleManager) {
        Serial.println("[BatteryManager] ERROR: No VehicleManager!");
        return false;
    }
    
    // Get references to services
    bapChannel = &vehicleManager->batteryControl();
    profileManager = &vehicleManager->profiles();
    wakeController = &vehicleManager->wake();
    
    // Register callbacks for BAP updates
    Serial.println("[BatteryManager] Registering BAP callbacks...");
    
    // Plug state callback (function 0x10)
    bapChannel->onPlugState([this](const PlugState& plug) {
        this->onPlugStateUpdate(plug);
    });
    
    // Charge state callback (function 0x11)
    bapChannel->onChargeState([this](const BatteryState& battery) {
        this->onChargeStateUpdate(battery);
    });
    
    Serial.println("[BatteryManager] Initialized:");
    Serial.println("[BatteryManager]   - CAN IDs: 0x5CA (BMS_07), 0x59E (BMS_06), 0x483 (Motor_Hybrid_06)");
    Serial.println("[BatteryManager]   - BAP callbacks: PlugState, ChargeState");
    Serial.println("[BatteryManager]   - Data source priority: BAP > CAN > Computed");
    
    return true;
}

void BatteryManager::loop() {
    // Update command state machine
    updateCommandStateMachine();
}

void BatteryManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // Called from CAN task (Core 0) with mutex held
    // MUST be fast (<1-2ms)
    
    if (dlc < 8) {
        return;  // Need full frame
    }
    
    switch (canId) {
        case CAN_ID_BMS_07:
            processBMS07(data);
            break;
            
        case CAN_ID_BMS_06:
            processBMS06(data);
            break;
            
        case CAN_ID_MOTOR_HYBRID_06:
            processMotorHybrid06(data);
            break;
            
        default:
            // Not our frame
            break;
    }
}

void BatteryManager::onWakeComplete() {
    // Optional: Request initial BAP state after wake
    // For now, BAP channel will send updates automatically
    Serial.println("[BatteryManager] Vehicle awake, waiting for BAP updates");
}

bool BatteryManager::isBusy() const {
    // Check if domain command state machine is busy
    return cmdState != CommandState::IDLE;
}

// =============================================================================
// CAN Frame Processors
// =============================================================================

void BatteryManager::processBMS07(const uint8_t* data) {
    // BMS_07 (0x5CA) - Charging status and energy content
    bms07Count++;
    
    BroadcastDecoder::BMS07Data decoded = BroadcastDecoder::decodeBMS07(data);
    
    // Update CAN-sourced fields
    state.energyWh = decoded.energyWh;
    state.maxEnergyWh = decoded.maxEnergyWh;
    state.chargingActive = decoded.chargingActive;
    state.balancingActive = decoded.balancingActive;
    state.energyUpdate = millis();
    state.balancingUpdate = millis();
    
    // Update unified charging field (CAN source - BAP will override if available)
    if (state.chargingSource != DataSource::BAP) {
        state.charging = decoded.chargingActive;
        state.chargingSource = DataSource::CAN_STD;
        state.chargingUpdate = millis();
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryManager::processBMS06(const uint8_t* data) {
    // BMS_06 (0x59E) - Battery temperature
    bms06Count++;
    
    float temp = BroadcastDecoder::decodeBMS06Temperature(data);
    
    state.temperature = temp;
    state.tempUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryManager::processMotorHybrid06(const uint8_t* data) {
    // Motor_Hybrid_06 (0x483) - Power meter for charging/climate
    motorHybrid06Count++;
    
    BroadcastDecoder::MotorHybrid06Data decoded = BroadcastDecoder::decodeMotorHybrid06(data);
    
    state.powerKw = decoded.powerKw;
    state.powerUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// BAP Callback Handlers
// =============================================================================

void BatteryManager::onPlugStateUpdate(const PlugState& plug) {
    // Called from CAN thread via BatteryControlChannel callback
    // Keep FAST - just copy data
    plugCallbackCount++;
    
    state.plugState = plug;
    state.plugStateSource = DataSource::BAP;
    state.plugStateUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryManager::onChargeStateUpdate(const BatteryState& battery) {
    // Called from CAN thread via BatteryControlChannel callback
    // Keep FAST - just copy data
    chargeCallbackCount++;
    
    // BAP SOC takes priority over CAN
    state.soc = battery.soc;
    state.socSource = DataSource::BAP;
    state.socUpdate = millis();
    
    // BAP charging info is more detailed than CAN
    state.charging = battery.charging;
    state.chargingSource = DataSource::BAP;
    state.chargingMode = battery.chargingMode;
    state.chargingStatus = battery.chargingStatus;
    state.chargingAmps = battery.chargingAmps;
    state.targetSoc = battery.targetSoc;
    state.remainingTimeMin = battery.remainingTimeMin;
    state.chargingUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// Command Interface (NEW - Phase 2: Domain State Machine)
// =============================================================================

bool BatteryManager::startCharging(uint8_t commandId, uint8_t targetSoc, uint8_t maxCurrent) {
    // Check if already busy
    if (cmdState != CommandState::IDLE) {
        Serial.println("[BatteryManager] Command already in progress");
        return false;
    }
    
    // Validate parameters (business logic)
    if (!validateChargingParams(targetSoc, maxCurrent)) {
        return false;
    }
    
    Serial.printf("[BatteryManager] Start charging command: id=%d, target=%d%%, maxCurrent=%dA\r\n", 
                  commandId, targetSoc, maxCurrent);
    
    // Store pending command
    pendingCmdType = PendingCommandType::START_CHARGING;
    pendingCommandId = commandId;
    pendingTargetSoc = targetSoc;
    pendingMaxCurrent = maxCurrent;
    
    // Ensure vehicle is awake and keep-alive is active
    wakeController->ensureAwake();
    
    // Start state machine
    if (!wakeController->isAwake()) {
        Serial.println("[BatteryManager] Vehicle not awake, requesting wake");
        setCommandState(CommandState::REQUESTING_WAKE);
    } else if (needsProfileUpdate(targetSoc, maxCurrent)) {
        Serial.println("[BatteryManager] Profile needs update");
        setCommandState(CommandState::UPDATING_PROFILE);
    } else {
        Serial.println("[BatteryManager] Profile OK, executing now");
        setCommandState(CommandState::EXECUTING_COMMAND);
    }
    
    return true;
}

bool BatteryManager::stopCharging(uint8_t commandId) {
    // Check if already busy
    if (cmdState != CommandState::IDLE) {
        Serial.println("[BatteryManager] Command already in progress");
        return false;
    }
    
    Serial.printf("[BatteryManager] Stop charging command: id=%d\r\n", commandId);
    
    // Store pending command
    pendingCmdType = PendingCommandType::STOP_CHARGING;
    pendingCommandId = commandId;
    
    // Ensure vehicle is awake and keep-alive is active
    wakeController->ensureAwake();
    
    // Stop doesn't need profile update, just execute
    if (!wakeController->isAwake()) {
        Serial.println("[BatteryManager] Vehicle not awake, requesting wake");
        setCommandState(CommandState::REQUESTING_WAKE);
    } else {
        Serial.println("[BatteryManager] Stopping profile 0");
        setCommandState(CommandState::EXECUTING_COMMAND);
    }
    
    return true;
}

// =============================================================================
// Command State Machine Implementation
// =============================================================================

void BatteryManager::updateCommandStateMachine() {
    if (cmdState == CommandState::IDLE || cmdState == CommandState::DONE || cmdState == CommandState::FAILED) {
        return;  // Nothing to do
    }
    
    unsigned long elapsed = millis() - cmdStateStartTime;
    
    switch (cmdState) {
        case CommandState::REQUESTING_WAKE:
            // Check if wake completed
            if (wakeController->isAwake()) {
                Serial.println("[BatteryManager] Wake complete");
                
                // Decide next state based on command type
                if (pendingCmdType == PendingCommandType::START_CHARGING && 
                    needsProfileUpdate(pendingTargetSoc, pendingMaxCurrent)) {
                    setCommandState(CommandState::UPDATING_PROFILE);
                } else {
                    setCommandState(CommandState::EXECUTING_COMMAND);
                }
            } else if (elapsed > WAKE_TIMEOUT) {
                failCommand("wake_timeout");
            }
            break;
            
        case CommandState::UPDATING_PROFILE: {
            // Build profile update
            ChargingProfileManager::ProfileFieldUpdate update;
            update.updateTargetSoc = true;
            update.targetSoc = pendingTargetSoc;
            update.updateMaxCurrent = true;
            update.maxCurrent = pendingMaxCurrent;
            
            // Request profile update (async)
            bool ok = profileManager->requestProfileUpdate(0, update, [this](bool success) {
                if (success) {
                    Serial.println("[BatteryManager] Profile update success");
                    setCommandState(CommandState::EXECUTING_COMMAND);
                } else {
                    failCommand("profile_update_failed");
                }
            });
            
            if (!ok) {
                failCommand("profile_update_busy");
                break;
            }
            
            // Move to next state immediately (callback will advance)
            setCommandState(CommandState::DONE);  // Will be overridden by callback
            break;
        }
            
        case CommandState::EXECUTING_COMMAND: {
            bool ok = false;
            
            if (pendingCmdType == PendingCommandType::START_CHARGING) {
                // Execute profile 0
                ok = profileManager->executeProfile0([this](bool success, const char* error) {
                    if (success) {
                        Serial.println("[BatteryManager] Charging started successfully");
                        completeCommand();
                    } else {
                        Serial.printf("[BatteryManager] Charging failed: %s\r\n", error ? error : "unknown");
                        failCommand(error ? error : "execution_failed");
                    }
                });
            } else if (pendingCmdType == PendingCommandType::STOP_CHARGING) {
                // Stop profile 0
                ok = profileManager->stopProfile0([this](bool success, const char* error) {
                    if (success) {
                        Serial.println("[BatteryManager] Charging stopped successfully");
                        completeCommand();
                    } else {
                        Serial.printf("[BatteryManager] Stop failed: %s\r\n", error ? error : "unknown");
                        failCommand(error ? error : "stop_failed");
                    }
                });
            }
            
            if (!ok) {
                failCommand("execution_busy");
                break;
            }
            
            // Move to waiting state (callback will complete)
            setCommandState(CommandState::DONE);  // Will be overridden by callback
            break;
        }
            
        default:
            break;
    }
}

void BatteryManager::setCommandState(CommandState newState) {
    if (cmdState == newState) {
        return;
    }
    
    const char* stateName[] = {"IDLE", "REQUESTING_WAKE", "UPDATING_PROFILE", "EXECUTING_COMMAND", "DONE", "FAILED"};
    Serial.printf("[BatteryManager] Command state: %s -> %s\r\n", 
                  stateName[(int)cmdState], stateName[(int)newState]);
    
    cmdState = newState;
    cmdStateStartTime = millis();
}

bool BatteryManager::validateChargingParams(uint8_t targetSoc, uint8_t maxCurrent) {
    // Business logic: Validate SOC
    if (targetSoc < 10 || targetSoc > 100) {
        Serial.printf("[BatteryManager] Invalid target SOC: %d%% (must be 10-100%%)\r\n", targetSoc);
        return false;
    }
    
    // Business logic: Validate current
    if (maxCurrent < 1 || maxCurrent > 32) {
        Serial.printf("[BatteryManager] Invalid max current: %dA (must be 1-32A)\r\n", maxCurrent);
        return false;
    }
    
    return true;
}

bool BatteryManager::needsProfileUpdate(uint8_t targetSoc, uint8_t maxCurrent) {
    // Check if profile 0 exists
    if (!profileManager->isProfileValid(0)) {
        Serial.println("[BatteryManager] Profile 0 not valid, needs read first");
        return true;  // Need to read profile first
    }
    
    const auto& profile0 = profileManager->getProfile(0);
    
    // Business logic: Check if SOC changed
    if (profile0.targetChargeLevel != targetSoc) {
        Serial.printf("[BatteryManager] SOC changed: %d%% -> %d%%\r\n", 
                      profile0.targetChargeLevel, targetSoc);
        return true;
    }
    
    // Business logic: Check if current changed
    if (profile0.maxCurrent != maxCurrent) {
        Serial.printf("[BatteryManager] Current changed: %dA -> %dA\r\n", 
                      profile0.maxCurrent, maxCurrent);
        return true;
    }
    
    Serial.println("[BatteryManager] Profile 0 already has correct settings");
    return false;
}

void BatteryManager::completeCommand() {
    Serial.printf("[BatteryManager] Command %d completed successfully\r\n", pendingCommandId);
    
    // Stop keep-alive - charging will keep vehicle awake if active
    wakeController->stopKeepAlive();
    
    // Reset state
    setCommandState(CommandState::IDLE);
    pendingCmdType = PendingCommandType::NONE;
    pendingCommandId = -1;
    
    // TODO: Emit event or call callback when we add event system
}

void BatteryManager::failCommand(const char* reason) {
    Serial.printf("[BatteryManager] Command %d failed: %s\r\n", pendingCommandId, reason);
    
    // Stop keep-alive - no need to keep vehicle awake
    wakeController->stopKeepAlive();
    
    // Reset state
    setCommandState(CommandState::FAILED);
    
    // Give it a moment to log, then reset
    delay(10);
    setCommandState(CommandState::IDLE);
    pendingCmdType = PendingCommandType::NONE;
    pendingCommandId = -1;
    
    // TODO: Emit event or call callback when we add event system
}
