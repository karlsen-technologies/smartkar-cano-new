# Protocol Specification

**Protocol Version:** 2.2  
**Last Updated:** 2026-01-30

This document describes the JSON protocol used for communication between the SmartKar-Cano device and the server. This specification is intended for implementing compatible server software.

## Version History

- **v2.2** (2026-01-31): 
  - Added full command lifecycle tracking with progress stages
  - Commands now send multiple `response` messages throughout execution (`in_progress` → `completed`/`failed`)
  - Added command execution stages: `accepted`, `requesting_wake`, `waiting_for_wake`, `updating_profile`, `sending_command`
  - Added `busy` response status with `currentCommand` details when another command is running
  - Deprecated command completion events (`commandCompleted`, `commandFailed`) - replaced by response lifecycle
  - Events now reserved for vehicle state changes only (not command lifecycle)
  - Implemented domain-based architecture: Commands route through domain managers (BatteryManager, ClimateManager) with state machines
- **v2.1**: Previous version with basic command/response
- **v2.0**: Initial protocol with event system

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

**Protocol v2.2** introduces full command lifecycle tracking. Commands now send multiple responses throughout their execution:

1. **In-Progress Response** - Sent when command is accepted and during execution stages
2. **Completed Response** - Sent when command completes successfully
3. **Failed Response** - Sent if command fails at any stage
4. **Busy Response** - Sent if another command is already running

**Example: Successful Climate Start (Vehicle Awake)**
```json
// 1. Accepted
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}

// 2. Updating profile
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}

// 3. Sending command
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}

// 4. Completed
{"type":"response","data":{"id":12,"ok":true,"status":"completed","elapsedMs":2400}}
```

**Example: Failed Command (Wake Timeout)**
```json
// 1. Accepted
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"accepted"}}

// 2. Requesting wake
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

// 3. Waiting for wake
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

// 4. Failed
{
  "type":"response",
  "data":{
    "id":13,
    "ok":false,
    "status":"failed",
    "stage":"waiting_for_wake",
    "error":"Wake timeout after 15000ms",
    "elapsedMs":15100
  }
}
```

**Example: Busy Response**
```json
{
  "type":"response",
  "data":{
    "id":15,
    "ok":false,
    "status":"busy",
    "error":"Another command is in progress",
    "currentCommand":{
      "id":12,
      "action":"vehicle.startClimate",
      "stage":"updating_profile",
      "elapsedMs":3200
    }
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Matches the command `id` |
| `ok` | boolean | Whether command succeeded (true for in_progress, false for failed/busy/error) |
| `status` | string | Status code (see table below) |
| `stage` | string | (Optional) Current execution stage for `in_progress` and `failed` responses |
| `error` | string | (Optional) Error message for failed commands |
| `elapsedMs` | integer | (Optional) Milliseconds since command started |
| `currentCommand` | object | (Optional) Info about active command when status is `busy` |
| *...data* | various | Response data flattened into data object |

### Response Status Values (v2.2)

| Status | ok | Description | Use Case |
|--------|-----|-------------|----------|
| *(no status field)* | true | Immediate success (legacy) | Very simple synchronous commands |
| `in_progress` | true | Command executing, see `stage` field | All tracked commands during execution |
| `completed` | true | Command finished successfully | Final response for successful async commands |
| `failed` | false | Command failed, see `error` & `stage` | Any failure (validation or execution) |
| `busy` | false | Another command in progress | Command rejected due to active command |
| `error` | false | General error | Generic errors |
| `not_supported` | false | Unknown command/domain | Unknown action |

### Command Execution Stages

Stages reported in `in_progress` and `failed` responses:

| Stage | Description |
|-------|-------------|
| `accepted` | Command accepted, about to start |
| `requesting_wake` | Requesting vehicle wake |
| `waiting_for_wake` | Waiting for vehicle to respond |
| `updating_profile` | Updating charging/climate profile |
| `sending_command` | Sending BAP frames to vehicle |

**Note:** Not all commands go through all stages. Fast synchronous commands may only show `accepted` → `completed`. Commands for awake vehicles may skip wake stages.

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
{"type":"command","data":{"id":12,"action":"vehicle.startCharging","targetSoc":80,"maxCurrent":16}}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `targetSoc` | integer | No | Target SOC % (0-100, default 80) |
| `maxCurrent` | integer | No | Max charging current in Amps (0-32A, default 32) |

**Response Sequence (Protocol v2.2):**
```json
// 1. Accepted
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}

