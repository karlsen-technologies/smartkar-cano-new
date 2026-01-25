# Sleep/Wake Behavior

## Overview

The device is designed for low-power operation in a vehicle environment. It must:

1. Stay awake while the car is active or server commands are being processed
2. Sleep when inactive to preserve the 12V battery
3. Wake on modem events (server communication)
4. Wake on timer (scheduled tasks)
5. Wake on CAN bus activity (car becoming active) - *future*

## Current Implementation

### Sleep Decision

Sleep is managed by `DeviceController::canSleep()` which checks:

1. **Minimum awake time** - Device must be awake for at least 10 seconds
2. **Activity timeout** - No activity for 60 seconds (configurable)
3. **Module busy state** - All modules must report not busy
4. **Explicit request** - `system.sleep` command bypasses timeout checks

```cpp
bool DeviceController::canSleep() {
    // Skip timeout checks if sleep was explicitly requested
    if (!sleepRequested) {
        if ((now - bootTime) < config.minAwakeTime)
            return false;
        if ((now - lastActivityTime) < config.activityTimeout)
            return false;
    }
    
    // Always check module states
    if (powerManager->isBusy()) return false;
    if (modemManager->isBusy()) return false;
    if (linkManager->isBusy()) return false;
    
    return true;
}
```

### Activity Tracking

Modules report activity via callbacks:

```cpp
// When activity occurs (command received, state change, etc.)
activityCallback();  // Resets sleep timer
```

Activity is reported for:
- Commands received from server
- Telemetry sent
- Connection state changes
- Network registration changes

### Wake Sources

Currently configured:

| Source | GPIO | Method | Description |
|--------|------|--------|-------------|
| Modem RI | GPIO 3 | EXT1 | Server sent TCP data |
| Timer | - | esp_sleep_enable_timer_wakeup | Optional scheduled wake |

Future:
| Source | GPIO | Method | Description |
|--------|------|--------|-------------|
| CAN INT | TBD | EXT1 | Vehicle CAN activity |

### Wake Cause Detection

On boot, `DeviceController::logWakeupCause()` identifies the wake source:

```cpp
esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:
        wakeCauseString = "timer";
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        wakeCauseString = "modem_ri";
        break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        wakeCauseString = "fresh_boot";
        break;
}
```

### Sleep Sequence

```
DeviceController::loop()
    │
    │ canSleep() returns true
    ▼
State → PREPARING_SLEEP
    │
    ├── linkManager->prepareForSleep()
    │   ├── Send state: "asleep" to server
    │   └── Close TCP connection
    │
    ├── modemManager->prepareForSleep()
    │   └── (Keep modem powered for RI wake)
    │
    └── powerManager->prepareForSleep()
    │
    ▼
State → SLEEPING
    │
    ├── Configure EXT1 wake on GPIO_NUM_3
    ├── If duration > 0: Configure timer wake
    └── esp_deep_sleep_start()
    │
    ▼
[DEEP SLEEP]
    │
    │ Wake event (modem RI, timer)
    ▼
[RESTART - back to INITIALIZING]
```

## Sleep Command

The device can be commanded to sleep via `system.sleep`:

```json
{
  "type": "command",
  "data": {
    "id": 1,
    "action": "system.sleep",
    "params": {
      "duration": 300
    }
  }
}
```

| Parameter | Description |
|-----------|-------------|
| `duration` | Sleep duration in seconds (0 = wake on interrupt only) |

The command sets `sleepRequested` flag which bypasses normal activity timeout checks.

## Module Busy States

### ModemManager

| State | isBusy() | Reason |
|-------|----------|--------|
| `MODEM_OFF` | false | Already off |
| `MODEM_STARTING` | true | Powering on |
| `MODEM_CONFIGURING` | true | Setting up modem |
| `MODEM_SEARCHING` | false | Can search while sleeping |
| `MODEM_REGISTERED` | true | About to connect |
| `MODEM_CONNECTED` | false | Stable state |
| `MODEM_ERROR` | false | Failed state |

### LinkManager

| State | isBusy() | Reason |
|-------|----------|--------|
| `LINK_DISCONNECTED` | false | Not doing anything |
| `LINK_CONNECTING` | true | TCP connecting |
| `LINK_AUTHENTICATING` | true | Mid-handshake |
| `LINK_CONNECTED` | false | Stable state |

### PowerManager

Always returns `false` for `isBusy()`.

## Power Considerations

### Deep Sleep Current

Target: < 1mA with modem powered (for RI wake capability)

| Component | State | Typical Current |
|-----------|-------|-----------------|
| ESP32-S3 | Deep sleep | ~7-10 µA |
| AXP2101 | Quiescent | ~20 µA |
| SIM7080G | PSM/eDRX | ~100 µA - 1mA |

### Modem Power During Sleep

The modem is kept powered during sleep to enable server-initiated wake:
- Server sends TCP packet → Modem receives → RI pin goes low → ESP32 wakes

If modem power is turned off:
- Lower sleep current
- Cannot wake on server command
- Only wake via timer or CAN

## Configuration

### Current (Testing)

```cpp
// DeviceController.cpp
#define DEFAULT_ACTIVITY_TIMEOUT    60000   // 1 minute
#define DEFAULT_MIN_AWAKE_TIME      10000   // 10 seconds
```

### Future (Production)

```cpp
// Should be based on CAN bus activity
#define ACTIVITY_TIMEOUT_CAR_OFF    30000   // 30 seconds after CAN idle
#define ACTIVITY_TIMEOUT_CAR_ON     300000  // 5 minutes while car active
#define MIN_AWAKE_TIME              5000    // 5 seconds minimum
```

## Future: CAN-Based Sleep Logic

When CAN bus is integrated:

```
Sleep Decision Logic:
1. CAN bus active → Car is ON → Stay awake, report telemetry
2. CAN bus idle > 30 seconds → Car is OFF → Start sleep timer
3. No server activity + CAN idle → Enter sleep
4. Wake on CAN interrupt → Car waking up → Stay awake

Wake Source Priority:
1. CAN interrupt → Car activity, stay awake long
2. Modem RI → Server command, process and maybe sleep again
3. Timer → Scheduled task, execute and sleep
```

## RTC Memory Persistence

Data preserved across deep sleep:

```cpp
RTC_DATA_ATTR struct {
    bool modemPowered;      // Modem power state
    // Future: departure timer, pending commands, etc.
} rtcData;
```

This enables "hot start" - when the device wakes, if the modem was powered before sleep, it can communicate immediately without full initialization.

## Known Limitations

1. **Fixed timeout** - Currently uses fixed 60-second timeout (testing). Production should be CAN-activity based.

2. **No CAN wake** - CAN bus integration not yet implemented.

3. **Modem always powered** - Cannot fully power off modem while maintaining server wake capability.

4. **No scheduled wake** - Timer wake is implemented but no scheduling system yet.
