# Keep-Alive Refactoring Plan

## Problem Statement

The current keep-alive implementation has several issues:

1. **Keep-alive doesn't start when vehicle is already awake**
   - Domains call `requestWake()` which returns early if already awake
   - Keep-alive never starts, vehicle could sleep mid-command
   
2. **Keep-alive doesn't stop when commands complete**
   - Runs for full 5-minute timeout even after command succeeds/fails
   - Wastes power sending unnecessary keep-alive frames
   
3. **Confusing API**
   - Domains need to call both `requestWake()` AND `notifyCommandActivity()`
   - Complex conditional logic needed: "if awake do X, else do Y"
   - Easy to misuse or forget

## Solution: Simplified Keep-Alive API

### New Public API

Simplify the WakeController API to just 2 lifecycle methods:

```cpp
class WakeController {
public:
    // State checking
    bool isAwake() const;
    WakeState getState() const;
    
    // Keep-alive lifecycle (NEW/CHANGED)
    void ensureAwake();       // Call at operation start
    void stopKeepAlive();     // Call at operation end (success or failure)
    
    // ... other existing methods unchanged ...

private:
    // Internal implementation (moved from public)
    bool requestWake();
    void notifyCommandActivity();
    void startKeepAlive();
};
```

### API Semantics

**`ensureAwake()`** - Prepare vehicle for operations
- If vehicle is asleep: Initiates wake sequence + starts keep-alive
- If vehicle is awake: Starts keep-alive (if not already active)
- Always updates activity timestamp
- Returns `void` (domains check `isAwake()` in their state machine)

**`stopKeepAlive()`** - End operation
- Stops keep-alive heartbeat immediately
- Safe to call even if already stopped (has guard)
- Call when command completes (success or failure)

### Domain Usage Pattern

**Simple and consistent pattern for all commands:**

```cpp
bool BatteryManager::startCharging(...) {
    // 1. Ensure vehicle will be awake
    wakeController->ensureAwake();
    
    // 2. State machine determines next action
    if (!wakeController->isAwake()) {
        setCommandState(CommandState::REQUESTING_WAKE);
    } else if (needsProfileUpdate(...)) {
        setCommandState(CommandState::UPDATING_PROFILE);
    } else {
        setCommandState(CommandState::EXECUTING_COMMAND);
    }
    
    return true;
}

void BatteryManager::completeCommand() {
    // Stop keep-alive - operation complete
    wakeController->stopKeepAlive();
    
    // ... rest of cleanup ...
}

void BatteryManager::failCommand(const char* reason) {
    // Stop keep-alive - operation failed
    wakeController->stopKeepAlive();
    
    // ... rest of cleanup ...
}
```

**Benefits:**
- ✅ One call handles both wake + keep-alive
- ✅ No conditional logic needed in domains
- ✅ Clear lifecycle: `ensureAwake()` → (do work) → `stopKeepAlive()`
- ✅ Hard to misuse - simple API

## Implementation Details

### Phase 1: WakeController Changes

#### File: `src/vehicle/services/WakeController.h`

**Changes:**

1. **Add `ensureAwake()` to public section** (after `requestWake()`)
   ```cpp
   /**
    * Ensure vehicle is awake with keep-alive active.
    * Call this before performing operations that require the vehicle to stay awake.
    * 
    * Behavior:
    * - If vehicle is asleep: Initiates wake sequence
    * - If vehicle is awake: Ensures keep-alive is active
    * - Always updates activity timestamp
    * 
    * After calling, check isAwake() in your state machine to determine
    * if you need to wait for wake completion.
    */
   void ensureAwake();
   ```

2. **Move `stopKeepAlive()` from private to public section** (after `ensureAwake()`)
   ```cpp
   /**
    * Stop keep-alive heartbeat.
    * Call this when an operation completes (successfully or not) and the 
    * vehicle no longer needs to be kept awake artificially.
    * 
    * Safe to call multiple times - will only stop if active.
    */
   void stopKeepAlive();
   ```

3. **Move `requestWake()` to private section**
   - Remove `IDomain* requester` parameter
   - Update signature: `bool requestWake();`

4. **Keep `notifyCommandActivity()` in private section** (already private)

5. **Update header comment** (lines 25-30)
   ```cpp
   * Keep-Alive Management:
   * - Call ensureAwake() at operation start
   * - Call stopKeepAlive() at operation end (success or failure)
   * - Keep-alive sends heartbeat every 500ms while active
   * - 5-minute timeout as safety fallback (if stopKeepAlive not called)
   * - Does NOT start on natural wake (door open, key fob, etc.)
   ```

#### File: `src/vehicle/services/WakeController.cpp`

**Changes:**

1. **Implement `ensureAwake()`** (add after `requestWake()`)
   ```cpp
   void WakeController::ensureAwake() {
       // Update activity timestamp and start keep-alive if already awake
       notifyCommandActivity();
       
       // If asleep, initiate wake sequence
       if (wakeState == WakeState::ASLEEP) {
           requestWake();
       }
   }
   ```

