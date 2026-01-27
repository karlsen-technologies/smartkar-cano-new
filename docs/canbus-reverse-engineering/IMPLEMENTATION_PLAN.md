# CAN Bus Implementation Plan

This document outlines the plan to complete CAN bus functionality for the SmartKar-Cano OCU replacement project.

## Current State

### Implemented (Broadcast Reading - 15 CAN IDs)

| Domain | CAN IDs | Data | Status |
|--------|---------|------|--------|
| BodyDomain | 0x3D0, 0x3D1, 0x583 | Doors, locks, windows | ✅ Complete |
| BatteryDomain | 0x5CA, 0x59E | Charging status, energy (low-res), temp | ✅ Complete |
| DriveDomain | 0x3C0, 0x0FD, 0x6B2 | Ignition, speed, odometer, time | ✅ Complete |
| ClimateDomain | 0x66E, 0x5E1 | Inside/outside temp, standby heat | ✅ Complete |
| GpsDomain | 0x484, 0x485, 0x486 | Position, altitude, heading, DOP | ✅ Complete |
| RangeDomain | 0x5F5, 0x5F7 | Range estimate, tendency, reserve | ✅ Complete |

### Implemented (Commands)

| Command | Method | Status |
|---------|--------|--------|
| Horn | TM_01 (0x5A7) | ✅ Confirmed working |
| Flash lights | TM_01 (0x5A7) | ✅ Confirmed working |
| Lock doors | TM_01 (0x5A7) | ✅ Implemented (needs testing) |
| Unlock doors | TM_01 (0x5A7) | ✅ Implemented (needs testing) |
| Panic | TM_01 (0x5A7) | ✅ Implemented |

### Not Implemented

| Feature | Why Needed | Blocked By |
|---------|------------|------------|
| Climate control | Start/stop preheating | BAP protocol |
| Accurate SOC% | Better than energy-based calc | BAP protocol |
| Charge start/stop | Remote charging control | BAP protocol |
| Plug state | Know if car is plugged in | BAP protocol |
| Server telemetry | Send vehicle data to server | VehicleProvider |
| Server commands | Receive vehicle commands | VehicleHandler |
| Wake-on-CAN | Wake device when car wakes | CanManager as IModule |

---

## Phase 1: BAP Protocol Foundation

**Goal:** Implement the BAP (Bedien- und Anzeigeprotokoll) protocol to enable two-way communication with Battery Control module (Device 0x25).

**Duration:** 2-3 sessions

### 1.1 BAP Protocol Layer

Create `src/vehicle/protocols/BapProtocol.h/.cpp`:

```
BapProtocol
├── Message encoding
│   ├── encodeShortMessage(device, function, opcode, payload) → frame
│   ├── encodeLongMessage(device, function, opcode, payload) → frames[]
│   └── Header format: byte0 = opcode<<4 | device>>2, byte1 = device<<6 | function
│
├── Message decoding
│   ├── decodeHeader(frame) → device, function, opcode
│   ├── isShortMessage() / isLongMessage()
│   └── reassembleLongMessage(frames[]) → payload
│
└── Constants
    ├── DEVICE_BATTERY_CONTROL = 0x25
    ├── CAN_ID_TX = 0x17332501
    └── CAN_ID_RX = 0x17332510
```

**OpCodes:**
- 0x00 = Reset
- 0x01 = Get
- 0x02 = SetGet
- 0x03 = HeartBeat
- 0x04 = Processing
- 0x05 = Indication (unsolicited)
- 0x06 = Void (unused)
- 0x07 = Error

### 1.2 BapDomain

Create `src/vehicle/domains/BapDomain.h/.cpp`:

```
BapDomain
├── Handles extended CAN IDs: 0x17332501, 0x17332510
├── State tracking
│   ├── BapChargeState (soc, available_kwh, charging, ac/dc type)
│   ├── BapPlugState (connected, lock_state)
│   └── BapClimateState (active, target_temp, defrost)
│
├── Commands (via VehicleManager)
│   ├── requestChargeState() → sends Get to function 0x11
│   ├── requestPlugState() → sends Get to function 0x10
│   ├── requestClimateState() → sends Get to function 0x12
│   ├── startClimate(temp) → sends SetGet to function 0x15
│   ├── stopClimate() → sends SetGet to function 0x15
│   ├── startCharging() → sends SetGet to function 0x14
│   └── stopCharging() → sends SetGet to function 0x14
│
└── Response handling
    └── processFrame() → decode BAP response, update state
```

