# BAP Protocol Architecture Refactoring Plan

## Executive Summary

This document outlines the refactoring plan for separating business logic from protocol handling in the BAP (Battery Control) implementation. The goal is to achieve clean separation of concerns while maintaining the profile-centric command execution pattern.

---

## Current Problems

### 1. Business Logic Leaks in BatteryControlChannel

**Violations Identified**:
- ❌ Command-specific decision making (operation mode selection, temperature tolerance)
- ❌ Profile management logic (deciding when profiles need updates)
- ❌ Wake state management
- ❌ Command event emission
- ❌ Composite commands (startChargingAndClimate)

**Evidence**: BatteryControlChannel.cpp lines 434-967 contain extensive business logic that should belong to domain managers.

### 2. Unclear Responsibility Boundaries

| Responsibility | Should Be | Currently In | Status |
|----------------|-----------|--------------|--------|
| Frame encoding/decoding | Protocol | BatteryControlChannel | ✅ Correct |
| Multi-frame assembly | Protocol | BatteryControlChannel | ✅ Correct |
| Wake coordination | Domain/Service | BatteryControlChannel | ❌ LEAK |
| Profile update decisions | Domain | BatteryControlChannel | ❌ LEAK |
| Operation mode selection | Domain | BatteryControlChannel | ❌ LEAK |
| Temperature tolerance | Domain | BatteryControlChannel | ❌ LEAK |

---

## Key Architectural Insights

### Profile-Based Command Pattern

Starting charging or climate control follows this pattern:

```
1. Update Profile 0 with desired configuration
   ↓ (temp, SOC, maxCurrent, operation mode)
   
2. Send "execute profile 0 now" command
   ↓ (function 0x18 OPERATION_MODE, SetGet opcode)
   
3. Car responds with HeartbeatStatus messages (0x03)
   ↓ (progress updates while car is working)
   
4. Car sends final Status response (0x04)
   ↓ (success or ERROR opcode 0x07)
```

**Critical Point**: This async pattern is specific to profile execution (function 0x18). No other BAP functions currently use this pattern.

### Stop Commands Are Different

Stop commands (`stopClimate()`, `stopCharging()`) are simpler:
- Send "stop profile 0" command (function 0x18)
- No profile update needed
- No complex validation needed
- Still async (wait for STATUS response)

---

## Proposed Architecture

### Layer Diagram

```
┌─────────────────────────────────────────────────────────────┐
│              Application Layer (Use Cases)                  │
│                    VehicleHandler                           │
│  • Composite operations (charging + climate)                │
│  • User-facing API                                          │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│         Domain Layer (Business Logic) - NEW APPROACH        │
│                                                             │
│  ┌──────────────────────┐       ┌──────────────────────┐   │
│  │  BatteryManager      │       │  ClimateManager      │   │
│  │                      │       │                      │   │
│  │ OWNS:                │       │ OWNS:                │   │
│  │ • Validation         │       │ • Validation         │   │
│  │ • Profile decisions  │       │ • Profile decisions  │   │
│  │ • Command state      │       │ • Command state      │   │
│  │   machine            │       │   machine            │   │
│  │ • Business rules     │       │ • Business rules     │   │
│  │                      │       │                      │   │
│  │ DELEGATES TO:        │       │ DELEGATES TO:        │   │
│  │ • ProfileManager     │       │ • ProfileManager     │   │
│  │   (update profile)   │       │   (update profile)   │   │
│  │ • ProfileManager     │       │ • ProfileManager     │   │
│  │   (execute profile)  │       │   (execute profile)  │   │
│  └──────────────────────┘       └──────────────────────┘   │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│      Data/Service Layer (Profile Operations)                │
│                                                             │
│              ChargingProfileManager                         │
│                                                             │
│  OWNS:                                                      │
│  • Profile data storage (4 profiles)                       │
│  • Profile update state machine                            │
│  • Profile execution state machine                         │
│  • BAP array encoding (function 0x19)                      │
│  • Operation mode tracking (function 0x18)                 │
│                                                             │
│  DOES NOT KNOW:                                            │
│  • When profile needs updating (domain decides)            │
│  • What operation mode to use (domain decides)             │
│  • Temperature tolerances (domain decides)                 │
│  • SOC/current limits (domain decides)                     │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│         Protocol Layer (BAP Message Transport)              │
│              BatteryControlChannel                          │
│                                                             │
│  OWNS:                                                      │
│  • Frame encode/decode                                      │
│  • Multi-frame assembly (BapFrameAssembler)                │
│  • Function-based message routing                          │
│  • CAN TX/RX                                                │
│  • Callback dispatch                                        │
│                                                             │
│  DOES NOT KNOW:                                            │
│  • Domain business logic                                   │
│  • Profile configuration                                   │
│  • Command orchestration                                   │
│  • Wake coordination                                       │
└─────────────────────────────────────────────────────────────┘
```

