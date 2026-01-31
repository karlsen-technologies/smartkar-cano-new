#include "ClimateManager.h"
#include "../VehicleManager.h"
#include "../bap/channels/BatteryControlChannel.h"
#include "../ChargingProfileManager.h"
#include "../services/WakeController.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store climate state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR ClimateManager::State rtcClimateState = {};

// =============================================================================
// Constructor
// =============================================================================

ClimateManager::ClimateManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , bapChannel(nullptr)
    , profileManager(nullptr)
    , wakeController(nullptr)
    , state(rtcClimateState) {  // Initialize reference to RTC memory
}

bool ClimateManager::setup() {
    Serial.println("[ClimateManager] Initializing...");
    
    if (!vehicleManager) {
        Serial.println("[ClimateManager] ERROR: No VehicleManager!");
        return false;
    }
    
    // Get references to services
    bapChannel = &vehicleManager->batteryControl();
    profileManager = &vehicleManager->profiles();
    wakeController = &vehicleManager->wake();
    
    // Register callback for BAP updates
    Serial.println("[ClimateManager] Registering BAP callback (SHARED channel)...");
    
    // Climate state callback (function 0x12)
    bapChannel->onClimateState([this](const ClimateState& climate) {
        this->onClimateStateUpdate(climate);
    });
    
    Serial.println("[ClimateManager] Initialized:");
    Serial.println("[ClimateManager]   - CAN IDs: 0x66E (Klima_03), 0x5E1 (Klima_Sensor_02)");
    Serial.println("[ClimateManager]   - BAP callbacks: ClimateState (function 0x12)");
    Serial.println("[ClimateManager]   - Data source priority: BAP > CAN");
    Serial.println("[ClimateManager]   - SHARED channel: BatteryControlChannel");
    
    return true;
}

void ClimateManager::loop() {
    // Update command state machine
    updateCommandStateMachine();
}

void ClimateManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // Called from CAN task (Core 0) with mutex held
    // MUST be fast (<1-2ms)
    
    if (dlc < 8) {
        return;  // Need full frame
    }
    
    switch (canId) {
        case CAN_ID_KLIMA_03:
            processKlima03(data);
            break;
            
        case CAN_ID_KLIMA_SENSOR_02:
            processKlimaSensor02(data);
            break;
            
        default:
            // Not our frame
            break;
    }
}

void ClimateManager::onWakeComplete() {
    // Optional: Request initial BAP state after wake
    // For now, BAP channel will send updates automatically
    Serial.println("[ClimateManager] Vehicle awake, waiting for BAP updates");
}

bool ClimateManager::isBusy() const {
    // Delegate to BAP channel
    return bapChannel ? bapChannel->isBusy() : false;
}

// =============================================================================
// CAN Frame Processors
// =============================================================================

