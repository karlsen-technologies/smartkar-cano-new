# Battery Control BAP Channel - VW e-Golf

The Battery Control channel (LSG 0x25) manages charging, climate control, and battery status on the VW e-Golf. It runs on the BAP protocol over the comfort CAN bus.

**CAN IDs:**
| Direction | CAN ID | Type |
|-----------|--------|------|
| ASG -> FSG (Commands) | `0x17332501` | Extended |
| FSG -> ASG (Responses) | `0x17332510` | Extended |

For BAP protocol details (message encoding, multi-frame handling, arrays), see [BAP_PROTOCOL.md](BAP_PROTOCOL.md).

---

## Table of Contents

1. [Function IDs](#function-ids)
2. [Battery Control Profiles](#battery-control-profiles)
3. [Command Reference](#command-reference)
4. [Data Structures](#data-structures)
5. [Implementation Notes](#implementation-notes)

---

## Function IDs

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

### Quick Header Reference

For LSG 0x25 (Battery Control) with OpCode SetGet (0x02):

| Function | Header Bytes | Calculation |
|----------|--------------|-------------|
| 0x10 PlugState | `29 50` | |
| 0x11 ChargeState | `29 51` | |
| 0x12 ClimateState | `29 52` | |
| 0x18 OperationMode | `29 58` | |
| 0x19 ProfilesArray | `29 59` | |

See [BAP_PROTOCOL.md](BAP_PROTOCOL.md#short-messages) for header encoding formula.

---

## Battery Control Profiles

Battery Control Profiles (BCPs) are the core mechanism for controlling charging and climate operations. They are stored in the Battery Control Unit and accessed via Function ID `0x19` (ProfilesArray).

### What Are BCPs?

BCPs are what the infotainment system and VW app call "Charge Locations". Each profile stores settings for:

- **Charging**: Target SOC, maximum current, minimum charge level
- **Climate**: Target temperature, whether to pre-heat/cool, seat heater settings
- **Timing**: Lead time before departure, hold times for climate

### Profile Slots

The car has 4 profile slots:

| Index | Name | UI Visibility | Description |
|-------|------|---------------|-------------|
| 0 | Immediate/Global | Hidden | Special profile for "now" operations and global settings |
| 1 | Timer 1 | Visible | First user-configurable departure timer |
| 2 | Timer 2 | Visible | Second user-configurable departure timer |
| 3 | Timer 3 | Visible | Third user-configurable departure timer |

**Profile 0 is special:**
- Not shown as a "Charge Location" in the infotainment
- Contains global settings (max charge limit, allow battery for climate, etc.)
- Used when you trigger "Start climate now" or "Start charging now"
- The OCU always updates profile 0 before executing immediate operations

### Profile Data Structures

Profiles are transmitted as BAP arrays (see [BAP_PROTOCOL.md](BAP_PROTOCOL.md#bap-arrays) for array header format).

**Full Profile (RecordAddr = 0)** - 20+ bytes, used when reading profiles:

| Byte | Field | Description |
|------|-------|-------------|
| 0 | operation | Operation flags (see bitmask below) |
| 1 | operation2 | Additional operation flags |
| 2 | maxCurrent | Maximum charging current (Amps) |
| 3 | minChargeLevel | Minimum SOC % (car charges to this immediately) |
| 4-5 | minRange | Minimum range (16-bit LE) |
| 6 | targetChargeLevel | Target SOC % |
| 7 | targetChargeDuration | Target charge duration |
| 8-9 | targetChargeRange | Target range (16-bit LE) |
| 10 | unitRange | Range unit (0=km, 1=miles) |
| 11 | rangeCalculationSetup | Bit 0: range calculation enabled |
| 12 | temperature | Encoded: actual = (value + 100) / 10 °C |
| 13 | temperatureUnit | 0=Celsius, 1=Fahrenheit |
| 14 | leadTime | Minutes before departure to start |
| 15 | holdingTimePlug | Climate duration when plugged in (minutes) |
| 16 | holdingTimeBattery | Climate duration on battery (minutes) |
| 17-18 | providerDataId | Power provider reference (16-bit LE) |
| 19 | nameLength | Length of profile name |
| 20+ | name | ASCII profile name |

**Compact Profile (RecordAddr = 6)** - 4 bytes, used for partial updates:

| Byte | Field | Description |
|------|-------|-------------|
| 0 | operation | Operation flags |
| 1 | operation2 | Additional operation flags |
| 2 | maxCurrent | Maximum charging current |
| 3 | targetChargeLevel | Target SOC % |

### Operation Flags (Byte 0)

```
Bit 0: charge                      - Enable charging
Bit 1: climate                     - Enable climate control
Bit 2: climateWithoutExternalSupply - Allow climate on battery (no plug required)
Bit 3: autoDefrost                 - Enable automatic defrost
Bit 4: seatHeaterFrontLeft         - Enable front left seat heater
Bit 5: seatHeaterFrontRight        - Enable front right seat heater
Bit 6: seatHeaterRearLeft          - Enable rear left seat heater
Bit 7: seatHeaterRearRight         - Enable rear right seat heater
```

**Common values:**

| Value | Binary | Meaning |
|-------|--------|---------|
| 0x01 | 00000001 | Charging only |
| 0x02 | 00000010 | Climate only (requires plug) |
| 0x03 | 00000011 | Charging + Climate |
| 0x05 | 00000101 | Charging + Climate allowed on battery |
| 0x06 | 00000110 | Climate + Climate allowed on battery |
| 0x07 | 00000111 | Charging + Climate + Climate on battery |

### Operation2 Flags (Byte 1)

```
Bit 0: windowHeaterFront   - Front windscreen heater
Bit 1: windowHeaterRear    - Rear window heater
Bit 2: parkHeater          - Auxiliary/park heater (fuel-based, if equipped)
Bit 3: parkHeaterAutomatic - Automatic park heater activation
```

### Temperature Encoding

Temperature is encoded to allow decimal precision in a single byte:

```python
# Encoding (to send to car)
def encode_temperature(celsius: float) -> int:
    return int(celsius * 10 - 100)

# Decoding (from car)
def decode_temperature(raw: int) -> float:
    return (raw + 100) / 10

# Examples:
# 22.0°C -> (22.0 * 10) - 100 = 120 -> 0x78
# 18.5°C -> (18.5 * 10) - 100 = 85  -> 0x55
# Raw 120 -> (120 + 100) / 10 = 22.0°C
```

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

Before starting climate/charging, configure what mode profile 0 should operate in.
This is a SetGet on ProfilesArray (0x19) with RecordAddr=6 (compact profile):

```
CAN ID: 0x17332501 (extended)

Frame 1 (Long BAP Start):
  80 08 29 59 22 06 00 01
  
  Breakdown:
    80 = Long BAP start, group 0
    08 = Payload length (8 bytes)
    29 59 = BAP header (OpCode=SetGet, LSG=0x25, Func=0x19)
    22 = Array header: PosTransmit=1, RecordAddr=6
    06 = startIndex (position indicator value)
    00 = elementCount
    01 = first byte of profile data (position)

Frame 2 (Continuation):
  C0 06 00 20 00
  
  Breakdown:
    C0 = Continuation, group 0, index 0
    06 = operation: climate(0x02) + climateWithoutExternalSupply(0x04)
    00 = operation2 (no window heaters)
    20 = maxCurrent (32 Amps)
    00 = targetChargeLevel (0%, not charging)
```

### Start Climate/Charging

After configuring the profile, start the operation via Function 0x18:

```
CAN ID: 0x17332501 (extended)
Data: 29 58 00 01 00 00 00 00

Breakdown:
  29 58 = BAP header (OpCode=SetGet, LSG=0x25, Func=0x18)
  00 01 = Payload (byte 0 = 0x00, byte 1 = 0x01 for immediately=true)
```

**Payload byte 1 bit mapping:**
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

### Reading All Profiles

To request all profiles:

```
CAN ID: 0x17332501 (extended)
Data: 19 59 (Short BAP: OpCode=GET, LSG=0x25, Func=0x19)

The FSG responds on 0x17332510 with a long BAP message containing
all 4 profiles in full format (RecordAddr=0).
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
    (0x17332501, True, [0x19, 0x42]),  # GET on BAP-Config
    
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

**Example (charger connected, charging):**
```
Raw: 1F 11
  LockSetup: 1 (Lock requested)
  LockState: F (Init)
  SupplyState: 1 (Active - power flowing)
  PlugState: 1 (Plugged)
```

### ChargeState (Function 0x11)

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

**Example (charging active):**
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

---

## Implementation Notes

### Key Considerations

1. **Keep-Alive Timing**: Send heartbeat message every 200-500ms to prevent car from sleeping

2. **Wake-Up Sequence**: Wait for car to fully wake before sending commands (observe responses)

3. **Message Timing**: Allow 50-100ms between CAN frames when sending multi-frame messages

4. **Response Handling**: Monitor response CAN ID 0x17332510 for acknowledgments and status updates

5. **Error Handling**: Check for BAP Error responses (OpCode 0x07)

### Testing Recommendations

1. **Start with read-only**: First implement status monitoring without sending commands
2. **Test wake sequence**: Verify car wakes reliably before adding commands
3. **Test climate first**: Climate start/stop is well-documented and lower risk
4. **Log everything**: Capture all CAN traffic during testing for debugging

---

## See Also

- [BAP_PROTOCOL.md](BAP_PROTOCOL.md) - BAP protocol specification
- [PROTOCOL.md](PROTOCOL.md) - Physical layer and CAN bus basics
- [CAN_IDS.md](CAN_IDS.md) - All CAN IDs and BAP channels
- [OCU_FEATURES.md](OCU_FEATURES.md) - Feature requirements for OCU replacement
