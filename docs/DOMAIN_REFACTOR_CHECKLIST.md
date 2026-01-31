# Domain-Based Architecture - Implementation Checklist

Quick reference for implementing the domain-based CAN architecture refactoring.

**Full Documentation:** See `DOMAIN_BASED_ARCHITECTURE.md`

---

## Phase 1: Infrastructure ✓

### Create Service Classes

- [ ] `src/vehicle/services/WakeController.h`
  - Extract wake state machine from VehicleManager
  - Methods: `setup()`, `loop()`, `requestWake()`, `onCanActivity()`, `isAwake()`
  - States: ASLEEP, WAKE_REQUESTED, WAKING, AWAKE, WAKE_FAILED

- [ ] `src/vehicle/services/WakeController.cpp`
  - Implementation of wake logic
  - Keep-alive frame transmission (0x5A7)
  - Timeout handling

- [ ] `src/vehicle/services/ActivityTracker.h`
  - Track CAN activity for sleep management
  - Methods: `onCanActivity()`, `isActive()`, `getLastActivityTime()`, `getFrameCount()`

- [ ] `src/vehicle/services/ActivityTracker.cpp`
  - Simple activity tracking implementation

### Create Domain Interface

- [ ] `src/vehicle/IDomain.h`
  ```cpp
  class IDomain {
      virtual const char* getName() const = 0;
      virtual bool setup() = 0;
      virtual void loop() = 0;
      virtual void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) = 0;
      virtual void onWakeComplete() {}
      virtual bool isBusy() const { return false; }
  };
  ```

### Update BatteryControlChannel

- [ ] Add callback typedefs to `BatteryControlChannel.h`
  ```cpp
  using PlugStateCallback = std::function<void(const PlugState&)>;
  using ChargeStateCallback = std::function<void(const ChargeState&)>;
  using ClimateStateCallback = std::function<void(const ClimateState&)>;
  using ProfilesCallback = std::function<void(const ChargingProfiles&)>;
  ```

- [ ] Add callback registration methods
  - `onPlugState(PlugStateCallback)`
  - `onChargeState(ChargeStateCallback)`
  - `onClimateState(ClimateStateCallback)`
  - `onProfiles(ProfilesCallback)`

- [ ] Add `processFrame()` method for raw CAN data
  - Takes canId, data, dlc
  - Calls frameAssembler
  - Notifies subscribers on complete message

- [ ] Add callback storage vectors to private section

- [ ] Implement notification methods
  - `notifyPlugState()`, `notifyChargeState()`, etc.
  - Called when state updates occur

### Testing
- [ ] Code compiles without errors
- [ ] No runtime changes yet (infrastructure only)

---

## Phase 2: BatteryManager (First Domain) ✓

### Create Domain

- [ ] `src/vehicle/domains/BatteryManager.h`
  - State struct with CAN + BAP fields
  - Data source tracking per field
  - Public API: `getState()`, `getPlugState()`, `getChargeState()`
  - Commands: `startCharging()`, `stopCharging()`
  - IDomain implementation
  - Reference to BatteryControlChannel

- [ ] `src/vehicle/domains/BatteryManager.cpp`
  - `setup()`: Register callbacks with BatteryControlChannel
  - `processCanFrame()`: Parse 0x5CA, 0x59E, 0x483
  - CAN parsing methods: `parseBms07()`, `parseBms06()`, `parseMotorHybrid06()`
  - Callback lambdas: Copy BAP data to domain state
  - `isBusy()`: Delegate to bapChannel

### Wire into VehicleManager (Parallel)

- [ ] Add `BatteryManager batteryManager;` member to VehicleManager
- [ ] Add `BatteryManager* battery()` accessor
- [ ] Update `setup()`: Call `batteryManager.setup(&batteryControlChannel)`
- [ ] Update `routeStandardFrame()`: Route battery CAN IDs to BOTH old and new
- [ ] Update `loop()`: Call `batteryManager.loop()`
- [ ] Keep old BatteryDomain running unchanged

### Testing

- [ ] Both old and new domains receive frames
- [ ] Compare state outputs (should match)
- [ ] Test startCharging command through new API
- [ ] Verify BAP callbacks work correctly
- [ ] Check memory usage
- [ ] Measure CAN thread processing time

---

## Phase 3: ClimateManager (Validate Sharing) ✓