void ClimateManager::processKlima03(const uint8_t* data) {
    // Klima_03 (0x66E) - Inside temperature
    // Note: CAN climate active flags are unreliable - use BAP only for active status
    klima03Count++;
    
    BroadcastDecoder::KlimaData decoded = BroadcastDecoder::decodeKlima03(data);
    
    // Update inside temperature (CAN source - only if BAP hasn't updated recently)
    // BAP takes priority when climate is actively controlled
    if (state.insideTempSource != DataSource::BAP || 
        (millis() - state.insideTempUpdate) > 5000) {
        state.insideTemp = decoded.insideTemp;
        state.insideTempSource = DataSource::CAN_STD;
        state.insideTempUpdate = millis();
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void ClimateManager::processKlimaSensor02(const uint8_t* data) {
    // Klima_Sensor_02 (0x5E1) - Outside temperature
    // BCM1_Aussen_Temp_ungef: Byte 0, scale 0.5, offset -50
    klimaSensor02Count++;
    
    uint8_t rawTemp = data[0];
    float outsideTemp = rawTemp * 0.5f - 50.0f;
    
    state.outsideTemp = outsideTemp;
    state.outsideTempUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// BAP Callback Handler
// =============================================================================

void ClimateManager::onClimateStateUpdate(const ClimateState& climate) {
    // Called from CAN thread via BatteryControlChannel callback
    // Keep FAST - just copy data
    climateCallbackCount++;
    
    // BAP climate state is authoritative
    state.climateActive = climate.climateActive;
    state.climateActiveSource = DataSource::BAP;
    state.heating = climate.heating;
    state.cooling = climate.cooling;
    state.ventilation = climate.ventilation;
    state.autoDefrost = climate.autoDefrost;
    state.climateTimeMin = climate.climateTimeMin;
    state.climateActiveUpdate = millis();
    
    // Also update inside temp if provided by BAP (more accurate during active climate)
    if (climate.insideTemp > 0.0f) {
        state.insideTemp = climate.insideTemp;
        state.insideTempSource = DataSource::BAP;
        state.insideTempUpdate = millis();
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// Command Interface (NEW - Phase 2: Domain State Machine)
// =============================================================================

bool ClimateManager::requestState() {
    if (!bapChannel) {
        Serial.println("[ClimateManager] ERROR: No BAP channel!");
        return false;
    }
    
    Serial.println("[ClimateManager] Requesting climate state from vehicle");
    
    return bapChannel->requestClimateState();
}

bool ClimateManager::startClimate(int commandId, float tempCelsius, bool allowBattery) {
    // Check if already busy
    if (cmdState != CommandState::IDLE) {
        Serial.println("[ClimateManager] Command already in progress");
        return false;
    }
    
    // Validate parameters (business logic)
    if (!validateClimateParams(tempCelsius)) {
        return false;
    }
    
    Serial.printf("[ClimateManager] Start climate command: id=%d, temp=%.1f°C, allowBattery=%s\r\n", 
                  commandId, tempCelsius, allowBattery ? "YES" : "no");
    
    // Store pending command
    pendingCmdType = PendingCommandType::START_CLIMATE;
    pendingCommandId = commandId;
    pendingTempCelsius = tempCelsius;
    pendingAllowBattery = allowBattery;
    
    // Start state machine
    if (!wakeController->isAwake()) {
        Serial.println("[ClimateManager] Vehicle not awake, requesting wake");
        setCommandState(CommandState::REQUESTING_WAKE);
        wakeController->requestWake();
    } else if (needsProfileUpdate(tempCelsius, allowBattery)) {
        Serial.println("[ClimateManager] Profile needs update");
        setCommandState(CommandState::UPDATING_PROFILE);
    } else {
        Serial.println("[ClimateManager] Profile OK, executing now");
        setCommandState(CommandState::EXECUTING_COMMAND);
    }
    
    return true;
}

bool ClimateManager::stopClimate(int commandId) {
    // Check if already busy
    if (cmdState != CommandState::IDLE) {
        Serial.println("[ClimateManager] Command already in progress");
        return false;
    }
    
    Serial.printf("[ClimateManager] Stop climate command: id=%d\r\n", commandId);
    
    // Store pending command
    pendingCmdType = PendingCommandType::STOP_CLIMATE;
    pendingCommandId = commandId;
    
    // Stop doesn't need profile update, just execute
    if (!wakeController->isAwake()) {
        Serial.println("[ClimateManager] Vehicle not awake, requesting wake");
        setCommandState(CommandState::REQUESTING_WAKE);
        wakeController->requestWake();
    } else {
        Serial.println("[ClimateManager] Stopping profile 0");
        setCommandState(CommandState::EXECUTING_COMMAND);
    }
    
    return true;
}

// =============================================================================
// Command State Machine Implementation
// =============================================================================

void ClimateManager::updateCommandStateMachine() {
    if (cmdState == CommandState::IDLE || cmdState == CommandState::DONE || cmdState == CommandState::FAILED) {
        return;  // Nothing to do
    }
    
    unsigned long elapsed = millis() - cmdStateStartTime;
    
    switch (cmdState) {
        case CommandState::REQUESTING_WAKE:
            // Check if wake completed
            if (wakeController->isAwake()) {
                Serial.println("[ClimateManager] Wake complete");
                
                // Decide next state based on command type
                if (pendingCmdType == PendingCommandType::START_CLIMATE && 
                    needsProfileUpdate(pendingTempCelsius, pendingAllowBattery)) {
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
            update.updateTemperature = true;
            update.temperature = pendingTempCelsius;
            update.updateOperation = true;
            update.operation = pendingAllowBattery 
                ? ChargingProfile::OperationMode::CLIMATE_ALLOW_BATTERY
                : ChargingProfile::OperationMode::CLIMATE_ONLY;
            
            // Request profile update (async)
            bool ok = profileManager->requestProfileUpdate(0, update, [this](bool success) {
                if (success) {
                    Serial.println("[ClimateManager] Profile update success");
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
            
            if (pendingCmdType == PendingCommandType::START_CLIMATE) {
                // Execute profile 0
                ok = profileManager->executeProfile0([this](bool success, const char* error) {
                    if (success) {
                        Serial.println("[ClimateManager] Climate started successfully");
                        completeCommand();
                    } else {
                        Serial.printf("[ClimateManager] Climate failed: %s\r\n", error ? error : "unknown");
                        failCommand(error ? error : "execution_failed");
                    }
                });
            } else if (pendingCmdType == PendingCommandType::STOP_CLIMATE) {
                // Stop profile 0
                ok = profileManager->stopProfile0([this](bool success, const char* error) {
                    if (success) {
                        Serial.println("[ClimateManager] Climate stopped successfully");
                        completeCommand();
                    } else {
                        Serial.printf("[ClimateManager] Stop failed: %s\r\n", error ? error : "unknown");
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

void ClimateManager::setCommandState(CommandState newState) {
    if (cmdState == newState) {
        return;
    }
    
    const char* stateName[] = {"IDLE", "REQUESTING_WAKE", "UPDATING_PROFILE", "EXECUTING_COMMAND", "DONE", "FAILED"};
    Serial.printf("[ClimateManager] Command state: %s -> %s\r\n", 
                  stateName[(int)cmdState], stateName[(int)newState]);
    
    cmdState = newState;
    cmdStateStartTime = millis();
}

bool ClimateManager::validateClimateParams(float tempCelsius) {
    // Business logic: Validate temperature range (VW e-Golf range)
    if (tempCelsius < 15.5f || tempCelsius > 30.0f) {
        Serial.printf("[ClimateManager] Invalid temperature: %.1f°C (must be 15.5-30.0°C)\r\n", tempCelsius);
        return false;
    }
    
    return true;
}

bool ClimateManager::needsProfileUpdate(float tempCelsius, bool allowBattery) {
    // Check if profile 0 exists
    if (!profileManager->isProfileValid(0)) {
        Serial.println("[ClimateManager] Profile 0 not valid, needs read first");
        return true;  // Need to read profile first
    }
    
    const auto& profile0 = profileManager->getProfile(0);
    
    // Business logic: Check temperature with 0.5°C tolerance
    float currentTemp = profile0.getTemperature();
    if (abs(currentTemp - tempCelsius) > 0.5f) {
        Serial.printf("[ClimateManager] Temperature changed: %.1f°C -> %.1f°C\r\n", 
                      currentTemp, tempCelsius);
        return true;
    }
    
    // Business logic: Check operation mode
    uint8_t desiredOp = allowBattery 
        ? ChargingProfile::OperationMode::CLIMATE_ALLOW_BATTERY
        : ChargingProfile::OperationMode::CLIMATE_ONLY;
    
    if (profile0.operation != desiredOp) {
        Serial.printf("[ClimateManager] Operation mode changed: %d -> %d\r\n", 
                      profile0.operation, desiredOp);
        return true;
    }
    
    Serial.println("[ClimateManager] Profile 0 already has correct settings");
    return false;
}

void ClimateManager::completeCommand() {
    Serial.printf("[ClimateManager] Command %d completed successfully\r\n", pendingCommandId);
    
    // Reset state
    setCommandState(CommandState::IDLE);
    pendingCmdType = PendingCommandType::NONE;
    pendingCommandId = -1;
    
    // TODO: Emit event or call callback when we add event system
}

void ClimateManager::failCommand(const char* reason) {
    Serial.printf("[ClimateManager] Command %d failed: %s\r\n", pendingCommandId, reason);
    
    // Reset state
    setCommandState(CommandState::FAILED);
    
    // Give it a moment to log, then reset
    delay(10);
    setCommandState(CommandState::IDLE);
    pendingCmdType = PendingCommandType::NONE;
    pendingCommandId = -1;
    
    // TODO: Emit event or call callback when we add event system
}
