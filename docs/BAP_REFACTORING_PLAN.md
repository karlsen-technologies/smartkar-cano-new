# BAP Architecture Refactoring Plan

## Executive Summary

This document outlines the plan to refactor the BAP (Bedien- und Anzeigeprotokoll) implementation to properly separate protocol concerns from application logic. The current implementation mixes Battery Control channel-specific logic with the generic BAP protocol layer, making it difficult to maintain and extend.

**Goal:** Create a clean layered architecture where the BAP protocol layer handles transport concerns (multi-frame assembly, encoding/decoding) while channel implementations handle device-specific functionality.

---

## Current Architecture Problems

### 1. Protocol and Application Logic Mixed

**Current state:**
- `BapProtocol.h/cpp` contains Battery Control-specific command builders (`buildClimateStart`, `buildChargeStart`, etc.)
- Protocol layer knows about "climate" and "charging" concepts
- Command builders for Battery Control (Device 0x25) are in the transport layer

**Why this is wrong:**
- BAP is a **transport protocol** used by multiple devices (Battery Control 0x25, Door Locking 0x0D, ENI 0x37)
- Transport layer should only handle message framing, not application semantics
- Violates separation of concerns

### 2. Missing Channel Abstraction

**Current state:**
- `BapDomain` directly implements Battery Control functionality
- No abstraction for multiple BAP channels
- Hard-coded CAN IDs and device-specific logic

**Why this is wrong:**
- Can't easily add other BAP channels (Door Locking, ENI)
- No clear boundary between "BAP protocol" and "Battery Control channel"
- Tight coupling makes testing difficult

### 3. Architecture Doesn't Match Documentation

**Documentation says (PROTOCOL.md:116-131):**
```
Application Layer (Climate Control, Charge Control)
    ↓
BAP Protocol Layer (Message encoding/decoding, multi-frame handling)
    ↓
CAN Driver Layer (Send/receive CAN frames)
    ↓
Hardware Layer (CAN transceiver)
```

**Current implementation:**
```
BapDomain (mixes protocol + Battery Control)
    ↓
BapProtocol (has Battery Control command builders)
    ↓
CanManager
```

---

## Target Architecture

### Layered Design

```
┌─────────────────────────────────────────────────────────────┐
│  Application Layer                                          │
│  - ChargingProfileManager                                   │
│  - High-level controllers                                   │
└────────────────────────────┬────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│  BAP Channel Layer (Device-Specific Protocols)              │
│  - BatteryControlChannel (Device 0x25)                      │
│  - Future: DoorLockingChannel, ENIChannel, etc.             │
│                                                              │
│  Knows about:                                               │
│  - Device-specific functions (PlugState, ChargeState, etc.) │
│  - Data structures (ProfileOperation, temperature encoding) │
│  - Command building for specific device                     │
└────────────────────────────┬────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│  BAP Protocol Layer (Pure Transport)                        │
│  - BapChannelRouter (routes frames to channels)             │
│  - BapFrameAssembler (multi-frame assembly)                 │
│  - BapProtocol (encoding/decoding primitives)               │
│                                                              │
│  Knows about:                                               │
│  - Short vs long messages                                   │
│  - Message groups and indices                               │
│  - OpCodes (GET, SET_GET, STATUS, etc.)                     │
│  - Header encoding (opcode, device ID, function ID)         │
│                                                              │
│  Does NOT know about:                                       │
│  - Specific devices (Battery Control, Door Lock, etc.)      │
│  - Specific functions (PlugState, ChargeState, etc.)        │
│  - Application semantics (climate, charging, etc.)          │
└────────────────────────────┬────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│  CAN Layer                                                   │
│  - CanManager (hardware abstraction)                        │
│  - Routes extended CAN IDs to BAP vs broadcast handlers     │
└────────────────────────────┬────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│  Hardware Layer                                              │
│  - ESP32 TWAI peripheral                                     │
└─────────────────────────────────────────────────────────────┘
```

