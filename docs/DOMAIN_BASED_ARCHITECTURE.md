# Domain-Based CAN Architecture Refactoring

## Overview

This document describes the new domain-based architecture for the e-Golf CAN bus implementation. The goal is to create a cleaner, more maintainable structure where domain managers encapsulate all communication (CAN broadcast + BAP channels) for their respective functional areas.

**Date:** 2026-01-30  
**Status:** Planning/Design Phase

---

## Current Architecture Problems

1. **Scattered Logic**: Battery data comes from both `BatteryDomain` (CAN) and `BatteryControlChannel` (BAP) - hard to get complete state
2. **Messy Routing**: `VehicleManager` has complex routing logic mixing concerns
3. **Duplicate Processing**: BAP frames could be processed multiple times by different handlers
4. **Unclear Ownership**: Not obvious which component owns which data
5. **Poor External API**: Server code must navigate complex structure to access vehicle data
6. **BAP Frame Overhead**: BapFrameAssembler runs for each domain using same channel

---

## New Architecture Principles

1. **Domain Encapsulation**: Each domain owns all data sources (CAN + BAP) for its functional area
2. **Shared BAP Channels**: BAP channels are shared resources that notify multiple domains via callbacks
3. **Thin Router**: VehicleManager is just a frame router, not a data owner
4. **Clean APIs**: External code interacts through domain managers with clear, simple APIs
5. **Minimal CAN Thread Work**: Frame processing optimized to prevent message loss
6. **Single Source of Truth**: Each piece of data has one canonical owner

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      CanManager                              │
│              (Hardware abstraction layer)                    │
└────────────────────────┬────────────────────────────────────┘
                         │ CAN frames
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                   VehicleManager                             │
│                  (Thin frame router)                         │
│                                                              │
│  Frame Router:                                               │
│  - Extended frames → routeBapFrame()                         │
│  - Standard frames → routeStandardFrame()                    │
│                                                              │
│  Domain Accessors:                                           │
│  - battery()  → BatteryManager*                             │
│  - climate()  → ClimateManager*                             │
│  - body()     → BodyManager*                                │
│  - drive()    → DriveManager*                               │
│  - gps()      → GpsManager*                                 │
│  - range()    → RangeManager*                               │
│                                                              │
│  Cross-domain services:                                     │
│  - WakeController (manages wake/sleep)                      │
│  - ActivityTracker (tracks CAN activity)                    │
└──────────┬───────────┬───────────┬────────────┬─────────────┘
           │           │           │            │
           ↓           ↓           ↓            ↓
    ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐
    │ Battery  │ │ Climate  │ │   Body   │ │  Drive   │
    │ Manager  │ │ Manager  │ │ Manager  │ │ Manager  │
    └────┬─────┘ └────┬─────┘ └──────────┘ └──────────┘
         │            │
         └────────┬───┘
                  │
                  ↓
         ┌────────────────────┐
         │BatteryControlChannel│
         │  (Shared BAP 0x25) │
         │  - PlugState       │
         │  - ChargeState     │
         │  - ClimateState    │
         │  - ChargingProfiles│
         └────────────────────┘
```

---

## Component Descriptions

### VehicleManager (Thin Router)

**Responsibility:** Route CAN frames to appropriate handlers

**Key Methods:**
```cpp
void onCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended);
BatteryManager* battery();
ClimateManager* climate();
BodyManager* body();
DriveManager* drive();
GpsManager* gps();
RangeManager* range();
WakeController* wake();
```

**Does NOT:**
- Own vehicle state
- Parse CAN messages
- Process BAP messages
- Implement business logic

---

### IDomain Interface

**Contract for all domain managers:**

```cpp
class IDomain {
public:
    // Core lifecycle
    virtual const char* getName() const = 0;
    virtual bool setup() = 0;
    virtual void loop() = 0;
    
    // CAN message handling (standard frames only)
    virtual void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) = 0;
    
    // Wake management
    virtual void onWakeComplete() {}
    virtual bool isBusy() const { return false; }
};
```

**Note:** Domains do NOT process BAP frames directly. BAP channels handle frames and notify domains via callbacks.

---

### Domain Managers

#### BatteryManager

**Responsibility:** Battery state, charging, power flow

**Data Sources:**
- CAN: 0x5CA (BMS_07), 0x59E (BMS_06), 0x483 (Motor_Hybrid_06)
- BAP: Battery Control Channel (0x25) - functions 0x10, 0x11, 0x19

**Public API:**
```cpp
struct State {
    // From CAN
    float soc;
    float energyWh;
    float maxEnergyWh;
    float temperature;
    float powerKw;
    bool chargingActive;
    bool balancingActive;
    
