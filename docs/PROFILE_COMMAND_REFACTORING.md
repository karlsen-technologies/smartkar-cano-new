# Profile-Based Command Refactoring Plan

## Problem Statement

Currently, when commands like `vehicle.startClimate` are issued with parameters (temperature, targetSoc, maxCurrent), these parameters are:
1. Stored locally in memory only
2. **Never sent to the vehicle**
3. The vehicle uses whatever settings are already in its Profile 0

**Result:** Command parameters are ignored by the vehicle.

## Root Cause

The VW e-Golf Battery Control system requires a **read-modify-write workflow** to update profile settings:

1. **Read Profile 0** - Full profile data (RecordAddr=0, 20+ bytes) must be fetched
2. **Modify locally** - Update temperature, targetSoc, maxCurrent, operation flags
3. **Write back** - Send complete updated profile to vehicle
4. **Start operation** - Trigger the operation mode start command

Currently, we skip steps 1-3 and only do step 4, so parameters have no effect.

## Why This Happens

From the BAP protocol documentation:
- **RecordAddr=6 (compact)**: 4 bytes, can only update operation/maxCurrent/targetChargeLevel, **NO temperature field**
- **RecordAddr=0 (full)**: 20+ bytes, includes temperature field at byte 12

To set temperature, we MUST use RecordAddr=0, which means sending the entire profile structure.

## Architecture Issues Found

### 1. ChargingProfileManager is NOT a BapChannel

**Current:**
```cpp
// VehicleManager.cpp:16
bapRouter.registerChannel(&batteryControlChannel);
// ChargingProfileManager is NOT registered!
```

**Impact:**
- ChargingProfileManager has `processProfilesArray()` method
- But it's never called because it's not in the BAP routing chain
- Profile responses from vehicle are likely being dropped

**Solutions:**
- Option A: Make ChargingProfileManager extend BapChannel
- Option B: Route PROFILES_ARRAY messages from BatteryControlChannel to ChargingProfileManager
- **Recommended:** Option B (less coupling, cleaner separation)

### 2. Profile Update Flow is Incomplete

**Current state in ChargingProfileManager.cpp:127-129:**
```cpp
// TODO: Send full profile update to car via BAP
// This requires building a longer BAP message with full profile data
Serial.printf("[ProfileMgr] TODO: Send full profile %d update\r\n", profileIndex);
```

The code to BUILD profile updates exists (`buildProfileConfig`), but the workflow to USE it is missing.

### 3. No Profile Read Support in BatteryControlChannel

BatteryControlChannel can:
- ✅ Request charge/climate/plug state
- ❌ Request profile data
- ❌ Handle profile responses

ChargingProfileManager can:
- ✅ Request all profiles (`requestAllProfiles()`)
- ✅ Parse profile responses (`processProfilesArray()`)
- ❌ But responses never reach it (not a BapChannel)

## Proposed Solution

### Phase 1: Fix Profile Message Routing

**Goal:** Ensure profile responses from vehicle reach ChargingProfileManager

**Changes:**

1. **Add profile handling to BatteryControlChannel**
   ```cpp
   // BatteryControlChannel.cpp - processMessage()
   case Function::PROFILES_ARRAY:
       // Forward to ChargingProfileManager
       manager->profiles().processProfilesArray(msg.payload, msg.payloadLen);
       profileFrames++;
       return true;
   ```

2. **Add statistics tracking**
   ```cpp
   // BatteryControlChannel.h
   volatile uint32_t profileFrames = 0;
   ```

3. **Test profile reading**
   - Send `chargingProfile.refresh` command
   - Verify `processProfilesArray()` gets called
   - Verify profiles are populated with valid data

### Phase 2: Implement Profile Update Workflow

**Goal:** Support sending full profile updates to vehicle

**Changes:**

1. **Add profile update BAP builder to ChargingProfileManager**
   ```cpp
   /**
    * Build full profile update message (RecordAddr=0)
    * @param startFrame First CAN frame (8 bytes)
    * @param contFrames Array for continuation frames (multiple 8-byte frames)
    * @param maxFrames Maximum continuation frames available
    * @param profileIndex Profile to update (0-3)
    * @param profile Profile data to send
    * @return Total number of frames (1 start + N cont)
    */
   uint8_t buildFullProfileUpdate(
       uint8_t* startFrame, 
       uint8_t* contFrames,
       uint8_t maxFrames,
       uint8_t profileIndex, 
       const Profile& profile
   );
   ```

2. **Implement full profile encoding**
   - RecordAddr = 0 (full profile)
   - 20+ byte payload (see BAP_BATTERY_CONTROL.md lines 92-112)
   - Multi-frame BAP message (1 start + 2-3 continuation frames)
   - Temperature encoding: `(celsius * 10) - 100`
   - Operation flags bitmask
   - All fields populated