---

## Component Responsibilities

### BatteryControlChannel (Protocol Layer)

**KEEPS**:
- ✅ Frame encoding/decoding (`encodeShortMessage`, `encodeLongStart`, etc)
- ✅ Multi-frame assembly (`BapFrameAssembler`)
- ✅ Message routing by function ID
- ✅ Callback registration (`onPlugState`, `onChargeState`, `onClimateState`)
- ✅ Callback dispatch (notify subscribers)
- ✅ CAN TX/RX via VehicleManager

**REMOVES**:
- ❌ All command methods (`startClimate`, `stopClimate`, `startCharging`, `stopCharging`, `startChargingAndClimate`)
- ❌ Command state machine (`CommandState` enum, `updateCommandStateMachine`)
- ❌ Pending command storage (`pendingCommand`, `pendingTempCelsius`, etc)
- ❌ Profile update decisions (`needsProfileUpdate()`)
- ❌ Profile update coordination (`requestProfileUpdateForPendingCommand()`)
- ❌ Wake coordination (checking `isAwake()`, calling `requestWake()`)
- ❌ Command event emission (`emitCommandEvent()`)

**NEW SIMPLIFIED API**:
```cpp
class BatteryControlChannel : public BapChannel {
public:
    // === Protocol Methods ===
    bool sendBapMessage(uint8_t opcode, uint8_t functionId, 
                        const uint8_t* payload, uint8_t len);
    
    // === Callback Registration ===
    void onPlugState(PlugStateCallback callback);
    void onChargeState(ChargeStateCallback callback);
    void onClimateState(ClimateStateCallback callback);
    
    // === Frame Processing ===
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    bool processMessage(const BapMessage& msg) override;
    
private:
    BapFrameAssembler frameAssembler;
    std::vector<PlugStateCallback> plugStateCallbacks;
    std::vector<ChargeStateCallback> chargeStateCallbacks;
    std::vector<ClimateStateCallback> climateStateCallbacks;
};
```

---

### ChargingProfileManager (Data/Service Layer)

**KEEPS**:
- ✅ Profile data storage (4 profiles array)
- ✅ Profile access API (`getProfile()`, `isProfileValid()`)
- ✅ Profile array parsing (`processProfilesArray()` for function 0x19)
- ✅ Profile update state machine (existing read-modify-write workflow)

**ADDS**:
- ✅ Profile execution tracking (function 0x18 responses)
- ✅ Simple execution methods (send command + track response)
- ✅ HeartbeatStatus handling (progress updates)

**DOES NOT ADD**:
- ❌ Profile configuration decisions (domains decide)
- ❌ Wake coordination (domains handle)
- ❌ Business validation (domains handle)

