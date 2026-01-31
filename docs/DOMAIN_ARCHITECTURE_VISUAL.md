# Domain-Based Architecture - Visual Reference

Quick visual reference for the new domain-based CAN architecture.

---

## Component Hierarchy

```
smartkar-cano-new
│
├── CanManager (Hardware Layer)
│   └── ESP32 TWAI Controller
│       ├── TX: GPIO 47
│       ├── RX: GPIO 21
│       └── Speed: 500kbps
│
└── VehicleManager (Router Layer)
    │
    ├── Services (Cross-Domain)
    │   ├── WakeController
    │   │   ├── State Machine
    │   │   ├── Keep-alive frames
    │   │   └── Wake coordination
    │   │
    │   └── ActivityTracker
    │       ├── Frame counting
    │       ├── Last activity time
    │       └── Sleep management
    │
    ├── BAP Channels (Shared Resources)
    │   │
    │   └── BatteryControlChannel (LSG 0x25)
    │       ├── BapFrameAssembler
    │       ├── ChargingProfileManager
    │       ├── Data:
    │       │   ├── PlugState
    │       │   ├── ChargeState
    │       │   ├── ClimateState
    │       │   └── ChargingProfiles
    │       ├── Callbacks:
    │       │   ├── onPlugState()
    │       │   ├── onChargeState()
    │       │   ├── onClimateState()
    │       │   └── onProfiles()
    │       └── Commands:
    │           ├── startCharging()
    │           ├── stopCharging()
    │           ├── startClimate()
    │           └── stopClimate()
    │
    └── Domain Managers (Functional Areas)
        │
        ├── BatteryManager
        │   ├── CAN IDs: 0x5CA, 0x59E, 0x483
        │   ├── BAP: via BatteryControlChannel
        │   ├── State:
        │   │   ├── SOC, energy, temperature
        │   │   ├── Power, charging status
        │   │   ├── PlugState (from BAP)
        │   │   └── ChargeState (from BAP)
        │   └── API:
        │       ├── getState()
        │       ├── getPlugState()
        │       ├── getChargeState()
        │       ├── startCharging()
        │       └── stopCharging()
        │
        ├── ClimateManager
        │   ├── CAN IDs: 0x66E, 0x5E1
        │   ├── BAP: via BatteryControlChannel
        │   ├── State:
        │   │   ├── Interior/exterior temp
        │   │   └── ClimateState (from BAP)
        │   └── API:
        │       ├── getState()
        │       ├── startClimate()
        │       └── stopClimate()
        │
        ├── BodyManager
        │   ├── CAN IDs: 0x3D0, 0x3D1, 0x583
        │   ├── BAP: None
        │   ├── State:
        │   │   ├── Door open/locked
        │   │   ├── Window positions
        │   │   └── Central lock
        │   └── API:
        │       ├── getState()
        │       ├── lock() / unlock()
        │       ├── horn() / flash()
        │       └── panic()
        │
        ├── DriveManager
        │   ├── CAN IDs: 0x3C0, 0x0FD, 0x6B2
        │   ├── BAP: None
        │   ├── State:
        │   │   ├── Ignition status
        │   │   ├── Speed
        │   │   ├── Odometer
        │   │   └── Vehicle time
        │   └── API:
        │       ├── getState()
        │       ├── isIgnitionOn()
        │       └── isVehicleMoving()
        │
        ├── GpsManager
        │   ├── CAN IDs: 0x484, 0x485, 0x486
        │   ├── BAP: None
        │   ├── State:
        │   │   ├── Position (lat/lon)
        │   │   ├── Altitude, heading
        │   │   ├── Satellites, DOP
        │   │   └── Fix type
        │   └── API:
        │       ├── getState()
        │       └── hasValidFix()
        │
        └── RangeManager
            ├── CAN IDs: 0x5F5, 0x5F7
            ├── BAP: None
            ├── State:
            │   ├── Total/electric range
            │   ├── Consumption
            │   └── Reserve warning
            └── API:
                └── getState()
```

---

## Message Flow: Standard CAN Frame

```
┌─────────────┐
│  CAN Frame  │  0x5CA (BMS_07) arrives
│   0x5CA     │
└──────┬──────┘
       │
       ▼
┌──────────────────────────────────────┐
│        CanManager (Core 0)           │
│  - Reads from TWAI hardware          │
│  - Calls frameCallback               │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│    VehicleManager::onCanFrame()      │
│  - Acquire mutex (10ms timeout)      │
│  - Track activity                    │
│  - Route by CAN ID                   │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  routeStandardFrame(0x5CA)           │
│  - switch(canId)                     │
│  - case 0x5CA: BatteryManager        │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  BatteryManager::processCanFrame()   │
│  - parseBms07(data, dlc)             │
│  - Extract signals:                  │
│    • energyWh (bits 12|11@LE)        │
│    • maxEnergyWh (bits 32|11@LE)     │
│    • chargingActive (bit 23)         │
│    • balancingActive (bits 30|2@LE)  │
│  - Update state with timestamps      │
│  - Mark source as CAN_STD            │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  Release mutex, return               │
│  Duration: ~1-2ms                    │
└──────────────────────────────────────┘

Total time on CAN thread: ~1-2ms
```

