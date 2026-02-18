# Vehicle Wake State Machine

## Overview

The wake state machine tracks vehicle CAN bus activity and manages wake-up/keep-alive when commands need to be sent. The vehicle controls when it sleeps/wakes based on its own logic - we only influence it via keep-alive frames.

## Key Concepts

### Vehicle Sleep/Wake Behavior

**Vehicle can wake up from:**
- User opens door
- Key fob button press
- Ignition on
- **Our wake sequence** (0x17330301 + 0x1B000067 + keep-alive)

**Vehicle goes to sleep when:**
- No activity for a period (vehicle-controlled timing)
- Ignition off + doors closed + no commands
- **Keep-alive stops** (vehicle decides when to sleep)

### Our Role

**Passive Monitoring:**
- We listen to CAN bus
- Track activity via `VehicleState::isAwake()` (frames received in last 5s)
- Update wake state accordingly

**Active Commands:**
- We request wake if vehicle asleep
- We send keep-alive to prevent sleep while commands pending
- We stop keep-alive after 5 min timeout (no commands)
- Vehicle decides when to actually sleep

---

## State Machine

### States

```
┌─────────────────────────────────────────────────────────┐
│ ASLEEP                                                  │
│ - No CAN activity detected                              │
│ - No keep-alive active                                  │
│ - Waiting for wake request or natural wake              │
└─────────────────────────────────────────────────────────┘
       │                           ↑
       │ requestWake()             │ No CAN activity
       │ (command needs vehicle)   │ (natural sleep)
       ↓                           │
┌─────────────────────────────────────────────────────────┐
│ WAKE_REQUESTED                                          │
│ - Command requested wake                                │
│ - About to send wake sequence                           │
└─────────────────────────────────────────────────────────┘
       │
       │ Send wake frame + BAP init
       │ Start keep-alive (0x5A7 @ 500ms)
       ↓
┌─────────────────────────────────────────────────────────┐
│ WAKING                                                  │
│ - Wake frames sent                                      │
│ - Keep-alive active                                     │
│ - Waiting for CAN activity                              │
│ - Timeout: 10 seconds                                   │
└─────────────────────────────────────────────────────────┘
       │                           │
       │ CAN activity detected     │ Timeout
       │ + 2s BAP init wait        │ (no response)
       ↓                           ↓
┌─────────────────────────────────────────────────────────┐
│ AWAKE                                                   │
│ - CAN activity present                                  │
│ - Vehicle responding                                    │
│ - Keep-alive active (if commands pending)               │
│ - Ready for BAP commands                                │
└─────────────────────────────────────────────────────────┘
       │
       │ No CAN activity
       │ (natural sleep)
       ↓
    ASLEEP
```

### Natural Wake Detection

**Vehicle wakes naturally:**
```
ASLEEP state:
  - Monitor VehicleState::isAwake()
  - If CAN activity detected → AWAKE
  - No wake sequence needed
```

**Example:** User opens door, vehicle wakes, we detect activity and transition to AWAKE.

---

## Keep-Alive Management

### Purpose
- Prevent vehicle from sleeping while we have commands to send
- **Not** to keep vehicle awake permanently
- **Not** to control when vehicle sleeps

### Behavior

**When Started:**
- Send 0x5A7 frame every 500ms
- Data: `00 00 00 00 00 00 00 00`
- Continue until stopped or timeout

**When Stopped:**
- After 5 min timeout (no new commands)
- When wake sequence fails
- When vehicle goes to sleep naturally
- **Vehicle decides when to actually sleep** (not immediate)

### Timeout Logic

```cpp
lastWakeActivity = millis();  // Updated when command requests wake

if (now - lastWakeActivity > KEEPALIVE_TIMEOUT) {  // 5 minutes
    stopKeepAlive();
    // Don't force ASLEEP - let vehicle sleep naturally
}
```

**Key Point:** Stopping keep-alive does NOT transition to ASLEEP. The transition happens when CAN activity stops (vehicle slept naturally).

---

## State Transitions

### 1. ASLEEP → WAKE_REQUESTED
**Trigger:** `requestWake()` called by command

```cpp
bool VehicleManager::requestWake() {
    // Skip if already waking/awake
    if (wakeState == WAKING || wakeState == AWAKE) return true;
    
    // Skip if vehicle already awake naturally
    if (state.isAwake()) {
        setWakeState(WakeState::AWAKE);
        startKeepAlive();
        return true;
    }
    
    setWakeState(WakeState::WAKE_REQUESTED);
    lastWakeActivity = millis();
    return true;
}
```

### 2. ASLEEP → AWAKE (Natural Wake)
**Trigger:** CAN activity detected

```cpp
case WakeState::ASLEEP:
    if (state.isAwake()) {
        Serial.println("Vehicle woke up (CAN activity detected)");
        setWakeState(WakeState::AWAKE);
    }
    break;
```

### 3. WAKE_REQUESTED → WAKING
**Trigger:** Wake sequence sent in `loop()`

```cpp
case WakeState::WAKE_REQUESTED:
    sendWakeFrame();        // 0x17330301
    startKeepAlive();       // 0x5A7 @ 500ms
    delay(100);
    sendBapInitFrame();     // 0x1B000067
    setWakeState(WakeState::WAKING);
    break;
```

