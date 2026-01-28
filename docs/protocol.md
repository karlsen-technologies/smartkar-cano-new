# Protocol Specification

This document describes the JSON protocol used for communication between the SmartKar-Cano device and the server. This specification is intended for implementing compatible server software.

## Transport Layer

| Property | Value |
|----------|-------|
| Protocol | TCP |
| Default Port | 4589 |
| Message Format | Newline-delimited JSON (`\n` terminated) |
| Encoding | UTF-8 |
| Max Message Size | 1024 bytes (device buffer limit) |

## Connection Lifecycle

```
┌──────────┐                                    ┌──────────┐
│  Device  │                                    │  Server  │
└────┬─────┘                                    └────┬─────┘
     │                                               │
     │──────────── TCP Connect ─────────────────────►│
     │                                               │
     │──────────── auth ────────────────────────────►│
     │                                               │
     │◄─────────── auth (response) ──────────────────│
     │                                               │
     │◄─────────── command (anytime) ────────────────│
     │──────────── response ────────────────────────►│
     │                                               │
     │──────────── state (periodic) ────────────────►│
     │                                               │
     │──────────── event (immediate) ───────────────►│
     │                                               │
     │──────────── bye ─────────────────────────────►│
     │──────────── TCP Disconnect ──────────────────►│
     │                                               │
```

## Message Types

| Type | Direction | Purpose |
|------|-----------|---------|
| `auth` | Both | Authentication handshake |
| `command` | Server → Device | Command to execute |
| `response` | Device → Server | Response to a command |
| `state` | Device → Server | Periodic telemetry snapshot |
| `event` | Device → Server | Immediate notification of important changes |
| `bye` | Device → Server | Device going offline |

---

## Authentication

### Auth Request (Device → Server)

Sent immediately after TCP connection is established.

```json
{"type":"auth","data":{"ccid":"8947080012345678901"}}
```

| Field | Type | Description |
|-------|------|-------------|
| `ccid` | string | SIM card ICCID (19-20 digits) |

### Auth Response (Server → Device)

**Accepted:**
```json
{"type":"auth","data":{"ok":true}}
```

**Rejected:**
```json
{"type":"auth","data":{"ok":false,"reason":"Unknown device"}}
```

| Field | Type | Description |
|-------|------|-------------|
| `ok` | boolean | Whether authentication succeeded |
| `reason` | string | (Optional) Rejection reason |

**Device Behavior:**
- On `ok: true`: Transition to connected state, begin sending telemetry
- On `ok: false`: Close connection, may retry after timeout

---

## Commands

### Command Message (Server → Device)

```json
{"type":"command","data":{"id":1,"action":"system.reboot"}}
{"type":"command","data":{"id":2,"action":"system.sleep","duration":300}}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Unique command identifier (for correlating responses) |
| `action` | string | Command action in format `domain.action` |
| *...params* | various | Command parameters flattened into data object |

### Response Message (Device → Server)

```json
{"type":"response","data":{"id":1,"ok":true}}
{"type":"response","data":{"id":2,"ok":true,"wakeMode":"timer"}}
{"type":"response","data":{"id":3,"ok":false,"status":"error","error":"Invalid parameter"}}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Matches the command `id` |
| `ok` | boolean | Whether command succeeded |
| `status` | string | Status code (when `ok` is false): `error`, `pending`, `not_supported` |
| `error` | string | Error message (when `ok` is false) |
| *...data* | various | Response data flattened into data object |

### Response Status Values

| Status | Description |
|--------|-------------|
| *(ok: true)* | Command executed successfully |
| `error` | Command failed (see `error` field) |
| `pending` | Command accepted, will complete asynchronously (watch for events) |
| `not_supported` | Unknown command or domain |

---

## Implemented Commands

### System Domain

#### system.reboot

Restarts the device.

```json
{"type":"command","data":{"id":1,"action":"system.reboot"}}
```

**Response:**
```json
{"type":"response","data":{"id":1,"ok":true,"message":"Rebooting"}}
```