### Create Domain

- [ ] `src/vehicle/domains/ClimateManager.h`
  - State struct with CAN + BAP fields
  - Public API: `getState()`, `startClimate()`, `stopClimate()`
  - IDomain implementation
  - Reference to BatteryControlChannel (shared!)

- [ ] `src/vehicle/domains/ClimateManager.cpp`
  - `setup()`: Register for ClimateState callbacks only
  - `processCanFrame()`: Parse 0x66E, 0x5E1
  - CAN parsing methods
  - Callback lambda: Copy climate state

### Wire into VehicleManager (Parallel)

- [ ] Add `ClimateManager climateManager;` member
- [ ] Add `ClimateManager* climate()` accessor
- [ ] Update `setup()`: Call `climateManager.setup(&batteryControlChannel)` (same channel!)
- [ ] Update `routeStandardFrame()`: Route climate CAN IDs to both old and new
- [ ] Update `loop()`: Call `climateManager.loop()`

### Testing

- [ ] Verify BatteryControlChannel notifies both domains
- [ ] Confirm frame assembly happens only once
- [ ] Check that profiles are stored once, not duplicated
- [ ] Test climate commands
- [ ] Verify no performance degradation
- [ ] Confirm mutex hold time still acceptable

---

## Phase 4: Remaining Domains ✓

### BodyManager

- [ ] `src/vehicle/domains/BodyManager.h/cpp`
- [ ] State: doors, locks, windows
- [ ] CAN only: 0x3D0, 0x3D1, 0x583
- [ ] Commands: `lock()`, `unlock()`, `horn()`, `flash()`, `panic()`
- [ ] Wire into VehicleManager
- [ ] Test parallel operation

### DriveManager

- [ ] `src/vehicle/domains/DriveManager.h/cpp`
- [ ] State: ignition, speed, odometer, time
- [ ] CAN only: 0x3C0, 0x0FD, 0x6B2
- [ ] Helper methods: `isIgnitionOn()`, `isVehicleMoving()`
- [ ] Wire into VehicleManager
- [ ] Test parallel operation

### GpsManager

- [ ] `src/vehicle/domains/GpsManager.h/cpp`
- [ ] State: GPS position, satellites, fix type
- [ ] CAN only: 0x484, 0x485, 0x486
- [ ] Wire into VehicleManager
- [ ] Test parallel operation

### RangeManager

- [ ] `src/vehicle/domains/RangeManager.h/cpp`
- [ ] State: range estimation data
- [ ] CAN only: 0x5F5, 0x5F7
- [ ] Wire into VehicleManager
- [ ] Test parallel operation

### Update VehicleManager

- [ ] Add all domain instances
- [ ] Add all accessors
- [ ] Build `std::vector<IDomain*> domains` for iteration
- [ ] Update `setup()` to initialize all domains
- [ ] Update `loop()` to call `loop()` on all domains
- [ ] Keep all old domains running

### Testing

- [ ] All domains receive their frames
- [ ] All states match between old and new
- [ ] All commands work through new APIs
- [ ] No frame drops on CAN thread
- [ ] Performance acceptable

---

## Phase 5: Update External Consumers ✓

### Find Consumers

- [ ] Search for `VehicleState` access
  - HTTP handlers
  - WebSocket code
  - Display/UI code
  - Telemetry code
  - Logging code

- [ ] Search for direct command calls
  - `BatteryControlChannel` usage
  - Domain handler calls
  - Any direct CAN sends

### Update Access Patterns

- [ ] Change `vehicleState.battery.soc` → `vehicleManager->battery()->getState().soc`
- [ ] Change command calls to use domain managers
- [ ] Update JSON serialization if needed
- [ ] Update WebSocket message builders

### Optional Compatibility Layer

If too many places to update at once:

- [ ] Add `VehicleState getLegacyState()` to VehicleManager
- [ ] Populate from all domain managers
- [ ] Mark as deprecated
- [ ] Gradually migrate consumers

### Testing

- [ ] All HTTP endpoints work
- [ ] WebSocket messages correct
- [ ] Commands execute properly
- [ ] Display shows correct data
- [ ] No regressions

---

## Phase 6: Remove Old Code ✓

### Remove Old Domains

