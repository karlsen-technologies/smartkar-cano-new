# CAN IDs Reference - VW e-Golf

This document lists all known CAN IDs on the e-Golf comfort CAN bus, including BAP channels and broadcast messages.

## Quick Reference

| Purpose | CAN ID | Type | Document |
|---------|--------|------|----------|
| Wake-up | 0x17330301 | Extended | [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) |
| Keep-alive | 0x000005A7 | Standard | [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) |
| Battery Control commands | 0x17332501 | Extended | [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) |
| Battery Control responses | 0x17332510 | Extended | [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) |
| Horn/Flash/Lock | 0x000005A7 | Standard | [HORN_FLASH_LOCK.md](HORN_FLASH_LOCK.md) |

---

## System/Wake Messages

| CAN ID | Extended | Direction | Purpose |
|--------|----------|-----------|---------|
| `0x17330301` | Yes | TX | Wake-up message |
| `0x000005A7` | No | TX | Keep-alive heartbeat (OCU active signal) |
| `0x1B000067` | Yes | TX | BAP channel initialization broadcast |
| `0x17F00067` | Yes | RX | BAP init response |

---

## BAP Channel CAN IDs

BAP channels use 29-bit extended CAN IDs to create isolated communication channels. See [BAP_PROTOCOL.md](BAP_PROTOCOL.md#channel-architecture) for how channels work.

The typical format is `0x1733XXYY` where XX is the LSG/Device ID and YY indicates message direction.

### CAN ID Structure

```
0x1733XXYY (typical pattern, but not guaranteed)

Where:
  1733     = BAP protocol identifier (common prefix)
  XX       = LSG/Device ID (e.g., 25 = Battery Control, 0D = Door Locking)
  YY       = Channel-specific suffix
```

**Important:** The suffix values (YY) are NOT standardized. Each channel defines its own ASG and FSG CAN IDs. You must look up the specific IDs for each channel - do not assume a pattern.

### Battery Control Channel (Device 0x25)

| CAN ID | Extended | Direction | Role |
|--------|----------|-----------|------|
| `0x17332501` | Yes | TX | ASG (client) to FSG (server) - Commands |
| `0x17332510` | Yes | RX | FSG (server) to ASG (client) - Responses |

See [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) for function details.

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

### All Known BAP Channels

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

### Key Channels for OCU Replacement

| LSG | Name | Purpose | Priority |
|-----|------|---------|----------|
| 0x25 | BatteryControl | SOC, charging, climate control | **Critical** |
| 0x0D | Doorlocking | Lock/unlock status | High |
| 0x37 | ENI | Trip data, vehicle info | Medium |
| 0x01 | Klima1 | Climate status | Medium |
| 0x1C | Hybrid | EV-specific status | Medium |

---

## Broadcast Messages (Non-BAP)

These are one-way broadcast messages using standard 11-bit CAN IDs. They don't use the BAP protocol.

### Common Broadcast IDs

| CAN ID | Name | Content | Notes |
|--------|------|---------|-------|
| `0x184` | - | Vehicle status | |
| `0x366` | - | Unknown | |
| `0x483` | Motor_Hybrid_06 | Power limits | See below |
| `0x583` | ZV_02 | Lock status | See [HORN_FLASH_LOCK.md](HORN_FLASH_LOCK.md) |
| `0x585` | - | Unknown | Changes frequently |
| `0x5A7` | TM_01 | Telematics commands | Horn, flash, lock |
| `0x5E1` | Klima_Sensor_02 | Outside temperature | |
| `0x5F0` | - | Unknown | |

### Motor_Hybrid_06 (CAN ID 0x483)

This standard CAN message (11-bit ID, 8 bytes) contains powermeter limits used by the instrument cluster display. It provides insight into charging power limits during AC charging.

**Message Format (all signals little-endian):**

| Signal | Start Bit | Length | Scale | Range | Description |
|--------|-----------|--------|-------|-------|-------------|
| Mo_Powermeter_Grenze | 0 | 12 bits | 1 | 0-4092 | General powermeter limit |
| MO_Text_Aktivierung_Antrieb | 12 | 4 bits | 1 | 0-15 | Drive text activation |
| MO_Powermeter_Inszenierung_aktiv | 17 | 1 bit | 1 | 0-1 | Powermeter staging active |
| MO_Powermeter_Charge_Grenze | 18 | 10 bits | 1 | 0-1021 | **Charge power limit** |
| MO_Powermeter_Grenze_strategisch | 28 | 12 bits | 1 | 0-4093 | Strategic limit |
| MO_Powermeter_untere_E_Grenze | 40 | 12 bits | 1 | 0-4093 | Lower E limit |
| MO_Powermeter_obere_E_Grenze | 52 | 12 bits | 1 | 0-4093 | Upper E limit |

**Decoding MO_Powermeter_Charge_Grenze:**

```python
def decode_charge_grenze(data):
    """Decode charging power limit from Motor_Hybrid_06 message.
    
    Args:
        data: 8 bytes of CAN message data
        
    Returns:
        Charging power limit in units (~10W per unit)
    """
    byte2 = data[2]
    byte3 = data[3]
    return (byte2 >> 2) | ((byte3 & 0x0F) << 6)
```

**Observed Values:**

| Scenario | MO_Powermeter_Charge_Grenze | Estimated Power |
|----------|----------------------------|-----------------|
| **AC Charging** | | |
| Idle (not charging) | ~565 | ~5.7 kW |
| Ramp-up phase | 565 → 741 | ~5.7 → 7.4 kW |
| Steady state @ 55% SOC | 741 | ~7.4 kW |
| Steady state @ 60% SOC | 727 | ~7.3 kW |
| **Climate Pre-conditioning** | | |
| Cold start (heat pump + resistive heater) | ~1020 | ~10.2 kW |
| Warming up | 991 → 609 | ~9.9 → 6.1 kW |
| Maintenance mode | ~340 | ~3.4 kW |

**Interpretation:**

- Units are approximately **10W per unit** (741 units ≈ 7.4 kW matches e-Golf's 7.2 kW max AC charging)
- The signal represents **power flow** for the powermeter display in the instrument cluster
- **During AC charging**: Shows actual charging power being drawn from the grid
- **During climate pre-conditioning**: Shows power consumption by HVAC system
  - High initial power (~10 kW) when cabin is cold (heat pump + resistive heater working hard)
  - Decreases as cabin warms up
  - Low steady-state power (~3.4 kW) for temperature maintenance
- The slight decrease from 741 → 727 at higher SOC during charging reflects BMS tapering

**Example Messages:**

```
# Charging at full power (~7.4 kW)
F0 0F 94 0B 00 00 00 00
  Mo_Powermeter_Grenze: 4080
  MO_Powermeter_Charge_Grenze: 741 (~7.4 kW charging)

# Climate cold start (~10 kW heating)
FC 03 F0 0F 00 00 00 00
  Mo_Powermeter_Grenze: 1020
  MO_Powermeter_Charge_Grenze: 1020 (~10.2 kW heating)

# Climate maintenance (~3.4 kW)
F0 0F 50 05 00 00 00 00
  Mo_Powermeter_Grenze: 4080
  MO_Powermeter_Charge_Grenze: 340 (~3.4 kW maintenance)
```

**Note:** This message originates from the motor/hybrid controller and is forwarded to comfort CAN via the gateway. It is present during charging and climate operations.

---

## Channel Diagrams

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

---

## See Also

- [BAP_PROTOCOL.md](BAP_PROTOCOL.md) - BAP protocol specification
- [PROTOCOL.md](PROTOCOL.md) - Physical layer and CAN bus basics
- [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) - Battery Control Unit commands
- [CORE_EV_SIGNALS.md](CORE_EV_SIGNALS.md) - EV-specific signal definitions
- [HORN_FLASH_LOCK.md](HORN_FLASH_LOCK.md) - Non-BAP command messages