### Parallel: Broadcast Message Handling

BAP and broadcast messages remain separate architectures:

```
CAN Frame Arrives (VehicleManager::onCanFrame)
       ↓
  ┌────┴────────────────┐
  ↓                     ↓
BAP Router          Broadcast Router
(extended IDs)      (standard IDs + other)
  ↓                     ↓
BAP Channels        Domain Handlers
(BatteryControl)    (Body, Battery, Drive, etc.)
```

**Why separate?**
- Different semantics (request-response vs fire-and-forget)
- Different processing (multi-frame vs single-frame)
- Different routing logic (channel + function vs direct CAN ID)
- Clearer separation of concerns

---

## Directory Structure Changes

### Before
```
src/vehicle/
├── protocols/
│   ├── BapProtocol.h/cpp         (mixed protocol + Battery Control)
│   ├── BroadcastDecoder.h
│   └── Tm01Commands.h
├── domains/
│   ├── BapDomain.h/cpp            (Battery Control via BAP)
│   ├── BatteryDomain.h/cpp        (Battery broadcast messages)
│   ├── BodyDomain.h/cpp
│   └── ...
└── VehicleManager.h/cpp
```

### After
```
src/vehicle/
├── protocols/
│   ├── BapProtocol.h/cpp         (PURE protocol only)
│   ├── BroadcastDecoder.h
│   └── Tm01Commands.h
│
├── bap/                          (NEW)
│   ├── BapChannel.h              (Base interface)
│   ├── BapChannelRouter.h/cpp    (Routes frames to channels)
│   │
│   └── channels/
│       └── BatteryControlChannel.h/cpp  (Device 0x25)
│
├── domains/
│   ├── (BapDomain removed)
│   ├── BatteryDomain.h/cpp       (Broadcast messages only)
│   ├── BodyDomain.h/cpp
│   └── ...
│
└── VehicleManager.h/cpp          (Uses BapChannelRouter)
```

---

## Implementation Phases

### Phase 1: Clean Up BAP Protocol Layer

**Goal:** Make `BapProtocol.h/cpp` pure transport protocol.

**Actions:**

1. **Keep (already correct):**
   - `BapHeader` struct
   - `BapMessage` struct
   - `BapFrameAssembler` class
   - `decodeHeader()`, `encodeHeader()`
   - `encodeShortMessage()`, `encodeLongStart()`, `encodeLongContinuation()`
   - Generic OpCode constants
   - Helper functions: `getShortPayload()`, etc.

2. **Remove (Battery Control specific):**
   - Device ID constants (`DEVICE_BATTERY_CONTROL`, etc.)
   - Function ID namespace (`Function::PLUG_STATE`, etc.)
   - Enums: `ChargeMode`, `ChargeStatus`, `PlugStatus`, `SupplyStatus`
   - Data structs: `PlugStateData`, `ChargeStateData`, `ClimateStateData`
   - Decoder functions: `decodePlugState()`, `decodeChargeState()`, `decodeClimateState()`
   - `ProfileOperation` namespace
   - Command builders: `buildClimateStart()`, `buildChargeStart()`, `buildProfileConfig()`, etc.

3. **Add generic helpers:**
   ```cpp
   // Generic request builders (any device, any function)
   uint8_t buildGetRequest(uint8_t* dest, uint8_t deviceId, uint8_t functionId);
   uint8_t buildSetGetRequest(uint8_t* dest, uint8_t deviceId, uint8_t functionId,
                               const uint8_t* payload, uint8_t payloadLen);
   ```

**Files modified:**
- `src/vehicle/protocols/BapProtocol.h`
- `src/vehicle/protocols/BapProtocol.cpp`

---

### Phase 2: Create BAP Channel Infrastructure

**Goal:** Establish base classes and routing for BAP channels.

#### 2.1: Create `BapChannel` Base Interface

