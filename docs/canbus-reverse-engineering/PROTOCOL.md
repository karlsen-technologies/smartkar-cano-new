# VW e-Golf Comfort CAN Bus Protocol Documentation

This document provides comprehensive technical documentation for communicating with the VW e-Golf (MY2018, MQB platform) via the comfort CAN bus. It is intended for developers implementing devices that need to read vehicle data and send commands.

## Table of Contents

1. [Physical Layer](#physical-layer)
2. [CAN Bus Basics](#can-bus-basics)
3. [Known CAN IDs](#known-can-ids)
4. [BAP Protocol](#bap-protocol)
5. [Battery Control Unit](#battery-control-unit)
6. [Command Reference](#command-reference)
7. [Data Structures](#data-structures)
8. [Implementation Guide](#implementation-guide)

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
- Exception: BAP header fields use bit-level packing (see BAP Protocol section)

---

## Known CAN IDs

### BAP Channel CAN IDs

BAP channels use 29-bit extended CAN IDs with the format `0x1733XXYY` where XX is the LSG/Device ID and YY indicates message direction. See [Channel Architecture](#channel-architecture) for details.

### System/Wake Messages

| CAN ID | Extended | Direction | Purpose |
|--------|----------|-----------|---------|
| `0x17330301` | Yes | TX | Wake-up message |
| `0x000005A7` | No | TX | Keep-alive heartbeat (OCU active signal) |
| `0x1B000067` | Yes | TX | BAP channel initialization broadcast |
| `0x17F00067` | Yes | RX | BAP init response |

### Battery Control Channel (Device 0x25)

| CAN ID | Extended | Direction | Role |
|--------|----------|-----------|------|
| `0x17332501` | Yes | TX | ASG (client) to FSG (server) - Commands |
| `0x17332510` | Yes | RX | FSG (server) to ASG (client) - Responses |

### Door Locking Channel (Device 0x0D)

| CAN ID | Extended | Direction | Role |
|--------|----------|-----------|------|
| `0x17330D00` | Yes | TX | ASG to FSG - Commands |
| `0x17330D01` | Yes | RX | FSG to ASG - Responses |
| `0x17330D02` | Yes | RX | FSG to ASG - Responses (alternate) |
| `0x17330D10` | Yes | RX | FSG to ASG - Status broadcasts |

### Extended Network Interface (Device 0x37)

| CAN ID | Extended | Direction | Role |
|--------|----------|-----------|------|
| `0x17333700` | Yes | TX | ASG to FSG |
| `0x17333710` | Yes | RX | FSG to ASG |
| `0x17333711` | Yes | RX | FSG to ASG (alternate) |

### Broadcast Data Channels (Non-BAP)

These are one-way broadcast messages from various modules:

| CAN ID | Extended | Content (Suspected) |
|--------|----------|---------------------|
| `0x00000184` | No | Vehicle status |
| `0x00000366` | No | Unknown |
| `0x00000583` | No | Unknown |
| `0x00000585` | No | Unknown (changes frequently) |
| `0x000005E1` | No | Unknown |
| `0x000005F0` | No | Unknown |

---

## BAP Protocol

BAP (Bedien- und Anzeigeprotokoll / Control and Display Protocol) is VW's proprietary two-way communication protocol implemented over CAN.

### Channel Architecture

BAP uses dedicated CAN IDs to create isolated communication channels between clients (ASG) and servers (FSG). Each logical device (LSG) has its own channel with unique CAN IDs, ensuring messages don't interfere with each other.

#### CAN ID Structure

BAP CAN IDs are 29-bit extended identifiers. They generally follow a pattern but the exact IDs must be defined per channel:

```
0x1733XXYY (typical pattern, but not guaranteed)

Where:
  1733     = BAP protocol identifier (common prefix)
  XX       = LSG/Device ID (e.g., 25 = Battery Control, 0D = Door Locking)
  YY       = Channel-specific suffix
```

**Important:** The suffix values (YY) are NOT standardized. Each channel defines its own ASG and FSG CAN IDs. You must look up the specific IDs for each channel - do not assume a pattern.

#### Channel Definitions

Each BAP channel must define:
- Which CAN ID(s) the ASG (client) transmits on
- Which CAN ID(s) the FSG (server) transmits on

**Battery Control (LSG 0x25):**
| CAN ID | Direction | Role |
|--------|-----------|------|
| 0x17332501 | ASG → FSG | Commands |
| 0x17332510 | FSG → ASG | Responses/Status |

**Door Locking (LSG 0x0D):**
| CAN ID | Direction | Role |
|--------|-----------|------|
| 0x17330D00 | ASG → FSG | Commands |
| 0x17330D01 | FSG → ASG | Responses |
| 0x17330D02 | FSG → ASG | Responses (alternate) |
| 0x17330D10 | FSG → ASG | Status broadcasts |

**Extended Network Interface (LSG 0x37):**
| CAN ID | Direction | Role |
|--------|-----------|------|
| 0x17333700 | ASG → FSG | Commands |
| 0x17333710 | FSG → ASG | Responses |
| 0x17333711 | FSG → ASG | Responses (alternate) |

#### Channel Isolation

```
Battery Control Channel:
  ┌─────────────────────────────────────────────────────────┐
  │  ASG (Client)              FSG (Server)                 │
  │       │                         │                       │
  │       │──── 0x17332501 ────────>│  Commands             │
  │       │<─── 0x17332510 ─────────│  Responses/Status     │
  └─────────────────────────────────────────────────────────┘

Door Locking Channel:
  ┌─────────────────────────────────────────────────────────┐
  │  ASG (Client)              FSG (Server)                 │
  │       │                         │                       │
  │       │──── 0x17330D00 ────────>│  Commands             │
  │       │<─── 0x17330D01 ─────────│  Responses            │
  │       │<─── 0x17330D02 ─────────│  Responses (alt)      │
  │       │<─── 0x17330D10 ─────────│  Status broadcasts    │
  └─────────────────────────────────────────────────────────┘
```

#### Why This Matters

1. **CAN Filtering**: Filter on specific CAN IDs to only receive messages for channels you care about
2. **No Collision**: Multiple BAP conversations can happen simultaneously without interference
3. **Simple Routing**: The CAN ID identifies which channel the message belongs to
4. **Channel-Specific**: Always look up the exact CAN IDs for each channel - don't assume patterns

#### All Known BAP Channels

The following channels were identified from e-Golf comfort CAN traces and DBC files:

| LSG ID | Name | ASG CAN ID(s) | FSG CAN ID(s) | Description |
|--------|------|---------------|---------------|-------------|
| 0x01 | Klima1 | 0x17330100 | 0x17330110 | Climate control |
| 0x03 | ParkHeater | 0x17330301 | - | Standheizung |
| 0x07 | RDK | - | 0x17330710 | Tire pressure (Reifendruck) |
| 0x08 | ? | 0x17330800 | 0x17330810 | Unknown |
| 0x09 | Aussenlicht | 0x17330900 | 0x17330910 | Exterior lights |
| 0x0A | ? | - | 0x17330A10, 0x17330A11 | Unknown |
| 0x0C | ? | 0x17330C00 | 0x17330C10 | Unknown |
| 0x0D | Doorlocking | 0x17330D00 | 0x17330D01, 0x17330D02, 0x17330D10 | Central locking |
| 0x0E | ? | 0x17330E00 | 0x17330E10 | Unknown |
| 0x0F | BC | 0x17330F00 | 0x17330F01, 0x17330F10 | Bordcomputer |
| 0x11 | Uhrzeit | 0x17331100 | 0x17331101, 0x17331102, 0x17331110 | Clock/time |
| 0x12 | ? | 0x17331200 | 0x17331201, 0x17331210 | Unknown |
| 0x13 | ? | 0x17331300 | 0x17331310 | Unknown |
| 0x19 | ? | - | 0x17331901, 0x17331910 | Unknown |
| 0x1A | ? | - | 0x17331A01, 0x17331A10 | Unknown |
| 0x1B | ? | - | 0x17331B01, 0x17331B10 | Unknown |
| 0x1C | Hybrid | - | 0x17331C10 | Hybrid/EV status |
| 0x1F | ? | - | 0x17331F10 | Unknown |
| 0x21 | ? | - | 0x17332101, 0x17332110 | Unknown |
| 0x23 | ? | 0x17332300 | 0x17332310 | Unknown |
| 0x25 | BatteryControl | 0x17332501 | 0x17332510 | Charging, climate, SOC |
| 0x28 | Telefon | 0x17332800, 0x17332801 | 0x17332810, 0x17332811 | Phone |
| 0x29 | ? | - | 0x17332910 | Unknown |
| 0x31 | Audio | 0x17333100, 0x17333101 | 0x17333110, 0x17333111 | Audio system |
| 0x32 | Navigation | 0x17333200, 0x17333201 | 0x17333210, 0x17333211 | Navigation |
| 0x33 | eCall | 0x17333300 | 0x17333310 | Emergency call |
| 0x37 | ENI | 0x17333700 | 0x17333710, 0x17333711 | Extended Network Interface |
| 0x3C | EfficiencyAssist | - | 0x17333C01, 0x17333C10 | Efficiency assistant |
| 0x46 | ? | 0x17334600 | 0x17334610 | Unknown |
| 0x67 | BAP Init | - | - | Channel initialization |

**Notes:**
- ASG = client (sends commands), FSG = server (sends responses)
- Some channels have multiple FSG CAN IDs for different response types
- CAN IDs with suffix 0x00-0x0F are typically ASG, 0x10+ are typically FSG
- "?" indicates channel seen in traces but name unknown

#### Key Channels for OCU Replacement

| LSG | Name | Purpose | Priority |
|-----|------|---------|----------|
| 0x25 | BatteryControl | SOC, charging, climate control | **Critical** |
| 0x0D | Doorlocking | Lock/unlock status | High |
| 0x37 | ENI | Trip data, vehicle info | Medium |
| 0x01 | Klima1 | Climate status | Medium |
| 0x1C | Hybrid | EV-specific status | Medium |

### Terminology

| Term | Full Name | Description |
|------|-----------|-------------|
| FSG | Funktionssteuergerät | Functional control unit (server) |
| ASG | Anzeigesteuergerät | Display control unit (client) |
| LSG | Logisches Steuergerät | Logical device within a physical module |

### Operation Codes

| OpCode | Name | Description |
|--------|------|-------------|
| 0x00 | Reset | Initialize/reset communication |
| 0x01 | Get | Request data from FSG |
| 0x02 | SetGet | Set value and get response |
| 0x03 | HeartbeatStatus | Periodic status update |
| 0x04 | Status | Status response |
| 0x05 | StatusAck | Acknowledge status received |
| 0x06 | Ack | Simple acknowledgment |
| 0x07 | Error | Error response |

### Message Types

BAP messages come in two formats based on payload size:

1. **Short Messages**: Payload ≤ 6 bytes, fits in single CAN frame
2. **Long Messages**: Payload > 6 bytes, spans multiple CAN frames

### Short BAP Message Format

Total: 8 bytes (2-byte header + 6-byte payload)

```
Byte 0: [0][OpCode:3][DeviceID_hi:4]
         │  │         └── Upper 4 bits of 6-bit device ID
         │  └── Operation code (3 bits)
         └── Message type: 0 = Short

Byte 1: [DeviceID_lo:2][FunctionID:6]
         │              └── Function ID (6 bits)
         └── Lower 2 bits of 6-bit device ID

Bytes 2-7: Payload (6 bytes)
```

**Encoding Example - Start Climate Command:**
```
Device ID: 0x25 (37 decimal, binary: 100101)
Function ID: 0x18 (24 decimal, binary: 011000)
OpCode: 0x02 (SetGet, binary: 010)
Payload: [0x00, 0x01] (immediately=true)

Byte 0: [0][010][1001] = 0x29
           │     └── DeviceID bits 5-2
           └── OpCode
           
Byte 1: [01][011000] = 0x58
         │   └── FunctionID
         └── DeviceID bits 1-0

Result: 29 58 00 01 00 00 00 00
```

### Long BAP Message Format

Long messages span multiple CAN frames. A **group** field in the header allows up to 4 concurrent message streams per CAN ID.

#### Byte 0 Format (Control Byte)

The first byte of every long BAP frame has this structure:

```
Byte 0: [Long:1][Cont:1][Group:2][Index:4]
         Bit 7   Bit 6   Bits 5-4  Bits 3-0

Long (Bit 7):  1 = Long message, 0 = Short message
Cont (Bit 6):  0 = Start frame, 1 = Continuation frame
Group (Bits 5-4): Message group (0-3), allows 4 concurrent streams
Index (Bits 3-0): Sequence counter (0-15), wraps to 0 after 15
```

**Group Values:**

| Group | Start Byte0 | Continuation Range |
|-------|-------------|-------------------|
| 0     | 0x80        | 0xC0 - 0xCF       |
| 1     | 0x90        | 0xD0 - 0xDF       |
| 2     | 0xA0        | 0xE0 - 0xEF       |
| 3     | 0xB0        | 0xF0 - 0xFF       |

**Key Facts (verified from trace analysis):**
- START frames always have Index = 0 (only 0x80, 0x90, 0xA0, 0xB0 observed)
- First continuation frame also has Index = 0 (same as start)
- Index increments with each subsequent continuation
- Index wraps from 15 back to 0 (stays within same group)
- Maximum 4 concurrent messages per CAN ID (one per group)

#### Start Frame (First CAN frame)

```
Byte 0: [1][0][Group:2][0000]
         │  │  │        └── Index always 0 for start frames
         │  │  └── Group (0-3)
         │  └── Frame type: 0 = Start
         └── Message type: 1 = Long

Byte 1: Total payload length (bytes after BAP header, not including header)

Byte 2: [0][OpCode:3][DeviceID_hi:4]  <- Same as short message header
Byte 3: [DeviceID_lo:2][FunctionID:6] <- Same as short message header

Bytes 4-7: First 4 bytes of payload
```

#### Continuation Frame (Subsequent CAN frames)

```
Byte 0: [1][1][Group:2][Index:4]
         │  │  │        └── Sequence counter (0-15, wraps)
         │  │  └── Group (must match start frame)
         │  └── Frame type: 1 = Continuation
         └── Message type: 1 = Long

Bytes 1-7: Next 7 bytes of payload
```

#### Continuation Sequence Example

For a message in Group 0 requiring 20 continuation frames:

```
START:   80  (group=0, index=0)
CONT 1:  C0  (group=0, index=0)  <- First cont has SAME index as start
CONT 2:  C1  (group=0, index=1)
CONT 3:  C2  (group=0, index=2)
...
CONT 16: CF  (group=0, index=15)
CONT 17: C0  (group=0, index=0)  <- Wraps back to 0, stays in group!
CONT 18: C1  (group=0, index=1)
CONT 19: C2  (group=0, index=2)
CONT 20: C3  (group=0, index=3)
```

#### Concurrent Messages Example

A single FSG can interleave frames from multiple messages using different groups:

```
CAN ID 0x17332510 (BatteryControl FSG):
  80 72 49 59 ...  <- Group 0 START: ProfilesArray (large message)
  90 1F 49 5A ...  <- Group 1 START: PowerProvidersArray (interleaved)
  C0 ...           <- Group 0 CONT: ProfilesArray continuation
  D0 ...           <- Group 1 CONT: PowerProvidersArray continuation
  A0 09 49 51 ...  <- Group 2 START: ChargeState (short, interleaved)
  C1 ...           <- Group 0 CONT: ProfilesArray continuation
  E0 ...           <- Group 2 CONT: ChargeState continuation
  D1 ...           <- Group 1 CONT: PowerProvidersArray continuation
  ...
```

#### Frame Assembly Algorithm

```python
# Assembling long BAP messages with group support
# Key: (can_id, group) - only one message per group at a time
assembly_buffers = {}

def process_frame(can_id, data):
    is_long = bool(data[0] & 0x80)
    if not is_long:
        return decode_short_message(data)
    
    is_cont = bool(data[0] & 0x40)
    group = (data[0] >> 4) & 0x03
    index = data[0] & 0x0F
    
    key = (can_id, group)
    
    if not is_cont:
        # START frame - index should always be 0
        assembly_buffers[key] = {
            'expected_length': data[1],
            'payload': list(data[4:]),  # First 4 payload bytes
            'next_index': 0,            # First cont will have index 0
        }
    else:
        # CONTINUATION frame
        if key in assembly_buffers:
            buf = assembly_buffers[key]
            
            # Verify index matches expected (handles wrap at 16)
            if index == buf['next_index']:
                buf['payload'].extend(data[1:])
                buf['next_index'] = (index + 1) & 0x0F  # Wrap at 16
                
                # Check if complete
                if len(buf['payload']) >= buf['expected_length']:
                    complete = assembly_buffers.pop(key)
                    return decode_long_message(data[2:4], complete['payload'])
    
    return None  # Message not yet complete

def decode_long_message(header_bytes, payload):
    """Decode completed long message."""
    opcode = (header_bytes[0] >> 4) & 0x07
    device_id = ((header_bytes[0] & 0x0F) << 2) | ((header_bytes[1] >> 6) & 0x03)
    function_id = header_bytes[1] & 0x3F
    return {
        'opcode': opcode,
        'device_id': device_id,
        'function_id': function_id,
        'payload': payload[:expected_length]  # Trim padding
    }
```

**Assembly buffer key: `(can_id, group)`** - not the full 6-bit field!

**Encoding Example - Configure Profile:**
```
Device ID: 0x25
Function ID: 0x19 (ProfilesArray)
OpCode: 0x02 (SetGet)
Payload: [0x22, 0x06, 0x00, 0x01, 0x06, 0x00, 0x20, 0x00] (8 bytes)

Frame 1 (Start, Group 0):
  Byte 0: [1][0][00][0000] = 0x80
          Long Cont Grp  Idx
  Byte 1: 0x08 (8 bytes payload length)
  Byte 2: [0][010][1001] = 0x29
  Byte 3: [01][011001] = 0x59
  Bytes 4-7: 0x22, 0x06, 0x00, 0x01
  
  Result: 80 08 29 59 22 06 00 01

Frame 2 (Continuation, Group 0, Index 0):
  Byte 0: [1][1][00][0000] = 0xC0
          Long Cont Grp  Idx
  Bytes 1-4: 0x06, 0x00, 0x20, 0x00
  
  Result: C0 06 00 20 00 (padded or truncated to actual length)
```

### BAP Header Encoding/Decoding Functions

```python
def encode_bap_header(opcode: int, device_id: int, function_id: int) -> tuple[int, int]:
    """Encode BAP header bytes from components."""
    byte0 = ((opcode & 0x07) << 4) | ((device_id >> 2) & 0x0F)
    byte1 = ((device_id & 0x03) << 6) | (function_id & 0x3F)
    return byte0, byte1

def decode_bap_header(byte0: int, byte1: int) -> tuple[int, int, int]:
    """Decode BAP header bytes to components."""
    opcode = (byte0 >> 4) & 0x07
    device_id = ((byte0 & 0x0F) << 2) | ((byte1 >> 6) & 0x03)
    function_id = byte1 & 0x3F
    return opcode, device_id, function_id
```

### Channel Initialization

Before communicating on a BAP channel, the ASG typically initializes the channel:

1. ASG sends GET request for BAP-Config (Function 0x02)
2. FSG responds with RESET containing protocol version and capabilities

This handshake is observed but may not be strictly required for all operations.

---

## Battery Control Unit

The Battery Control Unit (BCU) is accessed via BAP Device ID `0x25` on CAN IDs `0x17332501` (commands) and `0x17332510` (responses).

### Function IDs

| ID | Name | Description |
|----|------|-------------|
| 0x01 | GetAllProperties | Request all properties |
| 0x02 | BAP-Config | Protocol configuration |
| 0x03 | FunctionList | Available functions |
| 0x04 | HeartbeatConfig | Heartbeat timing config |
| 0x0E | FSG-Setup | FSG configuration |
| 0x0F | FSG-OperationState | FSG operation state |
| 0x10 | PlugState | Charging plug status |
| 0x11 | ChargeState | Charging status |
| 0x12 | ClimateState | Climate control status |
| 0x18 | ClimateOperationModeInstallation | Start/stop climate/charging |
| 0x19 | ProfilesArray | Battery Control Profiles |
| 0x1A | PowerProvidersArray | Power provider list |

### Battery Control Profiles (BCPs)

BCPs are stored profiles that define charge and climate settings. The car has 4 profile slots:

| Index | Name | Description |
|-------|------|-------------|
| 0 | Immediate/Global | Hidden profile for "now" operations and global settings |
| 1-3 | Timer 1-3 | User-configurable departure timer profiles |

Profile 0 is special - it contains global settings and is used for immediate (non-timer) operations.

---

## Command Reference

### Wake Up Sequence

To wake the car from sleep mode:

```
Step 1: Send wake-up message
  CAN ID: 0x17330301 (extended)
  Data: 40 00 01 1F

Step 2: Send keep-alive (repeat every ~500ms while active)
  CAN ID: 0x000005A7 (standard)
  Data: 00 00 00 00 00 00 00 00

Step 3: Send BAP initialization
  CAN ID: 0x1B000067 (extended)
  Data: 67 10 41 84 14 00 00 00
```

### Keep Alive

While your device is active and wants to prevent the car from sleeping:

```
CAN ID: 0x000005A7 (standard)
Data: 00 00 00 00 00 00 00 00
Interval: Every 200-500ms
```

### Configure Profile Operation Mode

Before starting climate/charging, configure what mode profile 0 should operate in:

```
CAN ID: 0x17332501 (extended)
Frame 1: 80 08 29 59 22 06 00 01
Frame 2: C0 06 00 20 00

Where byte at position "06" in continuation frame is the operation mode.
```

**Operation Mode Byte Values:**

| Value | Flags | Mode |
|-------|-------|------|
| 0x01 | charge | Charging only |
| 0x02 | climate | Climate only |
| 0x03 | charge + climate | Charging and climate |
| 0x05 | charge + climateWithoutExternalSupply | Charging, climate allowed on battery |
| 0x06 | climate + climateWithoutExternalSupply | Climate, allowed on battery |
| 0x07 | charge + climate + climateWithoutExternalSupply | Full mode |

**Operation Mode Byte Bit Mapping:**
```
Bit 0: charge
Bit 1: climate  
Bit 2: climateWithoutExternalSupply (allow battery use)
Bit 3: autoDefrost
Bit 4: seatHeaterFrontLeft
Bit 5: seatHeaterFrontRight
Bit 6: seatHeaterRearLeft
Bit 7: seatHeaterRearRight
```

### Start Climate/Charging

After configuring the profile, start the operation:

```
CAN ID: 0x17332501 (extended)
Data: 29 58 00 01 00 00 00 00

Breakdown:
  29 58 = BAP header (OpCode=SetGet, Device=0x25, Function=0x18)
  00 01 = Payload (byte 0 = 0x00, byte 1 = 0x01 for immediately=true)
```

**Payload Byte 1 Bit Mapping:**
```
Bit 0: immediately (profile 0)
Bit 1: timer1 (profile 1)
Bit 2: timer2 (profile 2)
Bit 3: timer3 (profile 3)
Bit 4: timer4 (if available)
```

### Stop Climate/Charging

```
CAN ID: 0x17332501 (extended)
Data: 29 58 00 00 00 00 00 00

Same as start, but with byte 1 = 0x00 (all timers disabled)
```

### Complete Climate Start Sequence

```python
# Full sequence to start climate from sleep:

messages = [
    # 1. Wake up
    (0x17330301, True, [0x40, 0x00, 0x01, 0x1F]),
    
    # 2. Keep-alive (send multiple times with delays)
    (0x000005A7, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]),
    
    # 3. BAP init
    (0x1B000067, True, [0x67, 0x10, 0x41, 0x84, 0x14, 0x00, 0x00, 0x00]),
    
    # 4. Request BAP config (optional handshake)
    (0x17332501, True, [0x19, 0x42]),  # GET on ProfilesArray
    
    # ... wait for car to wake, continue keep-alives ...
    
    # 5. Configure profile 0 for climate with battery allowed
    (0x17332501, True, [0x80, 0x08, 0x29, 0x59, 0x22, 0x06, 0x00, 0x01]),
    (0x17332501, True, [0xC0, 0x06, 0x00, 0x20, 0x00]),
    
    # 6. Start immediately
    (0x17332501, True, [0x29, 0x58, 0x00, 0x01]),
]
```

---

## Data Structures

### PlugState (Function 0x10)

Received on CAN ID `0x17332510` with function 0x10.

**Verified against `connect_charger_sleep.csv` trace - documentation is correct.**

```
Byte 0: [LockSetup:4][LockState:4]
Byte 1: [SupplyState:4][PlugState:4]
```

**LockSetup (upper nibble byte 0):**
| Value | Meaning |
|-------|---------|
| 0x0 | Unlock requested |
| 0x1 | Lock requested |
| 0xF | Init/Unknown |

**LockState (lower nibble byte 0):**
| Value | Meaning |
|-------|---------|
| 0x0 | Auto-lock error |
| 0x1 | Unlock error |
| 0xF | Init/Unknown |

**SupplyState (upper nibble byte 1):**
| Value | Meaning |
|-------|---------|
| 0x0 | Inactive |
| 0x1 | Active |
| 0x2 | Charge station connected |
| 0xF | Init/Unknown |

**PlugState (lower nibble byte 1):**
| Value | Meaning |
|-------|---------|
| 0x0 | Unplugged |
| 0x1 | Plugged |
| 0xF | Init/Unknown |

**Example from trace (charger connected, charging):**
```
Raw: 1F 11
  LockSetup: 1 (Lock requested)
  LockState: F (Init)
  SupplyState: 1 (Active - power flowing)
  PlugState: 1 (Plugged)
```

### ChargeState (Function 0x11)

**Verified against `connect_charger_sleep.csv` trace - documentation is correct.**

```
Byte 0: [ChargeMode:4][ChargeState:4]
Byte 1: currentChargeLevel (0-100%)
Byte 2: remainingChargeTime (minutes, 0xFF = unknown)
Byte 3: currentChargeRange
Byte 4: unitRange
Byte 5: current (charging current, 0xFF = unknown)
Byte 6: [batteryClimateState:4][reserved:4]
Byte 7: reserved
Byte 8: [startReason:4][targetSOC:4]
```

**ChargeMode (upper nibble byte 0):**
| Value | Meaning |
|-------|---------|
| 0x0 | Off |
| 0x1 | AC charging |
| 0x2 | DC charging |
| 0x3 | Conditioning |
| 0x4 | AC + Conditioning |
| 0x5 | DC + Conditioning |
| 0xF | Init/Unknown |

**ChargeState (lower nibble byte 0):**
| Value | Meaning |
|-------|---------|
| 0x0 | Init |
| 0x1 | Idle |
| 0x2 | Running |
| 0x3 | Conservation charging |
| 0x4 | Aborted - temperature too low |
| 0x5 | Aborted - general device error |
| 0x6 | Aborted - power supply not available |
| 0x7 | Aborted - not in park position |
| 0x8 | Completed |
| 0x9 | No error |

**StartReason (upper nibble byte 8):**
| Value | Meaning |
|-------|---------|
| 0x0 | Init |
| 0x1 | Timer 1 |
| 0x2 | Timer 2 |
| 0x3 | Timer 3 |
| 0x4 | Immediately |
| 0x5 | Push button |

**TargetSOC (lower nibble byte 8):**
| Value | Meaning |
|-------|---------|
| 0x0 | Min SOC |
| 0x1 | Max SOC |
| 0xF | Init |

**Example from trace (charging active):**
```
Raw: 12 2F 73 00 FF FF 00 FF 31
  ChargeMode: 1 (AC charging)
  ChargeState: 2 (Running)
  SOC: 47%
  Remaining time: 115 min
  startReason: 3 (Timer 3)
  targetSOC: 1 (Max SOC)
```

### ClimateState (Function 0x12)

```
Byte 0: climateMode (bitmask)
Byte 1: currentTemperature
Byte 2: temperatureUnit
Bytes 3-4: climatingTime (16-bit little-endian, minutes)
Byte 5: [climateState:4][reserved:4]
Byte 6: seatheaterWindowState
Byte 7: [seatheaterMode:4][windowheaterMode:4]
```

**climateMode Bitmask (byte 0):**
```
Bit 0: climating active
Bit 1: auto defrost
Bit 2: heating
Bit 3: cooling
Bit 4: ventilation
Bit 5: fuel-based heating
```

**Temperature Encoding:**
The temperature value uses the formula: `actual_temp = (raw_value + 100) / 10`

Example: raw value 120 = (120 + 100) / 10 = 22.0°C

### BatteryControlProfile (Function 0x19)

Profiles are accessed via BAP array operations. Full profile format (record address 0):

```
Byte 0: operation flags
Byte 1: operation2 flags
Byte 2: maxCurrent
Byte 3: minChargeLevel (%)
Bytes 4-5: minRange (16-bit LE)
Byte 6: targetChargeLevel (%)
Byte 7: targetChargeDuration
Bytes 8-9: targetChargeRange (16-bit LE)
Byte 10: unitRange
Byte 11: rangeCalculationSetup
Byte 12: temperature (encoded)
Byte 13: temperatureUnit
Byte 14: leadTime
Byte 15: holdingTimePlug
Byte 16: holdingTimeBattery
Bytes 17-18: providerDataId (16-bit LE)
Byte 19: nameLength
Bytes 20+: name (ASCII string)
```

Compact profile format (record address 6, used for updates):

```
Byte 0: operation flags
Byte 1: operation2 flags  
Byte 2: maxCurrent
Byte 3: targetChargeLevel
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

### Key Implementation Considerations

1. **Keep-Alive Timing**: Send heartbeat message every 200-500ms to prevent car from sleeping

2. **Wake-Up Sequence**: Wait for car to fully wake before sending commands (observe responses)

3. **Message Timing**: Allow 50-100ms between CAN frames when sending multi-frame messages

4. **Response Handling**: Monitor response CAN IDs for acknowledgments and status updates

5. **Error Handling**: Check for BAP Error responses (OpCode 0x07)

6. **State Machine**: Implement proper state tracking for:
   - Sleep/Wake state
   - BAP channel initialization state
   - Climate/Charge operation state

### CAN Filter Configuration

For efficient operation, configure CAN filters for:

```
Receive filters:
- 0x17332510 (Battery Control FSG responses)
- 0x17330D10 (Door Locking responses)
- 0x000005A7 (might see other OCU if present)

Optional broadcast data:
- 0x00000583, 0x00000585, etc.
```

### Testing Recommendations

1. **Start with read-only**: First implement status monitoring without sending commands
2. **Test wake sequence**: Verify car wakes reliably before adding commands
3. **Test climate first**: Climate start/stop is well-documented and lower risk
4. **Log everything**: Capture all CAN traffic during testing for debugging

---

## Appendix: Quick Reference

### Essential Messages

| Action | CAN ID | Extended | Data |
|--------|--------|----------|------|
| Wake up | 0x17330301 | Yes | 40 00 01 1F |
| Keep alive | 0x000005A7 | No | 00 00 00 00 00 00 00 00 |
| BAP init | 0x1B000067 | Yes | 67 10 41 84 14 00 00 00 |
| Start climate | 0x17332501 | Yes | 29 58 00 01 |
| Stop climate | 0x17332501 | Yes | 29 58 00 00 |

### BAP Header Quick Encode

For Device 0x25 (Battery Control):

| Function | Header Bytes |
|----------|--------------|
| 0x10 PlugState | 29 50 |
| 0x11 ChargeState | 29 51 |
| 0x12 ClimateState | 29 52 |
| 0x18 OperationMode | 29 58 |
| 0x19 ProfilesArray | 29 59 |

Formula: `byte0 = 0x29` (for SetGet + device 0x25), `byte1 = 0x40 + function_id`

---

## Document History

- Initial version based on reverse engineering of MY2018 VW e-Golf
- Protocol applicable to MQB platform vehicles
- Sources: CAN traces, vw-bap-analyser tool, discord documentation