### 4. WAKING → AWAKE
**Trigger:** CAN activity detected + 2s BAP init wait

```cpp
case WakeState::WAKING:
    if (state.isAwake() && elapsed >= BAP_INIT_WAIT) {
        setWakeState(WakeState::AWAKE);
    }
    break;
```

### 5. WAKING → ASLEEP (Timeout)
**Trigger:** No CAN activity after 10 seconds

```cpp
case WakeState::WAKING:
    if (elapsed > WAKE_TIMEOUT) {
        Serial.println("Wake timeout - no CAN activity");
        stopKeepAlive();
        setWakeState(WakeState::ASLEEP);
        wakeFailed++;
    }
    break;
```

### 6. AWAKE → ASLEEP (Natural Sleep)
**Trigger:** CAN activity stops

```cpp
case WakeState::AWAKE:
    if (!state.isAwake()) {
        Serial.println("Vehicle went to sleep (no CAN activity)");
        stopKeepAlive();
        setWakeState(WakeState::ASLEEP);
    }
    break;
```

---

## Timing Constants

```cpp
KEEPALIVE_INTERVAL = 500ms      // Send keep-alive frequency
KEEPALIVE_TIMEOUT = 300000ms    // Stop after 5 min (no commands)
BAP_INIT_WAIT = 2000ms          // Wait for BAP channel init
WAKE_TIMEOUT = 10000ms          // Give up if no response
```

---

## Example Scenarios

### Scenario 1: User Command (Vehicle Asleep)

```
1. User sends "start climate" via app
2. ChargingProfileManager calls requestWake()
3. State: ASLEEP → WAKE_REQUESTED
4. loop() sends wake sequence
5. State: WAKE_REQUESTED → WAKING
6. Vehicle wakes up, CAN frames appear
7. After 2s: State: WAKING → AWAKE
8. Command sends BAP frames
9. After 5 min (no more commands): Keep-alive stops
10. Vehicle eventually sleeps
11. State: AWAKE → ASLEEP (when CAN stops)
```

### Scenario 2: User Opens Door (Natural Wake)

```
1. Vehicle asleep, device monitoring
2. User opens door
3. Vehicle wakes up, sends CAN frames
4. VehicleState::isAwake() returns true
5. State: ASLEEP → AWAKE (automatic)
6. No wake sequence needed
7. If user sends command: Keep-alive starts
8. User closes door, walks away
9. After 5 min: Keep-alive stops
10. Vehicle sleeps naturally
11. State: AWAKE → ASLEEP (when CAN stops)
```

### Scenario 3: Wake Failure

```
1. Command requests wake
2. State: ASLEEP → WAKE_REQUESTED → WAKING
3. Wake frames sent, keep-alive active
4. Wait 10 seconds...
5. No CAN activity (vehicle didn't respond)
6. State: WAKING → ASLEEP
7. Keep-alive stopped
8. Command fails, event sent to server
```

### Scenario 4: Command While Already Awake

```
1. Vehicle awake (user opened door earlier)
2. State: AWAKE
3. Command requests wake via requestWake()
4. Detects state.isAwake() = true
5. Immediately returns success
6. No wake sequence needed
7. Start keep-alive (prevent sleep)
8. Command sends BAP frames
```

---

## Implementation Notes

### CAN Activity Detection

**VehicleState::isAwake():**
```cpp
bool isAwake() const {
    return (millis() - lastCanActivity < 5000);  // 5 seconds
}

void markCanActivity() {
    lastCanActivity = millis();
    canFrameCount++;
}
```

Called from `VehicleManager::onCanFrame()` on every received frame.

### Thread Safety

- Wake state machine runs on **Core 1** (main loop)
- CAN frame reception runs on **Core 0** (CAN task)
- `VehicleState` is lock-free (atomic reads)
- State machine reads may be slightly stale (acceptable)

### Keep-Alive Persistence

Keep-alive continues across multiple commands:
- Command 1 starts keep-alive
- Command 2 uses existing keep-alive (updates lastWakeActivity)
- Command 3 uses existing keep-alive (updates lastWakeActivity)
- After 5 min idle: Keep-alive stops

---

## Testing Checklist

### Wake Sequence
- [ ] Wake vehicle from sleep
- [ ] Verify wake frames sent (0x17330301, 0x1B000067)
- [ ] Verify keep-alive starts (0x5A7 @ 500ms)
- [ ] Verify BAP init wait (2 seconds)
- [ ] Verify state transitions (ASLEEP → WAKE_REQUESTED → WAKING → AWAKE)

### Natural Wake
- [ ] Open door while device monitoring
- [ ] Verify state transitions to AWAKE automatically
- [ ] No wake sequence sent

### Keep-Alive
- [ ] Send command, verify keep-alive starts
- [ ] Wait 5 min, verify keep-alive stops
- [ ] Send another command, verify keep-alive restarts

### Natural Sleep
- [ ] Vehicle awake, close all doors
- [ ] Wait for vehicle to sleep naturally
- [ ] Verify state transitions to ASLEEP when CAN stops

### Failure Cases
- [ ] Wake timeout (no response after 10s)
- [ ] CAN send failure
- [ ] Multiple wake requests (should not restart)
