# MQTT Migration Plan

**Created:** 2026-02-10  
**Status:** Planning  
**Author:** AI Assistant

This document outlines the plan to migrate from the current TCP-based protocol to MQTT, addressing NAT punch-through issues with cellular connectivity.

---

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Current Architecture](#current-architecture)
3. [Why MQTT](#why-mqtt)
4. [SIM7080G MQTT Capabilities](#sim7080g-mqtt-capabilities)
5. [Open Questions](#open-questions)
6. [Proposed Architecture](#proposed-architecture)
7. [Topic Structure](#topic-structure)
8. [Message Format](#message-format)
9. [Implementation Plan](#implementation-plan)
10. [Risk Assessment](#risk-assessment)
11. [Testing Strategy](#testing-strategy)

---

## Problem Statement

### Current Issue

The SmartKar-Cano device uses a custom JSON protocol over TCP (port 4589) to communicate with the server. After the device enters deep sleep for extended periods (hours), the cellular network's NAT mapping expires. When the server needs to send a command (e.g., start charging), it cannot reach the device because:

1. The device is in deep sleep (no active TCP connection)
2. The NAT mapping from the last session has expired
3. The server has no way to "punch through" to wake the device

### Impact

- Server-initiated commands fail when device has been sleeping
- User must wait for device to wake on its own (timer or vehicle activity)
- Poor user experience for remote control features

### Root Cause

TCP connections are stateful and bidirectional. When the device sleeps, the connection closes. The cellular NAT loses the port mapping after timeout (typically 5-30 minutes depending on carrier).

---

## Current Architecture

### Communication Flow (TCP)

```
┌──────────────────┐                        ┌──────────────────┐
│     Device       │                        │      Server      │
│  (ESP32-S3 +     │     TCP Port 4589      │                  │
│   SIM7080G)      │◄──────────────────────►│                  │
└──────────────────┘                        └──────────────────┘

Device initiates connection → Server accepts
Device sends: auth, state, event, bye
Server sends: auth response, command
```

### Key Files

| File | Purpose |
|------|---------|
| `src/modules/LinkManager.h/cpp` | TCP connection management, protocol handling |
| `src/modules/ModemManager.h/cpp` | Modem control, AT commands, RI pin handling |
| `src/core/CommandRouter.h/cpp` | Command routing, telemetry collection |
| `src/core/DeviceController.h/cpp` | Central coordinator, sleep/wake logic |

### Sleep/Wake Behavior

1. Device enters deep sleep to conserve battery
2. Wake sources: Modem RI pin, PMU IRQ, Timer
3. On wake: Re-initialize modem, reconnect TCP, authenticate

---

## Why MQTT

### Benefits

| Benefit | Description |
|---------|-------------|
| **Built-in Keep-Alive** | MQTT protocol has native PINGREQ/PINGRESP to maintain NAT mappings (`KEEPTIME` configurable 0-65535 seconds) |
| **Broker Handles Routing** | No need for server to maintain device connections; broker handles message routing |
| **QoS Levels** | QoS 0 (at most once), QoS 1 (at least once), QoS 2 (exactly once) |
| **Last Will Testament (LWT)** | Broker can notify server when device disconnects unexpectedly |
| **Retained Messages** | Server can publish commands that device receives when it reconnects |
| **Topic-Based Routing** | Clean separation of message types (telemetry, commands, events) |
| **Modem Native Support** | SIM7080G has built-in MQTT client, reducing firmware complexity |

### Key Advantage: Persistent Subscriptions

Even when the device disconnects, if using `CLEANSS=0` (persistent session), the broker:
1. Remembers device subscriptions
2. Queues messages (QoS 1/2) for offline devices
3. Delivers queued messages when device reconnects

This solves the NAT punch-through problem by inverting the model:
- Device maintains subscription to command topic
- Server publishes to topic (doesn't need direct connection to device)
- When device wakes and reconnects, it receives pending commands

### Potential Challenges

| Challenge | Mitigation |
|-----------|------------|
| **Requires MQTT Broker** | Self-host Mosquitto or use cloud service |
| **Protocol Redesign** | Careful topic/message design to preserve semantics |
| **RI Pin Wake (CRITICAL)** | Must verify `+SMSUB` URC triggers RI pin |
| **TLS Overhead** | May impact battery; test power consumption |
| **Message Size Limit** | SIM7080G: 1024 bytes max for `AT+SMPUB` |
| **Learning Curve** | Team needs MQTT expertise |

---

## SIM7080G MQTT Capabilities

### AT Commands Reference

Source: `SIM7070_SIM7080_SIM7090 Series_AT Command Manual_V1.05.pdf` (Chapter 17)

| Command | Description | Parameters |
|---------|-------------|------------|
| `AT+SMCONF` | Configure MQTT parameters | URL, KEEPTIME, CLIENTID, USERNAME, PASSWORD, CLEANSS, QOS, TOPIC, MESSAGE, RETAIN |
| `AT+SMSSL` | Configure TLS | SSL index (0-6) |
| `AT+SMCONN` | Connect to broker | - |
| `AT+SMDISC` | Disconnect from broker | - |
| `AT+SMSUB` | Subscribe to topic | topic (max 128 chars), QoS |
| `AT+SMUNSUB` | Unsubscribe from topic | topic |
| `AT+SMPUB` | Publish message | topic, length (max 1024), QoS, retain |
| `AT+SMSTATE` | Query connection status | Returns: 0=disconnected, 1=online, 2=online+session |

### Unsolicited Result Codes (URCs)

| URC | Description |
|-----|-------------|
| `+SMSUB: "<topic>","<message>"` | Received message on subscribed topic |
| `+SMSTATE: <state>` | Connection state changed |

### Configuration Limits

| Parameter | Limit |
|-----------|-------|
| Topic length | 128 characters max |
| Message size | 1024 bytes max |
| Keep-alive time | 0-65535 seconds |
| Subscriptions | Multiple (exact limit TBD) |

### Example Session

```
// 1. Configure broker
AT+SMCONF="URL","broker.example.com",1883
OK
AT+SMCONF="CLIENTID","smartkar-ABC123"
OK
AT+SMCONF="KEEPTIME",60
OK
AT+SMCONF="CLEANSS",0    // Persistent session
OK

// 2. Connect
AT+SMCONN
OK

// 3. Subscribe to command topic
AT+SMSUB="smartkar/ABC123/command",1
OK

// 4. Publish telemetry
AT+SMPUB="smartkar/ABC123/state",128,1,0
>{"device":{...},"vehicle":{...}}
OK

// 5. Receive command (URC)
+SMSUB: "smartkar/ABC123/command","{\"id\":1,\"action\":\"vehicle.startCharging\"}"

// 6. Publish response
AT+SMPUB="smartkar/ABC123/response",64,1,0
>{"id":1,"ok":true,"status":"in_progress","stage":"accepted"}
OK
```

---

## Open Questions

### Critical (Must Answer Before Implementation)

#### 1. RI Pin Wake for MQTT Messages

**Question:** Does the `+SMSUB` URC trigger the modem's RI (Ring Indicator) pin to wake the ESP32 from deep sleep?

**Why Critical:** This is the fundamental requirement for server-initiated wake. If the RI pin doesn't pulse on MQTT message receipt, the migration won't solve the original problem.

**Investigation Steps:**
1. Check AT command manual for RI configuration (`AT+CFGRI` or similar)
2. Search for community reports on SIM7080G + MQTT + RI behavior
3. Empirical testing on hardware

**Related AT Commands to Check:**
- `AT+CFGRI` - Configure RI behavior
- `AT+SMCONF` - MQTT-specific RI settings?

#### 2. MQTT Connection During Sleep

**Question:** Can the SIM7080G maintain an MQTT connection while the ESP32 is in deep sleep?

**Options:**
- **Option A:** Modem stays connected, ESP32 sleeps (modem handles keep-alive)
- **Option B:** Both sleep, reconnect on wake (loses persistent connection benefit)

**Why Critical:** If Option A is feasible, device can receive commands while sleeping. If not, we need a different wake mechanism.

### Important (Should Decide Before Implementation)

#### 3. MQTT Broker Choice

| Option | Pros | Cons |
|--------|------|------|
| **Self-hosted Mosquitto** | Full control, low cost, simple | Requires maintenance, single point of failure |
| **Self-hosted EMQX** | Clustering, dashboard, plugins | More complex setup |
| **AWS IoT Core** | Managed, scalable, device shadows | Cost at scale, vendor lock-in |
| **Azure IoT Hub** | Managed, integrates with Azure | Cost, complexity |
| **HiveMQ Cloud** | Easy setup, managed | Monthly cost |

**Recommendation:** Start with self-hosted Mosquitto on existing server, migrate to cloud later if needed.

#### 4. TLS Requirements

**Questions:**
- Is TLS required for production?
- Self-signed certificates or CA-signed?
- Certificate provisioning strategy for devices?

**Note:** SIM7080G supports TLS via `AT+SMSSL` and `AT+CSSLCFG`.

#### 5. Migration Strategy

| Strategy | Description | Risk |
|----------|-------------|------|
| **Full replacement** | Remove TCP, MQTT only | High - no fallback |
| **Gradual migration** | MQTT for some messages, TCP for others | Medium - complexity |
| **Coexistence** | Both protocols available | Medium - maintenance burden |
| **MQTT primary + TCP fallback** | Try MQTT, fall back to TCP if broker unreachable | Low - best of both |

**Recommendation:** MQTT primary + TCP fallback during transition.

#### 6. Server-Side Scope

**Questions:**
- What language/framework is the server implemented in?
- Is server-side MQTT implementation in scope?
- Should device support both protocols during migration?

---

## Proposed Architecture

### High-Level Design

```
┌──────────────────┐                        ┌──────────────────┐
│     Device       │                        │   MQTT Broker    │
│  (ESP32-S3 +     │        MQTT            │   (Mosquitto)    │
│   SIM7080G)      │◄──────────────────────►│                  │
└──────────────────┘                        └────────┬─────────┘
                                                     │
                                            ┌────────▼─────────┐
                                            │      Server      │
                                            │   (MQTT Client)  │
                                            └──────────────────┘

Device publishes: state, response, event, bye
Device subscribes: command
Server publishes: command
Server subscribes: state, response, event, bye (or wildcard)
```

### Module Changes

#### New: MqttManager

```cpp
// src/modules/MqttManager.h
class MqttManager : public Module {
public:
    // Lifecycle
    bool connect();
    bool disconnect();
    bool isConnected();
    
    // Pub/Sub
    bool publish(const String& topic, const String& payload, uint8_t qos = 1, bool retain = false);
    bool subscribe(const String& topic, uint8_t qos = 1);
    bool unsubscribe(const String& topic);
    
    // Configuration
    void setBroker(const String& url, uint16_t port);
    void setCredentials(const String& username, const String& password);
    void setClientId(const String& clientId);
    void setKeepAlive(uint16_t seconds);
    void setCleanSession(bool clean);
    void setLWT(const String& topic, const String& message);
    
    // URC handling
    void handleMqttURC(const String& urc);
    
    // Callbacks
    void setMessageCallback(std::function<void(const String& topic, const String& payload)> callback);
    
private:
    // AT command execution
    bool sendATCommand(const String& cmd, uint32_t timeout = 5000);
    bool waitForResponse(const String& expected, uint32_t timeout);
};
```

#### Modified: ModemManager

Add MQTT URC handling to existing URC processor:

```cpp
void ModemManager::processURC(const String& line) {
    // Existing URCs...
    
    // New: MQTT message received
    if (line.startsWith("+SMSUB:")) {
        if (mqttManager) {
            mqttManager->handleMqttURC(line);
        }
    }
    
    // New: MQTT state changed
    if (line.startsWith("+SMSTATE:")) {
        // Handle connection state change
    }
}
```

#### Modified: DeviceController

Integrate MqttManager alongside or replacing LinkManager:

```cpp
class DeviceController {
private:
    // Option A: Replace LinkManager
    MqttManager* mqttManager;
    
    // Option B: Coexistence
    LinkManager* linkManager;
    MqttManager* mqttManager;
    bool useMqtt;
};
```

---

## Topic Structure

### Proposed Topics

Using device CCID as unique identifier (already used for authentication).

```
smartkar/{ccid}/state      # Device → Broker (telemetry)
smartkar/{ccid}/response   # Device → Broker (command responses)
smartkar/{ccid}/event      # Device → Broker (events)
smartkar/{ccid}/command    # Broker → Device (commands from server)
smartkar/{ccid}/status     # LWT topic (broker publishes when device disconnects)
```

### Topic Permissions (ACL)

| Client | Publish | Subscribe |
|--------|---------|-----------|
| Device `{ccid}` | `smartkar/{ccid}/state`, `smartkar/{ccid}/response`, `smartkar/{ccid}/event` | `smartkar/{ccid}/command` |
| Server | `smartkar/+/command` | `smartkar/#` |

### Last Will Testament (LWT)

Device configures LWT on connect:

```
Topic: smartkar/{ccid}/status
Message: {"online": false, "timestamp": 1234567890}
QoS: 1
Retain: true
```

On successful connect, device publishes:

```
Topic: smartkar/{ccid}/status
Message: {"online": true, "timestamp": 1234567890}
QoS: 1
Retain: true
```

Server can check retained message on `status` topic to know if device is online.

---

## Message Format

### Design Principle

Preserve existing JSON message format from `protocol.md` as much as possible. The `type` field becomes implicit from the topic.

### State Messages

**Topic:** `smartkar/{ccid}/state`

```json
{
  "device": {
    "uptime": 125340,
    "freeHeap": 245760,
    "wakeCause": "modem_ri",
    "batteryVoltage": 4.15,
    "batteryPercent": 85
  },
  "network": {
    "modemState": "connected",
    "signalStrength": -75,
    "mqttConnected": true
  },
  "vehicle": {
    "battery": { ... },
    "climate": { ... },
    "plug": { ... }
  }
}
```

**Note:** Removed `type: "state"` (implicit from topic). Added `mqttConnected` to network.

### Command Messages

**Topic:** `smartkar/{ccid}/command`

```json
{
  "id": 12,
  "action": "vehicle.startCharging",
  "targetSoc": 80,
  "maxCurrent": 16
}
```

**Note:** Removed `type: "command"` wrapper (implicit from topic).

### Response Messages

**Topic:** `smartkar/{ccid}/response`

```json
{
  "id": 12,
  "ok": true,
  "status": "in_progress",
  "stage": "accepted"
}
```

**Note:** Same structure as TCP protocol, just on different topic.

### Event Messages

**Topic:** `smartkar/{ccid}/event`

```json
{
  "domain": "vehicle",
  "name": "chargingStarted",
  "soc": 75,
  "powerKw": 3.2
}
```

### Bye Messages

**Topic:** `smartkar/{ccid}/status`

```json
{
  "online": false,
  "reason": "sleep",
  "timestamp": 1234567890
}
```

---

## Implementation Plan

### Phase 1: Research & Verification (1-2 days)

**Goal:** Answer critical open questions before writing code.

| Task | Description | Owner |
|------|-------------|-------|
| 1.1 | Verify RI pin behavior for `+SMSUB` URC | - |
| 1.2 | Test modem MQTT connection during ESP32 sleep | - |
| 1.3 | Document findings in this plan | - |

**Deliverables:**
- [ ] RI pin verification results
- [ ] Sleep/wake test results
- [ ] Updated plan with findings

### Phase 2: Infrastructure Setup (1 day)

**Goal:** Set up MQTT broker for development and testing.

| Task | Description |
|------|-------------|
| 2.1 | Install Mosquitto on development server |
| 2.2 | Configure authentication (username/password) |
| 2.3 | Configure TLS (optional, can defer) |
| 2.4 | Set up topic ACLs |
| 2.5 | Test with MQTT client (e.g., MQTTX, mosquitto_pub/sub) |

**Deliverables:**
- [ ] Running MQTT broker
- [ ] Test credentials
- [ ] Basic connectivity verified

### Phase 3: MqttManager Implementation (3-5 days)

**Goal:** Implement MQTT client module for ESP32.

| Task | Description |
|------|-------------|
| 3.1 | Create `MqttManager.h/cpp` with basic structure |
| 3.2 | Implement broker configuration (`AT+SMCONF`) |
| 3.3 | Implement connect/disconnect (`AT+SMCONN`, `AT+SMDISC`) |
| 3.4 | Implement subscribe/unsubscribe (`AT+SMSUB`, `AT+SMUNSUB`) |
| 3.5 | Implement publish (`AT+SMPUB`) |
| 3.6 | Implement URC handling for `+SMSUB` |
| 3.7 | Implement connection state tracking (`AT+SMSTATE`) |
| 3.8 | Add error handling and retry logic |
| 3.9 | Integrate with ModemManager for URC routing |

**Deliverables:**
- [ ] Working MqttManager module
- [ ] Unit tests (if applicable)
- [ ] Integration with ModemManager

### Phase 4: Protocol Integration (2-3 days)

**Goal:** Integrate MQTT with existing command/response flow.

| Task | Description |
|------|-------------|
| 4.1 | Create MQTT message formatter (adapt existing JSON format) |
| 4.2 | Integrate with CommandRouter for command reception |
| 4.3 | Integrate with telemetry publishing |
| 4.4 | Integrate with event publishing |
| 4.5 | Implement LWT configuration |
| 4.6 | Update DeviceController for MQTT lifecycle |

**Deliverables:**
- [ ] Full message flow working over MQTT
- [ ] Commands received and executed
- [ ] Responses and events published

### Phase 5: Sleep/Wake Integration (2-3 days)

**Goal:** Ensure MQTT works with deep sleep cycles.

| Task | Description |
|------|-------------|
| 5.1 | Test wake-on-MQTT-message via RI pin |
| 5.2 | Implement quick reconnect after wake |
| 5.3 | Handle pending messages on reconnect |
| 5.4 | Test keep-alive timing for NAT traversal |
| 5.5 | Optimize reconnection speed |

**Deliverables:**
- [ ] Wake-on-message working (if supported)
- [ ] Reliable reconnection after sleep
- [ ] Documented keep-alive configuration

### Phase 6: Server Integration (2-5 days)

**Goal:** Update server to use MQTT.

| Task | Description |
|------|-------------|
| 6.1 | Add MQTT client to server |
| 6.2 | Subscribe to device topics (state, response, event) |
| 6.3 | Publish to command topics |
| 6.4 | Handle LWT for device status |
| 6.5 | Update API endpoints if needed |

**Deliverables:**
- [ ] Server MQTT integration
- [ ] Bidirectional communication verified

### Phase 7: Testing & Validation (2-3 days)

**Goal:** Comprehensive testing before production.

| Task | Description |
|------|-------------|
| 7.1 | End-to-end command execution tests |
| 7.2 | Sleep/wake cycle tests |
| 7.3 | Long-duration NAT traversal tests |
| 7.4 | Error recovery tests (broker restart, network loss) |
| 7.5 | Power consumption comparison |

**Deliverables:**
- [ ] Test results documented
- [ ] Issues fixed
- [ ] Go/no-go decision for production

### Phase 8: Production Deployment

**Goal:** Roll out to production.

| Task | Description |
|------|-------------|
| 8.1 | Set up production MQTT broker |
| 8.2 | Configure production credentials |
| 8.3 | Deploy server changes |
| 8.4 | Deploy firmware to devices (staged rollout) |
| 8.5 | Monitor for issues |
| 8.6 | Remove TCP fallback (later, when stable) |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| RI pin doesn't wake on MQTT | Medium | Critical | Test early in Phase 1; if fails, investigate SMS wake or other mechanisms |
| NAT timeout shorter than keep-alive | Low | High | Configure aggressive keep-alive (30-60s); test with carrier |
| Broker downtime | Medium | High | Self-host with monitoring; consider cloud backup |
| TLS certificate expiry | Medium | Medium | Implement certificate rotation; monitor expiry |
| Message loss | Low | Medium | Use QoS 1 for important messages |
| Increased power consumption | Medium | Medium | Measure and optimize; compare with TCP baseline |
| Protocol complexity | Low | Low | Preserve existing message format as much as possible |

---

## Testing Strategy

### Unit Tests

- MqttManager AT command generation
- Message formatting
- Topic construction
- URC parsing

### Integration Tests

- Connect/disconnect cycle
- Publish/subscribe flow
- Command execution via MQTT
- Event generation

### System Tests

| Test | Description | Pass Criteria |
|------|-------------|---------------|
| Basic connectivity | Connect to broker, subscribe, publish | Messages flow correctly |
| Command execution | Send command via MQTT, verify execution | Response received, vehicle responds |
| Sleep/wake cycle | Sleep device, send command, verify wake | Device wakes, command executes |
| NAT traversal | Leave device idle for 1+ hours, send command | Command received within reasonable time |
| Reconnection | Kill broker, restart, verify device reconnects | Auto-reconnect within 60s |
| Error handling | Send malformed message, verify device handles gracefully | No crash, error logged |

### Performance Tests

- Message latency comparison (MQTT vs TCP)
- Power consumption during keep-alive
- Reconnection time after sleep

---

## Appendix A: AT Command Reference

### MQTT Configuration Commands

```bash
# Set broker URL and port
AT+SMCONF="URL","<host>",<port>

# Set client ID (use CCID for uniqueness)
AT+SMCONF="CLIENTID","<client_id>"

# Set keep-alive time (seconds, 0 = disable)
AT+SMCONF="KEEPTIME",<seconds>

# Set clean session (0 = persistent, 1 = clean)
AT+SMCONF="CLEANSS",<0|1>

# Set username/password
AT+SMCONF="USERNAME","<username>"
AT+SMCONF="PASSWORD","<password>"

# Set QoS for operations
AT+SMCONF="QOS",<0|1|2>

# Set Last Will Testament
AT+SMCONF="TOPIC","<lwt_topic>"
AT+SMCONF="MESSAGE","<lwt_message>"
AT+SMCONF="RETAIN",<0|1>
```

### MQTT Operation Commands

```bash
# Connect to broker
AT+SMCONN

# Disconnect from broker
AT+SMDISC

# Check connection status
AT+SMSTATE?
# Returns: +SMSTATE: 0 (disconnected), 1 (connected), 2 (connected with session)

# Subscribe to topic
AT+SMSUB="<topic>",<qos>

# Unsubscribe from topic
AT+SMUNSUB="<topic>"

# Publish message
AT+SMPUB="<topic>",<length>,<qos>,<retain>
><message_content>

# Set publish data format to hex
AT+SMPUBHEX=<0|1>
```

### SSL/TLS Commands

```bash
# Configure SSL (if using MQTTS)
AT+CSSLCFG="sslversion",<ssl_index>,<version>
AT+CSSLCFG="cacert",<ssl_index>,"<cert_data>"
AT+CSSLCFG="clientcert",<ssl_index>,"<cert_data>"
AT+CSSLCFG="clientkey",<ssl_index>,"<key_data>"

# Select SSL config for MQTT
AT+SMSSL=<ssl_index>
```

---

## Appendix B: Related Documentation

- `docs/protocol.md` - Current JSON protocol specification (v2.2)
- `docs/modem.md` - ModemManager implementation details
- `docs/sleep-wake.md` - Deep sleep behavior and wake sources
- `docs/connection-manager.md` - LinkManager TCP implementation
- `docs/SIM7080_MQTT.md` - Basic MQTT AT command reference
- `SIM7070_SIM7080_SIM7090 Series_MQTT(S)_Application Note_V1.03.pdf`
- `SIM7070_SIM7080_SIM7090 Series_AT Command Manual_V1.05.pdf` (Chapter 17)

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-02-10 | 0.1 | Initial draft |
