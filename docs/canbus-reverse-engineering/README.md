# VW e-Golf CAN Bus Documentation

Documentation for communicating with the VW e-Golf (MY2018, MQB platform) via the comfort CAN bus. This project aims to enable OCU (Online Connectivity Unit) replacement for remote vehicle control.

## Quick Start

**Want to start/stop climate or charging?** See [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md)

**Looking for a specific CAN ID?** See [CAN_IDS.md](CAN_IDS.md)

**Need to understand the BAP protocol?** See [BAP_PROTOCOL.md](BAP_PROTOCOL.md)

## Documentation Index

### Core Protocol Documentation

| Document | Description | When to Use |
|----------|-------------|-------------|
| [PROTOCOL.md](PROTOCOL.md) | Physical layer, CAN bus basics, communication overview | Hardware setup, understanding the bus |
| [BAP_PROTOCOL.md](BAP_PROTOCOL.md) | BAP protocol specification, message encoding, arrays | Understanding how BAP works |
| [CAN_IDS.md](CAN_IDS.md) | All known CAN IDs, BAP channels, broadcast messages | Finding the right CAN ID |
| [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md) | Battery Control channel: charging, climate, profiles | Implementing remote control |

### Feature-Specific Documentation

| Document | Description |
|----------|-------------|
| [HORN_FLASH_LOCK.md](HORN_FLASH_LOCK.md) | Horn, flash, door lock commands (non-BAP) |
| [OCU_FEATURES.md](OCU_FEATURES.md) | Feature requirements for OCU replacement |
| [CORE_EV_SIGNALS.md](CORE_EV_SIGNALS.md) | EV-specific CAN signals (battery, motor, charging) |

### Reference Data

| Document | Description |
|----------|-------------|
| [extracted_mqb_signals.md](extracted_mqb_signals.md) | Raw signal extraction from MQB DBC file |
| [extracted_hcan_signals.md](extracted_hcan_signals.md) | Raw signal extraction from HCAN DBC file |
| [CHANGELOG.md](CHANGELOG.md) | Documentation change history |

## Physical Connection

- **Bus**: Comfort CAN (not OBD-II)
- **Speed**: 500 Kbps
- **Connection point**: OCU connector behind speedometer cluster
- **Wires**: CAN-H (orange/green), CAN-L (orange/brown)

See [PROTOCOL.md](PROTOCOL.md#physical-layer) for details.

## Common Tasks

### Start Climate Control

```
1. Wake up the car (0x17330301)
2. Send keep-alive (0x000005A7) every 200-500ms
3. Configure profile 0 for climate mode
4. Send start command (Function 0x18)
```

See [BAP_BATTERY_CONTROL.md](BAP_BATTERY_CONTROL.md#command-reference) for complete sequences.

### Read Battery SOC

SOC is available from:
- **BAP**: ChargeState (Function 0x11) on CAN ID 0x17332510
- **Broadcast**: Various BMS messages (see [CORE_EV_SIGNALS.md](CORE_EV_SIGNALS.md))

### Horn/Flash/Lock

These use simple broadcast messages on CAN ID 0x5A7, not BAP protocol.
See [HORN_FLASH_LOCK.md](HORN_FLASH_LOCK.md).

## Related Projects

- `vw-bap-analyser/` - TypeScript BAP message decoder/analyzer
- `e-golf-comfort-can/` - Additional documentation and examples

## Contributing

This documentation is based on reverse engineering. If you discover corrections or additional information, contributions are welcome.