**ENHANCED API**:
```cpp
class ChargingProfileManager {
public:
    // === Existing Profile Data API ===
    const ChargingProfile::Profile& getProfile(uint8_t index) const;
    Profile& getProfileMutable(uint8_t index);
    bool isProfileValid(uint8_t index) const;
    
    // === Existing Profile Update API ===
    bool requestProfileUpdate(
        uint8_t profileIndex,
        const ProfileFieldUpdate& updates,
        std::function<void(bool success)> callback
    );
    bool isUpdateInProgress() const;
    
    // === NEW: Profile Execution API ===
    
    /**
     * Execute profile 0 immediately.
     * Sends function 0x18 OPERATION_MODE with "execute now" command.
     * Tracks async response (HeartbeatStatus → Status).
     * 
     * Caller MUST ensure:
     * - Vehicle is awake (or wake is in progress)
     * - Profile 0 is configured correctly (or update is queued)
     * 
     * @param callback Called when car responds (success/failure)
     * @return true if queued, false if already executing
     */
    bool executeProfile0(std::function<void(bool success, const char* error)> callback);
    
    /**
     * Stop profile 0 execution.
     * Sends function 0x18 OPERATION_MODE with "stop" command.
     * 
     * @param callback Called when car responds
     * @return true if queued, false if already busy
     */
    bool stopProfile0(std::function<void(bool success, const char* error)> callback);
    
    /**
     * Check if profile execution is in progress
     */
    bool isExecutionInProgress() const;
    
    /**
     * Process OPERATION_MODE response (function 0x18).
     * Called by BatteryControlChannel when receiving:
     * - HeartbeatStatus (0x03) - car is working
     * - Status (0x04) - operation complete
     * - Error (0x07) - operation failed
     */
    void processOperationModeResponse(const BapMessage& msg);
    
    // === State Machine (called from VehicleManager::loop()) ===
    void loop();
    
private:
    // Profile storage
    ChargingProfile::Profile profiles[4];
    
    // Profile update state machine (existing)
    ProfileUpdateState updateState;
    // ... existing update fields
    
    // Profile execution state machine (NEW)
    enum class ExecutionState {
        IDLE,
        SENDING_EXECUTE,      // Sent execute command
        WAITING_RESPONSE,     // Waiting for STATUS
        SENDING_STOP,         // Sent stop command
        DONE,
        FAILED
    };
    
    ExecutionState execState = ExecutionState::IDLE;
    unsigned long execStateStartTime = 0;
    std::function<void(bool, const char*)> execCallback;
    uint8_t heartbeatCount = 0;
    
    static constexpr unsigned long EXECUTION_TIMEOUT = 30000;  // 30s
    
    void updateExecutionStateMachine();
    bool sendExecuteCommand();
    bool sendStopCommand();
};
```

---

### BatteryManager (Domain Layer)

**OWNS**:
- ✅ Charging business logic and validation
- ✅ Profile configuration decisions (what values to set)
- ✅ Command state machine (orchestrates multi-step process)
- ✅ Wake coordination (checks/requests wake)
- ✅ Event emission (command lifecycle events)

**DOES NOT OWN**:
- ❌ BAP protocol details
- ❌ Profile encoding/decoding
- ❌ CAN frame handling

**STATE MACHINE PATTERN**:
```cpp
class BatteryManager : public IDomain {
public:
    bool startCharging(int commandId, uint8_t targetSoc, uint8_t maxCurrent);
    bool stopCharging(int commandId);
    
    void loop() override;  // Process command state machine
    
private:
    enum class CommandState {
        IDLE,
        REQUESTING_WAKE,        // Waiting for vehicle wake
        UPDATING_PROFILE,       // Profile update in progress
        EXECUTING_COMMAND,      // Profile execution in progress
        DONE,
        FAILED
    };
    
    CommandState cmdState = CommandState::IDLE;
    unsigned long cmdStateStartTime = 0;
    
    // Pending command data
    enum class PendingCommandType {
        NONE,
        START_CHARGING,
        STOP_CHARGING
    };
    
    PendingCommandType pendingCmdType = PendingCommandType::NONE;
    int pendingCommandId = -1;
    uint8_t pendingTargetSoc = 80;
    uint8_t pendingMaxCurrent = 32;
    
    // Timeouts
    static constexpr unsigned long WAKE_TIMEOUT = 15000;
    static constexpr unsigned long PROFILE_TIMEOUT = 5000;
    
    // State machine methods
    void updateCommandStateMachine();
    void setCommandState(CommandState newState);
    
    // Business logic methods
    bool validateChargingParams(uint8_t targetSoc, uint8_t maxCurrent);
    bool needsProfileUpdate(uint8_t targetSoc, uint8_t maxCurrent);
    ChargingProfileManager::ProfileFieldUpdate buildProfileUpdate(uint8_t targetSoc, uint8_t maxCurrent);
    
    // Services
    ChargingProfileManager* profileManager;
    WakeController* wakeController;
};
```