    // From BAP
    PlugState plugState;
    ChargeState chargeState;
    
    // Data source tracking per field
};

const State& getState() const;
const PlugState& getPlugState() const;
const ChargeState& getChargeState() const;
bool startCharging(uint8_t commandId, uint8_t targetSoc, uint8_t maxCurrent);
bool stopCharging(uint8_t commandId);
```

**Setup:**
```cpp
bool setup(BatteryControlChannel* channel) {
    bapChannel = channel;
    
    // Register for BAP updates
    bapChannel->onPlugState([this](const PlugState& state) {
        this->state.plugState = state;
        this->state.plugStateSource = DataSource::BAP;
        this->state.plugStateUpdate = millis();
    });
    
    bapChannel->onChargeState([this](const ChargeState& state) {
        this->state.chargeState = state;
        // BAP SOC overrides CAN SOC (more accurate)
        if (state.hasValidSoc()) {
            this->state.soc = state.soc;
            this->state.socSource = DataSource::BAP;
        }
    });
    
    return true;
}
```

---

#### ClimateManager

**Responsibility:** Climate control, temperatures

**Data Sources:**
- CAN: 0x66E (Klima_03), 0x5E1 (Ext_Temp)
- BAP: Battery Control Channel (0x25) - function 0x12

**Public API:**
```cpp
struct State {
    // From CAN
    float interiorTemp;
    float exteriorTemp;
    
    // From BAP
    ClimateState climateState;
    
    // Data source tracking per field
};

const State& getState() const;
bool startClimate(uint8_t commandId, float tempCelsius, bool allowBattery);
bool stopClimate(uint8_t commandId);
```

---

#### BodyManager

**Responsibility:** Doors, locks, windows, horn/flash

**Data Sources:**
- CAN: 0x3D0 (TSG_FT_01), 0x3D1 (TSG_BT_01), 0x583 (ZV_02)
- BAP: None

**Public API:**
```cpp
struct State {
    bool driverDoorOpen;
    bool passengerDoorOpen;
    bool driverDoorLocked;
    bool passengerDoorLocked;
    uint8_t driverWindowPos;
    uint8_t passengerWindowPos;
    LockState centralLock;
};

const State& getState() const;
bool lock();
bool unlock();
bool horn();
bool flash();
bool panic();
```

---

#### DriveManager

**Responsibility:** Ignition, speed, odometer, vehicle time

**Data Sources:**
- CAN: 0x3C0 (Klemmen_Status_01), 0x0FD (ESP_21), 0x6B2 (Diagnose_01)
- BAP: None

**Public API:**
```cpp
struct State {
    bool keyInserted;
    bool ignitionOn;
    bool startRequested;
    float speedKmh;
    uint32_t odometerKm;
    VehicleTime time;
};

const State& getState() const;
bool isIgnitionOn() const;
bool isVehicleMoving() const;
```

---

#### GpsManager

**Responsibility:** CAN-based GPS data (from infotainment)

**Data Sources:**
- CAN: 0x484, 0x485, 0x486 (GPS messages)
- BAP: None

---

#### RangeManager

**Responsibility:** Range estimation

**Data Sources:**
- CAN: 0x5F5, 0x5F7 (Range messages)
- BAP: None

---

### Shared BAP Channels

#### BatteryControlChannel

**Responsibility:** LSG 0x25 (Battery Control) BAP protocol handler

**Why Shared:**
- Both BatteryManager and ClimateManager need data from this channel
- Contains large data structures (ChargingProfiles) that should only exist once
- Frame assembly should happen once, not duplicated per domain

**Key Features:**
```cpp
class BatteryControlChannel {
public:
    // Callback registration (domains subscribe)
    using PlugStateCallback = std::function<void(const PlugState&)>;
    using ChargeStateCallback = std::function<void(const ChargeState&)>;
    using ClimateStateCallback = std::function<void(const ClimateState&)>;
    
