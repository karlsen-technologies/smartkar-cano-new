# Architecture Overview

## System Purpose

SmartKar-Cano is an IoT control unit designed to be installed in a vehicle (VW e-Golf). Its primary functions are:

1. **Remote Command Execution** - Receive commands from a server (start charging, climate control, etc.)
2. **Telemetry Reporting** - Send device and vehicle status to the server with priority-based intervals
3. **Low Power Operation** - Sleep when inactive to preserve 12V battery
4. **Vehicle Integration** - Interface with CAN bus for vehicle control and monitoring via BAP protocol

## Hardware Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      LilyGo T-SIM7080G                          │
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │   ESP32-S3  │    │  SIM7080G   │    │   AXP2101   │         │
│  │             │◄──►│   Modem     │    │     PMU     │         │
│  │   Main MCU  │    │             │    │             │         │
│  │   Core 0: CAN RX│ │             │    │             │         │
│  │   Core 1: Main  │ │             │    │             │         │
│  └──────┬──────┘    └─────────────┘    └─────────────┘         │
│         │                                                       │
└─────────┼───────────────────────────────────────────────────────┘
          │
          │ SPI
          ▼
┌─────────────────────┐
│   VP231 CAN         │
│   Transceiver       │
└──────────┬──────────┘
           │
           ▼
    Vehicle CAN Bus (Convenience CAN + Gateway CAN)
    - Standard 11-bit IDs for vehicle state
    - Extended 29-bit IDs for BAP protocol
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
    ├── CommandRouter (Command and telemetry hub)
    │   │
    │   ├── Handlers (ICommandHandler implementations)
    │   │   ├── SystemHandler      → system.* commands (reboot, sleep, info)
    │   │   ├── VehicleHandler     → vehicle.* commands (climate, charging, state)
    │   │   └── ChargingProfileHandler → profiles.* commands (timer profiles)
    │   │
    │   └── Providers (ITelemetryProvider implementations)
    │       ├── DeviceProvider     → device telemetry (uptime, battery, memory)
    │       ├── NetworkProvider    → network telemetry (modem, signal, link)
    │       └── VehicleProvider    → vehicle telemetry + events (battery, drive, climate)
    │
    ├── Modules (IModule implementations)
    │   ├── PowerManager    → AXP2101 PMU (charging, sleep, wake sources)
    │   ├── ModemManager    → SIM7080G cellular (LTE Cat-M1/NB-IoT)
    │   ├── LinkManager     → TCP/Server protocol (JSON messages)
    │   └── CanManager      → CAN bus interface (Core 0 RX task)
    │
    └── VehicleManager (CAN message routing)
        ├── Wake State Machine (non-blocking wake sequences)
        ├── Domain Decoders (CAN → VehicleState)
        │   ├── BatteryDomain  → 0x191, 0x5CA, etc.
        │   ├── BodyDomain     → Doors, locks, windows
        │   ├── ClimateDomain  → Temperature, HVAC
        │   ├── DriveDomain    → Ignition, speed, odometer
        │   ├── GpsDomain      → GPS coordinates from CAN
        │   └── RangeDomain    → Range estimation
        │
        └── BAP Protocol (Battery and Parking heater protocol)
            ├── BapChannelRouter
            └── BatteryControlChannel → Climate, charging, plug control
```

### Component Dependencies

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ PowerManager │◄────│ ModemManager │◄────│ LinkManager  │◄────│CommandRouter │
└──────────────┘     └──────────────┘     └──────┬───────┘     └──────┬───────┘
                                                  │                    │
                                                  │                    ▼
┌──────────────┐                                  │          ┌──────────────────┐
│ CanManager   │──────────────────────────────────┤          │ Handlers         │
│ (Core 0 RX)  │                                  │          │ - SystemHandler  │
└──────┬───────┘                                  │          │ - VehicleHandler │
       │                                          │          │ - ProfileHandler │
       ▼                                          │          └──────────────────┘
┌──────────────┐                                  │
│VehicleManager│──────────────────────────────────┘          ┌──────────────────┐
│ - Domains    │                                             │ Providers        │
│ - BAP Channel│◄────────────────────────────────────────────│ - DeviceProvider │
│ - Profiles   │                                             │ - NetworkProvider│
└──────────────┘                                             │ - VehicleProvider│
                                                             └──────────────────┘
```