**File:** `src/vehicle/bap/BapChannel.h`

**Purpose:** Abstract interface that all BAP channels implement.

**Key methods:**
```cpp
class BapChannel {
    virtual uint8_t getDeviceId() const = 0;           // LSG ID (e.g., 0x25)
    virtual uint32_t getTxCanId() const = 0;           // Commands to FSG
    virtual uint32_t getRxCanId() const = 0;           // Responses from FSG
    virtual bool handlesCanId(uint32_t canId) const = 0;
    virtual bool processMessage(const BapMessage& msg) = 0;
    virtual const char* getName() const = 0;
    
protected:
    bool sendBapFrame(VehicleManager* mgr, const uint8_t* data, uint8_t len);
};
```

**What it provides:**
- Common interface for all BAP channels
- Channel registration and identification
- Message routing contract

#### 2.2: Create `BapChannelRouter`

**Files:** 
- `src/vehicle/bap/BapChannelRouter.h`
- `src/vehicle/bap/BapChannelRouter.cpp`

**Purpose:** Route CAN frames to appropriate BAP channels.

**Responsibilities:**
- Receive raw CAN frames from `VehicleManager`
- Use `BapFrameAssembler` to handle multi-frame messages
- Route complete messages to registered channels by CAN ID
- Maintain routing table of channels

**Key methods:**
```cpp
class BapChannelRouter {
    void registerChannel(BapChannel* channel);
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    void getStats(...);
    
private:
    BapChannel* channels[MAX_CHANNELS];
    BapFrameAssembler frameAssembler;
};
```

**Flow:**
```
CAN Frame → BapChannelRouter::processFrame()
    ↓
BapFrameAssembler::processFrame()
    ↓ (if complete)
Complete BapMessage
    ↓
findChannel(canId) → BapChannel*
    ↓
channel->processMessage(msg)
```

---

### Phase 3: Create BatteryControlChannel

**Goal:** Implement Battery Control (Device 0x25) as a BAP channel.

**Files:**
- `src/vehicle/bap/channels/BatteryControlChannel.h`
- `src/vehicle/bap/channels/BatteryControlChannel.cpp`

**What moves here from BapProtocol:**
- Device ID constant: `DEVICE_ID = 0x25`
- CAN IDs: `CAN_ID_TX = 0x17332501`, `CAN_ID_RX = 0x17332510`
- Function ID constants (all `Function::*`)
- Enums: `ChargeMode`, `ChargeStatus`, `PlugStatus`, `SupplyStatus`
- Data structures: `PlugStateData`, `ChargeStateData`, `ClimateStateData`
- Payload decoders: `decodePlugState()`, etc.
- Profile operation constants: `ProfileOperation::*`
- Command builders: `buildProfileConfig()`, `buildOperationModeStart()`, etc.

**What moves here from BapDomain:**
- State update logic: `processPlugState()`, `processChargeState()`, `processClimateState()`
- Command methods: `startClimate()`, `stopClimate()`, `startCharging()`, `stopCharging()`
- Request methods: `requestPlugState()`, etc.

**Key interface:**
```cpp
class BatteryControlChannel : public BapChannel {
public:
    // BapChannel interface
    uint8_t getDeviceId() const override { return 0x25; }
    uint32_t getTxCanId() const override { return 0x17332501; }
    uint32_t getRxCanId() const override { return 0x17332510; }
    bool processMessage(const BapMessage& msg) override;
    
    // Battery Control API
    bool startClimate(float tempCelsius = 21.0f);
    bool stopClimate();
    bool startCharging();
    bool stopCharging();
    bool requestPlugState();
    bool requestChargeState();
    bool requestClimateState();
    
    // State access
    const BapPlugState& getPlugState() const;
    const BapChargeState& getChargeState() const;
    const BapClimateState& getClimateState() const;
    
private:
    // Battery Control specifics
    void processPlugState(const uint8_t* payload, uint8_t len);
    void processChargeState(const uint8_t* payload, uint8_t len);
    void processClimateState(const uint8_t* payload, uint8_t len);
    
    PlugStateData decodePlugState(const uint8_t* payload, uint8_t len);
    ChargeStateData decodeChargeState(const uint8_t* payload, uint8_t len);
    ClimateStateData decodeClimateState(const uint8_t* payload, uint8_t len);
    
    uint8_t buildProfileConfig(uint8_t* start, uint8_t* cont, uint8_t mode);
    // ... other command builders
};
```