    void onPlugState(PlugStateCallback callback);
    void onChargeState(ChargeStateCallback callback);
    void onClimateState(ClimateStateCallback callback);
    
    // CAN frame processing (called from VehicleManager on CAN thread)
    bool handlesCanId(uint32_t canId) const;
    void processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    // Shared data access (read-only)
    const PlugState& getPlugState() const;
    const ChargeState& getChargeState() const;
    const ClimateState& getClimateState() const;
    const ChargingProfiles& getProfiles() const;
    
    // Command interface (called from domains)
    bool startCharging(uint8_t commandId, uint8_t targetSoc, uint8_t maxCurrent);
    bool stopCharging(uint8_t commandId);
    bool startClimate(uint8_t commandId, float tempCelsius, bool allowBattery);
    bool stopClimate(uint8_t commandId);
    
    bool isBusy() const;
    void onWakeComplete();
    void loop();  // Command state machine

private:
    // Single instances (not duplicated)
    BapFrameAssembler frameAssembler;
    ChargingProfiles profiles;
    PlugState plugState;
    ChargeState chargeState;
    ClimateState climateState;
    
    // Callbacks
    std::vector<PlugStateCallback> plugStateCallbacks;
    std::vector<ChargeStateCallback> chargeStateCallbacks;
    std::vector<ClimateStateCallback> climateStateCallbacks;
    
    // Command state machine
    // ... (existing implementation)
};
```

**Processing Flow (Optimized for CAN Thread):**

```
CAN Frame arrives (0x17332510)
    ↓
VehicleManager::onCanFrame() [CAN thread, mutex locked]
    ↓
routeBapFrame()
    ↓
BatteryControlChannel::processFrame()
    ↓
BapFrameAssembler::processFrame() [ONE assembly operation]
    ↓ (if complete message)
Parse BAP message by function ID
    ↓
Update internal state (plugState, chargeState, etc.)
    ↓
Notify relevant callbacks (fast - just data copy)
    ↓
BatteryManager callback: copy data to domain state
ClimateManager callback: copy data to domain state (if relevant function)
    ↓
Done - mutex released
```

**Benefits:**
- Frame assembled once
- ChargeProfiles parsed once, stored once
- Callbacks are fast (just data copying)
- No business logic on CAN thread
- Minimal mutex hold time

---

#### ChargingProfileManager

**Relationship:** Child component of BatteryControlChannel

**Responsibility:** Read/modify/write charging profiles

```cpp
class ChargingProfileManager {
public:
    ChargingProfileManager(BatteryControlChannel* parentChannel);
    
    bool requestProfileUpdate(uint8_t profileIndex, 
                             const ProfileUpdateParams& params,
                             std::function<void(bool)> callback);
    
    bool requestProfileRead(uint8_t profileIndex,
                           std::function<void(const Profile&)> callback);
    
    void loop();  // Handle timeouts

private:
    BatteryControlChannel* parentChannel;  // Access to parent state
    // Profile operation state machine
};
```

**Hierarchy:**
```
BatteryControlChannel (owns ChargingProfiles)
    └── ChargingProfileManager (reads/writes profiles)
```

---

### Cross-Domain Services

#### WakeController

**Responsibility:** Vehicle wake/sleep state management

**Extracted from:** Current VehicleManager

```cpp
class WakeController {
public:
    enum WakeState {
        ASLEEP,
        WAKE_REQUESTED,
        WAKING,
        AWAKE,
        WAKE_FAILED
    };
    
    void setup(CanManager* canManager);
    void loop();  // Send keep-alive frames, handle timeouts
    
    bool requestWake(IDomain* requester);
    void onCanActivity();
    