2. **Update `requestWake()` signature** (line 117)
   - Remove `IDomain* requester` parameter
   - Remove `requester` tracking logic (lines 127-130)
   - Remove `pendingRequester = requester;` assignment
   - Update signature: `bool WakeController::requestWake()`

3. **No other implementation changes needed**
   - `notifyCommandActivity()` unchanged
   - `startKeepAlive()` unchanged
   - `stopKeepAlive()` unchanged

### Phase 2: BatteryManager Changes

#### File: `src/vehicle/domains/BatteryManager.cpp`

**Change 1: `startCharging()` method** (around line 218)

Replace:
```cpp
// Start state machine
if (!wakeController->isAwake()) {
    Serial.println("[BatteryManager] Vehicle not awake, requesting wake");
    setCommandState(CommandState::REQUESTING_WAKE);
    wakeController->requestWake();
} else if (needsProfileUpdate(targetSoc, maxCurrent)) {
```

With:
```cpp
// Ensure vehicle is awake and keep-alive is active
wakeController->ensureAwake();

// Start state machine
if (!wakeController->isAwake()) {
    Serial.println("[BatteryManager] Vehicle not awake, requesting wake");
    setCommandState(CommandState::REQUESTING_WAKE);
} else if (needsProfileUpdate(targetSoc, maxCurrent)) {
```

**Change 2: `stopCharging()` method** (around line 249)

Replace:
```cpp
// Stop doesn't need profile update, just execute
if (!wakeController->isAwake()) {
    Serial.println("[BatteryManager] Vehicle not awake, requesting wake");
    setCommandState(CommandState::REQUESTING_WAKE);
    wakeController->requestWake();
} else {
```

With:
```cpp
// Ensure vehicle is awake and keep-alive is active
wakeController->ensureAwake();

// Stop doesn't need profile update, just execute
if (!wakeController->isAwake()) {
    Serial.println("[BatteryManager] Vehicle not awake, requesting wake");
    setCommandState(CommandState::REQUESTING_WAKE);
} else {
```

**Change 3: `completeCommand()` method** (line ~417)

Add at start of function:
```cpp
void BatteryManager::completeCommand() {
    Serial.printf("[BatteryManager] Command %d completed successfully\r\n", pendingCommandId);
    
    // Stop keep-alive - charging will keep vehicle awake if active
    wakeController->stopKeepAlive();
    
    // Reset state
    setCommandState(CommandState::IDLE);
    // ... rest unchanged ...
}
```

**Change 4: `failCommand()` method** (line ~428)

Add at start of function (after logging):
```cpp
void BatteryManager::failCommand(const char* reason) {
    Serial.printf("[BatteryManager] Command %d failed: %s\r\n", pendingCommandId, reason);
    
    // Stop keep-alive - no need to keep vehicle awake
    wakeController->stopKeepAlive();
    
    // Reset state
    setCommandState(CommandState::FAILED);
    // ... rest unchanged ...
}
```

### Phase 3: ClimateManager Changes

#### File: `src/vehicle/domains/ClimateManager.cpp`

**Same 4 changes as BatteryManager:**

**Change 1: `startClimate()` method** (around line 198)
- Add `wakeController->ensureAwake();` before state machine
- Remove `wakeController->requestWake();` call

**Change 2: `stopClimate()` method** (around line 227)
- Add `wakeController->ensureAwake();` before state machine
- Remove `wakeController->requestWake();` call

**Change 3: `completeCommand()` method** (line ~396)
- Add `wakeController->stopKeepAlive();` at start

**Change 4: `failCommand()` method** (line ~407)
- Add `wakeController->stopKeepAlive();` after logging

### Phase 4: WakeController.h Private Section Cleanup

#### File: `src/vehicle/services/WakeController.h`

**Change: Remove `pendingRequester` member** (line 124)
- Remove: `IDomain* pendingRequester = nullptr;`
- No longer needed since we removed requester tracking

## Summary of Changes

### Files Modified: 4
1. `src/vehicle/services/WakeController.h` - API reorganization + cleanup
2. `src/vehicle/services/WakeController.cpp` - Add `ensureAwake()`, update `requestWake()`
3. `src/vehicle/domains/BatteryManager.cpp` - Update 4 locations
4. `src/vehicle/domains/ClimateManager.cpp` - Update 4 locations

### Lines of Code Changed: ~20-25 lines total
- WakeController.h: ~5 line moves + 2 new comments + 1 line removed
- WakeController.cpp: ~8 new lines + ~3 lines changed
- BatteryManager.cpp: ~6 lines added/changed
- ClimateManager.cpp: ~6 lines added/changed

### API Changes
**Before:**
- Public: `requestWake(IDomain*)`, `notifyCommandActivity()`
- Pattern: `if (isAwake()) { notify() } else { requestWake() }`
- Keep-alive: automatic timeout only