- **PowerManager** - No dependencies, controls PMU
- **ModemManager** - Depends on PowerManager for modem power control
- **LinkManager** - Depends on ModemManager for TCP client, CommandRouter for message handling
- **CanManager** - Independent, runs CAN RX on Core 0
- **VehicleManager** - Depends on CanManager for sending frames, contains BAP and domain logic
- **CommandRouter** - Central hub, connects handlers and providers
- **Handlers** - Registered with CommandRouter, execute commands, may interact with VehicleManager
- **Providers** - Registered with CommandRouter, collect and emit telemetry

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

## Vehicle Architecture (Implemented)

### Vehicle State Tracking

The system maintains a comprehensive `VehicleState` structure with data from multiple sources:

```
src/vehicle/
├── VehicleManager.h/cpp       # Main vehicle coordinator, wake state machine
├── VehicleTypes.h             # Shared types (DataSource, LockState, etc.)
├── ChargingProfileManager.h   # Timer profile management (4 profiles)
├── domains/                   # Domain managers (each owns RTC-persisted state)
│   ├── BatteryManager         # SOC, charging, voltage (CAN + BAP)
│   ├── BodyManager            # Doors, locks, windows
│   ├── ClimateManager         # HVAC temperature (CAN + BAP)
│   ├── DriveManager           # Ignition, speed, odometer
│   ├── GpsManager             # GPS from CAN
│   └── RangeManager           # Range estimation
└── bap/                       # BAP (Battery and Parking heater) protocol
    ├── BapProtocol.h          # Protocol definitions
    ├── BapChannelRouter       # Routes BAP frames to channels
    └── channels/
        └── BatteryControlChannel.h  # Climate/charging control + callbacks
```

### Data Source Consolidation

Each domain manager maintains its own state with automatic prioritization:

| Data Point | CAN Source | BAP Source | Priority |
|------------|------------|------------|----------|
| **SOC** | 0x191/0x509 (not available) | Function 0x11 | BAP only |
| **Charging** | 0x5CA (boolean) | Function 0x11 (detailed) | BAP preferred |
| **Inside Temp** | 0x66E (passive) | Function 0x12 (active) | BAP when climate active, else CAN |
| **Plug State** | N/A | Function 0x14 | BAP only |

Each domain manager state includes:
- Value (e.g., `BatteryManager::State.soc`)
- Source tracking (e.g., `BatteryManager::State.socSource = DataSource::BAP`)
- Timestamp (e.g., `BatteryManager::State.socUpdate = millis()`)
- RTC persistence (state survives deep sleep)

### BAP Protocol

**BAP (Battery and Parking heater)** is VW's proprietary 29-bit CAN protocol for controlling climate and charging.

**Implemented Functions:**
- **0x11** - Charge state query/response (SOC, mode, status, amps, time remaining)
- **0x12** - Climate state query/response (active, heating, cooling, temp, time)
- **0x13** - Charge profile management (4 profiles: 0=immediate, 1-3=timers)
- **0x14** - Plug state (plugged, supply, lock state)
- **0x21** - Start charging
- **0x22** - Stop charging  
- **0x23** - Start climate
- **0x24** - Stop climate

**Non-Blocking Command Flow:**
```
Command → VehicleHandler → BatteryControlChannel
                              ↓
                        Command queued
                              ↓
                        loop() state machine:
                        IDLE → REQUESTING_WAKE → WAITING_FOR_WAKE 
                        → UPDATING_PROFILE → SENDING_COMMAND → DONE
```

All commands use async state machines with progress tracking via `CommandStateManager`.

### Vehicle Wake Management (Implemented)

The `VehicleManager` includes a non-blocking wake state machine:

**States:**
- `ASLEEP` - No CAN activity detected
- `WAKE_REQUESTED` - Command needs vehicle awake
- `WAKING` - Wake frames sent, waiting for response
- `AWAKE` - Vehicle responding to CAN, ready for commands

**Wake Sequence:**
1. Command requires vehicle awake → `requestWake()`
2. Send wake frame (0x17330301)
3. Initialize BAP (0x1B000067)
4. Start keep-alive (0x5A7 @ 500ms interval)
5. Wait for CAN activity (vehicle responds)
6. Transition to `AWAKE` after 2s BAP init delay
7. Keep-alive continues for 5 minutes or until command activity stops

**Natural Wake Detection:**
Vehicle can wake itself (door open, key fob) - detected automatically via CAN activity monitoring.

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