// 2. Requesting wake (if vehicle asleep)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

// 3. Waiting for wake
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

// 4. Updating profile
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}

// 5. Sending command
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}

// 6. Completed
{"type":"response","data":{"id":12,"ok":true,"status":"completed","elapsedMs":5400}}
```

**On validation failure:**
```json
{"type":"response","data":{"id":12,"ok":false,"status":"failed","stage":"accepted","error":"Invalid parameter: targetSoc must be 10-100"}}
```

**On execution failure:**
```json
{"type":"response","data":{"id":12,"ok":false,"status":"failed","stage":"waiting_for_wake","error":"Wake timeout after 15000ms","elapsedMs":15100}}
```

**Note:** Command execution is tracked through multiple progress responses. All responses include the original command `id` for correlation. Not all stages may appear (e.g., if vehicle is already awake).

---

#### vehicle.stopCharging

Stops the current charging session.

```json
{"type":"command","data":{"id":13,"action":"vehicle.stopCharging"}}
```

**Response Sequence:**
```json
// 1. Accepted
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"accepted"}}

// 2. Sending command (vehicle likely already awake)
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"sending_command"}}

// 3. Completed
{"type":"response","data":{"id":13,"ok":true,"status":"completed","elapsedMs":1200}}
```

**Note:** Stop commands typically execute faster than start commands since no profile update is needed. Track progress through multiple responses.

---

#### vehicle.startClimate

Starts climate control (heating/cooling/ventilation).

```json
{"type":"command","data":{"id":14,"action":"vehicle.startClimate","temperature":21.0,"allowBattery":true}}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `temperature` (or `temp`) | float | No | Target temperature in °C (15.5-30.0, default 21.0) |
| `allowBattery` | boolean | No | Allow climate on battery power (default: true) |

**Response Sequence:**
```json
// 1. Accepted
{"type":"response","data":{"id":14,"ok":true,"status":"in_progress","stage":"accepted"}}

// 2. Requesting wake (if needed)
{"type":"response","data":{"id":14,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

// 3. Waiting for wake
{"type":"response","data":{"id":14,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

// 4. Updating profile
{"type":"response","data":{"id":14,"ok":true,"status":"in_progress","stage":"updating_profile"}}

// 5. Sending command
{"type":"response","data":{"id":14,"ok":true,"status":"in_progress","stage":"sending_command"}}

// 6. Completed
{"type":"response","data":{"id":14,"ok":true,"status":"completed","elapsedMs":5400}}
```

**Note:** Track command execution through multiple progress responses. Stages depend on vehicle state (e.g., skip wake stages if already awake).

---

#### vehicle.stopClimate

Stops climate control.

```json
{"type":"command","data":{"id":15,"action":"vehicle.stopClimate"}}
```

**Response Sequence:**
```json
// 1. Accepted
{"type":"response","data":{"id":15,"ok":true,"status":"in_progress","stage":"accepted"}}

// 2. Sending command
{"type":"response","data":{"id":15,"ok":true,"status":"in_progress","stage":"sending_command"}}

// 3. Completed
{"type":"response","data":{"id":15,"ok":true,"status":"completed","elapsedMs":800}}
```

**Note:** Stop commands execute quickly with no profile update needed. Track progress through responses.

---

#### vehicle.requestState

Requests fresh BAP state data from the vehicle (plug, charge, climate).

```json
{"type":"command","data":{"id":16,"action":"vehicle.requestState"}}
```

**Response:**
```json
{"type":"response","data":{"id":16,"ok":true,"plugRequested":true,"chargeRequested":true,"climateRequested":true}}
```

**Note:** Triggers BAP state requests. Updated data will arrive in subsequent state messages.

---

### Charging Profile Domain

#### chargingProfile.updateProfile

Updates charging profile settings for timer profiles (1-3 only).

