# BAP Architecture Refactoring - Progress Tracker

**Started:** 2026-01-31  
**Current Phase:** Phase 1 ✅ COMPLETE  
**Next Phase:** Phase 2 (Add Domain State Machines)

---

## Overview

This document tracks progress on the BAP architecture refactoring plan detailed in `BAP_ARCHITECTURE_REFACTORING_PLAN.md`.

**Goal:** Separate business logic from protocol handling by moving command orchestration from BatteryControlChannel to domain managers (BatteryManager, ClimateManager).

---

## Phase Status

### ✅ Phase 1: Enhance ChargingProfileManager (COMPLETE)
**Status:** Complete - 2026-01-31  
**Commit:** `3e36a38` - feat(bap): Phase 1 - Add profile execution to ChargingProfileManager  
**Duration:** ~3 hours  
**Build Status:** ✅ SUCCESS (440KB flash, 20KB RAM)

**What Was Done:**
1. ✅ Added `ExecutionState` enum and state machine fields to ChargingProfileManager
2. ✅ Implemented `executeProfile0()` method for sending execute commands
3. ✅ Implemented `stopProfile0()` method for sending stop commands
4. ✅ Implemented `processOperationModeResponse()` for handling async responses
5. ✅ Added execution state machine to `loop()` method
6. ✅ Updated BatteryControlChannel to forward function 0x18 messages to ProfileManager
7. ✅ Build verified - existing commands continue to work

**Key Changes:**
- ChargingProfileManager.h: +86 lines (new API methods)
- ChargingProfileManager.cpp: +176 lines (state machine implementation)
- BatteryControlChannel.cpp: +18 lines (message forwarding)

**New API:**
```cpp
// Profile execution methods
bool executeProfile0(std::function<void(bool, const char*)> callback);
bool stopProfile0(std::function<void(bool, const char*)> callback);
bool isExecutionInProgress() const;
void processOperationModeResponse(const BapProtocol::BapMessage& msg);

// State machine
enum class ExecutionState {
    IDLE, SENDING_EXECUTE, WAITING_RESPONSE,
    SENDING_STOP, DONE, FAILED
};
```

**Testing:**
- ✅ Compiles successfully
- ⏳ Real vehicle testing pending (Phase 1+2 together recommended)

---

### ⏳ Phase 2: Add Domain State Machines
**Status:** Not started  
**Estimated Duration:** 5-6 hours  
**Target:** Next session

**Plan:**
1. Add `CommandState` enum to BatteryManager
2. Add `updateCommandStateMachine()` to BatteryManager
3. Implement new `startCharging()` that uses ProfileManager
4. Implement new `stopCharging()` that uses ProfileManager
5. Repeat for ClimateManager (startClimate, stopClimate)
6. Keep old BatteryControlChannel methods working (parallel paths)

**Files to Modify:**
- `src/vehicle/domains/BatteryManager.h` (add command state machine)
- `src/vehicle/domains/BatteryManager.cpp` (implement orchestration)
- `src/vehicle/domains/ClimateManager.h` (add command state machine)
- `src/vehicle/domains/ClimateManager.cpp` (implement orchestration)

**Success Criteria:**
- New path through domains works
- Old path through BatteryControlChannel still works
- Both paths produce same results

---

### ⏳ Phase 3: Switch VehicleHandler to Domains
**Status:** Not started  
**Estimated Duration:** 2-3 hours  
**Prerequisites:** Phase 2 complete

**Plan:**
1. Update `VehicleHandler::startClimate()` to call `climateManager->startClimate()`
2. Update `VehicleHandler::stopClimate()` to call `climateManager->stopClimate()`
3. Update `VehicleHandler::startCharging()` to call `batteryManager->startCharging()`
4. Update `VehicleHandler::stopCharging()` to call `batteryManager->stopCharging()`
5. Handle `startChargingAndClimate()` appropriately

**Files to Modify:**
- `src/handlers/VehicleHandler.cpp` (call domains instead of channel)

---

### ⏳ Phase 4: Cleanup BatteryControlChannel
**Status:** Not started  
**Estimated Duration:** 2-3 hours  
**Prerequisites:** Phase 3 complete

