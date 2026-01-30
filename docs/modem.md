# Modem Manager Module

**File:** `src/modules/ModemManager.h`, `src/modules/ModemManager.cpp`

**Current Implementation** - Updated January 2026

See also: [Architecture](architecture.md)

## Purpose

Controls the SIM7080G cellular modem for LTE Cat-M1/NB-IoT connectivity and implements the `IModule` interface. Responsible for:

- Modem power-on sequence and configuration
- Network registration (LTE Cat-M1 preferred)
- Internet connection (GPRS/PDP context activation)
- Interrupt handling for modem events (URCs)
- Providing TCP client to LinkManager
- Hot start detection (modem already powered from previous wake)

## Hardware

### SIM7080G Modem

The SIM7080G is a multi-band LTE Cat-M and NB-IoT module supporting:

- LTE Cat-M1: B1/B2/B3/B4/B5/B8/B12/B13/B18/B19/B20/B25/B26/B27/B28/B66/B85
- NB-IoT: B1/B2/B3/B4/B5/B8/B12/B13/B18/B19/B20/B25/B28/B66/B71/B85

### Pin Configuration

| Pin | GPIO | Purpose |
|-----|------|---------|
| PWR | 41 | Power key (toggle to turn on/off) |
| DTR | 42 | Data Terminal Ready |
| RI | 3 | Ring Indicator (interrupt input) |
| RXD | 4 | UART receive (ESP32 TX) |
| TXD | 5 | UART transmit (ESP32 RX) |

## State Machine

```
┌─────────────┐
│ MODEM_OFF   │  Initial state, modem powered off
└──────┬──────┘
       │ setup() or enableModem()
       ├─── Hot start detected? ───► MODEM_STARTING (skip power-on)
       │
       ▼
┌──────────────┐
│MODEM_STARTING│  Power-on sequence, wait for AT response
└──────┬───────┘
       │
       ├─── init() fails (no SIM) ───► MODEM_NOSIM (terminal)
       │
       │ init() success
       ▼
┌────────────────┐
│MODEM_CONFIGURING│  Set APN, network mode, enable RI
└──────┬─────────┘
       │
       │ Config complete
       ▼
┌────────────────┐
│MODEM_SEARCHING │◄──────────────────────────────────┐
└──────┬─────────┘                                   │
       │                                             │
       │ Check CEREG status                          │
       ├─────────────────────────────┐               │
       │                             │               │
       ▼                             ▼               │
┌────────────────┐         ┌─────────────────┐       │
│MODEM_REGISTERED│         │MODEM_UNREGISTERED│──────┘ (retry 1s)
└──────┬─────────┘         └─────────────────┘
       │                   ┌─────────────────┐
       │                   │  MODEM_DENIED   │──────┘ (retry 30s)
       │                   └─────────────────┘
       │ Activate PDP context (+CNACT)
       ▼
┌────────────────┐
│MODEM_CONNECTED │  Internet connected (+APP ACTIVE)
└────────────────┘
```

### State Descriptions

| State | Description | Blocks Sleep? | Timeout |
|-------|-------------|---------------|---------|
| `MODEM_OFF` | Modem powered off | No | - |
| `MODEM_STARTING` | Powering on, waiting for AT | Yes | - |
| `MODEM_CONFIGURING` | Setting APN, network mode, RI | Yes | - |
| `MODEM_NOSIM` | No SIM card detected | No | - |
| `MODEM_SEARCHING` | Searching for network | No | 1s check |
| `MODEM_REGISTERED` | Registered, activating PDP | Yes | 1s check |
| `MODEM_UNREGISTERED` | Lost registration | No | 1s retry |
| `MODEM_DENIED` | Registration denied | No | 30s retry |
| `MODEM_CONNECTED` | Internet connected | No | 10s check |

## API

### IModule Interface

```cpp
bool setup();           // Initialize UART, check for hot start
void loop();            // State machine tick, connection management
bool prepareForSleep(); // Keep modem powered for RI wake
bool isBusy();          // Returns true during state transitions
bool isReady();         // Returns true when MODEM_CONNECTED
```

### Public Methods

```cpp
bool enableModem();      // Power on modem (if currently off)
bool disableModem();     // Power off modem
ModemState getState();   // Get current modem state
String getSimCCID();     // Get SIM card ICCID (cached)
TinyGsm* getModem();     // Get TinyGsm instance for LinkManager
```

### Activity Callback

ModemManager reports activity to DeviceController when:
- State changes (registered, connected)
- Network events occur
- Connection established/lost

## Interrupt Handling

### RI Pin Interrupt

The modem signals events via the RI (Ring Indicator) pin:

```cpp
void IRAM_ATTR Modem::onModemInterrupt() {
    this->hasModemInterrupt = true;
}
```

The ISR only sets a flag. Actual processing happens in `handleModemInterrupt()`:

```cpp
void handleModemInterrupt() {
    // Check for URCs:
    // +APP      → Network state change (ACTIVE/DEACTIVE)
    // +CMT:     → SMS received
    // +CA       → TCP layer event (passed to ConnectionManager)
}
```

### URC (Unsolicited Result Code) Handling

| URC | Meaning | Action |
|-----|---------|--------|
| `+APP ACTIVE` | Internet connected | Set state to `MODEM_CONNECTED` |
| `+APP DEACTIVE` | Internet disconnected | Run `checkModemState()` |
| `+CMT:` | SMS received | Log message (testing only) |
| `+CA` | TCP event | Call `ConnectionManager::handleTCPInterrupt()` |

## Modem Configuration

### Enable Sequence (`enableModem()`)

1. Set state to `MODEM_STARTING`
2. Enable PMU power to modem (DC3)
3. Wait 100ms for power stabilization
4. Toggle PWR pin (LOW → HIGH 1s → LOW)
5. Wait 2.5s for modem boot
6. Call `modem->init()` (tests AT, gets SIM info)
7. Wait for "SMS Ready" URC
8. Enable RI pin interrupt

### Configuration (`MODEM_STARTING` state)

1. Set SMS mode to text (`AT+CMGF=1`)
2. Configure SMS delivery notifications (`AT+CNMI=2,2`)
3. Set network mode to auto (`setNetworkMode(2)`)
4. Set preferred mode to Cat-M (`setPreferredMode(1)`)
5. Configure APN (`AT+CGDCONT=1,"IP","hologram"`)
6. Enable RI pin signaling (`AT+CFGRI=1`)
7. Check registration state

### Network Registration Check

```cpp
void checkModemState() {
    // Send AT+CEREG?
    // Parse response for registration status
    // Update state based on status code
}
```

CEREG status codes:

| Code | Meaning | State |
|------|---------|-------|
| 0 | Not registered, not searching | `MODEM_UNREGISTERED` |
| 1 | Registered, home network | `MODEM_REGISTERED` |
| 2 | Searching | `MODEM_SEARCHING` |
| 3 | Registration denied | `MODEM_DENIED` |
| 5 | Registered, roaming | `MODEM_REGISTERED` |

### Internet Connection (`MODEM_REGISTERED` state)

1. Configure eDRX for power saving (`AT+CEDRXS=1,4,"0001"`)
2. Activate PDP context (`AT+CNACT=0,2`)
3. Wait for `+APP ACTIVE` URC

## Timing Constants

```cpp
#define MODEM_START_DELAY       SECONDS(1)   // Hot start check interval
#define MODEM_SEARCH_DELAY      SECONDS(1)   // Search state check interval
#define MODEM_UNREGISTERED_DELAY SECONDS(1)  // Unregistered retry interval
#define MODEM_DENIED_DELAY      SECONDS(30)  // Denied retry interval
#define MODEM_REGISTERED_DELAY  SECONDS(1)   // Registered state check
#define MODEM_CONNECTED_DELAY   SECONDS(10)  // Connected state check
```

## Dependencies

- `TinyGSM` - AT command library for cellular modems
- `StreamDebugger` - Debug output for AT commands (development)
- `PowerManager` - For modem power control

## Current Behavior Summary

1. On boot, checks if modem is already powered (hot start)
2. If not powered, waits for `enableModem()` call
3. Powers on modem, configures network settings
4. Searches for network, registers when found
5. Connects to internet when registered
6. Handles interrupts for network/TCP events

## Future Considerations

### Error Recovery

- **NOSIM**: Currently terminal. Should retry on next wake cycle.
- **Config failure**: If SMS/APN config fails, currently stuck. Need retry or reset logic.
- **Search timeout**: No timeout implemented. Could use `MODEM_TIMEOUT` state.

### Power Optimization

- eDRX is configured but effectiveness not verified
- Consider PSM (Power Saving Mode) for longer sleep periods
- Monitor actual power consumption in different states

### Configuration

- APN is hardcoded ("hologram"). Should be configurable.
- Network mode preference could be configurable (Cat-M vs NB-IoT)

## Known Issues

1. **MODEM_STARTING config failure** - If any configuration step fails, the modem gets stuck because `justChangedState()` becomes false on the next loop.

2. **No timeout for network search** - Device can stay in `MODEM_SEARCHING` indefinitely if no network is available.

3. **Debug output always enabled** - `StreamDebugger` outputs all AT commands which is verbose but useful for development.
