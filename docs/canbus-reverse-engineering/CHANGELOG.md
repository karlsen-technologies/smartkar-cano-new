# Changelog - VW e-Golf CAN Bus OCU Replacement Project

All notable changes to this project are documented in this file.

---

## [2026-01-27] - Range Estimate Signals (Reichweite)

### Added
- **Range estimate signals (0x5F5, 0x5F7)** - Confirmed on comfort CAN bus
  - 0x5F5 Reichweite_01: Total range, electric range, consumption
  - 0x5F7 Reichweite_02: Display values, range tendency, reserve warning
  - Provides car's calculated range estimate directly (no calculation needed)

### Updated
- `OCU_FEATURES.md` - Added complete signal definitions for range messages
- Feature overview now includes "Estimated range" as high priority

### Verified from Trace
- Electric range: 130-131 km
- Max display range: 247 km
- Consumption value: 11.5
- Range tendency: stable
- Invalid/init values: 2045-2047 (0x7FD-0x7FF)

---

## [2026-01-27] - BAP Protocol Verification

### Added
- **BAP Channel Architecture documentation** in `PROTOCOL.md`
  - Explained channel isolation concept (each LSG has dedicated CAN IDs)
  - Documented known channel definitions with their specific CAN IDs
  - Added channel isolation diagrams
  - Note: CAN ID suffixes are channel-specific, not a universal pattern

- **Complete BAP Channel List** - 29 channels identified from traces
  - Cross-referenced with DBC files for channel names
  - Listed all ASG and FSG CAN IDs for each channel
  - Identified key channels for OCU replacement (0x25, 0x0D, 0x37, 0x01, 0x1C)

- **Long Message Assembly Clarification**
  - Frame assembly must be tracked by (CAN ID + message index) tuple
  - Multiple channels can have concurrent long messages with same message index
  - Single channel can have multiple concurrent long messages with different indices
  - Added pseudocode for correct frame assembly algorithm

### Verified
- **ChargeState (0x11)** - Verified against `connect_charger_sleep.csv`
  - SOC reporting confirmed (47% = 0x2F)
  - ChargeMode: 0=Off, 1=AC charging, 2=DC charging
  - ChargeState: 1=Idle, 2=Running
  - Remaining time in minutes (0xFF=unknown)
  - startReason and targetSOC fields confirmed
  
- **PlugState (0x10)** - Verified against `connect_charger_sleep.csv`
  - PlugState: 0=Unplugged, 1=Plugged
  - SupplyState: 0=Inactive, 1=Active (power flowing)
  - LockSetup: 0=Unlock, 1=Lock requested

### Updated
- `PROTOCOL.md` - Added verification notes and examples from real traces

---

## [2026-01-27] - CAN Bus Availability Verification

### Fixed
- **Removed invalid CAN IDs from documentation** - Many signals from DBC files were documented but are NOT available on the comfort CAN bus. They exist on powertrain/drivetrain CAN instead.

### Removed from Documentation (NOT on Comfort Bus)
| CAN ID | Name | Actual Location |
|--------|------|-----------------|
| 0x191 | BMS_HV (SOC, voltage, current) | Powertrain CAN |
| 0x509 | BMS_Energie (high-res) | Powertrain CAN |
| 0x5A2 | BMS_Kapazitaet | Powertrain CAN |
| 0x564 | Charger temperature | Powertrain CAN |
| 0x578 | DC charging status | Powertrain CAN |
| 0x2AE | DCDC converter | Powertrain CAN |
| 0x5CD | DCDC faults | Powertrain CAN |
| 0x659 | Compressor | HVAC CAN |

### Verified Available on Comfort Bus
All remaining documented signals verified against `ignthenprofilethenoff.csv`:
- 0x583, 0x3D0, 0x3D1, 0x5A7 (security/doors)
- 0x5E1, 0x594, 0x588, 0x3E9, 0x668, 0x66E (climate)
- 0x484, 0x485, 0x486, 0x1A5554A8 (GPS)
- 0x3C0, 0x6B2 (vehicle status)
- 0x5CA, 0x59E (limited BMS data)

### Note
For full BMS/charging data, use **BAP protocol** (0x17332501/0x17332510) which provides SOC, charging status, and plug state through the Battery Control device.

---

## [2026-01-27] - GPS Signal Integration

### Added
- **GPS decoding to `broadcast_decoder.py`**
  - `GPSPosition` dataclass for NavPos_01 (0x486)
  - `GPSData` dataclass for NavData_02 (0x485)
  - `GPSHeading` dataclass for NavData_01 (0x484)
  - `GPSFixType` enum (NO_FIX, FIX_2D, FIX_3D, DGPS)
  - `ExtendedCANIds` class for 29-bit CAN IDs
  - Map-matched position decoder for 0x1A5554A8

- **GPS documentation in `OCU_FEATURES.md`**
  - Complete signal definitions for all 4 GPS messages
  - Bit-level encoding details
  - Example decoding from real trace data