**EXAMPLE FLOW - startCharging()**:
```cpp
bool BatteryManager::startCharging(int commandId, uint8_t targetSoc, uint8_t maxCurrent) {
    // 1. Check busy
    if (cmdState != CommandState::IDLE) {
        return false;  // Already executing command
    }
    
    // 2. Validate (business logic)
    if (!validateChargingParams(targetSoc, maxCurrent)) {
        return false;
    }
    
    // 3. Store command
    pendingCmdType = PendingCommandType::START_CHARGING;
    pendingCommandId = commandId;
    pendingTargetSoc = targetSoc;
    pendingMaxCurrent = maxCurrent;
    
    // 4. Start state machine
    if (!wakeController->isAwake()) {
        setCommandState(CommandState::REQUESTING_WAKE);
    } else if (needsProfileUpdate(targetSoc, maxCurrent)) {
        setCommandState(CommandState::UPDATING_PROFILE);
    } else {
        setCommandState(CommandState::EXECUTING_COMMAND);
    }
    
    return true;
}

void BatteryManager::updateCommandStateMachine() {
    switch (cmdState) {
        case CommandState::REQUESTING_WAKE:
            if (wakeController->requestWake()) {
                // Wait for wake (checked in next loop)
            }
            if (wakeController->isAwake()) {
                // Awake! Check profile
                if (needsProfileUpdate()) {
                    setCommandState(CommandState::UPDATING_PROFILE);
                } else {
                    setCommandState(CommandState::EXECUTING_COMMAND);
                }
            } else if (timeout(WAKE_TIMEOUT)) {
                failCommand("wake_timeout");
            }
            break;
            
        case CommandState::UPDATING_PROFILE:
            auto update = buildProfileUpdate(pendingTargetSoc, pendingMaxCurrent);
            
            if (profileManager->requestProfileUpdate(0, update, [this](bool ok) {
                if (ok) {
                    setCommandState(CommandState::EXECUTING_COMMAND);
                } else {
                    failCommand("profile_update_failed");
                }
            })) {
                // Waiting for callback
            } else {
                failCommand("profile_update_busy");
            }
            break;
            
        case CommandState::EXECUTING_COMMAND:
            if (profileManager->executeProfile0([this](bool ok, const char* err) {
                if (ok) {
                    completeCommand();
                } else {
                    failCommand(err);
                }
            })) {
                // Waiting for callback
            } else {
                failCommand("execution_busy");
            }
            break;
    }
}
```

---

### ClimateManager (Domain Layer)

**Similar pattern to BatteryManager**, but with climate-specific logic:

```cpp
class ClimateManager : public IDomain {
private:
    // Climate-specific business logic
    bool validateClimateParams(float tempCelsius) {
        if (tempCelsius < 15.5f || tempCelsius > 30.0f) {
            return false;  // Outside valid range
        }
        return true;
    }
    
    bool needsProfileUpdate(float tempCelsius, bool allowBattery) {
        const auto& profile0 = profileManager->getProfile(0);
        
        // Business rule: 0.5°C tolerance
        if (abs(profile0.getTemperature() - tempCelsius) > 0.5f) {
            return true;
        }
        
        // Business rule: Check operation mode
        uint8_t desiredOp = allowBattery 
            ? ChargingProfile::OperationMode::CLIMATE_ALLOW_BATTERY
            : ChargingProfile::OperationMode::CLIMATE_ONLY;
        
        if (profile0.operation != desiredOp) {
            return true;
        }
        
        return false;
    }
};
```

---

## Refactoring Phases

### Phase 1: Enhance ChargingProfileManager (Foundation) 🟢

**Goal**: Add profile execution tracking to ChargingProfileManager without breaking existing code.

**Steps**:
1. Add `ExecutionState` enum and state machine fields
2. Add `executeProfile0()` and `stopProfile0()` methods
3. Add `processOperationModeResponse()` method
4. Add execution state machine to `loop()`
5. Update BatteryControlChannel to forward function 0x18 messages to ProfileManager

**Files**:
- MODIFY: `ChargingProfileManager.h` (add execution API)
- MODIFY: `ChargingProfileManager.cpp` (implement execution state machine)
- MODIFY: `BatteryControlChannel.cpp` (forward 0x18 messages)

**Testing**:
- Existing commands still work through BatteryControlChannel
- ProfileManager can track execution state

**Estimated Effort**: 4-5 hours  
**Risk**: Low (additive, old code path still works)

---

### Phase 2: Add Domain State Machines 🟡

**Goal**: Move command orchestration from BatteryControlChannel to domain managers.

**Steps**:
1. Add `CommandState` enum to BatteryManager
2. Add `updateCommandStateMachine()` to BatteryManager
3. Implement new `startCharging()` that uses ProfileManager
4. Repeat for ClimateManager
5. Keep old BatteryControlChannel methods working (parallel paths)