**Note:** Response is sent before reboot. Device will disconnect and reconnect after restart.

---

#### system.sleep

Requests the device to enter deep sleep.

```json
{"type":"command","data":{"id":2,"action":"system.sleep","duration":300}}
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `duration` | integer | (Optional) Sleep duration in seconds. 0 or omitted = wake on interrupt only |

**Response:**
```json
{"type":"response","data":{"id":2,"ok":true,"duration":300,"wakeMode":"timer_or_interrupt"}}
```

| Response Field | Description |
|----------------|-------------|
| `duration` | Confirmed sleep duration |
| `wakeMode` | `"timer_or_interrupt"` or `"interrupt_only"` |

**Behavior:** Device will send response, then send `bye`, then enter deep sleep.

---

#### system.wakeup

Acknowledges that the device is awake. No-op command that confirms the device received the wakeup signal.

```json
{"type":"command","data":{"id":3,"action":"system.wakeup"}}
```

**Response:**
```json
{"type":"response","data":{"id":3,"ok":true,"message":"Device is awake","uptime":1234}}
```

---

#### system.telemetry

Forces immediate telemetry collection and send.

```json
{"type":"command","data":{"id":4,"action":"system.telemetry"}}
```

**Response:**
```json
{"type":"response","data":{"id":4,"ok":true,"message":"Telemetry collected"}}
```

**Note:** The state will be sent as a separate `state` message.

---

#### system.info

Returns detailed device information.

```json
{"type":"command","data":{"id":5,"action":"system.info"}}
```

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":5,
    "ok":true,
    "chipModel":"ESP32-S3",
    "chipRevision":0,
    "chipCores":2,
    "cpuFreqMHz":240,
    "freeHeap":245760,
    "minFreeHeap":234500,
    "heapSize":327680,
    "flashSize":16777216,
    "flashSpeed":80000000,
    "sdkVersion":"v4.4.6",
    "uptime":125340
  }
}
```

---

### Vehicle Domain

#### vehicle.wake

Wakes the vehicle CAN bus and initializes BAP communication.

```json
{"type":"command","data":{"id":10,"action":"vehicle.wake"}}
```

**Response:**
```json
{"type":"response","data":{"id":10,"ok":true,"message":"Wake requested"}}
```

**Behavior:** 
- Sends wake frame (0x17330301) to vehicle
- Sends BAP init frame (0x1B000067)
- Starts 500ms keep-alive heartbeat
- Returns immediately (check `vehicleAwake` in next state message to confirm)

---

#### vehicle.getState

Returns current vehicle state snapshot.

```json
{"type":"command","data":{"id":11,"action":"vehicle.getState"}}
```

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":11,
    "ok":true,
    "battery":{...},
    "drive":{...},
    "body":{...},
    "climate":{...},
    "plug":{...},
    "vehicleAwake":true,
    "canFrameCount":125340
  }
}
```

See [Vehicle State Structure](#vehicle-state-structure) for complete field listing.

---

#### vehicle.startCharging

Starts or updates charging session with specified profile.

```json
{"type":"command","data":{"id":12,"action":"vehicle.startCharging","targetSoc":80,"maxAmps":16}}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `targetSoc` | integer | No | Target SOC % (50-100, default 100) |
| `maxAmps` | integer | No | Max charging current (6-32A, default 32) |

**Response (Immediate):**
```json
{"type":"response","data":{"id":12,"ok":false,"status":"pending"}}
```

**Event (When Complete):**
```json
{"type":"event","data":{"domain":"vehicle","name":"commandCompleted","action":"startCharging"}}
```

**Or on failure:**
```json
{"type":"event","data":{"domain":"vehicle","name":"commandFailed","action":"startCharging","error":"Not plugged in"}}
```

---

#### vehicle.stopCharging

Stops the current charging session.

```json
{"type":"command","data":{"id":13,"action":"vehicle.stopCharging"}}
```

**Response:**
```json
{"type":"response","data":{"id":13,"ok":false,"status":"pending"}}
```

See `vehicle.startCharging` for event responses.