### Verified
- All GPS CAN IDs confirmed present in traces
- Decoded sample: 69.697540°N, 18.953311°E, 22m altitude, 5 satellites

---

## [2026-01-27] - Broadcast Signal Decoder Module

### Added
- **New file: `broadcast_decoder.py`**
  - `BroadcastDecoder` class for decoding standard CAN messages
  - `TM01Commands` class for generating horn/flash/lock commands
  - Data classes: `LockStatus`, `DoorModuleStatus`, `TemperatureData`, `ClimateSettings`, `SeatHeatingStatus`, `IgnitionStatus`, `OdometerData`
  - Enums: `LockState`, `DoorState`, `IgnitionState`

### Updated
- **`egolf_canbus.py`**
  - Added standard CAN IDs to `CANIds` class
  - Added TM_01 commands: `horn()`, `flash()`, `lock_doors()`, `unlock_doors()`, `panic()`, `horn_and_flash()`
  - Added methods to `EGolfCANBus` class

---

## [2026-01-27] - Outside Temperature Signal

### Added
- **Outside temperature signal (0x5E1)**
  - Signal: `BCM1_Aussen_Temp_ungef` from `Klima_Sensor_02`
  - Formula: `temp_celsius = raw_value * 0.5 - 50`
  - Verified: 0x7B = 11.5°C

### Updated
- `OCU_FEATURES.md` - Added signal to climate section and confirmation table

---

## Project Files

### Documentation
| File | Description |
|------|-------------|
| `OCU_FEATURES.md` | Feature requirements and signal mapping |
| `PROTOCOL.md` | BAP protocol documentation |
| `HORN_FLASH_LOCK.md` | TM_01 message documentation |
| `CORE_EV_SIGNALS.md` | BMS/DCDC signals from DBC (note: many not on comfort bus) |
| `CHANGELOG.md` | This file |

### Code
| File | Description |
|------|-------------|
| `egolf_canbus.py` | Main module - BAP protocol, commands, decoders |
| `broadcast_decoder.py` | Broadcast signal decoder for standard CAN messages |
| `protocol_analyzer.py` | Trace analysis tool |

---

## Verified CAN IDs Summary

### Comfort CAN Bus - Standard (11-bit)
| CAN ID | Name | Function | Status |
|--------|------|----------|--------|
| 0x3C0 | Klemmen_Status_01 | Ignition state | ✓ Verified |
| 0x3D0 | TSG_FT_01 | Driver door/window | ✓ Verified |
| 0x3D1 | TSG_BT_01 | Passenger door/window | ✓ Verified |
| 0x3E9 | Thermo_Verbrauch_01 | Thermal data | ✓ Verified |
| 0x484 | NavData_01 | GPS heading/DOP | ✓ Verified |
| 0x485 | NavData_02 | GPS altitude/UTC | ✓ Verified |
| 0x486 | NavPos_01 | GPS position | ✓ Verified |
| 0x583 | ZV_02 | Lock/door status | ✓ Verified |
| 0x588 | Seat/Window Heat | Heating status | ✓ Verified |
| 0x594 | Climate Settings | Target temp | ✓ Verified |
| 0x59E | BMS_06 | Battery temp | ✓ Verified |
| 0x5A7 | TM_01 | Horn/flash/lock cmd | ✓ Verified |
| 0x5CA | BMS_07 | Energy/charging | ✓ Verified |
| 0x5E1 | Klima_Sensor_02 | Outside temp | ✓ Verified |
| 0x5F5 | Reichweite_01 | Range data | ✓ Verified |
| 0x5F7 | Reichweite_02 | Range display | ✓ Verified |
| 0x668 | Klima_01 | Blower/seat heat | ✓ Verified |
| 0x66E | Klima_02 | Climate status | ✓ Verified |
| 0x6B2 | Diagnose_01 | Odometer/time | ✓ Verified |

### Comfort CAN Bus - Extended (29-bit)
| CAN ID | Name | Function | Status |
|--------|------|----------|--------|
| 0x17332501 | BAP Battery Control ASG | Commands | ✓ Verified |
| 0x17332510 | BAP Battery Control FSG | Responses | ✓ Verified |
| 0x1A5554A8 | NavPos_02_Map_Matched | Map-matched GPS | ✓ Verified |

### NOT on Comfort CAN Bus
| CAN ID | Name | Actual Bus |
|--------|------|------------|
| 0x191 | BMS_HV | Powertrain |
| 0x2AE | DCDC_01 | Powertrain |
| 0x509 | BMS_Energie | Powertrain |
| 0x564 | Ladegeraet | Powertrain |
| 0x578 | BMS_DCLS | Powertrain |
| 0x5A2 | BMS_Kapazitaet | Powertrain |
| 0x5CD | DCDC_02 | Powertrain |
| 0x659 | EKL_Kompressor | HVAC |