---

### Phase 4: Update VehicleManager

**Goal:** Integrate `BapChannelRouter` and `BatteryControlChannel`.

**Changes to `VehicleManager.h`:**

```cpp
class VehicleManager {
public:
    // Replace BapDomain access with BatteryControlChannel
    // OLD: BapDomain& bap() { return bapDomain; }
    BatteryControlChannel& batteryControl() { return batteryControlChannel; }
    
private:
    // Remove BapDomain
    // BapDomain bapDomain;
    
    // Add BAP infrastructure
    BapChannelRouter bapRouter;
    BatteryControlChannel batteryControlChannel;
};
```

**Changes to `VehicleManager.cpp`:**

1. **Constructor initialization:**
   ```cpp
   VehicleManager::VehicleManager(CanManager* canMgr)
       : canManager(canMgr)
       , batteryControlChannel(state, this)
       // ... other domains
   {
       // Register BAP channels with router
       bapRouter.registerChannel(&batteryControlChannel);
   }
   ```

2. **Frame routing in `onCanFrame()`:**
   ```cpp
   void VehicleManager::onCanFrame(uint32_t canId, const uint8_t* data, 
                                    uint8_t dlc, bool extended) {
       // Route BAP frames
       if (extended && (canId & 0xFFFF0000) == 0x17330000) {
           if (bapRouter.processFrame(canId, data, dlc)) {
               bapFrames++;
               return;
           }
       }
       
       // Route broadcast frames to domains
       if (bodyDomain.handlesCanId(canId)) { ... }
       // etc.
   }
   ```

**Files modified:**
- `src/vehicle/VehicleManager.h`
- `src/vehicle/VehicleManager.cpp`

---

### Phase 5: Update Dependent Code

**Goal:** Update code that used `BapDomain` to use `BatteryControlChannel`.

**Files to update:**

1. **ChargingProfileManager** (`src/vehicle/ChargingProfileManager.cpp`)
   - Change `bapDomain` references to `batteryControlChannel`
   - API should remain largely the same

2. **VehicleHandler** (`src/handlers/VehicleHandler.cpp`)
   - Change `vehicleManager.bap()` to `vehicleManager.batteryControl()`

3. **VehicleProvider** (`src/providers/VehicleProvider.cpp`)
   - Change state access from `bapDomain` to `batteryControlChannel`

**Search pattern:**
```bash
grep -r "\.bap()" src/
grep -r "bapDomain" src/
```

---

### Phase 6: Remove Old Code

**Goal:** Clean up obsolete files and code.

**Files to remove:**
- `src/vehicle/domains/BapDomain.h`
- `src/vehicle/domains/BapDomain.cpp`

**VehicleManager changes:**
- Remove `#include "domains/BapDomain.h"`
- Remove `BapDomain` member variable
- Remove `bapDomain` from domain list in comments

---

### Phase 7: Testing and Validation

**Goal:** Ensure refactored code works correctly.

**Test plan:**

1. **Compilation:**
   ```bash
   pio run
   ```
   Should compile without errors.

2. **Unit test BAP protocol layer:**
   - Test short message encoding/decoding
   - Test long message encoding/decoding
   - Test multi-frame assembly

3. **Integration test with vehicle:**
   - Connect to car
   - Request PlugState, ChargeState, ClimateState
   - Start/stop climate
   - Verify state updates in VehicleState

