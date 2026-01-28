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

### system.reboot

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

### system.sleep

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

### system.wakeup

Acknowledges that the device is awake. No-op command that confirms the device received the wakeup signal.

```json
{"type":"command","data":{"id":3,"action":"system.wakeup"}}
```

**Response:**
```json
{"type":"response","data":{"id":3,"ok":true,"message":"Device is awake","uptime":1234}}
```

---

### system.telemetry

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

### system.info

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

## State

### State Message (Device → Server)

Sent periodically (interval based on priority), contains full telemetry snapshot.

```json
{
  "type":"state",
  "data":{
    "device":{...},
    "network":{...}
  }
}
```

Multiple domains are included in a single state message.

### Device Domain

```json
{
  "type":"state",
  "data":{
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

### Network Domain

```json
{
  "type":"state",
  "data":{
    "network":{
      "modemState":"connected",
      "signalStrength":-75,
      "simCCID":"8947080012345678901",
      "modemConnected":true,
      "linkConnected":true,
      "linkState":"connected"
    }
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

### Vehicle Domain

Full vehicle telemetry from CAN bus and BAP protocol.

```json
{
  "type":"state",
  "data":{
    "vehicle":{
      "battery":{
        "soc":65.5,
        "socHiRes":65.48,
        "voltage":355.2,
        "current":-12.5,
        "powerKw":7.4,
        "energyWh":22400,
        "maxEnergyWh":35800,
        "temperature":25.5,
        "charging":true,
        "balancing":false,
        "dcdc12v":14.2,
        "dcdcCurrent":5.3
      },
      "drive":{
        "ignition":2,
        "keyInserted":true,
        "ignitionOn":true,
        "speedKmh":0.0,
        "odometerKm":45230
      },
      "body":{
        "locked":true,
        "centralLock":1,
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
        "consumption":15.2,
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
        "insideTemp":21.5,
        "outsideTemp":15.0,
        "standbyHeating":false,
        "standbyVent":false
      },
      "plug":{
        "plugged":true,
        "hasSupply":true,
        "state":"plugged"
      },
      "bapCharge":{
        "mode":"AC",
        "status":"running",
        "soc":65,
        "remainingMin":45,
        "targetSoc":80,
        "amps":32
      },
      "bapClimate":{
        "active":false,
        "heating":false,
        "cooling":false,
        "ventilation":false,
        "autoDefrost":false,
        "currentTemp":21.5,
        "remainingMin":0
      },
      "vehicleAwake":true,
      "canFrameCount":125340
    }
  }
}
```

#### Battery Subobject

| Field | Type | Description |
|-------|------|-------------|
| `soc` | float | Usable/display state of charge (%) |
| `socHiRes` | float | High-resolution SOC from BMS (%) |
| `voltage` | float | HV battery voltage (V) |
| `current` | float | Battery current (A, negative = discharge) |
| `powerKw` | float | Charging/climate power (kW, from 0x483) |
| `energyWh` | float | Current energy content (Wh) |
| `maxEnergyWh` | float | Maximum energy capacity (Wh) |
| `temperature` | float | Battery temperature (°C) |
| `charging` | boolean | Whether charging session is active |
| `balancing` | boolean | Whether cell balancing is active |
| `dcdc12v` | float | 12V system voltage from DC-DC converter |
| `dcdcCurrent` | float | 12V system current (A) |

#### Drive Subobject

| Field | Type | Description |
|-------|------|-------------|
| `ignition` | integer | Ignition state: 0=off, 1=accessory, 2=on, 3=start |
| `keyInserted` | boolean | Key is inserted (Kl. S) |
| `ignitionOn` | boolean | Ignition is on (Kl. 15) |
| `speedKmh` | float | Current speed (km/h) |
| `odometerKm` | integer | Total odometer (km) |

#### Body Subobject

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

#### Range Subobject (optional, only if valid)

| Field | Type | Description |
|-------|------|-------------|
| `totalKm` | integer | Total estimated range (km) |
| `electricKm` | integer | Electric range (km) |
| `displayKm` | integer | Range shown on cluster (km) |
| `consumption` | float | Current consumption (kWh/100km or km/kWh) |
| `tendency` | string | Range trend: `stable`, `increasing`, `decreasing`, `unknown` |
| `reserveWarning` | boolean | Low range warning active |

#### CAN GPS Subobject (optional, only if valid fix)

| Field | Type | Description |
|-------|------|-------------|
| `lat` | float | Latitude (degrees) |
| `lng` | float | Longitude (degrees) |
| `alt` | float | Altitude (meters) |
| `heading` | float | Heading (degrees, 0-359.9) |
| `satellites` | integer | Satellites in use |
| `fixType` | string | Fix type: `None`, `2D`, `3D`, `DGPS` |
| `hdop` | float | Horizontal dilution of precision |

#### Climate Subobject

| Field | Type | Description |
|-------|------|-------------|
| `insideTemp` | float | Inside temperature (°C) |
| `outsideTemp` | float | Outside temperature (°C) |
| `standbyHeating` | boolean | Standby heating active |
| `standbyVent` | boolean | Standby ventilation active |

#### Plug Subobject (optional, only if valid from BAP)

| Field | Type | Description |
|-------|------|-------------|
| `plugged` | boolean | Charging cable is plugged in |
| `hasSupply` | boolean | Charging station has power supply |
| `state` | string | Plug state: `plugged`, `unplugged`, `unknown` |

#### BAP Charge Subobject (optional, only if valid from BAP)

| Field | Type | Description |
|-------|------|-------------|
| `mode` | string | Charge mode: `off`, `AC`, `DC`, `conditioning`, `AC+cond`, `DC+cond`, `init` |
| `status` | string | Charge status: `idle`, `running`, `conservation`, `completed`, `aborted:*`, `init` |
| `soc` | integer | SOC from charge controller (%) |
| `remainingMin` | integer | Minutes to full charge |
| `targetSoc` | integer | Target SOC setting (%) |
| `amps` | integer | Charging current (A) |

#### BAP Climate Subobject (optional, only if valid from BAP)

| Field | Type | Description |
|-------|------|-------------|
| `active` | boolean | Climate control is active |
| `heating` | boolean | Heating is on |
| `cooling` | boolean | Cooling is on |
| `ventilation` | boolean | Ventilation is on |
| `autoDefrost` | boolean | Auto defrost enabled |
| `currentTemp` | float | Current cabin temperature (°C) |
| `remainingMin` | integer | Remaining climate time (minutes) |

#### Top-level Vehicle Fields

| Field | Type | Description |
|-------|------|-------------|
| `vehicleAwake` | boolean | Vehicle CAN bus is active (received data in last 5s) |
| `canFrameCount` | integer | Total CAN frames processed since boot |

---

## Events

### Event Message (Device → Server)

Sent immediately when important state changes occur. Used for real-time notifications.

```json
{"type":"event","data":{"domain":"charging","name":"started","power":7400}}
{"type":"event","data":{"domain":"charging","name":"stopped","reason":"complete"}}
{"type":"event","data":{"domain":"security","name":"doorOpened","door":"driver"}}
```

| Field | Type | Description |
|-------|------|-------------|
| `domain` | string | Event domain (e.g., `charging`, `security`, `climate`) |
| `name` | string | Event name (e.g., `started`, `stopped`, `doorOpened`) |
| *...details* | various | Event-specific data flattened into data object |

**Events vs State:**
- **Events** are immediate, partial, and indicate something alert-worthy happened
- **State** is periodic, complete, and provides full telemetry snapshot
- Server should trigger push notifications based on events
- Server should update dashboards/history based on state

### Planned Events

| Domain | Event | Description |
|--------|-------|-------------|
| `charging` | `started` | Charging began |
| `charging` | `stopped` | Charging ended (reason: `complete`, `unplugged`, `error`, `manual`) |
| `climate` | `started` | Climate control started |
| `climate` | `stopped` | Climate control stopped |
| `security` | `doorOpened` | Door opened |
| `security` | `doorClosed` | Door closed |
| `security` | `locked` | Vehicle locked |
| `security` | `unlocked` | Vehicle unlocked |
| `security` | `alarm` | Alarm triggered |

---

## Bye

### Bye Message (Device → Server)

Sent immediately before device disconnects intentionally.

```json
{"type":"bye","data":{"reason":"sleep"}}
```

| Field | Type | Description |
|-------|------|-------------|
| `reason` | string | `sleep`, `shutdown`, `reboot` |

**Note:** No response expected. Device disconnects immediately after sending.

This allows the server to distinguish between:
- Intentional disconnect (bye received) - device is offline by design
- Connection loss (no bye) - something went wrong, may need alerting

---

## Future Commands (Planned)

### charging.setLimit
```json
{"type":"command","data":{"id":10,"action":"charging.setLimit","percent":80}}
```

### charging.start / charging.stop
```json
{"type":"command","data":{"id":11,"action":"charging.start"}}
{"type":"command","data":{"id":12,"action":"charging.stop"}}
```

### climate.start
```json
{"type":"command","data":{"id":13,"action":"climate.start","targetTemp":21,"duration":15}}
```

### security.lock / security.unlock
```json
{"type":"command","data":{"id":14,"action":"security.lock"}}
{"type":"command","data":{"id":15,"action":"security.unlock"}}
```

---

## Error Handling

### Command Errors

```json
{"type":"response","data":{"id":5,"ok":false,"status":"error","error":"Invalid parameter: duration must be positive"}}
```

### Unknown Commands

```json
{"type":"response","data":{"id":6,"ok":false,"status":"not_supported","error":"Unknown domain: foo"}}
```

### Async Commands (Pending)

Some commands take time to complete (e.g., starting climate control). These return `pending` immediately, and the actual result comes via events:

```json
// Command
{"type":"command","data":{"id":7,"action":"climate.start","targetTemp":21}}

// Immediate response
{"type":"response","data":{"id":7,"ok":false,"status":"pending"}}

// Later, when climate actually starts (or fails)
{"type":"event","data":{"domain":"climate","name":"started","targetTemp":21}}
// or
{"type":"event","data":{"domain":"climate","name":"error","error":"Battery too low"}}
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
3. **Command Timeout** - Allow 30 seconds for command responses
4. **Reconnection** - Accept reconnection and re-authentication at any time

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

---

## Protocol Version

This document describes **Protocol Version 2.0**.