---

## Message Flow: BAP Frame (Extended CAN)

```
┌─────────────┐
│  CAN Frame  │  0x17332510 (BAP Response) arrives
│ 0x17332510  │
└──────┬──────┘
       │
       ▼
┌──────────────────────────────────────┐
│        CanManager (Core 0)           │
│  - Reads from TWAI hardware          │
│  - Extended frame detected           │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│    VehicleManager::onCanFrame()      │
│  - Acquire mutex                     │
│  - Track activity                    │
│  - extended == true                  │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  routeBapFrame(0x17332510)           │
│  - Check BatteryControlChannel       │
│    handlesCanId(0x17332510) → true   │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  BatteryControlChannel               │
│    ::processFrame()                  │
│  - BapFrameAssembler::processFrame() │
│  - Assemble multi-frame message      │
│  - Returns: true (complete)          │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  BatteryControlChannel               │
│    ::processMessage()                │
│  - Parse function ID: 0x11           │
│  - Process ChargeState               │
│  - Update internal chargeState       │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  Notify Callbacks                    │
│  - Call chargeStateCallbacks[0]      │
│    (BatteryManager lambda)           │
│  - Lambda: Copy to domain state      │
│  - Mark source as BAP                │
│  - Update timestamp                  │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│  Release mutex, return               │
│  Duration: ~2-3ms                    │
└──────────────────────────────────────┘

Total time on CAN thread: ~2-3ms
(Even with multiple subscribers - callbacks are fast!)
```

---

## Message Flow: Sending Command

```
┌─────────────────────────────────────┐
│  Server / External Code             │
│  vehicleManager->battery()          │
│    ->startCharging(cmdId, 80, 16)   │
└──────────────┬──────────────────────┘
               │ (Main Thread)
               ▼
┌─────────────────────────────────────┐
│  BatteryManager::startCharging()    │
│  - Delegate to BAP channel          │
│  - Return immediately                │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  BatteryControlChannel               │
│    ::startCharging()                │
│  - Queue command                     │
│  - Set state = IDLE                  │
│  - Store parameters                  │
│  - Return true                       │
└──────────────┬──────────────────────┘
               │
               │ ... (return to caller)
               │
               │ Later, in main loop:
               ▼
┌─────────────────────────────────────┐
│  VehicleManager::loop()              │
│  - batteryControlChannel.loop()      │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  BatteryControlChannel::loop()       │
│  State Machine:                      │
│                                      │
│  IDLE → UPDATING_PROFILE             │
│    • Update Profile 0                │
│    • Set target SOC, max current     │
│                                      │
│  UPDATING_PROFILE → REQUESTING_WAKE  │
│    • Profile update complete         │
│                                      │
│  REQUESTING_WAKE → WAITING_FOR_WAKE  │
│    • Call wakeController->request()  │
│    • Send wake frames                │
│                                      │
│  WAITING_FOR_WAKE → SENDING_COMMAND  │
│    • CAN activity detected           │
│    • Vehicle is awake                │
│                                      │
│  SENDING_COMMAND → DONE              │
│    • Build BAP SetGet command        │
│    • Function 0x11 (charge state)    │
│    • Payload: start charging         │
│    • Send via CanManager             │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  Command complete, state = DONE      │
└─────────────────────────────────────┘

All heavy work in main loop - CAN thread unaffected!
```

---

## Data Source Priority

When the same data is available from multiple sources, priority is:

```
1. BAP (Extended CAN, 29-bit)
   └── Most detailed and reliable
       Example: SOC from ChargeState

2. Standard CAN (11-bit broadcast)
   └── Good for real-time data
       Example: SOC from BMS_07

3. Computed
   └── Derived/calculated values
       Example: State of health from energy history
```

**Implementation:**
```cpp
struct BatteryState {
    float soc;              // Current value
    DataSource socSource;   // Where it came from
    unsigned long socUpdate; // Timestamp

    // If BAP updates arrive, override CAN data
    void updateFromBap(float newSoc) {
        if (socSource != DataSource::BAP || newSoc != soc) {
            soc = newSoc;
            socSource = DataSource::BAP;
            socUpdate = millis();
        }
    }

    // CAN updates only apply if no recent BAP data
    void updateFromCan(float newSoc) {
        if (socSource != DataSource::BAP) {
            soc = newSoc;
            socSource = DataSource::CAN_STD;
            socUpdate = millis();
        }
    }
};
```