```json
{
  "type":"command",
  "data":{
    "id":20,
    "action":"chargingProfile.updateProfile",
    "index":1,
    "targetSoc":80,
    "maxCurrent":16,
    "temperature":22.0,
    "enableCharging":true,
    "enableClimate":true,
    "allowBattery":false,
    "leadTime":0,
    "holdTimePlug":60,
    "holdTimeBattery":30,
    "name":"Night charging"
  }
}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `index` | integer | Yes | Profile index (1-3, timer profiles only) |
| `targetSoc` | integer | No | Target SOC % (0-100) |
| `maxCurrent` | integer | No | Max charging current (0-32A) |
| `temperature` | float | No | Climate target temperature (15.5-30.0°C) |
| `enableCharging` | boolean | No | Enable charging for this profile |
| `enableClimate` | boolean | No | Enable climate for this profile |
| `allowBattery` | boolean | No | Allow climate on battery (used when enableClimate=true) |
| `leadTime` | integer | No | Lead time in minutes |
| `holdTimePlug` | integer | No | Holding time when plugged (minutes) |
| `holdTimeBattery` | integer | No | Holding time on battery (minutes) |
| `name` | string | No | Profile name (max 20 chars) |

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":20,
    "ok":true,
    "message":"Profile updated",
    "profile":{
      "index":1,
      "valid":true,
      "chargingEnabled":true,
      "climateEnabled":true,
      "maxCurrent":16,
      "targetChargeLevel":80,
      "temperature":22.0,
      "name":"Night charging"
    }
  }
}
```

**Event:**
```json
{"type":"event","data":{"domain":"profiles","name":"profileUpdated","index":1}}
```

**Note:** Profiles are stored locally and sent to the vehicle. Profile 0 (immediate) cannot be updated via this command.

---

#### chargingProfile.setEnabled

Enables or disables a timer profile (1-3 only).

```json
{"type":"command","data":{"id":23,"action":"chargingProfile.setEnabled","index":1,"enabled":true}}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `index` | integer | Yes | Profile index (1-3, timer profiles only) |
| `enabled` | boolean | Yes | Enable (true) or disable (false) the timer |

**Response:**
```json
{"type":"response","data":{"id":23,"ok":true,"message":"Timer enabled","index":1,"enabled":true}}
```

**Event:**
```json
{"type":"event","data":{"domain":"profiles","name":"timerStateChanged","index":1,"enabled":true}}
```

**Note:** This enables/disables the timer activation. The profile settings are preserved.

---

#### chargingProfile.refresh

Requests all profiles to be refreshed from the vehicle.

```json
{"type":"command","data":{"id":24,"action":"chargingProfile.refresh"}}
```

**Response:**
```json
{"type":"response","data":{"id":24,"ok":true,"message":"Profile refresh requested"}}
```

**Note:** Triggers BAP requests to fetch all profile data from the vehicle. Updated profiles will be reflected in subsequent state or profile queries.

---

#### chargingProfile.getProfile

Gets a specific charging profile by index.

```json
{"type":"command","data":{"id":21,"action":"chargingProfile.getProfile","index":1}}
```

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `index` | integer | Yes | Profile index (0-3) |

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":21,
    "ok":true,
    "profile":{
      "index":1,
      "type":"timer",
      "description":"Timer 1",
      "valid":true,
      "lastUpdate":123456789,
      "operation":3,
      "operation2":0,
      "chargingEnabled":true,
      "climateEnabled":true,
      "climateAllowBattery":false,
      "maxCurrent":16,
      "minChargeLevel":50,
      "targetChargeLevel":80,
      "temperature":22.0,
      "temperatureUnit":"celsius",
      "leadTime":0,
      "holdTimePlug":60,
      "holdTimeBattery":30,
      "name":"Night charging"
    }
  }
}
```

---

#### chargingProfile.get

Gets all charging profiles (0-3).

```json
{"type":"command","data":{"id":22,"action":"chargingProfile.get"}}
```

**Response:**
```json
{
  "type":"response",
  "data":{
    "id":22,
    "ok":true,
    "profileCount":4,
    "lastUpdateTime":123456789,
    "updateCount":5,
    "profiles":[
      {
        "index":0,
        "type":"immediate",
        "description":"Used for 'start now' operations",
        "valid":true,
        "chargingEnabled":true,
        "climateEnabled":false,
        "maxCurrent":32,
        "targetChargeLevel":80,
        "temperature":21.0
      },
      {
        "index":1,
        "type":"timer",
        "description":"Timer 1",
        "valid":true,
        "chargingEnabled":true,
        "climateEnabled":true,
        "climateAllowBattery":false,
        "maxCurrent":16,
        "targetChargeLevel":80,
        "temperature":22.0,
        "name":"Night charging"
      },
      {"index":2,"valid":false},
      {"index":3,"valid":false}
    ]
  }
}
```

**Note:** Profile 0 is the immediate profile used for "start now" commands. Profiles 1-3 are timer profiles.

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

#### Command Completion Events (DEPRECATED in v2.2)

**⚠️ DEPRECATED:** These events are replaced by command lifecycle responses in Protocol v2.2. Use the multi-stage response system instead (see Command Execution Stages above).