    WakeState getState() const;
    bool isAwake() const;

private:
    WakeState state;
    std::vector<IDomain*> pendingDomains;
    unsigned long lastActivity;
    unsigned long wakeStartTime;
};
```

---

#### ActivityTracker

**Responsibility:** Track CAN bus activity for sleep management

```cpp
class ActivityTracker {
public:
    void onCanActivity();
    bool isActive(uint32_t timeoutMs = 5000) const;
    unsigned long getLastActivityTime() const;
    uint32_t getFrameCount() const;

private:
    unsigned long lastActivity;
    uint32_t frameCount;
};
```

---

## Message Flow Examples

### Standard CAN Frame (BMS_07 - Battery)

```
1. CAN frame 0x5CA arrives
2. VehicleManager::onCanFrame() [CAN thread]
3. Mutex acquired (10ms timeout)
4. routeStandardFrame(0x5CA, data, dlc)
5. BatteryManager::processCanFrame()
6. parseBms07() - extract signals
7. Update state.energyWh, state.chargingActive, etc.
8. Mutex released
```

**Duration on CAN thread:** ~1-2ms

---

### BAP Frame (Charge State)

```
1. CAN frame 0x17332510 arrives
2. VehicleManager::onCanFrame() [CAN thread]
3. Mutex acquired (10ms timeout)
4. routeBapFrame(0x17332510, data, dlc)
5. BatteryControlChannel::processFrame()
6. BapFrameAssembler::processFrame() [single assembly]
7. If complete: processMessage()
8. Parse function 0x11 (charge state)
9. Update internal chargeState
10. Call chargeStateCallbacks (lambda - just copy data)
11. BatteryManager: state.chargeState = receivedState
12. Mutex released
```

**Duration on CAN thread:** ~2-3ms (even with multiple subscribers)

---

### Sending Command

```
1. Server calls: vehicleManager->battery()->startCharging(...)
2. BatteryManager::startCharging() [Main thread]
3. bapChannel->startCharging(...)
4. BatteryControlChannel: queue command
5. Return immediately

Later, in main loop:
6. BatteryControlChannel::loop() [Main thread]
7. State machine: IDLE → UPDATING_PROFILE
8. Update Profile 0 via ChargingProfileManager
9. State machine: UPDATING_PROFILE → REQUESTING_WAKE
10. wakeController->requestWake()
11. State machine: REQUESTING_WAKE → WAITING_FOR_WAKE
12. Wait for CAN activity...
13. State machine: WAITING_FOR_WAKE → SENDING_COMMAND
14. Build BAP command, send via CanManager
15. State machine: SENDING_COMMAND → DONE
```

**Note:** All heavy work happens in main loop, not CAN thread

---

## File Structure

```
src/vehicle/
├── VehicleManager.h/cpp          # Thin router
├── IDomain.h                      # Domain interface
│
├── domains/                       # Domain managers
│   ├── BatteryManager.h/cpp      # Battery + charging
│   ├── ClimateManager.h/cpp      # Climate control
│   ├── BodyManager.h/cpp         # Doors, locks, windows
│   ├── DriveManager.h/cpp        # Ignition, speed, odometer
│   ├── GpsManager.h/cpp          # GPS data
│   └── RangeManager.h/cpp        # Range estimation
│
├── services/                      # Cross-domain services
│   ├── WakeController.h/cpp      # Wake/sleep management
│   └── ActivityTracker.h/cpp     # CAN activity tracking
│
├── protocols/                     # Protocol helpers (keep as-is)
│   ├── BapProtocol.h/cpp         # BAP decoder/encoder
│   ├── BapFrameAssembler.h/cpp   # Multi-frame assembly
│   ├── BroadcastDecoder.h/cpp    # CAN signal extraction
│   └── Tm01Commands.h/cpp        # TM_01 commands
│
└── bap/                           # BAP channels
    ├── BapChannel.h              # Base interface (optional)
    ├── BatteryControlChannel.h/cpp   # LSG 0x25 handler
    └── ChargingProfileManager.h/cpp  # Child of BatteryControlChannel
```

### Changes from Current:
- **Remove:** `VehicleState.h` (state now per-domain)
- **Remove:** `BapChannelRouter.h/cpp` (routing in VehicleManager)
- **Remove:** Old domain handlers in `domains/` (replaced with managers)
- **Keep:** All protocol helpers unchanged
- **Add:** New domain managers
- **Add:** Service classes

---

## External API Usage

### Before (Current):

```cpp
// Scattered access pattern
VehicleState state = vehicleManager.getState();
float soc = state.battery.soc;  // From CAN
bool charging = state.battery.chargeState.charging;  // From BAP?

// Unclear command routing
batteryControlChannel.startCharging(...);
```

### After (New):

```cpp
// Clean, obvious access
BatteryManager* battery = vehicleManager->battery();
float soc = battery->getState().soc;
bool charging = battery->getChargeState().charging;