---

#### vehicle.startClimate

Starts climate control (heating/cooling/ventilation).

```json
{"type":"command","data":{"id":14,"action":"vehicle.startClimate"}}
```

**Response:**
```json
{"type":"response","data":{"id":14,"ok":false,"status":"pending"}}
```

**Note:** Temperature and mode are controlled by vehicle settings, not API parameters.

---

#### vehicle.stopClimate

Stops climate control.

```json
{"type":"command","data":{"id":15,"action":"vehicle.stopClimate"}}
```

**Response:**
```json
{"type":"response","data":{"id":15,"ok":false,"status":"pending"}}
```

---

#### vehicle.startChargingAndClimate

Starts both charging and climate control simultaneously (efficient combo command).

```json
{"type":"command","data":{"id":16,"action":"vehicle.startChargingAndClimate","targetSoc":80,"maxAmps":16}}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `targetSoc` | integer | No | Target SOC % (50-100, default 100) |
| `maxAmps` | integer | No | Max charging current (6-32A, default 32) |

**Response:**
```json
{"type":"response","data":{"id":16,"ok":false,"status":"pending"}}
```

---

### Charging Profile Domain

#### chargingProfile.setProfile

Sets charging profile timers (3 profiles supported).

```json
{
  "type":"command",
  "data":{
    "id":20,
    "action":"chargingProfile.setProfile",
    "profile":1,
    "enabled":true,
    "startHour":22,
    "startMinute":0,
    "endHour":6,
    "endMinute":30,
    "targetSoc":80,
    "maxAmps":16,
    "climateEnabled":true
  }
}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `profile` | integer | Yes | Profile number (1-3) |
| `enabled` | boolean | No | Enable this profile (default: true) |
| `startHour` | integer | Yes | Start time hour (0-23) |
| `startMinute` | integer | Yes | Start time minute (0-59) |
| `endHour` | integer | Yes | End time hour (0-23) |
| `endMinute` | integer | Yes | End time minute (0-59) |
| `targetSoc` | integer | No | Target SOC % (50-100, default 100) |
| `maxAmps` | integer | No | Max charging current (6-32A, default 32) |
| `climateEnabled` | boolean | No | Also start climate (default: false) |

**Response:**
```json
{"type":"response","data":{"id":20,"ok":true,"message":"Profile 1 updated"}}
```

**Note:** Profiles are stored in device NVS. Time-based execution happens locally on device.

---

#### chargingProfile.getProfile

Gets a specific charging profile.

```json
{"type":"command","data":{"id":21,"action":"chargingProfile.getProfile","profile":1}}
```

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":21,
    "ok":true,
    "profile":1,
    "enabled":true,
    "startHour":22,
    "startMinute":0,
    "endHour":6,
    "endMinute":30,
    "targetSoc":80,
    "maxAmps":16,
    "climateEnabled":true
  }
}
```

---

#### chargingProfile.getAllProfiles

Gets all 3 charging profiles.

```json
{"type":"command","data":{"id":22,"action":"chargingProfile.getAllProfiles"}}
```

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":22,
    "ok":true,
    "profiles":[
      {"profile":1,"enabled":true,...},
      {"profile":2,"enabled":false,...},
      {"profile":3,"enabled":true,...}
    ]
  }
}
```

---

## State Messages

### State Message Structure

Sent periodically (30s when vehicle awake, 5min when asleep) with full telemetry snapshot.

```json
{
  "type":"state",
  "data":{
    "device":{...},
    "network":{...},
    "vehicle":{...}
  }
}
```

Multiple domains are included in a single state message.

---

### Device Domain