| Event Name | Details | Description |
|------------|---------|-------------|
| `commandCompleted` | `action` | *(Legacy)* Async command succeeded |
| `commandFailed` | `action`, `error` | *(Legacy)* Async command failed |

**Migration Note:** In v2.2, command progress is tracked through multiple `response` messages with the same command `id`, not through events. Events are now reserved for vehicle state changes only.

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

### Validation Errors

Parameter validation failures return immediately with `failed` status:

```json
{"type":"response","data":{"id":5,"ok":false,"status":"failed","stage":"accepted","error":"Invalid parameter: targetSoc must be 10-100"}}
```

### Unknown Commands

```json
{"type":"response","data":{"id":6,"ok":false,"status":"not_supported","error":"Unknown action: foo.bar"}}
```

### Busy Response (v2.2)

When a command is sent while another command is executing, device returns `busy` status with details about the active command:

```json
{
  "type":"response",
  "data":{
    "id":15,
    "ok":false,
    "status":"busy",
    "error":"Another command is in progress",
    "currentCommand":{
      "id":12,
      "action":"vehicle.startClimate",
      "stage":"updating_profile",
      "elapsedMs":3200
    }
  }
}
```

This allows the server to:
- Know exactly which command is blocking
- See the active command's current stage
- Estimate when the device will be available
- Retry the command after the current one completes

### Execution Failures

Commands can fail at any stage during execution. The `stage` field indicates where the failure occurred:

```json
{
  "type":"response",
  "data":{
    "id":13,
    "ok":false,
    "status":"failed",
    "stage":"waiting_for_wake",
    "error":"Wake timeout after 15000ms",
    "elapsedMs":15100
  }
}
```

Common failure stages:
- `accepted`: Validation error after initial acceptance
- `waiting_for_wake`: Vehicle didn't wake up in time (15s timeout)
- `updating_profile`: Profile update failed
- `sending_command`: Failed to send BAP frames to vehicle

### Async Commands (Protocol v2.2)

Commands that control the vehicle send multiple responses throughout their lifecycle:

```json
// Command
{"type":"command","data":{"id":12,"action":"vehicle.startCharging","targetSoc":80}}

// Response 1: Accepted
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}

// Response 2: Requesting wake
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

// Response 3: Waiting for wake
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

// Response 4: Updating profile
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}

// Response 5: Sending command
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}

// Response 6: Completed
{"type":"response","data":{"id":12,"ok":true,"status":"completed","elapsedMs":5400}}

// OR if failed at any stage:
{"type":"response","data":{"id":12,"ok":false,"status":"failed","stage":"waiting_for_wake","error":"Wake timeout after 15000ms","elapsedMs":15100}}
```

**Key Points:**
- All responses share the same command `id` for correlation
- Progress is tracked through `stage` field in `in_progress` responses
- Final response has `status: "completed"` or `status: "failed"`
- Vehicle state change events (e.g., `chargingStarted`) are separate from command lifecycle

---

## Server Implementation Notes

### Required Server Capabilities

1. **TCP Server** - Listen on configured port (default 4589)
2. **Device Registry** - Map CCID to device records for authentication
3. **Command Tracking (v2.2)** - Track command state by ID, handle multiple responses per command
4. **State Storage** - Store and query telemetry data
5. **Event Handling** - Process events, trigger push notifications

### Recommended Behavior

1. **Authentication Timeout** - Disconnect if no auth request within 30 seconds
2. **Keepalive** - Consider device offline if no message received for 10 minutes
3. **Command Timeout (v2.2)** - Allow up to 60 seconds for command completion (device sends progress updates)
4. **Busy Handling (v2.2)** - When receiving `busy` response, wait for `currentCommand` to complete before retrying
5. **Reconnection** - Accept reconnection and re-authentication at any time

### Command Tracking (Protocol v2.2)

Servers must track command state across multiple response messages:

```python
class CommandTracker:
    def __init__(self):
        self.active_commands = {}  # command_id -> CommandState
    
    def handle_response(self, response):
        cmd_id = response["id"]
        status = response["status"]
        
        if status == "in_progress":
            # Update command progress
            self.active_commands[cmd_id] = {
                "stage": response["stage"],
                "started_at": time.now() if cmd_id not in self.active_commands else self.active_commands[cmd_id]["started_at"],
                "last_update": time.now()
            }
            # Update UI: Show progress (e.g., "Waking vehicle...")
            
        elif status == "completed":
            # Command succeeded
            cmd = self.active_commands.pop(cmd_id)
            # Update UI: Success notification
            
        elif status == "failed":
            # Command failed
            cmd = self.active_commands.pop(cmd_id)
            error = response["error"]
            stage = response["stage"]
            # Update UI: Error notification with context
            
        elif status == "busy":
            # Device busy with another command
            current_cmd = response["currentCommand"]
            # Option 1: Retry after current command completes
            # Option 2: Show user "Device busy, please wait"
```

