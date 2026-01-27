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

#### Start Frame (First CAN frame)

```
Byte 0: [1][0][MessageIndex:6]
         │  │  └── Index for tracking multi-message transfers
         │  └── Frame type: 0 = Start
         └── Message type: 1 = Long

Byte 1: Total payload length (including header)

Byte 2: [0][OpCode:3][DeviceID_hi:4]  <- Same as short message header
Byte 3: [DeviceID_lo:2][FunctionID:6] <- Same as short message header

Bytes 4-7: First 4 bytes of payload
```

#### Continuation Frame (Subsequent CAN frames)

```
Byte 0: [1][1][MessageIndex:6]
         │  │  └── Must match start frame's index
         │  └── Frame type: 1 = Continuation
         └── Message type: 1 = Long

Bytes 1-7: Next 7 bytes of payload
```

**Encoding Example - Configure Profile:**
```
Device ID: 0x25
Function ID: 0x19 (ProfilesArray)
OpCode: 0x02 (SetGet)
Payload: [0x22, 0x06, 0x00, 0x01, 0x06, 0x00, 0x20, 0x00] (8 bytes)

Frame 1 (Start):
  Byte 0: [1][0][000000] = 0x80
  Byte 1: 0x08 (8 bytes total payload after header)
  Byte 2: [0][010][1001] = 0x29
  Byte 3: [01][011001] = 0x59
  Bytes 4-7: 0x22, 0x06, 0x00, 0x01
  
  Result: 80 08 29 59 22 06 00 01

Frame 2 (Continuation):
  Byte 0: [1][1][000000] = 0xC0
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

### ChargeState (Function 0x11)

```
Byte 0: [ChargeMode:4][ChargeState:4]
Byte 1: currentChargeLevel (0-100%)
Byte 2: remainingChargeTime (minutes)
Byte 3: currentChargeRange
Byte 4: unitRange
Byte 5: current (charging current)
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