**Files**:
- MODIFY: `BatteryManager.h` (add command state machine)
- MODIFY: `BatteryManager.cpp` (implement orchestration)
- MODIFY: `ClimateManager.h` (add command state machine)
- MODIFY: `ClimateManager.cpp` (implement orchestration)

**Testing**:
- New path through domains works
- Old path through BatteryControlChannel still works
- Both paths produce same results

**Estimated Effort**: 5-6 hours  
**Risk**: Medium (new state machines, need thorough testing)

---

### Phase 3: Switch VehicleHandler to Domains 🟡

**Goal**: Update application layer to use domain managers instead of BatteryControlChannel.

**Steps**:
1. Update `VehicleHandler::startClimate()` to call `climateManager->startClimate()`
2. Update `VehicleHandler::stopClimate()` to call `climateManager->stopClimate()`
3. Update `VehicleHandler::startCharging()` to call `batteryManager->startCharging()`
4. Update `VehicleHandler::stopCharging()` to call `batteryManager->stopCharging()`
5. Remove `startChargingAndClimate()` (implement as separate calls or profile config)

**Files**:
- MODIFY: `VehicleHandler.cpp` (call domains instead of channel)

**Testing**:
- All commands work through new path
- Old path through BatteryControlChannel is unused but still present

**Estimated Effort**: 2-3 hours  
**Risk**: Low (simple API switch)

---

### Phase 4: Cleanup BatteryControlChannel 🟢

**Goal**: Remove all command logic from BatteryControlChannel, make it a pure protocol layer.

**Steps**:
1. Remove all command methods from BatteryControlChannel
2. Remove CommandState enum and state machine
3. Remove pending command storage
4. Remove profile decision logic
5. Remove wake coordination
6. Remove event emission
7. Add simple `sendBapMessage()` API

**Files**:
- MODIFY: `BatteryControlChannel.h` (remove command API)
- MODIFY: `BatteryControlChannel.cpp` (remove state machine)

**Testing**:
- Ensure all commands still work through domains
- Verify protocol methods still function

**Estimated Effort**: 2-3 hours  
**Risk**: Low (cleanup, functionality already moved)

---

## Key Design Decisions

### 1. Domain State Machines

**Decision**: Each domain (BatteryManager, ClimateManager) has its own command state machine.

**Rationale**:
- Domains know their specific business logic
- Some code duplication is acceptable
- Simpler than shared orchestrator
- Each domain can evolve independently

**Trade-off**: Duplicate wake/profile coordination logic, but gains domain-specific control.

---

### 2. Profile Update Responsibility

**Decision**: Domains decide when profile needs updating, ProfileManager executes the update.

**API Flow**:
```
Domain: Check if profile needs update (business logic)
  ↓
Domain: Build ProfileFieldUpdate with changes
  ↓
Domain: Call profileManager->requestProfileUpdate()
  ↓
ProfileManager: Execute read-modify-write
  ↓
ProfileManager: Callback to domain when done
  ↓
Domain: Proceed to executeProfile0()
```

**Rationale**: 
- Temperature tolerance (0.5°C) is business logic → stays in domain
- SOC/current comparison is business logic → stays in domain
- BAP encoding/array headers are protocol → stays in ProfileManager

---

### 3. Wake Coordination

**Decision**: Domains handle wake coordination in their state machines.

**Pattern**:
```cpp
if (!wakeController->isAwake()) {
    wakeController->requestWake();
    cmdState = CommandState::REQUESTING_WAKE;
    return;
}

// In state machine loop
if (cmdState == CommandState::REQUESTING_WAKE) {
    if (wakeController->isAwake()) {
        // Proceed to next step
    } else if (timeout()) {
        // Fail with wake_timeout
    }
}
```

**Rationale**:
- Wake is a prerequisite for ALL commands (not profile-specific)
- Domains already have state machines for orchestration
- WakeController is a shared service (already exists)

**Future**: If wake coordination becomes complex, extract to WakeOrchestrator service.

---

### 4. Stop Commands

**Decision**: Stop commands go through ProfileManager for consistency.

**Flow**:
```
Domain: stopCharging()
  ↓
Domain: Check wake (simple, usually already awake)
  ↓
Domain: Call profileManager->stopProfile0()
  ↓
ProfileManager: Send stop command (function 0x18)
  ↓
ProfileManager: Track STATUS response
  ↓
ProfileManager: Callback to domain
```