**Best Practices:**
- Store `started_at` timestamp when receiving first `in_progress` response
- Update UI on each `stage` change to show progress to user
- Calculate timeout from `started_at`, not from last response
- Handle `busy` responses by tracking the blocking command's progress
- Clean up command state on `completed` or `failed`

### Message Parsing

```python
# Initialize command tracker
command_tracker = CommandTracker()

while socket.connected:
    line = socket.readline()
    message = json.loads(line)
    
    match message["type"]:
        case "auth":
            handle_auth(message["data"])
        
        case "response":
            # Handle command response (v2.2: may be one of many)
            command_tracker.handle_response(message["data"])
        
        case "state":
            # Periodic telemetry update
            store_state(message["data"])
        
        case "event":
            # Immediate notification (vehicle state changes only in v2.2)
            process_event(message["data"])
            # Examples: chargingStarted, doorOpened, ignitionOn
        
        case "bye":
            # Device going offline intentionally
            handle_bye(message["data"])
```

**v2.2 Migration Notes:**
- `response` messages now arrive multiple times per command (not just once)
- Events are for vehicle state changes only (not command completion)
- Track command progress by `id` across multiple responses
- Final response has `status: "completed"` or `status: "failed"`

### Data Source Priority

The device consolidates data from multiple sources (CAN bus + BAP protocol):

- **SOC**: BAP only (CAN source never populated on e-Golf)
- **Charging Status**: BAP provides detailed status, CAN provides basic boolean (BAP takes priority)
- **Inside Temperature**: BAP takes priority (within 5s), CAN fallback for passive readings
- **Climate Active Status**: BAP only (CAN flags found to be unreliable)

Fields with `xxxSource` suffix (e.g., `socSource`, `chargingSource`, `insideTempSource`, `activeSource`) indicate which protocol provided the data. This helps with debugging and data quality assessment.

---

## Migration Guide: v2.1 → v2.2

### Breaking Changes

**⚠️ BREAKING:** Protocol v2.2 is not backward compatible with v2.1 server implementations.

1. **Command Response Model Changed**
   - **Old (v2.1):** Single response per command with `status: "queued"`
   - **New (v2.2):** Multiple responses per command with progress tracking

2. **Command Lifecycle Events Removed**
   - **Removed Events:** `commandCompleted`, `commandFailed`, `chargingStartRequested`, `chargingStopRequested`, `climateStartRequested`, `climateStopRequested`
   - **Replacement:** Track command state through multiple `response` messages linked by `id`

3. **New Response Statuses**
   - Added: `in_progress`, `completed`, `busy`
   - Changed: `queued` → `in_progress` with `stage` field
   - Changed: `pending` → `in_progress` with `stage` field

### Required Server Changes

#### 1. Update Response Handler

**Before (v2.1):**
```python
def handle_response(response):
    cmd_id = response["id"]
    if response["ok"]:
        if response.get("status") == "queued":
            # Command accepted, wait for event
            mark_command_pending(cmd_id)
        else:
            # Synchronous success
            mark_command_complete(cmd_id, response)
    else:
        # Error
        mark_command_failed(cmd_id, response["error"])
```

**After (v2.2):**
```python
def handle_response(response):
    cmd_id = response["id"]
    status = response["status"]
    
    if status == "in_progress":
        # Command progressing
        update_command_stage(cmd_id, response["stage"])
        update_ui_progress(cmd_id, response["stage"])
    
    elif status == "completed":
        # Command succeeded
        mark_command_complete(cmd_id, response)
        show_success_notification(cmd_id)
    
    elif status == "failed":
        # Command failed
        mark_command_failed(cmd_id, response["error"], response["stage"])
        show_error_notification(cmd_id, response["error"])
    
    elif status == "busy":
        # Another command is running
        current = response["currentCommand"]
        show_busy_message(f"Device busy with command {current['id']} ({current['action']})")
        # Optionally: Queue for retry after current command completes
```

#### 2. Remove Event Handlers for Command Lifecycle

