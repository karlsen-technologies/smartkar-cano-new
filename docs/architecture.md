# Architecture Overview

## System Purpose

SmartKar-Cano is an IoT control unit designed to be installed in a vehicle (VW e-Golf). Its primary functions are:

1. **Remote Command Execution** - Receive commands from a server (start charging, preheat, etc.)
2. **Telemetry Reporting** - Send device and vehicle status to the server
3. **Low Power Operation** - Sleep when inactive to preserve 12V battery
4. **Vehicle Integration** - Interface with CAN bus for vehicle control and monitoring (planned)

## Hardware Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      LilyGo T-SIM7080G                          │
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │   ESP32-S3  │    │  SIM7080G   │    │   AXP2101   │         │
│  │             │◄──►│   Modem     │    │     PMU     │         │
│  │   Main MCU  │    │             │    │             │         │
│  └──────┬──────┘    └─────────────┘    └─────────────┘         │
│         │                                                       │
└─────────┼───────────────────────────────────────────────────────┘
          │
          │ (future: T-SIMHAT)
          ▼
┌─────────────────────┐
│   VP231 CAN         │
│   Transceiver       │
└──────────┬──────────┘
           │
           ▼
    Vehicle CAN Bus
```

## Software Architecture

### Core Interfaces

The system uses three core interfaces for extensibility:

```
┌─────────────────────────────────────────────────────────────┐
│                        IModule                               │
│  Lifecycle management for all hardware-interfacing modules   │
│  - setup() → bool                                            │
│  - loop()                                                    │
│  - prepareForSleep()                                         │
│  - isBusy() → bool                                           │
│  - isReady() → bool                                          │
│  - setActivityCallback(ActivityCallback)                     │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                    ICommandHandler                           │
│  Handles commands from server in a specific domain           │
│  - getDomain() → const char*                                 │
│  - handleCommand(CommandContext&) → CommandResult            │
│  - getSupportedActions(size_t&) → const char**               │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   ITelemetryProvider                         │
│  Provides telemetry data for a specific domain               │
│  - getDomain() → const char*                                 │
│  - hasChanged() → bool                                       │
│  - collect(JsonObject&)                                      │
│  - getPriority() → TelemetryPriority                         │
│  - onTelemetrySent()                                         │
└─────────────────────────────────────────────────────────────┘
```

### Module Hierarchy

```
DeviceController (Central Coordinator)
    │
    ├── CommandRouter
    │   ├── Handlers: SystemHandler, (future: VehicleHandler, ChargingHandler)
    │   └── Providers: DeviceProvider, NetworkProvider, (future: VehicleProvider)
    │
    ├── Modules (IModule implementations)
    │   ├── PowerManager    → AXP2101 PMU
    │   ├── ModemManager    → SIM7080G cellular
    │   └── LinkManager     → TCP/Server protocol
    │
    └── (future) CANManager → Vehicle CAN bus
```

### Component Dependencies

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ PowerManager │◄────│ ModemManager │◄────│ LinkManager  │
└──────────────┘     └──────────────┘     └──────┬───────┘
                                                  │
                                                  ▼
                                          ┌──────────────┐
                                          │CommandRouter │
                                          └──────┬───────┘
                                                 │
                          ┌──────────────────────┼──────────────────────┐
                          │                      │                      │
                          ▼                      ▼                      ▼
                   ┌─────────────┐      ┌──────────────┐      ┌──────────────┐
                   │SystemHandler│      │DeviceProvider│      │NetworkProvider│
                   └─────────────┘      └──────────────┘      └──────────────┘
```

- **PowerManager** - No dependencies, controls PMU
- **ModemManager** - Depends on PowerManager for modem power control
- **LinkManager** - Depends on ModemManager for TCP client, CommandRouter for message handling
- **Handlers/Providers** - Registered with CommandRouter, may reference DeviceController

## State Machines

### Device State (DeviceController)

```
INITIALIZING ──► RUNNING ──► PREPARING_SLEEP ──► SLEEPING
                    │                                 │
                    │◄────────── [wake] ──────────────┘
```

| State | Description |
|-------|-------------|
| `INITIALIZING` | Setting up modules and providers |
| `RUNNING` | Normal operation, processing commands |
| `PREPARING_SLEEP` | Notifying modules, closing connections |
| `SLEEPING` | Entering deep sleep |

### Modem State (ModemManager)

```
MODEM_OFF ──► MODEM_STARTING ──► MODEM_CONFIGURING ──► MODEM_SEARCHING
                                                              │
    ┌─────────────────────────────────────────────────────────┤
    │                                                         │
    ▼                                                         ▼
MODEM_ERROR                                           MODEM_REGISTERED
(NOSIM, etc.)                                                 │
                                                              ▼
                                                      MODEM_CONNECTED
```

| State | Description | Blocks Sleep? |
|-------|-------------|---------------|
| `MODEM_OFF` | Modem powered off | No |
| `MODEM_STARTING` | Powering on, waiting for AT | Yes |
| `MODEM_CONFIGURING` | Setting APN, network mode | Yes |
| `MODEM_SEARCHING` | Searching for network | No |
| `MODEM_REGISTERED` | Registered, activating PDP | Yes |
| `MODEM_CONNECTED` | Internet connected | No |
| `MODEM_ERROR` | Error state (no SIM, etc.) | No |

### Link State (LinkManager)

