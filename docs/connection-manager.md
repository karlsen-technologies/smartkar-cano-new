# Connection Manager Module

> **Note:** This documentation refers to the legacy implementation. The current implementation is in `src/modules/LinkManager.h/cpp` and implements the `IModule` interface. The protocol is now handled via `CommandRouter` with `ICommandHandler` and `ITelemetryProvider` interfaces. See [Architecture](architecture.md) and [Protocol](protocol.md) for current documentation.

**File:** `src/modules/LinkManager.cpp`, `src/modules/LinkManager.h`

## Purpose

Manages the TCP connection to the remote server. Responsible for:

- Establishing TCP connection when modem is connected
- Authentication handshake with server
- Processing incoming commands
- Sending responses and state updates
- Preparing for sleep (notifying server, closing connection)

## State Machine

```
┌──────────────────┐
│ LINK_DISCONNECTED│◄─────────────────────────────┐
└────────┬─────────┘                              │
         │                                        │
         │ modem CONNECTED?                       │
         │ client->connect() success              │
         ▼                                        │
┌───────────────────┐                             │
│LINK_AUTHENTICATING│                             │
└────────┬──────────┘                             │
         │                                        │
         │ send auth request                      │
         │ wait for response...                   │
         │                                        │
    ┌────┴────┐                                   │
    │         │                                   │
"accepted" "rejected"                             │
    │         │                                   │
    ▼         ▼                                   │
┌────────────┐ ┌─────────────┐                    │
│LINK_CONNECTED│ │LINK_REJECTED│                  │
└─────┬──────┘ └─────────────┘                    │
      │         (terminal currently)              │
      │                                           │
      │ +CA STATE:0,0 (disconnect)                │
      └───────────────────────────────────────────┘
```

### State Descriptions

| State | Description | Blocks Sleep? |
|-------|-------------|---------------|
| `LINK_DISCONNECTED` | Not connected to server | No |
| `LINK_AUTHENTICATING` | TCP connected, awaiting auth response | Yes |
| `LINK_CONNECTED` | Authenticated and ready | No |
| `LINK_REJECTED` | Server rejected authentication | No |

## API

### Public Methods

```cpp
bool setup();
```
Performs initial setup. If modem is in hot start, closes any existing TCP connection.

```cpp
void loop();
```
Main loop tick. Attempts to connect to server when modem is connected and link is disconnected.

```cpp
bool prepareForSleep();
```
Called before entering deep sleep. Sends "asleep" state to server and closes TCP connection.

```cpp
void handleTCPInterrupt();
```
Processes TCP events from modem. Called by Modem when `+CA` URC is received.

### Singleton Access

```cpp
static ConnectionManager* instance();
```
Returns singleton instance. Used by Modem for interrupt forwarding.

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

### Data Reception Flow

```
+CA URC received
       │
       ▼
handleTCPInterrupt()
       │
       ▼
DATAIND: (data available)
       │
       ▼
client->readStringUntil('\n')
       │
       ▼
deserializeJson()
       │
       ▼
Check "type" field
       │
       ├── "authentication" → Update link state
       │
       └── "command" → Execute and respond
```

## Command Handling

### Current Implementation

Only the `test` command is implemented:

```cpp
if (type == "test") {
    int id = command["id"];
    String response = "{\"type\":\"response\",\"data\":{\"id\":" + String(id) + ",\"action\":\"test\",\"status\":\"ok\"}}";
    client->println(response);
}
```

### Future Commands

Planned commands for vehicle control:
- Start/stop charging
- Preheat cabin
- Lock/unlock doors
- Request telemetry

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

- `Modem` - For modem state and TinyGsmClient
- `ArduinoJson` - JSON parsing/serialization
- `TinyGsmClient` - TCP client from TinyGSM

## Configuration

```cpp
#define CONNECTION_SERVER_PORT 4589
```

Server hostname is hardcoded in `loop()`:
```cpp
client->connect("gallant.kartech.no", CONNECTION_SERVER_PORT)
```

## Current Behavior Summary

1. Waits for modem to reach `MODEM_CONNECTED` state
2. Opens TCP connection to server
3. Sends authentication with SIM CCID
4. Waits for server response
5. Handles commands when connected
6. Sends "asleep" state before sleep

## Future Considerations

### Error Recovery

- **LINK_REJECTED**: Currently terminal. Should implement retry after timeout.
- **Auth timeout**: No timeout for auth response. Device could hang in `LINK_AUTHENTICATING`.
- **Connection failure**: Retries immediately every loop. Could add backoff (not critical for single device).

### Protocol Enhancements

- **Send "awake" state**: Notify server when connection established
- **Unknown command response**: Send error for unrecognized commands
- **Periodic status**: Send device/vehicle status periodically

### JSON Handling

Currently mixes String concatenation and ArduinoJson:
- Deserialization uses ArduinoJson
- Serialization uses String concatenation

Consider using ArduinoJson for both for consistency and less error-prone code.

### Activity Tracking

Should report activity to coordinator when:
- Commands received
- Responses sent
- Connection state changes

## Known Issues

1. **LINK_REJECTED is terminal** - No recovery path, requires device restart.

2. **No auth timeout** - If server doesn't respond to auth request, device stays in `LINK_AUTHENTICATING` indefinitely.

3. **No unknown command handling** - Unrecognized commands are logged but no error is sent to server.

4. **String-based JSON serialization** - Error-prone compared to using ArduinoJson consistently.