**Rationale**:
- Consistent interface (all profile operations through ProfileManager)
- ProfileManager can validate execution state
- Still tracks async response properly

---

### 5. HeartbeatStatus Handling

**Decision**: ChargingProfileManager tracks HeartbeatStatus internally, doesn't expose to domains.

**Implementation**:
```cpp
void ChargingProfileManager::processOperationModeResponse(const BapMessage& msg) {
    if (execState != ExecutionState::WAITING_RESPONSE) {
        return;  // Not expecting response
    }
    
    switch (msg.opcode) {
        case OpCode::HEARTBEAT:
            heartbeatCount++;
            Serial.printf("[ProfileMgr] Execution heartbeat %d\n", heartbeatCount);
            // Just tracking, don't notify domain
            break;
            
        case OpCode::STATUS:
            // Success!
            onExecutionComplete(true, nullptr);
            break;
            
        case OpCode::ERROR:
            // Failure
            onExecutionComplete(false, "car_rejected");
            break;
    }
}
```

**Rationale**:
- HeartbeatStatus is just progress indication
- Domains only care about final result
- Can add progress callbacks later if UI needs them

---

## Testing Strategy

### Unit Testing

**BatteryControlChannel**:
- Frame encoding/decoding
- Multi-frame assembly
- Message routing to correct callbacks
- Does NOT test command logic (removed)

**ChargingProfileManager**:
- Profile data storage/retrieval
- Profile update state machine
- Profile execution state machine
- BAP array encoding/decoding
- OperationMode response handling

**Domain Managers**:
- Validation logic
- Profile update decisions (needsProfileUpdate)
- Command state machine transitions
- Error handling

### Integration Testing

**End-to-End Command Flow**:
1. Call `batteryManager->startCharging(80, 32)`
2. Verify wake requested (if needed)
3. Verify profile update requested (if needed)
4. Verify profile execution requested
5. Simulate car STATUS response
6. Verify completion callback

**Real Vehicle Testing**:
- Test with actual vehicle on CAN bus
- Verify charging starts/stops correctly
- Verify climate starts/stops correctly
- Monitor for BAP protocol errors
- Check timing (wake, profile update, execution)

---

## Open Questions

### 1. Wake Service Enhancement

**Current**: Domains call `wakeController->requestWake()` and poll `isAwake()`.

**Future**: Should we have a wake orchestration service that handles:
- Multiple concurrent wake requests
- Wake callbacks
- Wake timeout handling
- Wake keep-alive

**Recommendation**: Address in separate refactoring after this is complete.

---

### 2. Error Handling Granularity

**Current Errors**:
- `wake_timeout`
- `profile_update_failed`
- `profile_update_timeout`
- `execution_busy`
- `send_failed`
- `car_rejected`

**Question**: Do we need more granular error codes, or is string message sufficient?

**Recommendation**: Start with string messages, add error codes if needed for programmatic handling.

---

### 3. Command Event Emission

**Current**: BatteryControlChannel emits events via CommandRouter.

**Question**: Should domains emit events, or should this be in VehicleHandler?

**Recommendation**: 
- Domains emit domain events (`battery.chargingStarted`, `climate.climateStarted`)
- VehicleHandler emits command events (`command.completed`, `command.failed`)

---

### 4. Composite Commands

**Current**: `startChargingAndClimate()` exists in BatteryControlChannel.

**Options**:
- **Option A**: Remove entirely, implement as two separate calls
- **Option B**: Profile 0 supports combined operation (CHARGING_AND_CLIMATE mode)
- **Option C**: VehicleHandler coordinates sequential calls

**Recommendation**: Option B - one profile execution with CHARGING_AND_CLIMATE operation mode.

---

## Success Criteria

### Clean Separation Achieved

