# Sleep/Wake Behavior

## Overview

The device is designed for low-power operation in a vehicle environment managed by `DeviceController`. It must:

1. Stay awake while the car is active or server commands are being processed
2. Sleep when inactive to preserve the 12V battery (target: <1mA)
3. Wake on modem events (server communication via RI pin)
4. Wake on PMU events (power connected, low battery)
5. Wake on timer (scheduled tasks, if configured)
6. Detect vehicle activity via CAN bus (vehicle wakes itself or we wake it)

## Current Implementation

### Device State Machine

`DeviceController` manages the overall device lifecycle:

**States:**
- `INITIALIZING` - Setting up modules and providers
- `RUNNING` - Normal operation, processing commands and telemetry
- `PREPARING_SLEEP` - Notifying modules, closing connections
- `SLEEPING` - Entering deep sleep

### Sleep Decision

Sleep is managed by `DeviceController::canSleep()` which checks:

1. **Minimum awake time** - Device must be awake for at least 10 seconds (configurable)
2. **Activity timeout** - No activity for 5 minutes (configurable)
3. **Module busy state** - All modules must report `!isBusy()`
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
    if (canManager->isBusy()) return false;
    if (vehicleManager->isBusy()) return false;
    
    return true;
}
```

### Activity Tracking

Modules report activity via callbacks registered during initialization:

```cpp
// In DeviceController::setup()
powerManager->setActivityCallback(getActivityCallback());
modemManager->setActivityCallback(getActivityCallback());
linkManager->setActivityCallback(getActivityCallback());
vehicleManager->setActivityCallback(getActivityCallback());
```

Activity is reported for:
- Commands received from server
- Telemetry sent (resets timer)
- Connection state changes (modem registered, link connected)
- Network registration changes
- Vehicle commands executing
- CAN activity detected

### Wake Sources

Currently configured wake sources (all set up by PowerManager):

| Source | GPIO | Method | Description |
|--------|------|--------|-------------|
| Modem RI | GPIO 3 | EXT1 | Server sent TCP data via modem |
| PMU IRQ | GPIO 6 | EXT1 | Low battery or USB power change |
| Timer | - | esp_sleep_enable_timer_wakeup | Optional scheduled wake |

**Note:** CAN interrupt wake is not currently implemented (VP231 INT pin not connected).

### Wake Cause Detection

On boot, `DeviceController::logWakeupCause()` identifies the wake source:

```cpp
esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:
        wakeCauseString = "timer";
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        // Determine which pin triggered
        int wakePin = powerManager->getWakeupPin();
        if (wakePin == 3) {
            wakeCauseString = "modem_ri";
        } else if (wakePin == 6) {
            wakeCauseString = "pmu_irq";
            powerManager->checkPmuWakeupCause();  // Log PMU event
        }
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
    │   ├── Send "bye" message: {"type":"bye","data":{"reason":"sleep"}}
    │   └── Close TCP connection
    │
    ├── modemManager->prepareForSleep()
    │   └── Keep modem powered for RI wake capability
    │
    ├── canManager->prepareForSleep()
    │   └── No action (CAN transceiver stays powered)
    │
    ├── vehicleManager->prepareForSleep()
    │   └── No action (state persists in RAM across wake)
    │
    └── powerManager->prepareForSleep()
        └── Configure EXT1 wake on GPIO3 (modem RI) + GPIO6 (PMU IRQ)
    │
    ▼
State → SLEEPING
    │
    ├── If duration > 0: Configure timer wake
    └── esp_deep_sleep_start()
    │
    ▼
[DEEP SLEEP - RAM powered off, RTC memory preserved]
    │
    │ Wake event (modem RI, PMU IRQ, or timer)
    ▼
[RESTART - DeviceController::setup() runs again]
```

## Low Power Mode

When battery drops to 10%, the device enters a special "low power mode" where the modem is disabled to conserve remaining battery.

### Low Battery Response

```
Battery at 10% → PMU WARNING_LEVEL1_IRQ
    │
    ├── DeviceController::handleLowBattery(level=1)
    │   ├── linkManager->prepareForSleep()  // Disconnect gracefully
    │   ├── modemManager->disable()         // Turn off modem
    │   ├── powerManager->enterLowPowerMode()
    │   └── requestSleep(0)                 // Sleep until interrupt
    │
    ▼
[DEEP SLEEP - modem OFF, lowPowerMode flag SET]
    │
    │ Wake (PMU IRQ - USB connected)
    ▼
DeviceController::setup()
    │
    ├── powerManager->isLowPowerMode() == true
    │
    ▼
handleLowPowerModeWake()
    │
    ├── isVbusConnected()? 
    │   ├── YES → exitLowPowerMode(), setup modem, continue normal
    │   └── NO  → Go back to sleep immediately (no modem)
```

### Critical Battery (5%)

If battery hits 5%, emergency shutdown:
```cpp
// Level 2 = critical - immediate shutdown
powerManager->setModemPower(false);
powerManager->enterLowPowerMode();
powerManager->enableDeepSleepWakeup();
esp_deep_sleep_start();
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

### Low Power Mode Current

With modem powered off (low battery mode):

| Component | State | Typical Current |
|-----------|-------|-----------------|
| ESP32-S3 | Deep sleep | ~7-10 µA |
| AXP2101 | Quiescent | ~20 µA |
| SIM7080G | Off | 0 µA |

Total: ~30 µA - can last weeks on remaining battery

### Modem Power During Sleep

Normal sleep: Modem is kept powered to enable server-initiated wake:
- Server sends TCP packet → Modem receives → RI pin goes low → ESP32 wakes

Low power mode: Modem is off to maximize battery life:
- Cannot wake on server command
- Only wake via PMU IRQ (USB connected) or timer

## Configuration

### Current Settings

```cpp
// DeviceController.cpp
#define DEFAULT_ACTIVITY_TIMEOUT    300000  // 5 minutes
#define DEFAULT_MIN_AWAKE_TIME      10000   // 10 seconds

// PowerManager.cpp
pmu->setLowBatWarnThreshold(10);     // 10% warning
pmu->setLowBatShutdownThreshold(5);  // 5% critical
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
3. PMU IRQ → Check if power restored
4. Timer → Scheduled task, execute and sleep
```

## RTC Memory Persistence

Data preserved across deep sleep:

```cpp
RTC_DATA_ATTR struct {
    bool modemPowered;   // Modem power state
    bool lowPowerMode;   // True when in ultra-low power mode due to low battery
} rtcConfig;
```

This enables:
- "Hot start" - modem communication without full init
- Low power mode tracking - don't start modem if battery still low

## Known Limitations

1. **No CAN wake** - CAN bus integration not yet implemented.

2. **No scheduled wake** - Timer wake is implemented but no scheduling system yet.

3. **Low battery event may be missed** - If battery drops from >10% to <5% while sleeping (unlikely), we may miss the 10% warning and only get the 5% critical.
