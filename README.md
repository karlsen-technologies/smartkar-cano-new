# SmartKar-Cano

ESP32-based IoT control unit for vehicle monitoring and remote command execution. Designed for the Volkswagen e-Golf, connecting to a remote server via LTE Cat-M1/NB-IoT to provide telemetry reporting and remote vehicle control via CAN bus.

## Project Status

**Active Development** - Core infrastructure complete. Telemetry and command system operational. CAN bus integration pending.

## Hardware

| Component | Details |
|-----------|---------|
| **Board** | LilyGo T-SIM7080G |
| **MCU** | ESP32-S3 (with PSRAM) |
| **Cellular Modem** | SIM7080G (LTE Cat-M1 / NB-IoT) |
| **Power Management** | AXP2101 PMU |
| **CAN Interface** | T-SIMHAT with VP231 transceiver (planned) |

## Features

### Implemented
- Modular architecture with lifecycle management (IModule interface)
- Cellular connectivity via LTE Cat-M1 with state machine
- TCP connection to remote server with automatic reconnection
- JSON-based command/telemetry protocol
- Command routing system with domain-based handlers
- Telemetry collection with priority-based intervals
- Deep sleep with wake-on-modem-interrupt and timer wake
- Power management (battery monitoring, charging detection)
- Activity-based sleep decisions
- Remote commands: `system.reboot`, `system.sleep`, `system.telemetry`, `system.info`

### Planned
- CAN bus integration for vehicle data
- Vehicle commands (start charging, preheat, climate control)
- Vehicle telemetry (battery SOC, range, temperatures)
- Wake-on-CAN-activity
- Departure timer / scheduling system

## Project Structure

```
smartkar-cano-new/
├── src/
│   ├── main.cpp                     # Entry point (calls DeviceController)
│   ├── util.h/cpp                   # Pin definitions, time macros
│   ├── core/
│   │   ├── IModule.h                # Module lifecycle interface
│   │   ├── ICommandHandler.h        # Command handler interface
│   │   ├── ITelemetryProvider.h     # Telemetry provider interface
│   │   ├── CommandRouter.h/cpp      # Command routing & telemetry collection
│   │   └── DeviceController.h/cpp   # Central coordinator
│   ├── modules/
│   │   ├── PowerManager.h/cpp       # AXP2101 PMU control
│   │   ├── ModemManager.h/cpp       # SIM7080G modem state machine
│   │   └── LinkManager.h/cpp        # TCP connection & protocol
│   ├── providers/
│   │   ├── DeviceProvider.h/cpp     # Device telemetry
│   │   └── NetworkProvider.h/cpp    # Network telemetry
│   ├── handlers/
│   │   └── SystemHandler.h/cpp      # System commands
│   └── _old/                        # Legacy code (excluded from build)
├── docs/                            # Documentation
├── platformio.ini                   # PlatformIO configuration
└── README.md
```

## Building

This project uses [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor
```

## Configuration

### Server Connection

Configure in `src/modules/LinkManager.cpp`:
```cpp
#define SERVER_HOST "your-server.example.com"
#define SERVER_PORT 4589
```

### APN

Configure in `src/modules/ModemManager.cpp`:
```cpp
#define CELLULAR_APN "your-apn"
```

## Documentation

See the `docs/` folder for detailed documentation:

- [Architecture Overview](docs/architecture.md) - Module system, state machines, data flow
- [Protocol Specification](docs/protocol.md) - JSON message format, commands, telemetry
- [Sleep/Wake Behavior](docs/sleep-wake.md) - Power management, wake sources

## Protocol Overview

The device communicates with the server using newline-delimited JSON messages over TCP:

```json
// Telemetry (device -> server)
{"type":"telemetry","data":{"device":{...},"network":{...}}}

// Command (server -> device)
{"type":"command","data":{"id":123,"action":"system.reboot","params":{}}}

// Response (device -> server)
{"type":"response","data":{"id":123,"status":"ok","data":{...}}}
```

See [Protocol Specification](docs/protocol.md) for complete details.

## License

Private project - not licensed for public use.