**Delete these handlers:**
```python
# DELETE - No longer sent in v2.2
def on_command_completed_event(event):
    ...

def on_command_failed_event(event):
    ...

def on_charging_start_requested_event(event):
    ...

def on_climate_start_requested_event(event):
    ...
```

#### 3. Add Command Tracking

```python
class CommandTracker:
    def __init__(self):
        self.commands = {}  # cmd_id -> {stage, started_at, last_update}
    
    def update(self, cmd_id, stage):
        if cmd_id not in self.commands:
            self.commands[cmd_id] = {"started_at": time.now()}
        self.commands[cmd_id]["stage"] = stage
        self.commands[cmd_id]["last_update"] = time.now()
    
    def complete(self, cmd_id):
        if cmd_id in self.commands:
            del self.commands[cmd_id]
    
    def get_elapsed(self, cmd_id):
        if cmd_id in self.commands:
            return time.now() - self.commands[cmd_id]["started_at"]
        return 0
```

#### 4. Update UI Progress Display

Map stages to user-friendly messages:

```python
STAGE_MESSAGES = {
    "accepted": "Command accepted",
    "requesting_wake": "Waking vehicle...",
    "waiting_for_wake": "Waiting for vehicle...",
    "updating_profile": "Updating settings...",
    "sending_command": "Sending command...",
}

def update_ui_progress(cmd_id, stage):
    message = STAGE_MESSAGES.get(stage, f"Stage: {stage}")
    ui.show_progress(cmd_id, message)
```

### Non-Breaking Changes

These vehicle state events continue to work as before:
- ✅ `chargingStarted` / `chargingStopped`
- ✅ `plugged` / `unplugged`
- ✅ `doorOpened` / `doorClosed`
- ✅ `ignitionOn` / `ignitionOff`
- ✅ `locked` / `unlocked`
- ✅ `socThreshold`
- ✅ `chargingComplete`
- ✅ `climateStarted` / `climateStopped`

### Testing Checklist

- [ ] Command progress updates display correctly in UI
- [ ] Final `completed` response triggers success notification
- [ ] Final `failed` response shows error with context (stage + error message)
- [ ] `busy` responses handled gracefully (show message or queue for retry)
- [ ] Vehicle state events still trigger notifications (charging, doors, etc.)
- [ ] Command timeout increased to 60 seconds (from 30s)
- [ ] No errors from removed event handlers (`commandCompleted`, etc.)

### Rollout Strategy

**Recommended approach:**

1. **Update server first** - Server can handle both v2.1 and v2.2 responses during transition
2. **Deploy server to production** - Test with existing v2.1 devices (backward compatible)
3. **Update firmware** - Deploy v2.2 firmware to devices gradually
4. **Remove v2.1 compatibility** - After all devices updated, remove legacy code

**Backward compatibility option:**

```python
def handle_response(response):
    status = response.get("status")
    
    # v2.2 responses
    if status in ["in_progress", "completed", "failed", "busy"]:
        handle_v22_response(response)
    
    # v2.1 responses (legacy)
    elif status == "queued":
        handle_v21_queued_response(response)
    
    # Synchronous responses (both versions)
    else:
        handle_sync_response(response)
```

---

## Protocol Version

This document describes **Protocol Version 2.2**.

**Changes from 2.1:**
- Full command lifecycle tracking with multiple response messages per command
- Command execution stages (`accepted`, `requesting_wake`, `waiting_for_wake`, `updating_profile`, `sending_command`)
- New response statuses: `in_progress`, `completed`, `busy`
- `busy` responses include `currentCommand` details for better debugging
- Deprecated command lifecycle events (`commandCompleted`, `commandFailed`) - use response tracking instead
- Events now strictly for vehicle state changes (charging, doors, ignition, etc.)
- Single command queue enforced - only one command executes at a time

**Changes from 2.0:**
- Added vehicle control commands (wake, charging, climate)
- Added charging profile management
- Unified vehicle state (removed duplicate `bapCharge`, `bapClimate` - data now consolidated with source tracking)
- Added 8 new events (doors, battery thresholds, climate, trunk)
- Added data source tracking fields (`socSource`, `chargingSource`, etc.)
- Changed event structure (domain is now always `"vehicle"` instead of separate domains)

**Architecture Notes:**
- Device uses domain-based architecture with state machines for command orchestration
- Commands handled by specialized managers: BatteryManager (charging), ClimateManager (climate control)
- ChargingProfileManager handles BAP protocol profile operations
- VehicleManager coordinates wake sequences and CAN communication
- All commands go through CommandRouter which enforces single-command-at-a-time policy