- [ ] Delete `src/vehicle/domains/BatteryDomain.h`
- [ ] Delete `src/vehicle/domains/BatteryDomain.cpp`
- [ ] Delete `src/vehicle/domains/ClimateDomain.h`
- [ ] Delete `src/vehicle/domains/ClimateDomain.cpp`
- [ ] Delete `src/vehicle/domains/BodyDomain.h`
- [ ] Delete `src/vehicle/domains/BodyDomain.cpp`
- [ ] Delete `src/vehicle/domains/DriveDomain.h`
- [ ] Delete `src/vehicle/domains/DriveDomain.cpp`
- [ ] Delete `src/vehicle/domains/GpsDomain.h`
- [ ] Delete `src/vehicle/domains/GpsDomain.cpp`
- [ ] Delete `src/vehicle/domains/RangeDomain.h`
- [ ] Delete `src/vehicle/domains/RangeDomain.cpp`

### Remove BapChannelRouter

- [ ] Delete `src/vehicle/bap/BapChannelRouter.h`
- [ ] Delete `src/vehicle/bap/BapChannelRouter.cpp`
- [ ] Remove from VehicleManager includes
- [ ] BAP routing now in VehicleManager directly

### Remove/Update VehicleState

- [ ] If fully migrated: Delete `src/vehicle/VehicleState.h`
- [ ] If keeping compatibility: Mark as deprecated

### Clean VehicleManager

- [ ] Remove old domain instances
- [ ] Remove old routing code
- [ ] Remove BapChannelRouter usage
- [ ] Simplify to thin router only
- [ ] Remove compatibility layer (if added in Phase 5)

### Testing

- [ ] Full compilation successful
- [ ] All tests pass
- [ ] All functionality works
- [ ] No references to old code
- [ ] Clean build with no warnings

---

## Phase 7: Optimization & Polish ✓

### Performance Validation

- [ ] Measure CAN thread processing time
  - Target: <2ms per frame
  - Use `micros()` for timing
  
- [ ] Check for frame drops
  - Compare CanManager RX count vs processed count
  - Should be zero drops under normal load
  
- [ ] Verify mutex contention
  - Check skip count from mutex timeouts
  - Should be minimal/zero

- [ ] Memory usage
  - Measure heap usage
  - Should see savings from shared channel
  - No memory leaks

### Add Telemetry

- [ ] Domain-level statistics
  - Frames processed per domain
  - Last update time per domain
  - Error counts
  
- [ ] State change events
  - Track when important state changes
  - Log for debugging
  
- [ ] Command tracking
  - Success/failure rates
  - Average command duration
  - Timeout counts

### Documentation

- [ ] Update README with new architecture overview
- [ ] Add API documentation for domain managers
- [ ] Document how to add new domains
- [ ] Document how to add new BAP channels
- [ ] Add architecture diagrams
- [ ] Update existing docs that reference old structure

### Code Quality

- [ ] Run linter/formatter
- [ ] Add unit tests for domains (if framework available)
- [ ] Review TODOs and FIXMEs
- [ ] Code review

---

## Rollback Plan

If issues are discovered at any phase:

### Quick Rollback (During Parallel Operation)

1. Comment out new domain initialization in VehicleManager::setup()
2. Comment out new domain routing
3. Keep only old code running
4. Investigate and fix issues
5. Re-enable when ready

### Full Rollback (After Old Code Removed)

1. `git revert` to before Phase 6
2. Fix issues in isolation
3. Re-apply Phase 6 when ready

---

## Success Criteria

### Functional
- ✅ All vehicle data accessible through domain managers
- ✅ All commands work correctly
- ✅ No regressions in functionality

### Performance
- ✅ No frame drops on CAN thread
- ✅ CAN thread processing time <2ms per frame
- ✅ Mutex hold time minimal
- ✅ Memory usage reduced or same

### Code Quality
- ✅ Clean, understandable architecture
- ✅ Easy to add new domains/channels
- ✅ Good test coverage
- ✅ Well documented

---

## Notes

- Always test after each phase before proceeding
- Keep commits small and logical
- Use feature branches for each phase
- Run on hardware frequently during development
- Monitor CAN thread performance closely
- Ask for help if mutex contention increases

---

## Current Status

**Phase:** Not Started  
**Last Updated:** 2026-01-30  
**Next Step:** Phase 1 - Create Infrastructure

---

## Contact

Questions? See `DOMAIN_BASED_ARCHITECTURE.md` for full details.
