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
     │──────────── Authentication Request ──────────►│
     │                                               │
     │◄─────────── Authentication Response ──────────│
     │                                               │
     │◄─────────── Commands (anytime) ───────────────│
     │──────────── Responses ───────────────────────►│
     │                                               │
     │──────────── Telemetry (periodic) ────────────►│
     │                                               │
     │──────────── Events (when triggered) ─────────►│
     │                                               │
     │──────────── State: asleep ───────────────────►│
     │                                               │
     │──────────── TCP Disconnect ──────────────────►│
     │                                               │
```

## Message Structure

All messages follow this structure:

```json
{
  "type": "<message_type>",
  "data": { ... }
}
```

### Message Types

| Type | Direction | Description |
|------|-----------|-------------|
| `authentication` | Both | Authentication handshake |
| `command` | Server → Device | Command to execute |
| `response` | Device → Server | Response to a command |
| `telemetry` | Device → Server | Periodic status data |
| `event` | Device → Server | Real-time event notification |
| `state` | Device → Server | Device state change |

---

## Authentication

### Authentication Request (Device → Server)

Sent immediately after TCP connection is established.

```json
{
  "type": "authentication",
  "data": {
    "action": "authenticate",
    "ccid": "<SIM_ICCID>"
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `action` | string | Always `"authenticate"` |
| `ccid` | string | SIM card ICCID (19-20 digits) |

**Example:**
```json
{
  "type": "authentication",
  "data": {
    "action": "authenticate",
    "ccid": "8947080012345678901"
  }
}
```

### Authentication Response (Server → Device)

Server must respond with acceptance or rejection.

**Accepted:**
```json
{
  "type": "authentication",
  "data": {
    "action": "accepted"
  }
}
```

**Rejected:**
```json
{
  "type": "authentication",
  "data": {
    "action": "rejected",
    "reason": "Unknown device"
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `action` | string | `"accepted"` or `"rejected"` |
| `reason` | string | (Optional) Rejection reason |

**Device Behavior:**
- On `accepted`: Transition to connected state, begin telemetry
- On `rejected`: Close connection, may retry after timeout

---

## Commands

### Command Message (Server → Device)

```json
{
  "type": "command",
  "data": {
    "id": <command_id>,
    "action": "<domain>.<action>",
    "params": { ... }
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Unique command identifier (for correlating responses) |
| `action` | string | Command action in format `domain.action` |
| `params` | object | Command-specific parameters (optional, can be empty `{}`) |

### Response Message (Device → Server)

```json
{
  "type": "response",
  "data": {
    "id": <command_id>,
    "status": "<status>",
    "data": { ... },
    "error": "<error_message>"
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Matches the command `id` |
| `status` | string | `"ok"`, `"error"`, `"pending"`, `"not_supported"` |
| `data` | object | Response payload (when `status` is `"ok"`) |
| `error` | string | Error message (when `status` is `"error"`) |

### Command Status Values

| Status | Description |
|--------|-------------|
| `ok` | Command executed successfully |
| `error` | Command failed (see `error` field) |
| `pending` | Command accepted, will complete asynchronously |
| `not_supported` | Unknown command or domain |

---

## Implemented Commands

### system.reboot

Restarts the device.

**Request:**
```json
{
  "type": "command",
  "data": {
    "id": 1,
    "action": "system.reboot",
    "params": {}
  }
}
```

**Response:**
```json
{
  "type": "response",
  "data": {
    "id": 1,
    "status": "ok",
    "data": {
      "message": "Rebooting"
    }
  }
}
```

**Note:** Response is sent before reboot. Device will disconnect and reconnect after restart.

---

### system.sleep

Requests the device to enter deep sleep.

**Request:**
```json
{
  "type": "command",
  "data": {
    "id": 2,
    "action": "system.sleep",
    "params": {
      "duration": 300
    }
  }
}
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `duration` | integer | (Optional) Sleep duration in seconds. 0 or omitted = wake on interrupt only |

**Response:**
```json
{
  "type": "response",
  "data": {
    "id": 2,
    "status": "ok",
    "data": {
      "duration": 300,
      "wakeMode": "timer_or_interrupt"
    }
  }
}
```

| Response Field | Description |
|----------------|-------------|
| `duration` | Confirmed sleep duration |
| `wakeMode` | `"timer_or_interrupt"` or `"interrupt_only"` |

**Behavior:** Device will send response, then send `state: asleep`, then enter deep sleep.

---

### system.telemetry

Forces immediate telemetry collection.

**Request:**
```json
{
  "type": "command",
  "data": {
    "id": 3,
    "action": "system.telemetry",
    "params": {}
  }
}
```

**Response:**
```json
{
  "type": "response",
  "data": {
    "id": 3,
    "status": "ok",
    "data": {
      "message": "Telemetry collected",
      "collected": true
    }
  }
}
```

**Note:** This triggers telemetry collection. The telemetry will be sent as a separate `telemetry` message.

---

### system.info

Returns detailed device information.

**Request:**
```json
{
  "type": "command",
  "data": {
    "id": 4,
    "action": "system.info",
    "params": {}
  }
}
```

**Response:**
```json
{
  "type": "response",
  "data": {
    "id": 4,
    "status": "ok",
    "data": {
      "chipModel": "ESP32-S3",
      "chipRevision": 0,
      "chipCores": 2,
      "cpuFreqMHz": 240,
      "freeHeap": 245760,
      "minFreeHeap": 234500,
      "heapSize": 327680,
      "flashSize": 16777216,
      "flashSpeed": 80000000,
      "sdkVersion": "v4.4.6",
      "uptime": 125340
    }
  }
}
```

| Response Field | Type | Description |
|----------------|------|-------------|
| `chipModel` | string | ESP32 chip model |
| `chipRevision` | integer | Chip revision number |
| `chipCores` | integer | Number of CPU cores |
| `cpuFreqMHz` | integer | CPU frequency in MHz |
| `freeHeap` | integer | Current free heap in bytes |
| `minFreeHeap` | integer | Minimum free heap since boot |
| `heapSize` | integer | Total heap size in bytes |
| `flashSize` | integer | Flash size in bytes |
| `flashSpeed` | integer | Flash speed in Hz |
| `sdkVersion` | string | ESP-IDF SDK version |
| `uptime` | integer | Milliseconds since boot |

---

## Telemetry

### Telemetry Message (Device → Server)

Sent periodically based on priority (5 seconds to 5 minutes).

```json
{
  "type": "telemetry",
  "data": {
    "<domain>": { ... },
    "<domain>": { ... }
  }
}
```

Multiple domains can be included in a single telemetry message.

### Device Telemetry Domain

Domain: `device`

```json
{
  "type": "telemetry",
  "data": {
    "device": {
      "uptime": 125340,
      "freeHeap": 245760,
      "wakeCause": "modem_ri",
      "batteryVoltage": 4.15,
      "batteryPercent": 85,
      "charging": true,
      "vbusConnected": true,
      "chipModel": "ESP32-S3",
      "chipRevision": 0,
      "cpuFreqMHz": 240
    }
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `uptime` | integer | Milliseconds since boot |
| `freeHeap` | integer | Free heap memory in bytes |
| `wakeCause` | string | Wake reason (see below) |
| `batteryVoltage` | float | Battery voltage (V) |
| `batteryPercent` | integer | Battery percentage (0-100) |
| `charging` | boolean | Whether battery is charging |
| `vbusConnected` | boolean | Whether USB/external power is connected |
| `chipModel` | string | ESP32 chip model |
| `chipRevision` | integer | Chip silicon revision |
| `cpuFreqMHz` | integer | CPU frequency |

**Wake Cause Values:**

| Value | Description |
|-------|-------------|
| `fresh_boot` | Initial power-on |
| `timer` | Timer wake from deep sleep |
| `gpio` | GPIO wake |
| `modem_ri` | Modem RI pin (EXT1) wake |
| `unknown` | Unknown wake cause |

**Trigger Conditions:**
- Initial report on device wake
- Every 5 minutes
- Battery level change > 5%
- Charging state change

### Network Telemetry Domain

Domain: `network`

```json
{
  "type": "telemetry",
  "data": {
    "network": {
      "modemState": "connected",
      "signalStrength": -75,
      "simCCID": "8947080012345678901",
      "modemConnected": true,
      "linkConnected": true,
      "linkState": "connected"
    }
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `modemState` | string | Current modem state (see below) |
| `signalStrength` | integer | Signal strength in dBm |
| `simCCID` | string | SIM card ICCID |
| `modemConnected` | boolean | Whether modem has internet |
| `linkConnected` | boolean | Whether TCP link is connected |
| `linkState` | string | Current link state (see below) |

**Modem State Values:**

| Value | Description |
|-------|-------------|
| `off` | Modem powered off |
| `starting` | Modem powering on |
| `configuring` | Configuring modem settings |
| `searching` | Searching for network |
| `registered` | Registered, activating PDP |
| `connected` | Internet connected |
| `error` | Error state |

**Link State Values:**

| Value | Description |
|-------|-------------|
| `disconnected` | Not connected to server |
| `connecting` | TCP connecting |
| `authenticating` | Waiting for auth response |
| `connected` | Authenticated |

**Trigger Conditions:**
- Initial report on device wake
- Every 2 minutes
- Modem or link state change
- Signal strength change > 10 dBm

---

## Events

### Event Message (Device → Server)

Sent immediately when significant events occur.

```json
{
  "type": "event",
  "data": {
    "domain": "<domain>",
    "event": "<event_name>",
    "timestamp": <unix_timestamp>,
    "details": { ... }
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `domain` | string | Event domain (e.g., `"system"`, `"vehicle"`) |
| `event` | string | Event name |
| `timestamp` | integer | Unix timestamp (seconds) |
| `details` | object | Event-specific data |

**Example (future vehicle event):**
```json
{
  "type": "event",
  "data": {
    "domain": "charging",
    "event": "chargingStarted",
    "timestamp": 1706200000,
    "details": {
      "chargerPower": 7400,
      "targetSoc": 80
    }
  }
}
```

---

## State Updates

### State Message (Device → Server)

Sent when device state changes significantly.

```json
{
  "type": "state",
  "data": {
    "type": "device",
    "state": "<state>"
  }
}
```

**Current States:**

| State | Description |
|-------|-------------|
| `asleep` | Device is entering deep sleep |
| `awake` | Device has woken up (future) |

**Example:**
```json
{
  "type": "state",
  "data": {
    "type": "device",
    "state": "asleep"
  }
}
```

**Note:** The `asleep` state is sent just before the device disconnects and enters deep sleep.

---

## Future Commands (Planned)

These commands are planned for vehicle integration:

### charging.start
```json
{
  "type": "command",
  "data": {
    "id": 10,
    "action": "charging.start",
    "params": {
      "targetSoc": 80,
      "maxCurrent": 16
    }
  }
}
```

### charging.stop
```json
{
  "type": "command",
  "data": {
    "id": 11,
    "action": "charging.stop",
    "params": {}
  }
}
```

### climate.preheat
```json
{
  "type": "command",
  "data": {
    "id": 12,
    "action": "climate.preheat",
    "params": {
      "targetTemp": 21,
      "duration": 15
    }
  }
}
```

### security.lock / security.unlock
```json
{
  "type": "command",
  "data": {
    "id": 13,
    "action": "security.lock",
    "params": {}
  }
}
```

---

## Error Handling

### Command Errors

When a command fails, the response includes an error message:

```json
{
  "type": "response",
  "data": {
    "id": 5,
    "status": "error",
    "error": "Invalid parameter: duration must be positive"
  }
}
```

### Unknown Commands

Unknown domains or actions return `not_supported`:

```json
{
  "type": "response",
  "data": {
    "id": 6,
    "status": "not_supported",
    "error": "Unknown domain: foo"
  }
}
```

### Connection Loss

If the TCP connection is lost:
- Device will attempt to reconnect when modem is connected
- Server should consider the device offline after timeout
- Any pending commands should be retried after reconnection

---

## Server Implementation Notes

### Required Server Capabilities

1. **TCP Server** - Listen on configured port (default 4589)
2. **Device Registry** - Map CCID to device records for authentication
3. **Command Queue** - Store pending commands for offline devices
4. **Telemetry Storage** - Store and query telemetry data
5. **Event Handling** - Process real-time events

### Recommended Behavior

1. **Authentication Timeout** - Disconnect if no auth request within 30 seconds
2. **Keepalive** - Consider device offline if no message received for 10 minutes
3. **Command Timeout** - Allow 30 seconds for command responses
4. **Reconnection** - Accept reconnection and re-authentication at any time

### Message Parsing

```
while (socket is connected):
    line = read until '\n'
    message = JSON.parse(line)
    
    switch (message.type):
        case "authentication":
            handle_authentication(message.data)
        case "response":
            handle_response(message.data)
        case "telemetry":
            store_telemetry(message.data)
        case "event":
            process_event(message.data)
        case "state":
            update_device_state(message.data)
```

### Example Server Pseudocode

```python
class DeviceConnection:
    def __init__(self, socket):
        self.socket = socket
        self.authenticated = False
        self.device_id = None
    
    def handle_message(self, message):
        if message['type'] == 'authentication':
            ccid = message['data']['ccid']
            device = database.find_device_by_ccid(ccid)
            
            if device:
                self.authenticated = True
                self.device_id = device.id
                self.send({
                    'type': 'authentication',
                    'data': {'action': 'accepted'}
                })
                # Send any pending commands
                self.send_pending_commands()
            else:
                self.send({
                    'type': 'authentication',
                    'data': {'action': 'rejected', 'reason': 'Unknown device'}
                })
                self.socket.close()
        
        elif message['type'] == 'response':
            command_id = message['data']['id']
            database.update_command_status(command_id, message['data'])
        
        elif message['type'] == 'telemetry':
            database.store_telemetry(self.device_id, message['data'])
        
        elif message['type'] == 'state':
            if message['data']['state'] == 'asleep':
                database.set_device_offline(self.device_id)
    
    def send_command(self, action, params={}):
        command_id = database.create_command(self.device_id, action, params)
        self.send({
            'type': 'command',
            'data': {
                'id': command_id,
                'action': action,
                'params': params
            }
        })
```

---

## Protocol Version

This document describes **Protocol Version 1.0**.

Future versions may add:
- Protocol version negotiation during authentication
- Binary message encoding option
- Message compression
- Encryption (beyond TCP/TLS)