- ✅ BatteryControlChannel has NO business logic
- ✅ Domains own ALL business logic and validation
- ✅ ChargingProfileManager is domain-agnostic (doesn't know about SOC limits, temp ranges, etc)
- ✅ Clear API boundaries between layers

### Functionality Preserved

- ✅ All commands work correctly (start/stop climate, start/stop charging)
- ✅ Profile updates happen when needed
- ✅ Wake coordination works
- ✅ Async tracking works (HeartbeatStatus → Status)
- ✅ Error handling works

### Code Quality Improved

- ✅ No duplicate code in protocol layer
- ✅ Some acceptable duplication in domain state machines
- ✅ Clear responsibility boundaries
- ✅ Easier to test each layer independently
- ✅ Easier to add new BAP functions or domains

---

## Timeline Estimate

| Phase | Description | Effort | Risk |
|-------|-------------|--------|------|
| Phase 1 | Enhance ProfileManager | 4-5 hours | Low |
| Phase 2 | Add domain state machines | 5-6 hours | Medium |
| Phase 3 | Switch VehicleHandler | 2-3 hours | Low |
| Phase 4 | Cleanup BatteryControlChannel | 2-3 hours | Low |
| **Total** | | **13-17 hours** | **Low-Medium** |

**Recommendation**: Execute in phases with testing between each phase. Total calendar time: 2-3 days with proper testing.

---

## Appendix: Code Snippets

### A. Simplified BatteryControlChannel (After Refactoring)

```cpp
class BatteryControlChannel : public BapChannel {
public:
    BatteryControlChannel(VehicleManager* mgr);
    
    // === BapChannel Interface ===
    uint8_t getDeviceId() const override { return DEVICE_ID; }
    uint32_t getTxCanId() const override { return CAN_ID_TX; }
    uint32_t getRxCanId() const override { return CAN_ID_RX; }
    bool handlesCanId(uint32_t canId) const override;
    const char* getName() const override { return "BatteryControl"; }
    bool processMessage(const BapMessage& msg) override;
    
    // === Callback Registration ===
    void onPlugState(PlugStateCallback callback);
    void onChargeState(ChargeStateCallback callback);
    void onClimateState(ClimateStateCallback callback);
    
    // === Frame Processing ===
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    // === Protocol Sending ===
    bool sendBapMessage(uint8_t opcode, uint8_t functionId,
                        const uint8_t* payload, uint8_t len);
    
private:
    VehicleManager* manager;
    BapProtocol::BapFrameAssembler frameAssembler;
    
    std::vector<PlugStateCallback> plugStateCallbacks;
    std::vector<ChargeStateCallback> chargeStateCallbacks;
    std::vector<ClimateStateCallback> climateStateCallbacks;
    
    void processPlugState(const uint8_t* payload, uint8_t len);
    void processChargeState(const uint8_t* payload, uint8_t len);
    void processClimateState(const uint8_t* payload, uint8_t len);
    
    void notifyPlugStateCallbacks(const PlugState& plugData);
    void notifyChargeStateCallbacks(const BatteryState& batteryData);
    void notifyClimateStateCallbacks(const ClimateState& climateData);
};
```

### B. Domain Manager State Machine Pattern

```cpp
class BatteryManager : public IDomain {
private:
    enum class CommandState {
        IDLE,
        REQUESTING_WAKE,
        UPDATING_PROFILE,
        EXECUTING_COMMAND,
        DONE,
        FAILED
    };
    
    void updateCommandStateMachine() {
        switch (cmdState) {
            case CommandState::REQUESTING_WAKE:
                handleWakeState();
                break;
            case CommandState::UPDATING_PROFILE:
                handleProfileUpdateState();
                break;
            case CommandState::EXECUTING_COMMAND:
                handleExecutionState();
                break;
            // ...
        }
    }
    
    void handleWakeState() {
        if (wakeController->isAwake()) {
            if (needsProfileUpdate()) {
                setCommandState(CommandState::UPDATING_PROFILE);
            } else {
                setCommandState(CommandState::EXECUTING_COMMAND);
            }
        } else if (timeout(WAKE_TIMEOUT)) {
            failCommand("wake_timeout");
        }
    }
    
    void handleProfileUpdateState() {
        auto update = buildProfileUpdate();
        if (profileManager->requestProfileUpdate(0, update, [this](bool ok) {
            if (ok) {
                setCommandState(CommandState::EXECUTING_COMMAND);
            } else {
                failCommand("profile_update_failed");
            }
        })) {
            // Waiting for callback
        } else {
            failCommand("profile_update_busy");
        }
    }
    
    void handleExecutionState() {
        if (profileManager->executeProfile0([this](bool ok, const char* err) {
            if (ok) {
                completeCommand();
            } else {
                failCommand(err);
            }
        })) {
            // Waiting for callback
        } else {
            failCommand("execution_busy");
        }
    }
};
```

---

*Document Version: 1.0*  
*Last Updated: 2026-01-31*  
*Author: Architecture Review*
