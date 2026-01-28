# VW e-Golf Comfort CAN Bus - Physical Layer & Overview

This document covers the physical layer and CAN bus basics for the VW e-Golf (MY2018, MQB platform) comfort CAN bus.

**Related Documentation:**
- [BAP_PROTOCOL.md](BAP_PROTOCOL.md) - BAP protocol specification (message format, arrays, etc.)
- [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) - Battery Control channel (charging, climate)
- [CAN_IDS.md](CAN_IDS.md) - All CAN IDs and BAP channels
- [README.md](README.md) - Overview and quick start

---

## Table of Contents

1. [Physical Layer](#physical-layer)
2. [CAN Bus Basics](#can-bus-basics)
3. [Communication Types](#communication-types)
4. [Implementation Guide](#implementation-guide)

---

## Physical Layer

### Comfort CAN Bus Specifications

| Parameter | Value |
|-----------|-------|
| Bus Speed | 500 Kbps |
| CAN Standard | CAN 2.0B (supports extended 29-bit IDs) |
| CAN-HIGH Wire | Orange with green stripes |
| CAN-LOW Wire | Orange with brown stripes |
| Termination | Bus is already terminated by existing modules |

### Connection Point

The recommended connection point is the OCU (Online Connectivity Unit) connector located behind the speedometer cluster. The OCU is the original telematics unit that communicates on the comfort CAN bus.

### Why Comfort CAN Bus (Not OBD-II)

The OBD-II port on MQB vehicles connects to the gateway module, which only forwards messages when the ignition is ON. The comfort CAN bus allows direct communication even when the car is in sleep mode, enabling:
- Remote climate control
- Remote charging control
- Wake-up from sleep
- Status monitoring while parked

---

## CAN Bus Basics

### Message Format

This bus uses both standard (11-bit) and extended (29-bit) CAN IDs:

```
Standard CAN Frame:
+--------+------+--------+
| ID     | DLC  | Data   |
| 11-bit | 4-bit| 0-8 B  |
+--------+------+--------+

Extended CAN Frame:
+--------+------+--------+
| ID     | DLC  | Data   |
| 29-bit | 4-bit| 0-8 B  |
+--------+------+--------+
```

### Byte Order

- Multi-byte integers are generally **little-endian** (LSB first)
- Exception: BAP header fields use bit-level packing (see [BAP_PROTOCOL.md](BAP_PROTOCOL.md))

---

## Communication Types

There are two main types of communication on this CAN bus:

### 1. Broadcast Messages (Data Channels)

One-way communication where modules publish data that other modules can receive.

- Uses standard 11-bit CAN IDs
- No acknowledgment or request-response
- Multiple listeners, single publisher per ID
- Examples: vehicle speed, temperature sensors, lock status

See [CAN_IDS.md](CAN_IDS.md#broadcast-messages-non-bap) for known broadcast IDs.

### 2. BAP Protocol (Two-Way Communication)

VW's proprietary protocol for structured request-response communication.

- Uses extended 29-bit CAN IDs
- Request-response pattern between ASG (client) and FSG (server)
- Supports properties, arrays, and multi-frame messages
- Examples: climate control, charging control, configuration

See [BAP_PROTOCOL.md](BAP_PROTOCOL.md) for the full BAP specification.

**Quick BAP Summary:**
```
┌─────────────────────────────────────────────────────────┐
│  ASG (Client)              FSG (Server)                 │
│       │                         │                       │
│       │──── Commands ──────────>│                       │
│       │<─── Responses ──────────│                       │
└─────────────────────────────────────────────────────────┘
```

---

## Implementation Guide

### Recommended Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│  (Climate Control, Charge Control, Status Monitoring)   │
├─────────────────────────────────────────────────────────┤
│                    BAP Protocol Layer                    │
│  (Message encoding/decoding, multi-frame handling)      │
├─────────────────────────────────────────────────────────┤
│                    CAN Driver Layer                      │
│  (Send/receive CAN frames, filtering)                   │
├─────────────────────────────────────────────────────────┤
│                    Hardware Layer                        │
│  (CAN transceiver, microcontroller)                     │
└─────────────────────────────────────────────────────────┘
```

### CAN Filter Configuration

For efficient operation, configure CAN filters for:

```
Receive filters (BAP channels):
- 0x17332510 (Battery Control FSG responses)
- 0x17330D10 (Door Locking responses)

Broadcast data:
- 0x00000583, 0x00000585 (lock status, etc.)
- 0x000005A7 (keep-alive detection)
```

### Hardware Recommendations

- Use a CAN transceiver rated for 500 Kbps
- Consider galvanic isolation for safety
- Ensure proper ground connection
- Do NOT add termination resistors (bus is already terminated)

### Testing Setup

1. Use a CAN interface that supports both standard and extended frames
2. Log all traffic before sending any commands
3. Start with passive monitoring to understand message patterns
4. Test in a safe environment (car parked, ignition off)

---

## Document History

- Initial version based on reverse engineering of MY2018 VW e-Golf
- Protocol applicable to MQB platform vehicles

---

## See Also

- [BAP_PROTOCOL.md](BAP_PROTOCOL.md) - BAP protocol specification
- [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) - Battery Control channel
- [CAN_IDS.md](CAN_IDS.md) - Complete CAN ID reference
- [CORE_EV_SIGNALS.md](CORE_EV_SIGNALS.md) - EV-specific signal definitions