```json
{
  "device":{
    "uptime":125340,
    "freeHeap":245760,
    "wakeCause":"modem_ri",
    "batteryVoltage":4.15,
    "batteryPercent":85,
    "charging":true,
    "vbusConnected":true
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `uptime` | integer | Milliseconds since boot |
| `freeHeap` | integer | Free heap memory in bytes |
| `wakeCause` | string | Wake reason: `fresh_boot`, `timer`, `gpio`, `modem_ri`, `unknown` |
| `batteryVoltage` | float | Battery voltage (V) |
| `batteryPercent` | integer | Battery percentage (0-100) |
| `charging` | boolean | Whether battery is charging |
| `vbusConnected` | boolean | Whether USB/external power is connected |

---

### Network Domain

```json
{
  "network":{
    "modemState":"connected",
    "signalStrength":-75,
    "simCCID":"8947080012345678901",
    "modemConnected":true,
    "linkConnected":true,
    "linkState":"connected"
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `modemState` | string | `off`, `starting`, `configuring`, `searching`, `registered`, `connected`, `error` |
| `signalStrength` | integer | Signal strength in dBm |
| `simCCID` | string | SIM card ICCID |
| `modemConnected` | boolean | Whether modem has internet |
| `linkConnected` | boolean | Whether TCP link is connected |
| `linkState` | string | `disconnected`, `connecting`, `authenticating`, `connected` |

---

### Vehicle Domain

<a name="vehicle-state-structure"></a>

Full vehicle telemetry from CAN bus and BAP protocol.

```json
{
  "vehicle":{
    "battery":{
      "soc":85.0,
      "socSource":"bap",
      "charging":true,
      "chargingSource":"bap",
      "chargingMode":1,
      "chargingStatus":2,
      "chargingAmps":16,
      "targetSoc":100,
      "remainingMin":45,
      "powerKw":3.2,
      "energyWh":22500,
      "maxEnergyWh":26500,
      "temperature":22.5,
      "balancing":false
    },
    "drive":{
      "ignition":0,
      "keyInserted":false,
      "ignitionOn":false,
      "speedKmh":0.0,
      "odometerKm":45230
    },
    "body":{
      "locked":true,
      "centralLock":2,
      "trunkOpen":false,
      "anyDoorOpen":false,
      "doors":{
        "driverOpen":false,
        "passengerOpen":false,
        "rearLeftOpen":false,
        "rearRightOpen":false
      }
    },
    "range":{
      "totalKm":185,
      "electricKm":185,
      "displayKm":185,
      "consumption":13.2,
      "tendency":"stable",
      "reserveWarning":false
    },
    "canGps":{
      "lat":52.520008,
      "lng":13.404954,
      "alt":34.5,
      "heading":245.3,
      "satellites":8,
      "fixType":"3D",
      "hdop":1.2
    },
    "climate":{
      "insideTemp":18.5,
      "insideTempSource":"bap",
      "outsideTemp":12.0,
      "active":true,
      "activeSource":"bap",
      "heating":true,
      "cooling":false,
      "ventilation":false,
      "autoDefrost":false,
      "remainingMin":30
    },
    "plug":{
      "plugged":true,
      "hasSupply":true,
      "state":"plugged_supply",
      "lockState":1
    },
    "vehicleAwake":true,
    "canFrameCount":125340
  }
}
```

---

#### Battery Object

| Field | Type | Description |
|-------|------|-------------|
| `soc` | float | State of charge % (unified from CAN/BAP, BAP takes priority) |
| `socSource` | string | Data source: `"bap"` or `"can"` |
| `charging` | boolean | Whether charging is active (unified from CAN/BAP) |
| `chargingSource` | string | Data source: `"bap"`, `"can"`, or `"none"` |
| `chargingMode` | integer | *(BAP only)* 0=off, 1=AC, 2=DC, 3=conditioning, etc. |
| `chargingStatus` | integer | *(BAP only)* 0=idle, 1=running, 2=conservation, 3=completed, etc. |
| `chargingAmps` | integer | *(BAP only)* Charging current (A) |
| `targetSoc` | integer | *(BAP only)* Target SOC % |
| `remainingMin` | integer | *(BAP only)* Minutes to target SOC |
| `powerKw` | float | Charging/climate power (kW, from CAN 0x483) |
| `energyWh` | float | Current energy content (Wh) |
| `maxEnergyWh` | float | Maximum energy capacity (Wh) |
| `temperature` | float | Battery temperature (°C) |
| `balancing` | boolean | Cell balancing active |

**Note:** Fields with `Source` suffix indicate which protocol provided the data (BAP = detailed, CAN = basic).

---

#### Drive Object

| Field | Type | Description |
|-------|------|-------------|
| `ignition` | integer | Ignition state: 0=off, 1=accessory, 2=on, 3=start |
| `keyInserted` | boolean | Key is inserted (Kl. S) |
| `ignitionOn` | boolean | Ignition is on (Kl. 15) |
| `speedKmh` | float | Current speed (km/h) |
| `odometerKm` | integer | Total odometer (km) |

---

#### Body Object

| Field | Type | Description |
|-------|------|-------------|
| `locked` | boolean | Vehicle is locked |
| `centralLock` | integer | Lock state: 0=unknown, 1=locked, 2=unlocked |
| `trunkOpen` | boolean | Trunk/hatch is open |
| `anyDoorOpen` | boolean | Any door is open |
| `doors.driverOpen` | boolean | Driver door open |
| `doors.passengerOpen` | boolean | Passenger door open |
| `doors.rearLeftOpen` | boolean | Rear left door open |
| `doors.rearRightOpen` | boolean | Rear right door open |

---

#### Range Object

*(Optional - only included if valid data available from CAN)*

| Field | Type | Description |
|-------|------|-------------|
| `totalKm` | integer | Total estimated range (km) |
| `electricKm` | integer | Electric range (km) |
| `displayKm` | integer | Range shown on cluster (km) |
| `consumption` | float | Current consumption (kWh/100km or km/kWh) |
| `tendency` | string | Range trend: `"stable"`, `"increasing"`, `"decreasing"`, `"unknown"` |
| `reserveWarning` | boolean | Low range warning active |

---

#### CAN GPS Object

*(Optional - only included if valid fix available from CAN bus)*

| Field | Type | Description |
|-------|------|-------------|
| `lat` | float | Latitude (degrees) |
| `lng` | float | Longitude (degrees) |
| `alt` | float | Altitude (meters) |
| `heading` | float | Heading (degrees, 0-359.9) |
| `satellites` | integer | Satellites in use |
| `fixType` | string | Fix type: `"None"`, `"2D"`, `"3D"`, `"DGPS"` |
| `hdop` | float | Horizontal dilution of precision |

---

#### Climate Object

| Field | Type | Description |
|-------|------|-------------|
| `insideTemp` | float | Inside temperature (°C, unified CAN/BAP) |
| `insideTempSource` | string | Data source: `"bap"` or `"can"` |
| `outsideTemp` | float | Outside temperature (°C, CAN only) |
| `active` | boolean | Climate control active (BAP-only, CAN unreliable) |
| `activeSource` | string | Data source: `"bap"` only (omitted if no data) |
| `heating` | boolean | *(BAP only)* Heating mode active |
| `cooling` | boolean | *(BAP only)* Cooling mode active |
| `ventilation` | boolean | *(BAP only)* Ventilation mode active |
| `autoDefrost` | boolean | *(BAP only)* Auto defrost enabled |
| `remainingMin` | integer | *(BAP only)* Remaining climate time (minutes) |

**Note:** Climate active status is determined exclusively from BAP. CAN climate flags (0x66E KL_STH_aktiv/KL_STL_aktiv) were found to be unreliable.

---

#### Plug Object

*(Optional - only included if valid data from BAP)*

| Field | Type | Description |
|-------|------|-------------|
| `plugged` | boolean | Charging cable is plugged in |
| `hasSupply` | boolean | Charging station has power supply |
| `state` | string | Plug state string (e.g., `"plugged_supply"`, `"plugged_no_supply"`, `"unplugged"`) |
| `lockState` | integer | Plug lock state |

---

#### Top-level Vehicle Fields

| Field | Type | Description |
|-------|------|-------------|
| `vehicleAwake` | boolean | Vehicle CAN bus is active (received data in last 30s) |
| `canFrameCount` | integer | Total CAN frames processed since boot |

---

## Events

### Event Message Structure

Sent immediately when important state changes occur. Used for real-time notifications.

```json
{"type":"event","data":{"domain":"vehicle","name":"chargingStarted","soc":75,"powerKw":3.2}}
{"type":"event","data":{"domain":"vehicle","name":"doorOpened","door":"driver"}}
{"type":"event","data":{"domain":"vehicle","name":"climateStarted","heating":true,"cooling":false,"temp":18.5}}
```

| Field | Type | Description |
|-------|------|-------------|
| `domain` | string | Event domain (currently always `"vehicle"`) |
| `name` | string | Event name (see below) |
| *...details* | various | Event-specific data flattened into data object |

**Events vs State:**
- **Events** are immediate, partial, and indicate something alert-worthy happened
- **State** is periodic, complete, and provides full telemetry snapshot
- Server should trigger push notifications based on events
- Server should update dashboards/history based on state

---

### Implemented Events

#### Vehicle State Events

| Event Name | Details | Description |
|------------|---------|-------------|
| `ignitionOn` | *(none)* | Ignition turned on |
| `ignitionOff` | *(none)* | Ignition turned off |
| `locked` | *(none)* | Vehicle locked |
| `unlocked` | *(none)* | Vehicle unlocked |

**Examples:**
```json
{"type":"event","data":{"domain":"vehicle","name":"ignitionOn"}}
{"type":"event","data":{"domain":"vehicle","name":"unlocked"}}
```

---

#### Charging Events

| Event Name | Details | Description |
|------------|---------|-------------|
| `chargingStarted` | `soc`, `powerKw` | Charging began |
| `chargingStopped` | `soc`, `powerKw` | Charging ended |
| `plugged` | `hasSupply` | Charger plugged in |
| `unplugged` | `hasSupply` | Charger unplugged |
| `chargingComplete` | `soc`, `threshold` | Charging reached 100% or target SOC |
| `socThreshold` | `soc`, `threshold` | SOC crossed threshold (20%, 50%, 80%, 100%) |
| `lowBattery` | `soc` | SOC dropped below 20% |

**Examples:**
```json
{"type":"event","data":{"domain":"vehicle","name":"chargingStarted","soc":65.5,"powerKw":3.2}}
{"type":"event","data":{"domain":"vehicle","name":"plugged","hasSupply":true}}
{"type":"event","data":{"domain":"vehicle","name":"socThreshold","soc":80.5,"threshold":"80%"}}
{"type":"event","data":{"domain":"vehicle","name":"chargingComplete","soc":100,"threshold":"100%"}}
{"type":"event","data":{"domain":"vehicle","name":"lowBattery","soc":18}}
```

---

#### Door Events

| Event Name | Details | Description |
|------------|---------|-------------|
| `doorOpened` | `door` | Door opened |
| `doorClosed` | `door` | Door closed |
| `trunkOpened` | *(none)* | Trunk/hatch opened |
| `trunkClosed` | *(none)* | Trunk/hatch closed |

**Door values:** `"driver"`, `"passenger"`, `"rearLeft"`, `"rearRight"`

**Examples:**
```json
{"type":"event","data":{"domain":"vehicle","name":"doorOpened","door":"driver"}}
{"type":"event","data":{"domain":"vehicle","name":"trunkOpened"}}
```

---

#### Climate Events

| Event Name | Details | Description |
|------------|---------|-------------|
| `climateStarted` | `heating`, `cooling`, `temp` | Climate control started |
| `climateStopped` | `heating`, `cooling`, `temp` | Climate control stopped |

**Examples:**
```json
{"type":"event","data":{"domain":"vehicle","name":"climateStarted","heating":true,"cooling":false,"temp":18.5}}
{"type":"event","data":{"domain":"vehicle","name":"climateStopped","heating":false,"cooling":false,"temp":22.0}}
```

---

#### Command Completion Events

| Event Name | Details | Description |
|------------|---------|-------------|
| `commandCompleted` | `action` | Async command succeeded |
| `commandFailed` | `action`, `error` | Async command failed |

**Examples:**
```json
{"type":"event","data":{"domain":"vehicle","name":"commandCompleted","action":"startCharging"}}
{"type":"event","data":{"domain":"vehicle","name":"commandFailed","action":"startClimate","error":"Battery too low"}}
```

---

## Bye Message

### Bye Message (Device → Server)

Sent immediately before device disconnects intentionally.

```json
{"type":"bye","data":{"reason":"sleep"}}
```

| Field | Type | Description |
|-------|------|-------------|
| `reason` | string | `"sleep"`, `"shutdown"`, `"reboot"` |

**Note:** No response expected. Device disconnects immediately after sending.

This allows the server to distinguish between:
- Intentional disconnect (bye received) - device is offline by design
- Connection loss (no bye) - something went wrong, may need alerting

---

## Error Handling

### Command Errors

```json
{"type":"response","data":{"id":5,"ok":false,"status":"error","error":"Invalid parameter: targetSoc must be 50-100"}}
```

### Unknown Commands

```json
{"type":"response","data":{"id":6,"ok":false,"status":"not_supported","error":"Unknown action: foo.bar"}}
```

### Async Commands (Pending)

Commands that control the vehicle return `pending` immediately, with actual result delivered via events:

```json
// Command
{"type":"command","data":{"id":12,"action":"vehicle.startCharging","targetSoc":80}}

// Immediate response
{"type":"response","data":{"id":12,"ok":false,"status":"pending"}}

// Later, when charging starts (or fails)
{"type":"event","data":{"domain":"vehicle","name":"commandCompleted","action":"startCharging"}}
// or
{"type":"event","data":{"domain":"vehicle","name":"commandFailed","action":"startCharging","error":"Not plugged in"}}
```

---

## Server Implementation Notes

### Required Server Capabilities

1. **TCP Server** - Listen on configured port (default 4589)
2. **Device Registry** - Map CCID to device records for authentication
3. **Command Queue** - Store pending commands for offline devices
4. **State Storage** - Store and query telemetry data
5. **Event Handling** - Process events, trigger push notifications

### Recommended Behavior

1. **Authentication Timeout** - Disconnect if no auth request within 30 seconds
2. **Keepalive** - Consider device offline if no message received for 10 minutes
3. **Command Timeout** - Allow 30 seconds for synchronous command responses
4. **Async Command Timeout** - Allow up to 60 seconds for vehicle commands (watch for completion events)
5. **Reconnection** - Accept reconnection and re-authentication at any time

### Message Parsing

```python
while socket.connected:
    line = socket.readline()
    message = json.loads(line)
    
    match message["type"]:
        case "auth":
            handle_auth(message["data"])
        case "response":
            handle_response(message["data"])
        case "state":
            store_state(message["data"])
        case "event":
            process_event(message["data"])  # trigger push notifications
        case "bye":
            handle_bye(message["data"])
```

### Data Source Priority

The device consolidates data from multiple sources (CAN bus + BAP protocol):

- **SOC**: BAP only (CAN source never populated on e-Golf)
- **Charging Status**: BAP provides detailed status, CAN provides basic boolean (BAP takes priority)
- **Inside Temperature**: BAP takes priority (within 5s), CAN fallback for passive readings
- **Climate Active Status**: BAP only (CAN flags found to be unreliable)

Fields with `xxxSource` suffix (e.g., `socSource`, `chargingSource`, `insideTempSource`, `activeSource`) indicate which protocol provided the data. This helps with debugging and data quality assessment.

---

## Protocol Version

This document describes **Protocol Version 2.1**.

**Changes from 2.0:**
- Added vehicle control commands (wake, charging, climate)
- Added charging profile management
- Unified vehicle state (removed duplicate `bapCharge`, `bapClimate` - data now consolidated with source tracking)
- Added 8 new events (doors, battery thresholds, climate, trunk)
- Added data source tracking fields (`socSource`, `chargingSource`, etc.)
- Changed event structure (domain is now always `"vehicle"` instead of separate domains)