```
LINK_DISCONNECTED ──► LINK_CONNECTING ──► LINK_AUTHENTICATING ──► LINK_CONNECTED
        ▲                                                              │
        └──────────────────────────────────────────────────────────────┘
```

| State | Description | Blocks Sleep? |
|-------|-------------|---------------|
| `LINK_DISCONNECTED` | Not connected to server | No |
| `LINK_CONNECTING` | TCP connecting | Yes |
| `LINK_AUTHENTICATING` | Waiting for auth response | Yes |
| `LINK_CONNECTED` | Authenticated, ready | No |

## Data Flow

### Command Flow (Server → Device)

```
Server                    LinkManager              CommandRouter           Handler
  │                           │                         │                     │
  │──── JSON command ────────►│                         │                     │
  │                           │                         │                     │
  │                           │── handleCommand() ─────►│                     │
  │                           │                         │                     │
  │                           │                         │── handleCommand() ──►│
  │                           │                         │                      │
  │                           │                         │◄── CommandResult ────│
  │                           │                         │                      │
  │                           │◄── JSON response ───────│                      │
  │                           │                         │                      │
  │◄─── JSON response ────────│                         │                      │
```

### Telemetry Flow (Device → Server)

```
DeviceController            CommandRouter           Providers           LinkManager
      │                          │                      │                    │
      │                          │                      │                    │
      │                          │◄─ hasChanged()? ─────│                    │
      │                          │                      │                    │
      │                          │── collect() ────────►│                    │
      │                          │                      │                    │
      │                          │◄── JSON data ────────│                    │
      │                          │                      │                    │
      │                          │─── telemetry JSON ──────────────────────►│
      │                          │                                           │
      │                          │◄── onTelemetrySent() ─────────────────────│
      │                          │                      │                    │
      │                          │── onTelemetrySent() ►│                    │
```

### Telemetry Intervals

The system uses priority-based telemetry intervals:

| Priority | Interval | Use Case |
|----------|----------|----------|
| `PRIORITY_LOW` | 5 minutes | Stable device metrics |
| `PRIORITY_NORMAL` | 2 minutes | Network status |
| `PRIORITY_HIGH` | 30 seconds | Active operations |
| `PRIORITY_REALTIME` | 5 seconds | Critical events |

Providers report their priority, and CommandRouter uses the highest priority to determine the send interval. Providers can also trigger immediate sends via `hasChanged()`.

## Sleep/Wake Architecture

### Activity Tracking

Modules report activity via callbacks:
```cpp
void reportActivity();  // Called when meaningful activity occurs
```

The DeviceController tracks `lastActivityTime` and checks:
1. No modules are busy (`isBusy()` returns false)
2. Activity timeout has elapsed
3. Minimum awake time has passed
4. OR sleep was explicitly requested via command

### Wake Sources

| Source | GPIO | Purpose |
|--------|------|---------|
| Modem RI | GPIO 3 | Server communication (always enabled) |
| Timer | - | Optional scheduled wake |
| CAN INT | TBD | Vehicle activity (future) |

### Sleep Sequence

```
1. DeviceController::canSleep() returns true
2. State → PREPARING_SLEEP
3. LinkManager::prepareForSleep()
   - Send "asleep" state to server
   - Close TCP connection
4. ModemManager::prepareForSleep()
   - (Keep modem powered for RI wake)
5. PowerManager::prepareForSleep()
6. State → SLEEPING
7. Configure wake sources
8. esp_deep_sleep_start()
```

## Future Architecture

### Vehicle Modules (Planned)

```
src/vehicle/
├── CANManager.h/cpp        # CAN bus interface
├── ChargingModule.h/cpp    # Charging control & telemetry
├── ClimateModule.h/cpp     # HVAC control & telemetry
├── VehicleModule.h/cpp     # Odometer, 12V battery, GPS
└── SecurityModule.h/cpp    # Locks, horn, lights
```

Each vehicle module implements both `ICommandHandler` and `ITelemetryProvider`:

```cpp
class ChargingModule : public ICommandHandler, public ITelemetryProvider {
    // Commands: charging.start, charging.stop, charging.setLimit
    // Telemetry: soc, range, chargingState, chargerPower, etc.
};
```

### Wake-on-CAN

```
┌─────────────────────────────────────────────────────────────┐
│                     Wake Source Logic                        │
│                                                             │
│  if (wakeSource == MODEM_RI)                                │
│      → Server wants to send command                          │
│      → Process command, send response, maybe sleep again     │
│                                                             │
│  if (wakeSource == CAN_INTERRUPT)                           │
│      → Car is waking up                                      │
│      → Stay awake, start telemetry reporting                 │
│      → Sleep when CAN bus idle + no server activity          │
└─────────────────────────────────────────────────────────────┘
```

## Configuration

### Compile-Time Configuration

Located in source files (to be moved to config header):

| Setting | Location | Default |
|---------|----------|---------|
| Server host | `LinkManager.cpp` | `gallant.kartech.no` |
| Server port | `LinkManager.cpp` | `4589` |
| APN | `ModemManager.cpp` | `hologram` |
| Activity timeout | `DeviceController.cpp` | `60000` ms |
| Min awake time | `DeviceController.cpp` | `10000` ms |

### Future: Runtime Configuration

Consider storing in NVS:
- Server host/port
- APN
- Sleep timeouts
- Telemetry intervals