// Clear ownership
battery->startCharging(commandId, 80, 16);
```

---

## Implementation Plan

### Phase 1: Infrastructure (No Breaking Changes)

**Goal:** Build new structure alongside old code

1. Create `src/vehicle/services/WakeController.h/cpp`
   - Extract wake logic from VehicleManager
2. Create `src/vehicle/services/ActivityTracker.h/cpp`
   - Extract activity tracking
3. Create `src/vehicle/IDomain.h`
   - Define domain interface
4. Update `BatteryControlChannel` for callbacks
   - Add callback registration methods
   - Add `processFrame()` method

**Testing:** Compile check only

---

### Phase 2: First Domain (BatteryManager)

**Goal:** Create and test one complete domain

1. Create `src/vehicle/domains/BatteryManager.h/cpp`
   - Implement IDomain
   - Copy parsing from BatteryDomain
   - Register callbacks with BatteryControlChannel
2. Wire into VehicleManager (parallel with old code)
   - Keep old BatteryDomain running
   - Add new BatteryManager
   - Route to both
3. Test parallel operation
   - Verify identical behavior
   - Compare outputs

**Testing:** Both old and new running, verify match

---

### Phase 3: ClimateManager

**Goal:** Validate shared BAP channel design

1. Create `src/vehicle/domains/ClimateManager.h/cpp`
2. Wire into VehicleManager (parallel)
3. Verify shared channel works correctly
   - Both Battery and Climate receive updates
   - No duplicate processing
   - Memory savings from single profile storage

**Testing:** Shared channel with multiple subscribers

---

### Phase 4: Remaining Domains

**Goal:** Complete migration

1. Create remaining domain managers:
   - BodyManager
   - DriveManager
   - GpsManager
   - RangeManager
2. Wire all into VehicleManager (parallel)

**Testing:** All domains running in parallel

---

### Phase 5: Update External Consumers

**Goal:** Migrate server/websocket/display code

1. Identify all VehicleState consumers
2. Update to new domain API pattern
3. Add compatibility layer if needed

**Testing:** External consumers work with new API

---

### Phase 6: Remove Old Code

**Goal:** Clean up

1. Remove old domain handlers
2. Remove BapChannelRouter
3. Remove VehicleState.h (if fully migrated)
4. Simplify VehicleManager

**Testing:** Full system with new architecture only

---

### Phase 7: Optimization & Polish

**Goal:** Fine-tune

1. Performance validation
   - Measure CAN thread time
   - Verify no frame drops
2. Add telemetry/logging
3. Documentation

---

## Migration Principles

1. **No Big Bang** - Keep old code until new is proven
2. **Incremental Testing** - Test each phase thoroughly
3. **Parallel Operation** - Run old and new simultaneously
4. **Easy Rollback** - Can revert at any phase
5. **Domain-by-Domain** - One at a time, not all at once

---

## Benefits Summary

### Code Quality
- ✅ **Clear Ownership:** Each domain owns its complete functional area
- ✅ **Better Encapsulation:** Implementation details hidden behind clean APIs
- ✅ **Easier Testing:** Domains testable in isolation
- ✅ **Simpler Routing:** VehicleManager is straightforward

### Performance
- ✅ **Minimal CAN Thread Work:** Frame assembly happens once
- ✅ **No Duplicate Processing:** Shared channels prevent redundant work
- ✅ **Efficient Memory:** Single instance of large structures (profiles)
- ✅ **Fast Callbacks:** Just data copying, no business logic

### Maintainability
- ✅ **Easier to Extend:** Adding domains/channels is straightforward
- ✅ **Clear External API:** Server code is simple and obvious
- ✅ **Better Documentation:** Component responsibilities are clear
- ✅ **Fewer Bugs:** Less complexity means fewer places for errors

---

## Open Questions

1. **Command ID Generation:** Auto-generate in domains or pass from caller?
2. **Error Handling:** Return codes, callbacks, or log only?
3. **State History:** Should domains keep historical data?
4. **Testing Strategy:** Hardware available or need extra logging?
5. **Wake Management:** Should domains call WakeController directly or via VehicleManager?

---

## References

- Current implementation: `src/vehicle/VehicleManager.h/cpp`
- BAP protocol: `src/vehicle/protocols/BapProtocol.h/cpp`
- Existing domains: `src/vehicle/domains/*.h/cpp`
- Wake documentation: `docs/sleep-wake.md`
