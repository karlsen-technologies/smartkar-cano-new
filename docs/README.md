# SmartKar-Cano Documentation

## Overview

This directory contains documentation for the SmartKar-Cano IoT vehicle control system.

## Core Documentation

These documents describe the **current implemented architecture** and are kept up-to-date:

### System Architecture
- **[architecture.md](architecture.md)** - System overview, module hierarchy, component dependencies
- **[protocol.md](protocol.md)** - JSON protocol specification (v2.2) for server communication

### Module Documentation
- **[connection-manager.md](connection-manager.md)** - LinkManager: TCP connection and server protocol
- **[modem.md](modem.md)** - ModemManager: SIM7080G cellular modem control
- **[power-manager.md](power-manager.md)** - PowerManager: AXP2101 PMU, battery, sleep/wake
- **[sleep-wake.md](sleep-wake.md)** - Sleep/wake behavior and power management

## Implementation Plans & Historical Documents

These documents were **planning documents** created during development and may not reflect the current implementation. They are kept for historical reference:

- **COMMAND_HANDLING_REFACTOR_PLAN.md** - Plan for v2.2 command lifecycle tracking (✅ Implemented)
- **PROFILE_COMMAND_REFACTORING.md** - Charging profile refactoring plan
- **VEHICLE_STATE_CONSOLIDATION.md** - VehicleState data source consolidation plan (✅ Implemented)
- **ARCHITECTURE_REFACTORING_SUMMARY.md** - CAN architecture refactoring summary (✅ Implemented)
- **WAKE_STATE_MACHINE.md** - Wake state machine implementation details (✅ Implemented)
- **CAN_ARCHITECTURE_V2.md** - Detailed CAN and BAP architecture
- **BAP_REFACTORING_PLAN.md** - BAP protocol refactoring plan

**Note:** For the most accurate information about current implementation, refer to the **Core Documentation** files above and the source code itself.

## CAN Bus Reverse Engineering

The `canbus-reverse-engineering/` subdirectory contains:
- Protocol analysis documents (BAP, broadcast, CAN IDs)
- Python analysis scripts
- DBC files
- Test traces and work files

This is reference material for understanding the VW e-Golf CAN bus protocols.

## Quick Links

- [Main README](../README.md) - Project overview and build instructions
- [Source Code](../src/) - Implementation
- [Protocol Specification](protocol.md) - Server integration guide

## Contributing to Documentation

When updating documentation:
1. **Core Documentation** files should always reflect the current implementation
2. Mark outdated sections with a note and date
3. When making significant code changes, update relevant documentation
4. Implementation plans can be moved to an `archive/` subfolder once completed
