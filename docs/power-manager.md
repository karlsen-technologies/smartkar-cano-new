# Power Manager Module

**File:** `src/modules/PowerManager.h`, `src/modules/PowerManager.cpp`

## Purpose

Controls the AXP2101 Power Management Unit (PMU) on the LilyGo T-SIM7080G board. Responsible for:

- Modem power supply control (DC3 rail)
- Deep sleep configuration and wake source management
- Battery monitoring and charging configuration
- Low battery warnings via hardware IRQ
- Low power mode management (ultra-low power when battery critical)

## Hardware

### AXP2101 PMU

The AXP2101 is a highly integrated power management IC that provides:

- Multiple DC-DC converters
- Multiple LDO regulators
- Li-ion battery charging with configurable parameters
- Power path management
- Hardware IRQ for battery/power events

### Power Channels Used

| Channel | Voltage | Purpose | Status |
|---------|---------|---------|--------|
| DC3 | 3.0V | SIM7080G modem main power | Active |
| BLDO2 | 3.3V | GPS antenna power | Disabled |

### Wake Sources

| Source | GPIO | Description |
|--------|------|-------------|
| Modem RI | GPIO 3 | Modem Ring Indicator - server sent data |
| PMU IRQ | GPIO 6 | Low battery, USB connect/disconnect |

## API

### IModule Interface

```cpp
bool setup();           // Initialize PMU, configure charging, setup IRQs
void loop();            // Handle PMU IRQs, periodic battery logging
void prepareForSleep(); // Enable deep sleep wake sources
bool isBusy();          // Always false
bool isReady();         // True when initialized
```

### Power Control

```cpp
bool setModemPower(bool enable);  // Enable/disable modem DC3 rail
bool isModemPowered();            // Check modem power state (RTC-persistent)
void enableDeepSleepWakeup();     // Configure GPIO3+GPIO6 as EXT1 wake sources
void disableDeepSleepWakeup();    // Remove wake source configuration
```

### Battery Monitoring

```cpp
uint16_t getBatteryVoltage();     // Battery voltage in mV
uint8_t getBatteryPercent();      // Battery percentage 0-100
bool isCharging();                // True if actively charging
bool isVbusConnected();           // True if USB/external power connected
const char* getChargingState();   // "trickle", "precharge", "cc", "cv", "done", "stopped"
uint16_t getChargeCurrentSetting(); // Configured charge current in mA
void printBatteryStatus();        // Verbose battery/charging dump to Serial
```

### Low Power Mode

```cpp
bool isLowPowerMode();            // Check if in ultra-low power mode
void enterLowPowerMode();         // Enter low power mode (modem will stay off)
void exitLowPowerMode();          // Exit low power mode (resume normal operation)
int getWakeupPin();               // Get which GPIO triggered EXT1 wake (-1 if not EXT1)
bool checkPmuWakeupCause();       // Read and log PMU IRQ status after wake
```

### Callbacks

```cpp
void setActivityCallback(ActivityCallback callback);  // For DeviceController integration
void setLowBatteryCallback(LowBatteryCallback callback);  // Called on low battery IRQ
```

## Battery Charging Configuration

Configured in `setup()` for 18650 Li-ion cells:

| Parameter | Value | Notes |
|-----------|-------|-------|
| Target voltage | 4.2V | Standard Li-ion |
| Charge current | 500mA | ~0.2C for 2500mAh cell |
| Precharge current | 50mA | For deeply discharged (<3.0V) |
| Termination current | 25mA | Stop when current drops below |
| VBUS input limit | 900mA | USB 3.0 standard |
| Low battery warning | 10% | Triggers IRQ |
| Low battery shutdown | 5% | Triggers IRQ |
| TS Pin | Disabled | Board has no NTC thermistor |

## PMU IRQ Handling

### Enabled IRQs

- `WARNING_LEVEL1_IRQ` - Battery at 10%
- `WARNING_LEVEL2_IRQ` - Battery at 5% (critical)
- `BAT_INSERT_IRQ` - Battery connected
- `BAT_REMOVE_IRQ` - Battery disconnected
- `VBUS_INSERT_IRQ` - USB power connected
- `VBUS_REMOVE_IRQ` - USB power disconnected

### IRQ Flow

1. PMU asserts IRQ pin (GPIO6 goes LOW)
2. ISR sets `pmuIrqTriggered` flag
3. `loop()` calls `handlePmuIrq()` to process
4. IRQ status read, events logged, callbacks invoked
5. IRQ status cleared

## Low Power Mode

### Purpose

When battery drops to 10%, the device enters "low power mode":
- Modem is disabled to conserve power
- Device sleeps with only PMU IRQ as wake source
- On wake, checks if external power restored before enabling modem

### Flow

```
Battery at 10% → PMU IRQ
    │
    ├── DeviceController::handleLowBattery(1)
    │   ├── Disconnect from server
    │   ├── Disable modem
    │   ├── Enter low power mode
    │   └── Request sleep
    │
    ▼
[DEEP SLEEP - modem off]
    │
    │ Wake (PMU IRQ or other)
    ▼
DeviceController::handleLowPowerModeWake()
    │
    ├── VBUS connected? → Exit low power mode, start modem
    │
    └── No VBUS? → Go back to sleep immediately
```

### RTC Persistence

```cpp
RTC_DATA_ATTR struct {
    bool modemPowered;   // Modem power state
    bool lowPowerMode;   // True when in ultra-low power mode
} rtcConfig;
```

## State Persistence

Uses RTC memory to persist state across deep sleep:

- `modemPowered` - Whether modem DC3 was enabled
- `lowPowerMode` - Whether device is in low battery power-save mode

This enables "hot start" - when the device wakes, if the modem was powered before sleep, it can communicate immediately without a full power-on sequence.

## Periodic Logging

Every 30 seconds, logs battery status:
```
[POWER] Battery: 3850mV 65% | VBUS: yes | Charging: cc
```

## Dependencies

- `XPowersLib` - Library for AXP2101 communication
- `driver/rtc_io.h` - ESP-IDF RTC GPIO functions

## Pin Definitions

From `src/util.h`:
```cpp
#define I2C_SDA         (15)
#define I2C_SCL         (7)
#define PMU_INPUT_PIN   (6)   // PMU IRQ - active low
```