4. **Verify no regressions:**
   - ChargingProfileManager still works
   - VehicleHandler commands still work
   - Telemetry still reports correctly

**Success criteria:**
- All code compiles
- Can request and receive Battery Control states
- Can start/stop climate control
- No memory leaks (check with ESP32 heap monitoring)
- Performance unchanged (check frame processing times)

---

## Benefits of Refactored Architecture

### 1. Clear Separation of Concerns

- **Protocol layer** handles transport (framing, assembly)
- **Channel layer** handles device-specific logic
- **Application layer** handles user intent

Each layer has a single responsibility.

### 2. Extensibility

**Adding a new BAP channel (e.g., Door Locking):**

1. Create `DoorLockingChannel.h/cpp` in `src/vehicle/bap/channels/`
2. Implement `BapChannel` interface
3. Register with `BapChannelRouter` in `VehicleManager`
4. Done! No changes to protocol layer needed.

### 3. Testability

- Protocol layer can be tested independently with mock frames
- Channels can be tested independently with mock protocol layer
- Clear interfaces make mocking easy

### 4. Maintainability

- Easy to understand: "Where is climate start logic?" → `BatteryControlChannel`
- Easy to debug: routing is explicit
- Matches documentation architecture

### 5. Reusability

- `BapProtocol` layer could be used in other projects
- `BapChannelRouter` is generic (works for any channel)
- Channel implementations encapsulate device knowledge

---

## Migration Path

### Incremental Approach

1. **Create new structure alongside old** (Phases 1-3)
   - Old `BapDomain` still exists
   - New `BatteryControlChannel` created
   - Both can coexist temporarily

2. **Switch VehicleManager to use new structure** (Phase 4)
   - Change routing logic
   - Test thoroughly

3. **Update dependent code** (Phase 5)
   - One file at a time
   - Test after each change

4. **Remove old code** (Phase 6)
   - Only after everything works
   - Clean up

### Rollback Plan

If issues arise, we can easily rollback:
- Git has full history
- Old code not deleted until Phase 6
- New structure is additive (doesn't break old code until Phase 4)

---

## Open Questions

1. **Should `BapChannelRouter` be a singleton or managed by `VehicleManager`?**
   - **Decision:** Managed by `VehicleManager` (better for testing, no globals)

2. **Should channels hold a reference to `VehicleManager` or `CanManager`?**
   - **Decision:** `VehicleManager` (cleaner abstraction, VehicleManager knows routing)

3. **Should we keep `BapDomain` naming or change to `BatteryControlChannel` everywhere?**
   - **Decision:** `BatteryControlChannel` (clearer intent, matches architecture)

4. **What about wake-up sequence and keep-alive messages?**
   - **Decision:** Keep in `VehicleManager` or create `BapWakeManager` (separate concern from protocol)

---

## Timeline Estimate

Assuming focused implementation:

- **Phase 1** (Protocol cleanup): 2-3 hours
- **Phase 2** (Infrastructure): 3-4 hours  
- **Phase 3** (BatteryControlChannel): 4-5 hours
- **Phase 4** (VehicleManager integration): 2-3 hours
- **Phase 5** (Update dependents): 1-2 hours
- **Phase 6** (Cleanup): 1 hour
- **Phase 7** (Testing): 3-4 hours

**Total: ~16-22 hours of focused work**

Can be split into multiple sessions/days.

---

## Conclusion

This refactoring transforms the BAP implementation from a monolithic, Battery Control-specific solution into a clean, layered architecture that properly separates transport protocol from application logic. 

**The result:**
- Protocol layer that could be reused in other projects
- Easy addition of new BAP channels
- Clear boundaries and responsibilities
- Maintainable, testable code
- Architecture that matches documentation

**Next steps:**
1. Review and approve this plan
2. Commit current state
3. Begin Phase 1 implementation
