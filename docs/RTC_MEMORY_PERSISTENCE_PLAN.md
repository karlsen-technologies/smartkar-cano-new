# RTC Memory Persistence Plan

**Date:** 2026-01-31  
**Status:** Ready for Implementation  
**Prerequisites:** Domain-based architecture refactor (✅ Complete)

---

## Executive Summary

Convert all vehicle state to RTC-backed storage, ensuring state persists across deep sleep. This solves the "null data" problem where the device wakes from deep sleep with no knowledge of vehicle state.

**Key Decision:** Each component owns its RTC memory directly (no central VehicleStateRTC.h)

---

## Problem Statement

### Current Behavior

**After deep sleep:**
- ❌ ESP32 RAM cleared (all vehicle state lost)
- ❌ Don't know SOC, plug state, climate status, etc.
- ❌ Don't know Profile 0 configuration
- ❌ Must wait for CAN broadcasts (can take minutes) or request BAP data
- ❌ Can't respond to server immediately with current state

### Why This Matters

1. **Server commands after wake:** Device wakes on modem RI interrupt when server sends command
   - Need to execute quickly
   - Can't send "null" data in response
   
2. **CAN broadcasts can't be requested:** Must wait for natural broadcasts
   - Some messages broadcast every 100ms
   - Some messages broadcast only when values change
   - Can't rely on timely CAN data after wake

3. **BAP data can be requested:** But adds 1-2 second delay
   - Profile data requests work
   - State requests work
   - But slows down command execution

### Solution: RTC Memory Persistence