---

## Thread Safety

### CAN Thread (Core 0)
- Receives frames from TWAI hardware
- Minimal processing only
- Acquires mutex before routing
- Timeout: 10ms (skips frame if busy)
- Target duration: <3ms per frame

### Main Thread (Core 1)
- Runs domain `loop()` methods
- Processes command state machines
- Sends commands to CAN
- Acquires mutex for state access
- Timeout: 100ms

### Mutex Strategy
```cpp
// In VehicleManager::onCanFrame() [CAN thread]
if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
    // Mutex busy - skip this frame
    // Will be caught by frame loss tracking
    return;
}

// ... route frame ...

xSemaphoreGive(mutex);
```

---

## External API Examples

### Read Battery State

```cpp
BatteryManager* battery = vehicleManager->battery();

const auto& state = battery->getState();
Serial.printf("SOC: %.1f%% (source: %s)\n", 
              state.soc, 
              dataSourceToString(state.socSource));
Serial.printf("Energy: %.1f / %.1f Wh\n", 
              state.energyWh, 
              state.maxEnergyWh);
Serial.printf("Temperature: %.1f°C\n", state.temperature);
Serial.printf("Power: %.2f kW\n", state.powerKw);

const auto& plug = battery->getPlugState();
Serial.printf("Plug: %s, Locked: %s\n",
              plug.connected ? "connected" : "disconnected",
              plug.locked ? "yes" : "no");
```

### Send Commands

```cpp
BatteryManager* battery = vehicleManager->battery();
ClimateManager* climate = vehicleManager->climate();
BodyManager* body = vehicleManager->body();

// Start charging to 80%, max 16A
if (battery->startCharging(getNextCommandId(), 80, 16)) {
    Serial.println("Charging started");
}

// Start climate to 21°C, allow battery use
if (climate->startClimate(getNextCommandId(), 21.0, true)) {
    Serial.println("Climate started");
}

// Lock doors
if (body->lock()) {
    Serial.println("Doors locked");
}
```

### Check Multiple Domains

```cpp
bool canCharge = vehicleManager->battery()->getPlugState().connected
              && vehicleManager->body()->getState().centralLock == LOCKED
              && !vehicleManager->drive()->isIgnitionOn();

if (canCharge) {
    vehicleManager->battery()->startCharging(...);
}
```

---

## File Organization

```
src/vehicle/
│
├── VehicleManager.h          # Thin router
├── VehicleManager.cpp
├── IDomain.h                  # Domain interface
│
├── domains/                   # NEW: Domain managers
│   ├── BatteryManager.h
│   ├── BatteryManager.cpp
│   ├── ClimateManager.h
│   ├── ClimateManager.cpp
│   ├── BodyManager.h
│   ├── BodyManager.cpp
│   ├── DriveManager.h
│   ├── DriveManager.cpp
│   ├── GpsManager.h
│   ├── GpsManager.cpp
│   ├── RangeManager.h
│   └── RangeManager.cpp
│
├── services/                  # NEW: Cross-domain services
│   ├── WakeController.h
│   ├── WakeController.cpp
│   ├── ActivityTracker.h
│   └── ActivityTracker.cpp
│
├── protocols/                 # KEEP: Protocol helpers
│   ├── BapProtocol.h
│   ├── BapProtocol.cpp
│   ├── BapFrameAssembler.h
│   ├── BapFrameAssembler.cpp
│   ├── BroadcastDecoder.h
│   ├── BroadcastDecoder.cpp
│   └── Tm01Commands.h/cpp
│
└── bap/                       # KEEP: BAP channels
    ├── BapChannel.h
    ├── BatteryControlChannel.h
    ├── BatteryControlChannel.cpp
    └── ChargingProfileManager.h/cpp
```

---

## Summary: Before vs After

### Before
```
❌ Complex: Battery data scattered across BatteryDomain + BatteryControlChannel
❌ Unclear: Which component owns which data?
❌ Inefficient: BAP frames processed multiple times
❌ Messy: VehicleManager has routing + state + coordination logic
❌ Hard to use: Server must navigate complex structure
```

### After
```
✅ Simple: All battery data in BatteryManager
✅ Clear: Each domain owns its functional area
✅ Efficient: BAP frames assembled once, shared via callbacks
✅ Clean: VehicleManager is just a router
✅ Easy: vehicleManager->battery()->getState()
```

---

See `DOMAIN_BASED_ARCHITECTURE.md` for complete details.
