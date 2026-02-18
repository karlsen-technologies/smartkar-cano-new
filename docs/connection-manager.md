# Link Manager Module

**File:** `src/modules/LinkManager.h`, `src/modules/LinkManager.cpp`

**Current Implementation** - Updated January 2026

See also: [Architecture](architecture.md) and [Protocol](protocol.md)

## Purpose

Manages the TCP connection to the remote server and implements the `IModule` interface. Responsible for:

- Establishing TCP connection when modem is connected
- Authentication handshake with server (CCID-based)
- Receiving and parsing JSON command messages
- Routing commands to CommandRouter for execution
- Sending telemetry at priority-based intervals
- Preparing for sleep (send "bye" message, close connection gracefully)

## State Machine

```
┌──────────────────┐
│ LINK_DISCONNECTED│◄────────────────────────────────┐
└────────┬─────────┘                                 │
         │                                           │
         │ modem CONNECTED?                          │
         │ client->connect() success                 │
         ▼                                           │
┌───────────────────┐                                │
│  LINK_CONNECTING  │                                │
└────────┬──────────┘                                │
         │                                           │
         │ TCP connected                             │
         │ send auth request (CCID)                  │
         ▼                                           │
┌───────────────────┐                                │
│LINK_AUTHENTICATING│                                │
└────────┬──────────┘                                │
         │                                           │
         │ wait for auth response                    │
         │                                           │
    ┌────┴────┐                                      │
    │         │                                      │
"ok:true"  "ok:false"                                │
    │         │                                      │
    ▼         ▼                                      │
┌────────────┐ ┌─────────────┐                       │
│LINK_CONNECTED│ │LINK_REJECTED│                     │
└─────┬──────┘ └─────────────┘                       │
      │         (retries after timeout)              │
      │                                              │
      │ Connection lost / prepareForSleep()          │
      └──────────────────────────────────────────────┘
```

### State Descriptions

| State | Description | Blocks Sleep? |
|-------|-------------|---------------|
| `LINK_DISCONNECTED` | Not connected to server | No |
| `LINK_CONNECTING` | TCP connecting | Yes |
| `LINK_AUTHENTICATING` | Waiting for auth response (CCID-based) | Yes |
| `LINK_CONNECTED` | Authenticated, sending/receiving messages | No |
| `LINK_REJECTED` | Server rejected authentication (will retry) | No |

## API

### IModule Interface

```cpp
bool setup();           // Initialize TCP client
void loop();            // Attempt connection, handle messages, send telemetry
bool prepareForSleep(); // Send "bye" message, close TCP
bool isBusy();          // Returns true during LINK_CONNECTING/LINK_AUTHENTICATING
bool isReady();         // Returns true when LINK_CONNECTED
```

### Public Methods

```cpp
LinkState getState();        // Get current connection state
bool isConnected();          // Check if authenticated and connected
void sendMessage(const String& message);  // Send message to server
```

### Telemetry Management

LinkManager works with `CommandRouter` to send periodic telemetry:

```cpp
void loop() {
    // ... connection handling ...
    
    // Send telemetry at priority-based intervals
    TelemetryPriority priority = commandRouter->getHighestPriority();
    unsigned long interval = getTelemetryInterval(priority);
    
    if (millis() - lastTelemetrySent >= interval) {
        String telemetry = commandRouter->collectTelemetry(true);
        if (telemetry.length() > 0) {
            sendMessage(telemetry);
            lastTelemetrySent = millis();
        }
    }
}
```

**Telemetry Intervals:**
- `PRIORITY_REALTIME` - 5 seconds
- `PRIORITY_HIGH` - 30 seconds
- `PRIORITY_NORMAL` - 2 minutes
- `PRIORITY_LOW` - 5 minutes

Priority is determined by the highest priority among all providers with changed data.

## Server Protocol

### Connection

| Parameter | Value |
|-----------|-------|
| Server | `gallant.kartech.no` |
| Port | `4589` (defined as `CONNECTION_SERVER_PORT`) |
| Protocol | TCP |
| Format | JSON, newline-delimited |

### Message Structure

All messages use a common JSON structure:

```json
{
  "type": "<message_type>",
  "data": { ... }
}
```

### Message Types

#### Authentication Request (Device → Server)

```json
{
  "type": "authentication",
  "data": {
    "action": "authenticate",
    "ccid": "<SIM_CCID>"
  }
}
```

#### Authentication Response (Server → Device)

```json
{
  "type": "authentication",
  "data": {
    "action": "accepted"
  }
}
```

Or rejection:

```json
{
  "type": "authentication",
  "data": {
    "action": "rejected"
  }
}
```

#### Command (Server → Device)

```json
{
  "type": "command",
  "data": {
    "action": "<command_type>",
    "id": <command_id>,
    ... command-specific fields ...
  }
}
```

Currently implemented commands:
- `test` - Simple ping/pong test

#### Response (Device → Server)

```json
{
  "type": "response",
  "data": {
    "id": <command_id>,
    "action": "<command_type>",
    "status": "ok|error",
    ... response-specific fields ...
  }
}
```

#### State Update (Device → Server)

```json
{
  "type": "state",
  "data": {
    "type": "device",
    "state": "asleep"
  }
}
```

## TCP Interrupt Handling

When Modem receives `+CA` URC, it calls `ConnectionManager::handleTCPInterrupt()`:

```cpp
void handleTCPInterrupt() {
    // Wait for URC type
    int urc = modem->waitResponse("STATE:", "DATAIND:");
    
    if (urc == STATE) {
        // Connection state changed
        // "0,1" = connected
        // "0,0" = disconnected
    }
    else if (urc == DATAIND) {
        // Data available
        // Read and parse JSON
        // Handle based on message type
    }
}
```

## Message Handling

### Data Reception Flow

```
TCP data available (+CA DATAIND URC)
       │
       ▼
client->readStringUntil('\n')
       │
       ▼
deserializeJson()
       │
       ▼
Check message "type" field
       │
       ├── "auth" (response) → Update link state
       │
       ├── "command" → Extract id, action, params
       │                → commandRouter->handleCommand(action, id, params)
       │                → CommandRouter sends responses automatically
       │
       └── Other types → Log and ignore
```

### Command Routing

Commands are routed through `CommandRouter`, which:
1. Checks if another command is active (via `CommandStateManager`)
2. If busy, sends busy response immediately
3. If not busy, starts command tracking and routes to appropriate handler
4. Handler executes command (sync or async)
5. CommandStateManager sends progress updates as command progresses

LinkManager's role is limited to:
- Receiving command JSON
- Extracting id, action, and params
- Calling `commandRouter->handleCommand()`
- The actual response sending is done by CommandStateManager through a callback

See [Protocol](protocol.md) for full command lifecycle documentation.

## Connection Lifecycle

### Connect Sequence

1. Check modem state is `MODEM_CONNECTED`
2. Check link state is `LINK_DISCONNECTED`
3. Call `client->connect(server, port)`
4. On success, set state to `LINK_AUTHENTICATING`
5. Send authentication message with SIM CCID
6. Wait for auth response via interrupt

### Disconnect Handling

When `+CA STATE:0,0` is received:
1. Set state to `LINK_DISCONNECTED`
2. Next `loop()` iteration will attempt reconnect

### Sleep Preparation

```cpp
bool prepareForSleep() {
    if (state == LINK_CONNECTED) {
        // Notify server
        client->println("{\"type\":\"state\",\"data\":{\"type\":\"device\",\"state\":\"asleep\"}}");
        // Close connection
        client->stop();
        state = LINK_DISCONNECTED;
    }
    return true;
}
```

## Dependencies

- `ModemManager` - For modem state and TinyGsmClient instance
- `CommandRouter` - For command routing and telemetry collection
- `CommandStateManager` - For command lifecycle tracking (indirect via CommandRouter)
- `ArduinoJson` - JSON parsing/serialization
- `TinyGsmClient` - TCP client from TinyGSM library

## Configuration

Server connection configured in `LinkManager.cpp`:
```cpp
#define SERVER_HOST "gallant.kartech.no"
#define SERVER_PORT 4589
```

## Current Behavior

1. Waits for modem to reach `MODEM_CONNECTED` state
2. Opens TCP connection to server
3. Sends authentication with SIM CCID: `{"type":"auth","data":{"ccid":"..."}}`
4. Waits for auth response: `{"type":"auth","data":{"ok":true}}`
5. On auth success: State → `LINK_CONNECTED`
   - Starts periodic telemetry sending (priority-based intervals)
   - Routes incoming commands to CommandRouter
   - Sends responses via callback from CommandStateManager
6. On disconnect or sleep:
   - Sends bye message: `{"type":"bye","data":{"reason":"sleep"}}`
   - Closes TCP connection gracefully

## Error Recovery

- **Connection failure**: Retries on next loop iteration (no backoff currently)
- **Auth rejection** (`ok: false`): Currently logs error and retries after timeout
- **Disconnect during operation**: Auto-reconnect on next loop
- **Modem disconnection**: Waits for modem to reconnect before attempting TCP