### 1.3 VehicleManager Updates

- Route extended CAN frames (29-bit) to BapDomain
- Add `sendBapCommand()` helper
- Track BAP frame statistics

### 1.4 VehicleState Updates

Add to `VehicleState.h`:

```cpp
struct BapChargeState {
    uint8_t soc = 0;              // 0-100%
    float availableKwh = 0.0f;
    bool charging = false;
    bool acCharging = false;
    bool dcCharging = false;
    unsigned long lastUpdate = 0;
};

struct BapPlugState {
    bool connected = false;
    uint8_t lockState = 0;        // 0=unlocked, 1=locked, 2=error
    unsigned long lastUpdate = 0;
};

struct BapClimateState {
    bool active = false;
    float targetTemp = 0.0f;
    bool defrostActive = false;
    unsigned long lastUpdate = 0;
};
```

---

## Phase 2: Server Integration

**Goal:** Connect vehicle data and commands to the server communication layer.

**Duration:** 1-2 sessions

### 2.1 VehicleProvider

Create `src/providers/VehicleProvider.h/.cpp`:

Implements `ITelemetryProvider` to send vehicle state to server.

```cpp
class VehicleProvider : public ITelemetryProvider {
    const char* getDomain() override { return "vehicle"; }
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void collect(JsonObject& obj) override;
};
```

**Telemetry JSON structure:**
```json
{
    "vehicle": {
        "ignition": "on|off|accessory",
        "speed": 45.2,
        "odometer": 12345,
        "locked": true,
        "doors": {
            "driver": {"open": false, "window": 0},
            "passenger": {"open": false, "window": 0}
        },
        "battery": {
            "soc": 75,
            "energy": 18500,
            "maxEnergy": 24000,
            "temp": 22.5,
            "charging": false
        },
        "climate": {
            "inside": 21.5,
            "outside": 15.0,
            "active": false
        },
        "range": {
            "total": 142,
            "electric": 142,
            "tendency": "stable"
        },
        "gps": {
            "lat": 69.697540,
            "lon": 18.953311,
            "alt": 22,
            "heading": 180.5,
            "satellites": 5,
            "fix": "3D"
        }
    }
}
```

### 2.2 VehicleHandler

Create `src/handlers/VehicleHandler.h/.cpp`:

Implements `ICommandHandler` to receive commands from server.

```cpp
class VehicleHandler : public ICommandHandler {
    const char* getDomain() override { return "vehicle"; }
    CommandResult handleCommand(CommandContext& ctx) override;
};
```

**Supported actions:**
- `vehicle.horn` - Honk horn
- `vehicle.flash` - Flash lights
- `vehicle.lock` - Lock doors
- `vehicle.unlock` - Unlock doors
- `vehicle.climate.start` - Start preheating (params: temp)
- `vehicle.climate.stop` - Stop climate
- `vehicle.charge.start` - Start charging
- `vehicle.charge.stop` - Stop charging

### 2.3 Registration

Update `DeviceController` to register:
- VehicleProvider with CommandRouter
- VehicleHandler with CommandRouter
- Pass VehicleManager reference to both

---

## Phase 3: Additional Signals & Polish

**Goal:** Add remaining broadcast signals, improve reliability.

**Duration:** 1-2 sessions

### 3.1 Additional Broadcast Signals

| CAN ID | Name | Data to Add |
|--------|------|-------------|
| 0x594 | Climate Settings | Target temp, departure timer active |
| 0x588 | Heating Controls | Seat heating, window defrost |
| 0x668 | Klima_02 | Blower voltage, seat heat settings |
| 0x1A5554A8 | NavPos_02 | Map-matched GPS (extended frame) |

### 3.2 CanManager as IModule

Convert CanManager to implement IModule interface:
- `isBusy()` - Returns true if actively receiving frames
- `prepareForSleep()` - Graceful shutdown
- `setActivityCallback()` - Report CAN activity to DeviceController

### 3.3 Wake-on-CAN

- Configure CAN transceiver wake interrupt
- Add wake source detection in DeviceController
- Stay awake while CAN bus is active

### 3.4 Reliability Improvements

- Add timeout handling for BAP requests
- Implement retry logic for failed commands
- Add error reporting to server
- Validate signal values before using