3. **Update `updateTimerProfile()` to actually send**
   ```cpp
   bool ChargingProfileManager::updateTimerProfile(uint8_t profileIndex, 
                                                    const Profile& profile) {
       if (profileIndex < 1 || profileIndex > 3) {
           return false;
       }
       
       // Store locally
       profiles[profileIndex] = profile;
       profiles[profileIndex].valid = true;
       profiles[profileIndex].lastUpdate = millis();
       
       // Build and send full profile update
       uint8_t frames[32]; // 1 start + 3 cont = 32 bytes max
       uint8_t* startFrame = frames;
       uint8_t* contFrames = frames + 8;
       
       uint8_t numFrames = buildFullProfileUpdate(
           startFrame, contFrames, 3, profileIndex, profile
       );
       
       // Send all frames back-to-back
       for (uint8_t i = 0; i < numFrames; i++) {
           if (!sendFrame(frames + (i * 8), 8)) {
               return false;
           }
       }
       
       return true;
   }
   ```

### Phase 3: Extend Command State Machine

**Goal:** Add read-modify-write cycle to command flow

**Changes to BatteryControlChannel:**

1. **Expand CommandState enum**
   ```cpp
   enum class CommandState {
       IDLE,
       REQUESTING_WAKE,
       WAITING_FOR_WAKE,
       READING_PROFILE,        // NEW: Requested profile read
       WAITING_FOR_PROFILE,    // NEW: Waiting for profile data
       UPDATING_PROFILE,       // NEW: Sending updated profile
       WAITING_FOR_UPDATE,     // NEW: Waiting for update ACK
       STARTING_OPERATION,     // NEW: Sending operation start
       DONE,
       FAILED
   };
   ```

2. **Add profile tracking to command context**
   ```cpp
   // Store original profile 0 data
   Profile originalProfile0;
   bool hasProfile0 = false;
   unsigned long profile0Timestamp = 0;
   
   // Timeouts
   static constexpr unsigned long PROFILE_READ_TIMEOUT = 5000;   // 5s
   static constexpr unsigned long PROFILE_UPDATE_TIMEOUT = 5000; // 5s
   ```

3. **Implement new state machine logic**
   ```cpp
   void BatteryControlChannel::updateCommandStateMachine() {
       switch (commandState) {
           case CommandState::WAITING_FOR_WAKE:
               if (manager->isAwake()) {
                   // Vehicle awake - check if we need profile 0
                   if (needsProfileUpdate()) {
                       // Check cache
                       bool haveProfile0 = manager->profiles().isProfileValid(0);
                       
                       if (!haveProfile0) {
                           // Need to read profile first
                           setCommandState(CommandState::READING_PROFILE);
                       } else {
                           // Have cached profile, skip to update
                           setCommandState(CommandState::UPDATING_PROFILE);
                       }
                   } else {
                       // No profile update needed (e.g., stop commands)
                       setCommandState(CommandState::STARTING_OPERATION);
                   }
               }
               break;
               
           case CommandState::READING_PROFILE:
               // Request profile 0 only
               if (manager->profiles().requestProfile(0)) {
                   hasProfile0 = false; // Will be set when we receive it
                   setCommandState(CommandState::WAITING_FOR_PROFILE);
               } else {
                   setCommandState(CommandState::FAILED);
                   emitCommandEvent("commandFailed", "profile_read_request_failed");
               }
               break;
               
           case CommandState::WAITING_FOR_PROFILE:
               // Check if we received profile 0
               if (manager->profiles().isProfileValid(0)) {
                   hasProfile0 = true;
                   setCommandState(CommandState::UPDATING_PROFILE);
               } else if (elapsed > PROFILE_READ_TIMEOUT) {
                   setCommandState(CommandState::FAILED);
                   emitCommandEvent("commandFailed", "profile_read_timeout");
               }
               break;
               
           case CommandState::UPDATING_PROFILE:
               // Modify profile 0 with command parameters
               Profile updatedProfile = manager->profiles().getProfile(0);
               applyCommandParametersToProfile(updatedProfile);
               
               // Send updated profile
               if (manager->profiles().updateProfile(0, updatedProfile)) {
                   setCommandState(CommandState::WAITING_FOR_UPDATE);
               } else {
                   setCommandState(CommandState::FAILED);
                   emitCommandEvent("commandFailed", "profile_update_send_failed");
               }
               break;
               
           case CommandState::WAITING_FOR_UPDATE:
               // Wait for vehicle to ACK the profile update
               // (Profile manager sets a flag when it receives updated profile back)
               if (manager->profiles().wasProfileUpdated(0)) {
                   setCommandState(CommandState::STARTING_OPERATION);
               } else if (elapsed > PROFILE_UPDATE_TIMEOUT) {
                   setCommandState(CommandState::FAILED);
                   emitCommandEvent("commandFailed", "profile_update_timeout");
               }
               break;
               
           case CommandState::STARTING_OPERATION:
               // Now send the operation mode start command
               if (executePendingCommand()) {
                   setCommandState(CommandState::DONE);
               } else {
                   setCommandState(CommandState::FAILED);
                   emitCommandEvent("commandFailed", "operation_start_failed");
               }
               break;
       }
   }
   ```

