# Power Manager Module

> **Note:** This documentation refers to the legacy implementation. The current implementation is in `src/modules/PowerManager.h/cpp` and implements the `IModule` interface. See [Architecture](architecture.md) for the new module system.

**File:** `src/modules/PowerManager.cpp`, `src/modules/PowerManager.h`

## Purpose

Controls the AXP2101 Power Management Unit (PMU) on the LilyGo T-SIM7080G board. Responsible for:

- Modem power supply control
- Deep sleep configuration
- Charging LED control (exposed via `getPMU()`)

## Hardware

### AXP2101 PMU

The AXP2101 is a highly integrated power management IC that provides:

- Multiple DC-DC converters
- Multiple LDO regulators
- Battery charging
- Power path management

### Power Channels Used

| Channel | Voltage | Purpose | Status |
|---------|---------|---------|--------|
| DC3 | 3.0V | SIM7080G modem main power | Active |
| BLDO2 | 3.3V | GPS antenna power | Disabled |

## API

### Public Methods

```cpp
bool setup();
```
Initializes the PMU via I2C. Configures voltage levels and initial power state based on RTC memory. Returns `false` if PMU communication fails.

```cpp
void loop();
```
Currently empty. Reserved for future periodic PMU tasks.

```cpp
bool isModemPowered();
```
Returns whether the modem power channel (DC3) is enabled. State is tracked in RTC memory to persist across sleep.

```cpp
bool modemPower(bool enable);
```
Enables or disables the modem power channel. Updates RTC memory state.

```cpp
XPowersPMU* getPMU();
```
Returns direct access to the PMU object for advanced operations (e.g., charging LED control).

```cpp
void enableDeepSleep();
```
Configures ESP32 wake sources for deep sleep. Sets up GPIO_NUM_3 (modem RI pin) as EXT1 wake source.

```cpp
void disableDeepSleep();
```
Removes wake source configuration. Called during normal operation.

## State Persistence

Uses RTC memory to persist modem power state across deep sleep:

```cpp
RTC_DATA_ATTR PowerManagerConfig config = {
    .modemPowered = false,
};
```

This enables "hot start" - when the device wakes, if the modem was powered before sleep, it can attempt to communicate immediately without a full power-on sequence.

## Deep Sleep Configuration

### Wake Sources

Currently configured:
- **EXT1** on GPIO_NUM_3 (modem RI pin) - wakes when modem has data

Future:
- CAN bus interrupt for vehicle activity wake

### GPIO Configuration for Sleep

```cpp
rtc_gpio_init(GPIO_NUM_3);
rtc_gpio_set_direction(GPIO_NUM_3, RTC_GPIO_MODE_INPUT_ONLY);
rtc_gpio_pulldown_dis(GPIO_NUM_3);
rtc_gpio_pullup_dis(GPIO_NUM_3);
esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_3, ESP_EXT1_WAKEUP_ANY_LOW);
```

## Current Behavior

### Initialization (`setup()`)

1. Initialize I2C communication with AXP2101
2. Configure DC3 (modem) voltage to 3.0V
3. Enable/disable DC3 based on RTC memory state
4. Configure BLDO2 (GPS) voltage to 3.3V, but keep disabled
5. Disable TS pin measurement (required for battery charging)
6. Disable deep sleep wake sources (will be enabled before sleep)

### Power On Sequence

When `modemPower(true)` is called:
1. Update RTC memory state
2. Enable DC3 channel

When `modemPower(false)` is called:
1. Update RTC memory state
2. Disable DC3 channel

## Future Considerations

### Battery Monitoring

The AXP2101 can report:
- Battery voltage
- Battery percentage
- Charging status

This could be useful for:
- Low battery warnings
- Preventing operations when battery is critical
- Reporting battery status to server

### GPS Power

The GPS antenna power (BLDO2) is configured but disabled. If GPS functionality is ever needed, it can be enabled here.

### Power Optimization

Consider:
- Reducing voltages where possible
- Disabling unused PMU features
- Monitoring power consumption

## Dependencies

- `XPowersLib` - Library for AXP2101 communication
- `driver/rtc_io.h` - ESP-IDF RTC GPIO functions

## Known Issues

1. **Empty `loop()` function** - Exists but does nothing. Could be removed or used for periodic battery monitoring.

2. **No destructor cleanup** - PMU object is allocated but never freed. Not a practical issue since PowerManager lives for the entire program lifetime.
