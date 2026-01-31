# BAP Refactoring Plan - Discussion Points

**Date:** 2026-01-31  
**Status:** Discussion Phase - Before Implementation

---

## Current State Analysis

### What BatteryControlChannel Does Now

Looking at the current implementation (`BatteryControlChannel.cpp`), the channel has **significant business logic**:

#### 1. **Command Orchestration** (Lines 434-803)
- Command state machine with 7 states
- Wake coordination logic
- Profile update decision making
- Command queuing and execution

#### 2. **Profile Management Logic** (Lines 805-890)
- `needsProfileUpdate()` - Decides when profile needs updating
- Temperature tolerance checking (0.5°C threshold) ← **Business logic**
- SOC/current comparison ← **Business logic**
- Operation mode selection ← **Business logic**

#### 3. **Command Parameters & Validation** (Lines 434-638)
- Stores pending command data
- Validates parameters (implicit in the code)
- Manages command IDs for tracking

#### 4. **Event Emission** (Throughout)
- Emits command events via `CommandRouter` singleton
- Progress tracking with `commandId`

---

## The Problem: Business Logic in Protocol Layer

### Example 1: Temperature Tolerance (Line 819)
```cpp
// This is a BUSINESS DECISION, not protocol handling:
if (abs(profile0.getTemperature() - pendingTempCelsius) > 0.5f) {
    return true;  // Need profile update
}
```

**Why it's wrong:**
- The 0.5°C tolerance is a **business rule** specific to climate control
- Protocol layer shouldn't know about temperature tolerances
- ClimateManager should make this decision

### Example 2: Operation Mode Selection (Lines 810-811)
```cpp
uint8_t desiredOp = pendingAllowBattery ? ProfileOperation::CLIMATE_ALLOW_BATTERY 
                                        : ProfileOperation::CLIMATE;
```

**Why it's wrong:**
- Deciding which operation mode to use is **business logic**
- Protocol layer shouldn't know about `allowBattery` semantics
- ClimateManager should decide the operation mode

### Example 3: Wake Coordination (Lines 682-733)
```cpp
case CommandState::REQUESTING_WAKE:
    if (manager->requestWake()) {
        setCommandState(CommandState::WAITING_FOR_WAKE);
    }
    // ... wake timeout handling
```

**Why it's wrong:**
- Wake is not specific to BAP protocol
- All commands (across all channels) need wake coordination
- Should be handled by domain managers or a shared service

---

## The Proposed Solution

### Layer Responsibilities (After Refactoring)

```
┌─────────────────────────────────────────────────────┐
│ APPLICATION LAYER                                   │
│ VehicleHandler                                      │
│ - User-facing API                                   │
│ - HTTP/MQTT request handling                        │
└──────────────────┬──────────────────────────────────┘
                   ↓
┌─────────────────────────────────────────────────────┐
│ DOMAIN LAYER (Business Logic)                       │
│                                                     │
│ BatteryManager          ClimateManager              │
│ ✓ Validation            ✓ Validation                │
│ ✓ Profile decisions     ✓ Profile decisions         │
│ ✓ Command state         ✓ Command state             │
│ ✓ Business rules        ✓ Business rules            │
│   (SOC limits)            (temp tolerance)          │
│                                                     │
│ Delegates to:           Delegates to:               │
│ → ProfileManager        → ProfileManager            │
│ → WakeController        → WakeController            │
└──────────────────┬──────────────────────────────────┘
                   ↓
┌─────────────────────────────────────────────────────┐
│ SERVICE LAYER (Profile Operations)                  │
│                                                     │
│ ChargingProfileManager                              │
│ ✓ Profile data storage                             │
│ ✓ Profile update state machine                     │
│ ✓ Profile execution state machine (NEW)            │
│ ✓ BAP array encoding                               │
│                                                     │
│ Does NOT know:                                     │
│ ✗ When to update (domain decides)                  │
│ ✗ What operation mode (domain decides)              │
│ ✗ Temperature tolerance (domain decides)            │
└──────────────────┬──────────────────────────────────┘
                   ↓
┌─────────────────────────────────────────────────────┐
│ PROTOCOL LAYER (Message Transport)                  │
│                                                     │
│ BatteryControlChannel                               │
│ ✓ Frame encode/decode                              │
│ ✓ Multi-frame assembly                             │
│ ✓ Function-based routing                           │
│ ✓ Callback dispatch                                │
│                                                     │
│ Does NOT know:                                     │
│ ✗ Business logic                                   │
│ ✗ Profile configuration                            │
│ ✗ Command orchestration                            │
│ ✗ Wake coordination                                │
└─────────────────────────────────────────────────────┘
```

---

## Discussion Questions

### Question 1: Incremental vs. Big Bang Approach

**Option A: Incremental (Proposed - 4 Phases)**
- Phase 1: Add execution to `ChargingProfileManager` (4-5h)
- Phase 2: Add state machines to domains (5-6h)
- Phase 3: Switch VehicleHandler to domains (2-3h)
- Phase 4: Clean up BatteryControlChannel (2-3h)

**Option B: Big Bang**
- Refactor everything at once
- Faster but riskier
- No intermediate checkpoint

**My Recommendation:** Option A (Incremental)
- Can test each phase independently
- Easy to rollback if issues
- Parallel paths work during migration

**Your thoughts?**

---

### Question 2: Code Duplication in Domain State Machines

The plan accepts **some code duplication** between BatteryManager and ClimateManager:

```cpp
// Both managers will have similar wake coordination:
if (!wakeController->isAwake()) {
    wakeController->requestWake();
    cmdState = CommandState::REQUESTING_WAKE;
}
```

**Alternative:** Create a `CommandOrchestrator` base class to avoid duplication

**Trade-offs:**
- **Accept duplication:** Simpler, each domain independent, easier to customize
- **Shared orchestrator:** DRY principle, but adds coupling, harder to customize per-domain

**Plan recommends:** Accept duplication for now, extract later if patterns emerge

**Do you agree with this approach?**

---

### Question 3: Profile Execution State Machine Location

The plan proposes adding execution tracking to `ChargingProfileManager`:

```cpp
class ChargingProfileManager {
    // NEW: Profile execution state machine
    enum class ExecutionState {
        IDLE, SENDING_EXECUTE, WAITING_RESPONSE, DONE, FAILED
    };
    
    bool executeProfile0(std::function<void(bool, const char*)> callback);
    void processOperationModeResponse(const BapMessage& msg);  // HeartbeatStatus → Status
};
```

**Why here?**
- Profile execution is tightly coupled to profile data
- Function 0x18 (OPERATION_MODE) is profile-specific
- Async response tracking (HeartbeatStatus messages)

**Alternative:** Put execution in domain managers (BatteryManager, ClimateManager)

**Your preference?**

---

### Question 4: Event Emission Location

Currently `BatteryControlChannel` emits events via `CommandRouter`:

```cpp
emitCommandEvent("commandCompleted", nullptr);
emitCommandEvent("commandFailed", "wake_timeout");
```

**After refactoring, where should events be emitted?**

**Option A:** Domain managers emit events
- Pro: Domains own the business logic
- Con: Need to add event system to each domain

**Option B:** VehicleHandler emits events
- Pro: Application layer owns user-facing concerns
- Con: VehicleHandler needs to track command completion

**Option C:** Both
- Domain events: `battery.chargingStarted`, `climate.climateStarted`
- Command events: `command.completed`, `command.failed`

**My Recommendation:** Option C (layered events)

**Your thoughts?**

---

### Question 5: Composite Commands (startChargingAndClimate)

Current implementation has `startChargingAndClimate()` that sends a combined command.

**Options:**

**A) Keep combined command** (using `CHARGING_AND_CLIMATE` operation mode)
- Single profile execution
- Matches vehicle behavior
- Less complex

**B) Sequential calls** (start climate, then start charging)
- Two separate operations
- More flexible
- But vehicle might not support this pattern

**C) Remove entirely** (let users call both separately)
- Simplest API
- User controls order

**Plan recommends:** Option A (single profile with combined mode)

**Do you agree?**

---

### Question 6: Current Implementation Status

Looking at the code, I notice:

1. **BatteryControlChannel already has:**
   - ✅ Frame assembly (`BapFrameAssembler`)
   - ✅ Callback system (domain-based architecture ready)
   - ✅ Command state machine (working)
   - ✅ Profile update coordination

2. **This means:**
   - The architecture is more complete than the plan suggests
   - Phase 1-2 might be lighter than estimated
   - Main work is **extraction**, not creation

**Should we adjust the phases?** Maybe combine Phase 1+2?

---

## Key Constraints & Preferences

From the original plan:

1. ✅ **Some code duplication acceptable** - Don't over-engineer
2. ✅ **Keep profile encoding in ChargingProfileManager** - Don't abstract BAP arrays
3. ✅ **Domains make business decisions** - Temperature tolerance, SOC limits
4. ✅ **ChargingProfileManager is domain-agnostic** - No business rules
5. ✅ **Incremental migration** - Each phase works alongside existing code
6. ✅ **ESP32 constraints** - Avoid heap allocations, keep code fast

---

## What I'd Like to Discuss

1. **Do you agree with the layer separation?** (Domain owns business logic, protocol is dumb transport)

2. **Are the 4 phases reasonable?** Or should we combine/split differently?

3. **Code duplication vs. abstraction** - Your preference for domain state machines?

4. **Profile execution location** - ChargingProfileManager or domains?

5. **Event emission strategy** - Where should command events originate?

6. **Composite commands** - Keep, split, or remove?

7. **Current state** - Should we adjust the plan based on existing implementation?

8. **Priority** - Is this refactoring urgent, or can it wait?

---

## My Initial Thoughts

Based on the code review:

### ✅ What's Good About Current Implementation
- Frame assembly works correctly
- Callback system is clean
- Command state machine is functional
- Profile integration exists

### ⚠️ What Needs Refactoring
- Business logic leak (temperature tolerance, operation mode selection)
- Wake coordination duplicated across channels
- Profile update decisions in protocol layer
- Command orchestration mixed with protocol

### 💡 Proposed Approach
1. **Start with Phase 1** - Add execution to ChargingProfileManager
2. **Evaluate after Phase 1** - Is the improvement worth continuing?
3. **Consider** - Maybe current design is "good enough" if it works reliably
4. **Document** - Whatever we decide, document the reasoning

---

## Next Steps

After this discussion:

1. **Agreement on approach** - Incremental or different strategy?
2. **Phase 1 implementation** - If we proceed, start with ChargingProfileManager
3. **Testing strategy** - How to verify each phase works?
4. **Success criteria** - When do we consider refactoring "done"?

---

**Your thoughts? What should we discuss first?**