**After:**
- Public: `ensureAwake()`, `stopKeepAlive()`
- Pattern: `ensureAwake()` → (work) → `stopKeepAlive()`
- Keep-alive: explicit lifecycle management

## Expected Behavior After Changes

### Scenario 1: Start Command (Vehicle Asleep)
```
1. Domain: startCharging/startClimate
2. Call ensureAwake()
   → notifyCommandActivity() (updates timestamp)
   → requestWake() (starts wake sequence + keep-alive)
3. State machine: REQUESTING_WAKE
4. [Wait for wake...]
5. Vehicle wakes up
6. State machine: UPDATING_PROFILE or EXECUTING_COMMAND
7. Command executes
8. completeCommand() → stopKeepAlive()
9. Result: Charging/climate keeps vehicle awake naturally
```

### Scenario 2: Start Command (Vehicle Already Awake)
```
1. Domain: startCharging/startClimate
2. Call ensureAwake()
   → notifyCommandActivity() (starts keep-alive ✅)
   → Skip requestWake() (already awake)
3. State machine: UPDATING_PROFILE or EXECUTING_COMMAND
4. Command executes
5. completeCommand() → stopKeepAlive()
6. Result: Charging/climate keeps vehicle awake naturally
```

### Scenario 3: Stop Command (Vehicle Awake from Operation)
```
1. Domain: stopCharging/stopClimate
2. Call ensureAwake()
   → notifyCommandActivity() (starts/extends keep-alive)
3. State machine: EXECUTING_COMMAND
4. Command executes (stops operation)
5. completeCommand() → stopKeepAlive()
6. Result: Vehicle can sleep naturally
```

### Scenario 4: Command Failure (Any Stage)
```
1. Domain: any command
2. Call ensureAwake()
3. [Failure occurs: wake timeout, BAP error, etc.]
4. failCommand() → stopKeepAlive()
5. Result: No wasted keep-alive frames
```

### Scenario 5: Natural Wake (No Command)
```
1. Door opens / key press / etc.
2. WakeController detects CAN activity
3. State: ASLEEP → AWAKE
4. Keep-alive: NOT started (correct behavior)
5. Result: Vehicle can go back to sleep when done
```

## Testing Checklist

### Unit-Level Behavior
- [ ] `ensureAwake()` when asleep → starts wake sequence + keep-alive
- [ ] `ensureAwake()` when awake → starts keep-alive (if not active)
- [ ] `ensureAwake()` when waking → updates timestamp only
- [ ] `stopKeepAlive()` when active → stops keep-alive
- [ ] `stopKeepAlive()` when inactive → no-op (early return)

### Integration Testing
- [ ] startCharging (asleep) → wake → execute → stop keep-alive
- [ ] startCharging (awake) → execute → stop keep-alive
- [ ] stopCharging (awake) → execute → stop keep-alive
- [ ] startClimate (asleep) → wake → execute → stop keep-alive
- [ ] startClimate (awake) → execute → stop keep-alive
- [ ] stopClimate (awake) → execute → stop keep-alive
- [ ] Command failure → stop keep-alive immediately
- [ ] Wake timeout → failCommand → stop keep-alive
- [ ] Natural wake (door) → no keep-alive started

### Power Consumption Testing
- [ ] Keep-alive stops within 1 second of command completion
- [ ] No keep-alive frames after stopCharging/stopClimate
- [ ] Natural wake doesn't trigger keep-alive
- [ ] Failed wake doesn't leave keep-alive running

### Logging Verification
- [ ] "Ensuring vehicle is awake" message not needed (internal)
- [ ] "Starting keep-alive" appears when expected
- [ ] "Stopping keep-alive" appears on command completion
- [ ] Wake request logs show wake initiated

## Rollback Plan

If issues are discovered after deployment:

1. **Revert WakeController API changes**
   - Make `requestWake(IDomain*)` public again
   - Make `notifyCommandActivity()` public again
   - Keep `ensureAwake()` but mark as deprecated

2. **Revert domain changes**
   - Replace `ensureAwake()` calls with `requestWake()` calls
   - Remove `stopKeepAlive()` calls
   - Rely on 5-minute timeout

3. **Test thoroughly before redeployment**

## Future Enhancements

After this refactoring is stable, consider:

1. **Reference counting** - Track multiple concurrent operations
2. **Per-operation timeout** - Different timeout based on operation type
3. **Metrics** - Track keep-alive duration per command type
4. **Dynamic intervals** - Adjust keep-alive interval based on battery level

## Notes

- The 5-minute timeout remains as a safety fallback
- `ensureAwake()` is safe to call multiple times
- `stopKeepAlive()` is safe to call even if not active
- No breaking changes to other modules (CAN, BAP, etc.)
- All changes are backward compatible (5-minute timeout still works)