4. **Add helper methods**
   ```cpp
   bool needsProfileUpdate() const {
       // Start commands need profile update, stop commands don't
       return pendingCommand == PendingCommand::START_CLIMATE ||
              pendingCommand == PendingCommand::START_CHARGING ||
              pendingCommand == PendingCommand::START_CHARGING_AND_CLIMATE;
   }
   
   void applyCommandParametersToProfile(Profile& profile) {
       switch (pendingCommand) {
           case PendingCommand::START_CLIMATE:
               profile.setTemperature(pendingTempCelsius);
               profile.enableClimate(true, pendingAllowBattery);
               profile.enableCharging(false);
               break;
               
           case PendingCommand::START_CHARGING:
               profile.enableCharging(true);
               profile.setTargetSoc(pendingTargetSoc);
               profile.setMaxCurrent(pendingMaxCurrent);
               profile.enableClimate(false);
               break;
               
           case PendingCommand::START_CHARGING_AND_CLIMATE:
               profile.enableCharging(true);
               profile.setTargetSoc(pendingTargetSoc);
               profile.setMaxCurrent(pendingMaxCurrent);
               profile.setTemperature(pendingTempCelsius);
               profile.enableClimate(true, pendingAllowBattery);
               break;
       }
   }
   ```

### Phase 4: Add Single Profile Read Support

**Goal:** Read single profile instead of all 4 (optional optimization)

**Changes to ChargingProfileManager:**

1. **Add requestProfile() method**
   ```cpp
   /**
    * Request a single profile from vehicle
    * @param index Profile index (0-3)
    * @return true if request sent
    */
   bool requestProfile(uint8_t index);
   ```

2. **Build single profile request**
   ```cpp
   bool ChargingProfileManager::requestProfile(uint8_t index) {
       if (index >= PROFILE_COUNT) {
           return false;
       }
       
       uint8_t frame[16]; // Long BAP message
       uint8_t* startFrame = frame;
       uint8_t* contFrame = frame + 8;
       
       // Build BAP array GET for specific index
       // RecordAddr=0 (full profile), startIndex=index, elementCount=1
       // ... (implementation details)
       
       Serial.printf("[ProfileMgr] Requesting profile %d...\n", index);
       return sendFrame(startFrame, 8) && sendFrame(contFrame, 8);
   }
   ```

### Phase 5: Add Profile Update Tracking

**Goal:** Detect when vehicle confirms profile updates

**Changes to ChargingProfileManager:**

1. **Add update tracking flags**
   ```cpp
   // Track which profiles were recently updated
   bool profileUpdatePending[PROFILE_COUNT] = {false};
   unsigned long profileUpdateTime[PROFILE_COUNT] = {0};
   ```

2. **Set flag when sending update**
   ```cpp
   bool ChargingProfileManager::updateProfile(uint8_t index, const Profile& profile) {
       // ... send update ...
       
       profileUpdatePending[index] = true;
       profileUpdateTime[index] = millis();
       return true;
   }
   ```

3. **Clear flag when receiving updated profile**
   ```cpp
   void ChargingProfileManager::processProfilesArray(const uint8_t* payload, uint8_t len) {
       // ... parse profiles ...
       
       for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
           if (profileUpdatePending[i]) {
               // Check if this profile was included in response
               // and matches what we sent
               profileUpdatePending[i] = false;
           }
       }
   }
   ```

4. **Add query method**
   ```cpp
   bool wasProfileUpdated(uint8_t index) const {
       if (index >= PROFILE_COUNT) return false;
       
       // Return true if:
       // 1. No update was pending (nothing to wait for), OR
       // 2. Update was pending but is now cleared (received confirmation)
       return !profileUpdatePending[index];
   }
   ```

## Implementation Order

### Milestone 1: Profile Message Routing (Highest Priority)
**Blocking issue - must be fixed first**

- [ ] Route PROFILES_ARRAY messages from BatteryControlChannel to ChargingProfileManager
- [ ] Add statistics tracking for profile frames
- [ ] Test with `chargingProfile.refresh` command
- [ ] Verify profiles populate correctly

**Validation:** Run `chargingProfile.get` command, verify all 4 profiles return with valid data

### Milestone 2: Full Profile Update Support
**Enables proper profile modification**