**ESP32-S3 RTC Memory:** 8KB SRAM that survives deep sleep
- Unlimited write cycles (it's SRAM, not flash)
- Fast access (no different from regular RAM)
- Perfect for vehicle state storage

**Current RTC Usage:** Only 2 bytes (modemPowered, lowPowerMode flags)

---

## Architecture Decision: Distributed RTC State

### Why Not Create VehicleStateRTC.h?

**Rejected Approach:**
```cpp
// Central RTC state file
RTC_DATA_ATTR struct VehicleStateRTC {
    BatteryState battery;
    ClimateState climate;
    // ... all states
} rtcVehicleState;
```

**Problems with central file:**
- Creates new legacy code (VehicleState.h is already being removed)
- Violates encapsulation (state leaks out of domain managers)
- Single point of change (any domain change requires editing central file)
- Duplicates state structures

### Chosen Approach: Each Component Owns Its RTC Memory

**Pattern:**
```cpp
// In BatteryManager.cpp (outside class)
RTC_DATA_ATTR BatteryManager::State rtcBatteryState = {};

// In BatteryManager.h (class definition)
class BatteryManager : public IDomain {
private:
    State& state = rtcBatteryState;  // Reference to RTC memory
};
```

**Benefits:**
- ✅ Encapsulation preserved (state stays in domain)
- ✅ No boilerplate (no save/restore methods needed)
- ✅ Always in sync (RTC state IS the working state)
- ✅ Zero runtime overhead (direct access)
- ✅ Easy to understand (each component manages itself)

---

## Current State Storage Analysis

### Domain Managers (Already Have State Structs)

All domain managers follow consistent pattern:

```cpp
class DomainManager : public IDomain {
private:
    State state;  // Currently in RAM
};
```

**BatteryManager::State** (~98 bytes):
- Energy, SOC, temperature, power
- Plug state (lock, connection, supply)
- Charging details (mode, status, amps, target, time)
- Timestamps and data sources

**ClimateManager::State** (~48 bytes):
- Inside/outside temperature
- Climate active, heating, cooling, ventilation
- Auto defrost, remaining time
- Timestamps and data sources

**BodyManager::State** (~40 bytes):
- Central lock state
- 4 door states (open, closed, locked, windows)
- Trunk open status
- Timestamps

**DriveManager::State** (~36 bytes):
- Ignition state (OFF, ACC, ON, START)
- Speed, odometer
- Vehicle time (year, month, day, hour, minute, second)
- Timestamps

**GpsManager::State** (~32 bytes):
- Latitude, longitude
- Speed, heading, altitude
- Satellite count, fix quality
- Timestamps

**RangeManager::State** (~16 bytes):
- Electric range, fuel range
- Timestamps

**Total Domain State:** ~270 bytes

---

### ChargingProfileManager (Has Profiles Array)

**Current:**
```cpp
// In ChargingProfileManager.h (line 211)
ChargingProfile::Profile profiles[ChargingProfile::PROFILE_COUNT];  // RAM
```

**Profile size:** ~30 bytes each × 4 profiles = **120 bytes**

**Total Profiles:** ~120 bytes

---

### Grand Total: ~390 bytes

**RTC Memory Available:** 8KB (8192 bytes)  
**Usage:** 390 bytes = **4.8% of available RTC memory**

**Conclusion:** We have TONS of space. No concerns about RTC memory limits.

---

## Implementation Plan

### Phase 0: RTC Memory Conversion

**Goal:** Make all vehicle state RTC-backed before starting BAP refactoring

**Why Phase 0?**
- Foundation for all subsequent work
- BAP refactoring assumes state persistence
- Simple change with huge benefits
- Can be done quickly (1-2 hours)

---

### Step 1: ChargingProfileManager RTC Conversion

**File:** `src/vehicle/ChargingProfileManager.cpp`

**Add at top of file (outside namespace):**
```cpp
#include "ChargingProfileManager.h"
#include "VehicleManager.h"

using namespace ChargingProfile;
using namespace BapProtocol;

// ===================================================================
// RTC Memory - Profiles persist across deep sleep
// ===================================================================

RTC_DATA_ATTR Profile rtcProfiles[PROFILE_COUNT] = {};
```

**File:** `src/vehicle/ChargingProfileManager.h`

**Change line 211 from:**
```cpp
ChargingProfile::Profile profiles[ChargingProfile::PROFILE_COUNT];
```

**To:**
```cpp
ChargingProfile::Profile* profiles;  // Points to RTC memory
```

**Update constructor in ChargingProfileManager.cpp:**
```cpp
ChargingProfileManager::ChargingProfileManager(VehicleManager* mgr)
    : manager(mgr),
      profiles(rtcProfiles)  // Point to RTC memory
{
    // Initialize all profiles to cleared state (only on fresh boot)
    // RTC memory preserves state across deep sleep
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        if (!profiles[i].valid) {
            profiles[i].clear();
        }
    }
}
```

**Changes Required:**
- Declare `rtcProfiles` array with `RTC_DATA_ATTR` in .cpp file
- Change `profiles` from array to pointer in .h file
- Initialize pointer to RTC memory in constructor
- Test that profiles survive deep sleep

**Estimated Time:** 20 minutes

---

### Step 2: BatteryManager RTC Conversion

**File:** `src/vehicle/domains/BatteryManager.cpp`

**Add at top of file:**
```cpp
#include "BatteryManager.h"
#include "../VehicleManager.h"
#include "../bap/channels/BatteryControlChannel.h"

// ===================================================================
// RTC Memory - Battery state persists across deep sleep
// ===================================================================

RTC_DATA_ATTR BatteryManager::State rtcBatteryState = {};
```

**File:** `src/vehicle/domains/BatteryManager.h`

**Change line 192 from:**
```cpp
State state;
```

**To:**
```cpp
State& state;  // Reference to RTC memory
```

**Update constructor:**
```cpp
BatteryManager::BatteryManager(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager),
      bapChannel(nullptr),
      state(rtcBatteryState)  // Reference RTC memory
{
}
```

**Estimated Time:** 15 minutes

---

### Step 3: ClimateManager RTC Conversion

**File:** `src/vehicle/domains/ClimateManager.cpp`

**Add at top:**
```cpp
// ===================================================================
// RTC Memory - Climate state persists across deep sleep
// ===================================================================

RTC_DATA_ATTR ClimateManager::State rtcClimateState = {};
```

**File:** `src/vehicle/domains/ClimateManager.h`

**Change line 168 from:**
```cpp
State state;
```

**To:**
```cpp
State& state;  // Reference to RTC memory
```

**Update constructor:**
```cpp
ClimateManager::ClimateManager(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager),
      bapChannel(nullptr),
      state(rtcClimateState)  // Reference RTC memory
{
}
```

**Estimated Time:** 15 minutes

---

### Step 4: BodyManager RTC Conversion

**Pattern:** Same as BatteryManager

**Add to BodyManager.cpp:**
```cpp
RTC_DATA_ATTR BodyManager::State rtcBodyState = {};
```

**Change in BodyManager.h:**
```cpp
State& state;  // Reference to RTC memory
```

**Update constructor:**
```cpp
BodyManager::BodyManager(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager),
      state(rtcBodyState)
{
}
```

**Estimated Time:** 15 minutes

---

### Step 5: DriveManager, GpsManager, RangeManager RTC Conversion

**Pattern:** Identical to BodyManager for each

**DriveManager.cpp:**
```cpp
RTC_DATA_ATTR DriveManager::State rtcDriveState = {};
```

**GpsManager.cpp:**
```cpp
RTC_DATA_ATTR GpsManager::State rtcGpsState = {};
```

**RangeManager.cpp:**
```cpp
RTC_DATA_ATTR RangeManager::State rtcRangeState = {};
```

**Each .h file:** Change `State state;` → `State& state;`

**Each constructor:** Add `state(rtcXxxState)` to initializer list

**Estimated Time:** 30 minutes (10 minutes each)

---

### Step 6: Testing

**Test deep sleep persistence:**

```cpp
void testDeepSleepPersistence() {
    Serial.println("\n=== RTC Memory Persistence Test ===");
    
    // Check boot count
    static uint32_t bootCount = 0;
    RTC_DATA_ATTR static uint32_t rtcBootCount = 0;
    rtcBootCount++;
    bootCount = rtcBootCount;
    
    Serial.printf("Boot #%u\n", bootCount);
    
    // Display domain states
    auto battery = vehicleManager->battery();
    Serial.printf("Battery SOC: %.1f%% (valid=%d)\n", 
                  battery->getSoc(), 
                  battery->getState().isValid());
    
    auto climate = vehicleManager->climate();
    Serial.printf("Inside temp: %.1f°C (valid=%d)\n",
                  climate->getInsideTemp(),
                  climate->getState().isValid());
    
    // Display profiles
    auto& profile0 = vehicleManager->profiles().getProfile(0);
    Serial.printf("Profile 0: targetSoc=%d%%, maxCurrent=%dA (valid=%d)\n",
                  profile0.targetChargeLevel,
                  profile0.maxCurrent,
                  profile0.valid);
    
    Serial.println("=== State should persist across deep sleep ===\n");
    
    // Sleep for 10 seconds, then wake and check again
    if (bootCount == 1) {
        Serial.println("Sleeping for 10 seconds...");
        delay(1000);
        esp_sleep_enable_timer_wakeup(10 * 1000000ULL);  // 10 seconds
        esp_deep_sleep_start();
    }
}
```

**Expected Result:**
- Boot #1: State values from car (if received)
- Sleep 10 seconds
- Boot #2: Same state values (persisted in RTC)
- Profile 0 configuration preserved
- Domain states preserved

**Estimated Time:** 30 minutes

---

## Total Implementation Time

| Step | Component | Time |
|------|-----------|------|
| 1 | ChargingProfileManager | 20 min |
| 2 | BatteryManager | 15 min |
| 3 | ClimateManager | 15 min |
| 4 | BodyManager | 15 min |
| 5 | DriveManager, GpsManager, RangeManager | 30 min |
| 6 | Testing | 30 min |
| **Total** | | **~2 hours** |

---

## Benefits After Implementation

### 1. Solves "Null Data" Problem
✅ Always have last known state after wake  
✅ Can respond to server requests immediately  
✅ No "waiting for CAN broadcasts" delay

### 2. Faster Command Execution
✅ Profile update decisions work immediately  
✅ Can check if profile needs updating (without waiting for BAP read)  
✅ Reduced latency for user commands

### 3. Better User Experience
✅ Server gets instant telemetry (even if slightly stale)  
✅ Commands execute faster  
✅ More reliable behavior

### 4. Simplifies BAP Refactoring
✅ Domain managers don't need special "after wake" logic  
✅ Profile update decisions work normally  
✅ No complex state reconstruction needed

### 5. Enables Future Features
✅ Can implement "optimistic update skip" (if profile in RTC matches desired)  
✅ Can track boot count for diagnostics  
✅ Can store calibration data, learned behaviors, etc.

---

## Important Notes

### RTC Memory Characteristics

**Unlimited Write Cycles:** RTC memory is SRAM, not flash
- Can write as frequently as needed
- No wear leveling concerns
- Update in real-time as data changes

**No Special Handling Needed:**
- Domain managers write to `state.field = value;`
- Automatically persists to RTC
- No `saveToRTC()` or `restoreFromRTC()` methods

**Power Requirements:**
- RTC memory powered as long as ESP32 has ANY power
- Survives deep sleep
- Lost on complete power loss (but car battery removal is extremely rare)

### Data Staleness

**Not a concern due to CAN wake:**
- Device wakes on CAN activity (when implemented)
- Device won't sleep while car is active
- RTC state represents "state when car went to sleep"
- Fresh data arrives shortly after car wakes

**Stale data scenario:**
- Device sleeps for 2 hours
- Car charges/drives while device sleeps (extremely unlikely without waking device)
- RTC state would be stale

**Mitigation:**
- CAN wake prevents this scenario
- Even if stale, server gets timestamp of last update
- Fresh data arrives within seconds of wake

### State Initialization

**Fresh Boot (No Valid RTC State):**
```cpp
if (!state.isValid()) {
    // No data received yet
    // State fields are zero/default
    // Wait for car broadcasts or BAP requests
}
```

**After Deep Sleep (Valid RTC State):**
```cpp
if (state.isValid()) {
    // Have last known state from before sleep
    // Can respond immediately
    // Fresh data will update incrementally
}
```

**Pattern:** Each domain's `State::isValid()` method checks if any data received

---

## Integration with BAP Refactoring

### Phase 0 (This Plan) → Phase 1-4 (BAP Refactoring)

**This plan is Phase 0 because:**
1. Must be done BEFORE BAP refactoring
2. BAP refactoring assumes state persists
3. Profile update decisions need access to RTC-backed profiles
4. Simple, low-risk change

**After Phase 0:**
- All state is RTC-backed
- BAP refactoring can proceed normally
- Domain state machines work correctly
- Profile update decisions work correctly

**Phase 1: Enhance ChargingProfileManager**
- Profiles already in RTC memory ✅
- No special handling needed

**Phase 2: Domain State Machines**
- Domain state already in RTC memory ✅
- State machine can check `state.field` normally
- No "after wake" special cases

**Phase 3-4: Complete BAP Refactoring**
- No changes needed for RTC persistence
- Everything works automatically

---

## Next Steps

1. **Implement Phase 0** (this plan) - ~2 hours
2. **Test deep sleep persistence** - verify state survives
3. **Commit Phase 0** - clean checkpoint
4. **Proceed with BAP refactoring** - Phases 1-4 as planned

---

## Success Criteria

✅ All domain state stored in RTC memory  
✅ Profiles stored in RTC memory  
✅ State survives deep sleep (tested)  
✅ No code duplication (each component owns its RTC state)  
✅ No performance impact (direct access)  
✅ No special save/restore logic needed  
✅ Ready for BAP refactoring phases

---

**Status:** Ready to implement  
**Risk:** Very Low (additive change, no breaking changes)  
**Time:** ~2 hours  
**Blockers:** None

Let's do Phase 0 before starting BAP refactoring!