**Plan:**
1. Remove all command methods from BatteryControlChannel
2. Remove CommandState enum and state machine
3. Remove pending command storage
4. Remove profile decision logic
5. Remove wake coordination
6. Remove event emission
7. Add simple `sendBapMessage()` API

**Result:** Pure protocol layer (encode/decode/callbacks only)

---

## Overall Progress

| Phase | Status | Effort | Risk |
|-------|--------|--------|------|
| **Phase 1** | ✅ COMPLETE | 3h | Low |
| **Phase 2** | ⏳ Pending | 5-6h | Medium |
| **Phase 3** | ⏳ Pending | 2-3h | Low |
| **Phase 4** | ⏳ Pending | 2-3h | Low |
| **TOTAL** | **25%** | **12-15h** | **Medium** |

**Current Completion:** 25% (Phase 1 of 4)  
**Remaining Effort:** ~10-12 hours

---

## Design Decisions Made

### 1. Profile Execution in ChargingProfileManager ✅
**Decision:** Profile execution tracking lives in ChargingProfileManager, not domains.

**Rationale:**
- Function 0x18 (OPERATION_MODE) is profile-specific
- Async response tracking (HeartbeatStatus → Status) is protocol concern
- Domains don't need to know about HeartbeatStatus messages
- Consistent interface for all profile operations

### 2. Execution State Machine Pattern ✅
**Implementation:**
```cpp
IDLE → SENDING_EXECUTE → WAITING_RESPONSE → DONE/FAILED
       SENDING_STOP    → WAITING_RESPONSE → DONE/FAILED
```

**Features:**
- 30-second timeout for execution
- HeartbeatStatus tracking (progress indication)
- Status response = success
- Error response = failure
- Callback-based async completion

### 3. Message Forwarding ✅
**Pattern:** BatteryControlChannel forwards function 0x18 responses to ProfileManager
- HeartbeatStatus (0x03) → progress tracking
- Status (0x04) → success
- Error (0x07) → failure

### 4. Backward Compatibility ✅
**Approach:** Additive changes only in Phase 1
- Old command methods in BatteryControlChannel still work
- New execution API available but not required
- No breaking changes to existing code

---

## Known Issues

### None Currently
All Phase 1 changes compile and build successfully.

---

## Next Steps

### Immediate (Next Session)
1. **Start Phase 2** - Add domain state machines to BatteryManager and ClimateManager
2. **Test Phase 1+2 together** - New orchestration path through domains
3. **Keep parallel paths working** - Both old and new command paths functional

### Testing Strategy (After Phase 2)
1. Test new domain path: `vehicleHandler → domain → profileManager`
2. Verify old path still works: `vehicleHandler → batteryControlChannel`
3. Compare results (both should behave identically)
4. Deploy to vehicle for real-world testing

### Future Work (Phase 3+4)
1. Switch VehicleHandler to use domains exclusively
2. Clean up BatteryControlChannel (remove command logic)
3. Update documentation
4. Final vehicle testing

---

## Architecture Goals (Reminder)

### Current (Before Refactoring)
```
VehicleHandler → BatteryControlChannel
                  ├─ Profile decisions (business logic ❌)
                  ├─ Wake coordination (business logic ❌)
                  ├─ Command state machine (business logic ❌)
                  └─ Protocol encoding/decoding ✅
```

### Target (After Refactoring)
```
VehicleHandler → BatteryManager / ClimateManager
                  ├─ Profile decisions (business logic ✅)
                  ├─ Wake coordination (business logic ✅)
                  ├─ Command state machine (business logic ✅)
                  └─ ProfileManager → BatteryControlChannel
                                       └─ Protocol encoding/decoding ✅
```

---

## Session Log

### Session 1: 2026-01-31
- **Duration:** ~3 hours
- **Work Completed:** Phase 1 implementation
- **Commits:**
  - `3e36a38` - Phase 1 complete
  - `1a8b3be` - Documentation updates
  - `ded33fe` - VehicleState.h deletion
  - `0cc8c4c` - VehicleState.h removal (VehicleTypes.h)
  - `8099a11` - RTC memory persistence
- **Build Status:** ✅ SUCCESS
- **Next:** Phase 2 - Domain state machines

---

**For next session: Read this document and `BAP_ARCHITECTURE_REFACTORING_PLAN.md` to resume work on Phase 2.**
