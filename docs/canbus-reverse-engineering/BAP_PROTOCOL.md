# BAP Protocol Specification

**BAP** (Bedien- und Anzeigeprotokoll / Control and Display Protocol) is Volkswagen's proprietary two-way communication protocol implemented over CAN. It enables structured request-response communication between control units in the vehicle.

This document describes the BAP protocol itself. For specific BAP channels and their functions, see:
- [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) - Battery Control channel (charging, climate)
- [CAN_IDS.md](CAN_IDS.md) - List of all BAP channels and CAN IDs

---

## Table of Contents

1. [Overview](#overview)
2. [Terminology](#terminology)
3. [Channel Architecture](#channel-architecture)
4. [Operation Codes](#operation-codes)
5. [Message Format](#message-format)
   - [Short Messages](#short-messages)
   - [Long Messages](#long-messages)
6. [BAP Arrays](#bap-arrays)
7. [Channel Initialization](#channel-initialization)
8. [Implementation Guide](#implementation-guide)

---

## Overview

BAP provides a structured application-layer protocol on top of CAN, enabling:

- **Request-response communication** between control units
- **Property get/set operations** on device functions
- **Array data structures** for lists and tables
- **Multi-frame messages** for payloads larger than 6 bytes
- **Concurrent message streams** via message groups

Unlike raw CAN broadcast messages, BAP creates logical "channels" where clients (ASG) can query or control servers (FSG).

---

## Terminology

| Term | German | English | Description |
|------|--------|---------|-------------|
| **FSG** | Funktionssteuergerät | Functional Control Unit | The server in a BAP channel. Provides data and accepts commands. |
| **ASG** | Anzeigesteuergerät | Display Control Unit | The client in a BAP channel. Requests data and sends commands. |
| **LSG** | Logisches Steuergerät | Logical Control Unit | A logical device identifier. One physical module can host multiple LSGs. |
| **BAP-Config** | - | BAP Configuration | Standard function (ID 0x02) for protocol version negotiation |
| **OpCode** | - | Operation Code | Defines the type of operation (Get, Set, Status, etc.) |

---

## Channel Architecture

BAP uses dedicated CAN IDs to create isolated communication channels. Each channel connects one or more ASGs (clients) to one FSG (server).

```
BAP Channel Structure:
  ┌─────────────────────────────────────────────────────────┐
  │                                                         │
  │  ASG 1 (Client)  ──── CAN ID A ────┐                   │
  │                                     ├──>  FSG (Server)  │
  │  ASG 2 (Client)  ──── CAN ID B ────┘        │          │
  │                                             │          │
  │                     <──── CAN ID C ─────────┘          │
  │                      (shared response ID)              │
  │                                                         │
  └─────────────────────────────────────────────────────────┘
```

**Key characteristics:**
- Each ASG has its own CAN ID for sending to the FSG
- The FSG typically uses one CAN ID to respond to all ASGs
- ASGs cannot communicate with each other directly on a BAP channel
- Messages are identified by the LSG ID and Function ID in the BAP header

**Example - Battery Control Channel:**
```
ASG (0x17332501) ────> FSG (Battery Control Unit)
                 <──── FSG (0x17332510)
```

---

## Operation Codes

| OpCode | Value | Name | Direction | Description |
|--------|-------|------|-----------|-------------|
| Reset | 0x00 | Reset | FSG->ASG | Initialize/reset communication, version info |
| Get | 0x01 | Get | ASG->FSG | Request data from FSG |
| SetGet | 0x02 | SetGet | ASG->FSG | Set value and request confirmation |
| HeartbeatStatus | 0x03 | HeartbeatStatus | FSG->ASG | Periodic status update |
| Status | 0x04 | Status | FSG->ASG | Status response to Get/SetGet |
| StatusAck | 0x05 | StatusAck | ASG->FSG | Acknowledge status received |
| Ack | 0x06 | Ack | FSG->ASG | Simple acknowledgment |
| Error | 0x07 | Error | FSG->ASG | Error response |

**Common patterns:**
- ASG sends `Get` -> FSG responds with `Status`
- ASG sends `SetGet` -> FSG responds with `Status` or `Ack`
- FSG sends `HeartbeatStatus` periodically (if configured)

---

## Message Format

BAP messages come in two formats based on payload size:

| Type | Payload Size | CAN Frames |
|------|--------------|------------|
| Short | 0-6 bytes | 1 frame |
| Long | 7+ bytes | Multiple frames |

### Short Messages

A short BAP message fits within a single 8-byte CAN frame.

```
CAN Frame (8 bytes):
┌─────────────────────────────────────────────────────────┐
│ Byte 0   │ Byte 1   │ Bytes 2-7                        │
│ Header 0 │ Header 1 │ Payload (6 bytes)                │
└─────────────────────────────────────────────────────────┘

Header Byte 0:
  Bit 7:     Message type (0 = Short)
  Bits 6-4:  OpCode (3 bits)
  Bits 3-0:  LSG ID upper 4 bits

Header Byte 1:
  Bits 7-6:  LSG ID lower 2 bits
  Bits 5-0:  Function ID (6 bits)
```

**Bit layout visualization:**
```
Byte 0: [0][OpCode:3][LSG_hi:4]
         │  │         └── Upper 4 bits of 6-bit LSG ID
         │  └── Operation code (3 bits, 0-7)
         └── Short message flag (always 0)

Byte 1: [LSG_lo:2][FunctionID:6]
         │         └── Function ID (6 bits, 0-63)
         └── Lower 2 bits of LSG ID
```

**Encoding/Decoding:**

```python
def encode_bap_header(opcode: int, lsg_id: int, function_id: int) -> tuple[int, int]:
    """Encode BAP header from components."""
    byte0 = ((opcode & 0x07) << 4) | ((lsg_id >> 2) & 0x0F)
    byte1 = ((lsg_id & 0x03) << 6) | (function_id & 0x3F)
    return byte0, byte1

def decode_bap_header(byte0: int, byte1: int) -> tuple[int, int, int]:
    """Decode BAP header to components."""
    opcode = (byte0 >> 4) & 0x07
    lsg_id = ((byte0 & 0x0F) << 2) | ((byte1 >> 6) & 0x03)
    function_id = byte1 & 0x3F
    return opcode, lsg_id, function_id
```

**Example - GET request for PlugState:**
```
LSG ID: 0x25 (37, binary: 100101)
Function ID: 0x10 (16, binary: 010000)
OpCode: 0x01 (Get)

Byte 0: [0][001][1001] = 0x19
Byte 1: [01][010000] = 0x50

Full message: 19 50 00 00 00 00 00 00
```

### Long Messages

Long messages span multiple CAN frames when the payload exceeds 6 bytes. BAP uses a **group** mechanism to allow up to 4 concurrent message streams per CAN ID.

#### Control Byte (Byte 0)

Every frame in a long message starts with a control byte:

```
Control Byte: [Long:1][Cont:1][Group:2][Index:4]
               Bit 7   Bit 6   Bits 5-4  Bits 3-0

Long (Bit 7):   1 = Long message
Cont (Bit 6):   0 = Start frame, 1 = Continuation frame
Group (Bits 5-4): Message group (0-3)
Index (Bits 3-0): Frame sequence index (0-15, wraps)
```

**Group values and byte ranges:**

| Group | Start Byte | Continuation Range |
|-------|------------|-------------------|
| 0 | 0x80 | 0xC0 - 0xCF |
| 1 | 0x90 | 0xD0 - 0xDF |
| 2 | 0xA0 | 0xE0 - 0xEF |
| 3 | 0xB0 | 0xF0 - 0xFF |

#### Start Frame

The first frame of a long message:

```
Byte 0: Control byte [1][0][Group:2][0000]
        - Long=1, Cont=0 (start), Index always 0
Byte 1: Total payload length (not including BAP header)
Byte 2: BAP Header byte 0 (same format as short messages)
Byte 3: BAP Header byte 1
Bytes 4-7: First 4 bytes of payload
```

#### Continuation Frame

Subsequent frames:

```
Byte 0: Control byte [1][1][Group:2][Index:4]
        - Long=1, Cont=1 (continuation)
Bytes 1-7: Next 7 bytes of payload
```

#### Index Sequencing (IMPORTANT)

The index field behavior has some non-obvious characteristics:

1. **Start frames always have Index = 0** (observed: only 0x80, 0x90, 0xA0, 0xB0)
2. **First continuation frame also has Index = 0** (same as start!)
3. **Index increments with each subsequent continuation**
4. **Index wraps from 15 back to 0** (stays within same group)

**Example sequence for Group 0, 20 continuation frames:**
```
Frame 1: 80 (START, group=0, index=0)
Frame 2: C0 (CONT,  group=0, index=0)  <- First cont has SAME index
Frame 3: C1 (CONT,  group=0, index=1)
Frame 4: C2 (CONT,  group=0, index=2)
...
Frame 17: CF (CONT, group=0, index=15)
Frame 18: C0 (CONT, group=0, index=0)  <- Wraps back to 0
Frame 19: C1 (CONT, group=0, index=1)
Frame 20: C2 (CONT, group=0, index=2)
Frame 21: C3 (CONT, group=0, index=3)
```

#### Concurrent Message Streams

The group field allows a single FSG to interleave frames from up to 4 different messages:

```
CAN ID 0x17332510 (BatteryControl FSG) traffic:
  80 72 49 59 ...  <- Group 0 START: ProfilesArray
  90 1F 49 5A ...  <- Group 1 START: PowerProvidersArray  
  C0 ...           <- Group 0 CONT: ProfilesArray data
  D0 ...           <- Group 1 CONT: PowerProvidersArray data
  A0 09 49 51 ...  <- Group 2 START: ChargeState
  C1 ...           <- Group 0 CONT: ProfilesArray data
  E0 ...           <- Group 2 CONT: ChargeState data
  D1 ...           <- Group 1 CONT: PowerProvidersArray data
```

#### Message Assembly Algorithm

```python
# Assembly buffers keyed by (can_id, group)
assembly_buffers = {}

def process_frame(can_id: int, data: bytes):
    """Process a CAN frame and return complete BAP message if ready."""
    
    is_long = bool(data[0] & 0x80)
    if not is_long:
        return decode_short_message(data)
    
    is_cont = bool(data[0] & 0x40)
    group = (data[0] >> 4) & 0x03
    index = data[0] & 0x0F
    
    key = (can_id, group)
    
    if not is_cont:
        # START frame
        total_length = data[1]
        opcode, lsg_id, function_id = decode_bap_header(data[2], data[3])
        
        assembly_buffers[key] = {
            'total_length': total_length,
            'opcode': opcode,
            'lsg_id': lsg_id,
            'function_id': function_id,
            'payload': list(data[4:]),  # First 4 bytes
            'next_index': 0,  # First continuation will have index 0
        }
    else:
        # CONTINUATION frame
        if key not in assembly_buffers:
            return None  # Orphaned continuation, ignore
            
        buf = assembly_buffers[key]
        
        # Verify sequence (handles wrap at 16)
        if index != buf['next_index']:
            # Sequence error - could drop or attempt recovery
            del assembly_buffers[key]
            return None
        
        buf['payload'].extend(data[1:])
        buf['next_index'] = (index + 1) & 0x0F
        
        # Check if complete
        if len(buf['payload']) >= buf['total_length']:
            complete = assembly_buffers.pop(key)
            return {
                'opcode': complete['opcode'],
                'lsg_id': complete['lsg_id'],
                'function_id': complete['function_id'],
                'data': complete['payload'][:complete['total_length']]
            }
    
    return None  # Message not yet complete
```

**Example - Long message encoding:**
```
LSG ID: 0x25, Function: 0x19 (ProfilesArray), OpCode: 0x02 (SetGet)
Payload: [0x22, 0x06, 0x00, 0x01, 0x06, 0x00, 0x20, 0x00] (8 bytes)

Frame 1 (Start, Group 0):
  Byte 0: 0x80 [1][0][00][0000]
  Byte 1: 0x08 (payload length)
  Byte 2: 0x29 [0][010][1001] (OpCode=2, LSG upper)
  Byte 3: 0x59 [01][011001] (LSG lower, Func=0x19)
  Bytes 4-7: 0x22, 0x06, 0x00, 0x01
  
  Result: 80 08 29 59 22 06 00 01

Frame 2 (Continuation, Group 0, Index 0):
  Byte 0: 0xC0 [1][1][00][0000]
  Bytes 1-4: 0x06, 0x00, 0x20, 0x00
  
  Result: C0 06 00 20 00
```

---

## BAP Arrays

BAP includes a standardized array data type for transmitting lists and tables. Arrays are used for functions that return multiple records (e.g., profiles, power providers, destinations).

### Array Header Structure

Arrays have a header that describes the array layout and operation:

**GET Array Header (ASG -> FSG request):**
```
Byte 0: [ASG-ID:4][Transaction-ID:4]
Byte 1: [LargeIdx:1][PosTransmit:1][Backward:1][Shift:1][RecordAddr:4]
Byte 2: startIndex (or bytes 2-3 if LargeIdx=1, little-endian)
Byte 3: elementCount (or bytes 4-5 if LargeIdx=1)
```

**STATUS Array Header (FSG -> ASG response):**
```
Byte 0: [ASG-ID:4][Transaction-ID:4]
Byte 1: totalElementsInList
Byte 2: [LargeIdx:1][PosTransmit:1][Backward:1][Shift:1][RecordAddr:4]
Byte 3: startIndex (or bytes 3-4 if LargeIdx=1)
Byte 4: elementCount (or bytes 5-6 if LargeIdx=1)
```

### Header Fields

| Field | Bits | Description |
|-------|------|-------------|
| **ASG-ID** | 4 | Identifies requesting ASG (for multi-ASG channels) |
| **Transaction-ID** | 4 | Transaction identifier, echoed in response |
| **LargeIdx** | 1 | 0 = 8-bit indexes, 1 = 16-bit indexes (little-endian) |
| **PosTransmit** | 1 | 1 = Array position included with each element |
| **Backward** | 1 | 1 = Array direction is backward |
| **Shift** | 1 | 1 = Shift array position |
| **RecordAddr** | 4 | Record format selector (function-specific) |
| **startIndex** | 8/16 | First element index to return |
| **elementCount** | 8/16 | Number of elements requested/returned |
| **totalElementsInList** | 8 | (STATUS only) Total elements in the array |

### Record Address

The RecordAddr field selects between different record formats for the same function. This allows:
- Full records vs. compact records
- Different views of the same data
- Partial updates

Example (BatteryControl ProfilesArray, Function 0x19):
- RecordAddr 0: Full profile (20+ bytes per record)
- RecordAddr 6: Compact profile (4 bytes per record)

### Array Data Layout

When `PosTransmit = 0`:
```
[Array Header][Element 0][Element 1][Element 2]...
```

When `PosTransmit = 1`:
```
[Array Header][Pos0][Element 0][Pos1][Element 1][Pos2][Element 2]...
```

Position values are 8 or 16 bits depending on LargeIdx.

### Example - Reading Profiles

**Request (GET, Function 0x19):**
```
BAP Header: 19 59 (OpCode=GET, LSG=0x25, Func=0x19)
No additional payload needed for "get all"
```

**Response (STATUS, Function 0x19):**
```
Long message with:
  Array Header:
    Byte 0: 0x00 (ASG-ID=0, Transaction=0)
    Byte 1: 0x04 (totalElementsInList = 4)
    Byte 2: 0x40 (LargeIdx=0, PosTransmit=1, Backward=0, Shift=0, RecordAddr=0)
    Byte 3: 0x00 (startIndex = 0)
    Byte 4: 0x04 (elementCount = 4)
  
  Data:
    [Position 0][Profile 0 data...]
    [Position 1][Profile 1 data...]
    [Position 2][Profile 2 data...]
    [Position 3][Profile 3 data...]
```

---

## Channel Initialization

When a BAP channel is first used (after wake-up or power-on), the ASG typically initializes the channel with a configuration handshake:

```
1. ASG sends: GET request for BAP-Config (Function 0x02)
   Message: [OpCode=GET][LSG][0x02]
   
2. FSG responds: RESET with protocol version and capabilities
   Message: [OpCode=RESET][LSG][0x02][version][capabilities...]
```

**BAP-Config Response Structure:**
```
Byte 0: BAP major version
Byte 1: BAP minor version  
Byte 2: [HeartbeatSupported:1][ArraysSupported:1][...]
...additional capability flags
```

This handshake is observed in all ASG implementations but may not be strictly required for basic operations. It's recommended to perform initialization for full compatibility.

---

## Implementation Guide

### Recommended Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│  (Climate Control, Charge Control, Status Monitoring)   │
├─────────────────────────────────────────────────────────┤
│                    BAP Protocol Layer                    │
│  (Message assembly, header encoding, arrays)            │
├─────────────────────────────────────────────────────────┤
│                    CAN Driver Layer                      │
│  (Send/receive CAN frames, filtering)                   │
├─────────────────────────────────────────────────────────┤
│                    Hardware Layer                        │
│  (CAN transceiver, microcontroller)                     │
└─────────────────────────────────────────────────────────┘
```

### Key Considerations

1. **Message Assembly Buffers**: Use `(can_id, group)` as the key for assembly buffers, not just CAN ID. There can be up to 4 concurrent streams per CAN ID.

2. **Index Wrapping**: The 4-bit index wraps from 15 to 0. Implementations must handle this correctly for messages longer than ~120 bytes.

3. **Timeout Handling**: If a long message is not completed within a reasonable time (~1 second), discard the partial buffer.

4. **Frame Timing**: When sending multi-frame messages, allow 10-50ms between frames to avoid overwhelming the receiver.

5. **Error Responses**: Always check for OpCode 0x07 (Error) in responses. Error payloads contain function-specific error codes.

6. **Concurrent Requests**: Avoid sending multiple requests to the same FSG simultaneously unless using different groups.

### Standard Functions

Most BAP devices implement these standard functions:

| Function ID | Name | Description |
|-------------|------|-------------|
| 0x01 | GetAllProperties | Request all properties at once |
| 0x02 | BAP-Config | Protocol configuration/version |
| 0x03 | FunctionList | List of supported functions |
| 0x04 | HeartbeatConfig | Configure periodic status updates |
| 0x0E | FSG-Setup | FSG configuration |
| 0x0F | FSG-OperationState | FSG operation state |

Device-specific functions start at 0x10 and above.

---

## References

- Original BAP specification (VAG internal)
- [BAP reverse engineering (volkswagen PQ)](https://blog.dietmann.org/?p=324) by Karl Dietmann
- [vw-bap-analyser](./vw-bap-analyser/) reference implementation

---

## See Also

- [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) - Battery Control channel documentation
- [CAN_IDS.md](CAN_IDS.md) - BAP channel CAN IDs
- [PROTOCOL.md](PROTOCOL.md) - Physical layer and CAN bus details