- [ ] Implement `buildFullProfileUpdate()` with RecordAddr=0
- [ ] Handle multi-frame BAP encoding
- [ ] Implement temperature encoding/decoding
- [ ] Update `updateTimerProfile()` to send BAP messages
- [ ] Test with `chargingProfile.updateProfile` command

**Validation:** Update timer profile via command, verify changes persist on vehicle after reboot

### Milestone 3: Single Profile Read (Optional but Recommended)
**Performance optimization**

- [ ] Implement `requestProfile(index)` for single profile reads
- [ ] Build BAP array GET with specific index
- [ ] Test reading profile 0 only

**Validation:** Request profile 0, verify only profile 0 is returned (not all 4)

### Milestone 4: Profile Update Tracking
**Enables state machine wait logic**

- [ ] Add update pending flags
- [ ] Set flags on send, clear on receive
- [ ] Implement `wasProfileUpdated()` query
- [ ] Add timeout handling (5 seconds)

**Validation:** Send update, verify flag clears when vehicle responds

### Milestone 5: Extended Command State Machine
**Final integration - makes command parameters work**

- [ ] Expand CommandState enum
- [ ] Add profile read/update states
- [ ] Implement state transitions
- [ ] Add helper methods
- [ ] Update timeout constants

**Validation:** Send `vehicle.startClimate` with temperature=18.5°C, verify vehicle actually uses 18.5°C

### Milestone 6: Integration Testing
**End-to-end validation**

- [ ] Test full cold start (no cached profiles)
- [ ] Test with cached profiles
- [ ] Test timeout scenarios
- [ ] Test concurrent command rejection
- [ ] Verify all events are emitted correctly
- [ ] Load test (rapid commands)

**Validation:** All command parameters work as documented in protocol.md

## Testing Strategy

### Unit Tests
- Profile encoding/decoding (temperature, operation flags)
- BAP message building (multi-frame)
- State machine transitions

### Integration Tests
- Profile read → successful parse
- Profile update → vehicle ACK
- Full command flow → correct operation

### Manual Tests
1. **Cold Start Test:** Device boots, no profiles cached
   - Send `vehicle.startClimate temperature=18.5`
   - Verify: Profile read → update → start → climate at 18.5°C

2. **Cached Profile Test:** Profiles already loaded
   - Send `vehicle.startClimate temperature=22.0`
   - Verify: Skip read → update → start → climate at 22.0°C

3. **Timeout Test:** Vehicle doesn't respond
   - Disconnect CAN bus during profile read
   - Verify: Command fails after 5s with clear error

4. **Concurrent Command Test:** Multiple rapid commands
   - Send `vehicle.startClimate`
   - Immediately send `vehicle.startCharging`
   - Verify: Second command rejected with "busy" error

## Risk Assessment

### Low Risk
- Profile message routing (simple forwarding)
- Single profile read (optional feature)

### Medium Risk
- Full profile update encoding (complex but well-documented)
- Update tracking (edge cases with timing)

### High Risk
- Extended state machine (many new states, complex logic)
- Multi-frame BAP messages (timing sensitive)

**Mitigation:**
- Implement in phases with validation gates
- Extensive logging at each state transition
- Timeout guards on all wait states
- Fallback to existing behavior if profile operations fail

## Open Questions

1. **Profile 0 Cache TTL:** Should we refresh profile 0 periodically even if not commanded?
   - Recommendation: No - only read when needed. Vehicle sends updates automatically.

2. **Partial vs Full Reads:** Should we support reading just profile 0, or always read all 4?
   - Recommendation: Implement single profile read for efficiency.

3. **Update Confirmation:** How do we know the vehicle accepted the profile update?
   - Recommendation: Vehicle sends back the updated profile in response (need to verify).

4. **Failure Recovery:** If profile update fails, should we retry or fail the command?
   - Recommendation: Fail immediately, let user retry command.

5. **Stop Commands:** Do stop commands need profile updates?
   - Answer: No - operation mode stop doesn't use profile parameters.

## Success Criteria

✅ **Phase 1 Complete When:**
- Profile responses are received and parsed
- All 4 profiles populate with valid data
- `chargingProfile.get` command returns correct data

✅ **Phase 2 Complete When:**
- `chargingProfile.updateProfile` command actually updates vehicle
- Temperature changes persist after vehicle reboot
- All profile fields can be modified

✅ **Phase 3 Complete When:**
- `vehicle.startClimate temperature=X` actually sets climate to X°C
- `vehicle.startCharging targetSoc=Y maxCurrent=Z` uses correct values
- Command parameters work as documented in protocol.md

✅ **Complete Success When:**
- All commands work with parameters
- State machine handles all edge cases gracefully
- No blocking or hanging on failures
- Clear error messages for all failure modes