---

## File Structure (After All Phases)

```
src/vehicle/
├── VehicleManager.h/.cpp        # Main orchestrator
├── VehicleState.h               # All state structures
├── protocols/
│   ├── BroadcastDecoder.h       # Signal extraction utilities (exists)
│   ├── Tm01Commands.h           # TM_01 command builder (exists)
│   └── BapProtocol.h/.cpp       # BAP encoding/decoding (NEW)
└── domains/
    ├── BodyDomain.h/.cpp        # Doors, locks, windows (exists)
    ├── BatteryDomain.h/.cpp     # BMS data (exists)
    ├── DriveDomain.h/.cpp       # Ignition, speed, odo (exists)
    ├── ClimateDomain.h/.cpp     # Temperatures (exists)
    ├── GpsDomain.h/.cpp         # GPS from infotainment (exists)
    ├── RangeDomain.h/.cpp       # Range estimation (exists)
    └── BapDomain.h/.cpp         # BAP protocol handling (NEW)

src/providers/
├── DeviceProvider.h/.cpp        # Device telemetry (exists)
├── NetworkProvider.h/.cpp       # Network telemetry (exists)
└── VehicleProvider.h/.cpp       # Vehicle telemetry (NEW)

src/handlers/
├── SystemHandler.h/.cpp         # System commands (exists)
└── VehicleHandler.h/.cpp        # Vehicle commands (NEW)

src/modules/
├── PowerManager.h/.cpp          # PMU control (exists)
├── ModemManager.h/.cpp          # Cellular (exists)
├── LinkManager.h/.cpp           # TCP/server (exists)
└── CanManager.h/.cpp            # CAN bus (exists, needs IModule)
```

---

## BAP Protocol Reference

### CAN IDs

| Direction | CAN ID | Description |
|-----------|--------|-------------|
| TX (to car) | 0x17332501 | Commands to Battery Control |
| RX (from car) | 0x17332510 | Responses from Battery Control |

### Device 0x25 Functions

| Function | Name | Type | Description |
|----------|------|------|-------------|
| 0x10 | PlugState | Get | Plug connection and lock state |
| 0x11 | ChargeState | Get | SOC, energy, charging status |
| 0x12 | ClimateState | Get | Climate active, target temp |
| 0x14 | StartStopCharge | SetGet | Start or stop charging |
| 0x15 | StartStopClimate | SetGet | Start or stop climate |
| 0x18 | OperationMode | SetGet | Set operation mode |
| 0x19 | ProfilesArray | Get | Charging/climate profiles |

### Short Message Format (≤6 byte payload)

```
Byte 0: (opcode << 4) | (device_id >> 2)
Byte 1: (device_id << 6) | function_id
Bytes 2-7: Payload (up to 6 bytes)
```

### Long Message Format (>6 byte payload)

**Start frame:**
```
Byte 0: (opcode << 4) | (device_id >> 2) | 0x80  (bit 7 = long msg)
Byte 1: (device_id << 6) | function_id
Byte 2: Total length (high byte)
Byte 3: Total length (low byte)
Bytes 4-7: First 4 bytes of payload
```

**Continuation frames:**
```
Byte 0: Sequence number (1, 2, 3...)
Bytes 1-7: Next 7 bytes of payload
```

---

## Success Criteria

### Phase 1 Complete When:
- [ ] Can send BAP Get request and receive response
- [ ] ChargeState decoded (SOC%, energy, charging flag)
- [ ] PlugState decoded (connected, locked)
- [ ] ClimateState decoded (active, target temp)
- [ ] Can send climate start/stop command

### Phase 2 Complete When:
- [ ] Vehicle telemetry appears in server dashboard
- [ ] Server can send vehicle commands
- [ ] Commands execute and return status

### Phase 3 Complete When:
- [ ] All documented signals decoded
- [ ] Device wakes on CAN activity
- [ ] Device sleeps when CAN idle
- [ ] Error handling and retries working

---

## References

- `docs/canbus-reverse-engineering/PROTOCOL.md` - Full BAP protocol documentation
- `docs/canbus-reverse-engineering/OCU_FEATURES.md` - Feature requirements and signal mapping
- `docs/canbus-reverse-engineering/egolf_canbus.py` - Python reference implementation
- `docs/canbus-reverse-engineering/broadcast_decoder.py` - Python broadcast decoder
