# CAN Architecture V2 - Non-Blocking State Machines

## Design Principles

1. **No blocking delays** - Everything uses state machines
2. **Core 0 = RX only** - CAN reception task never blocks
3. **Core 1 = TX + state machines** - Command logic runs in main loop
4. **Single command at a time** - Reject new commands while one is executing
5. **Event-based errors** - Send events to server on failures

---

## Layer Architecture

```
┌─────────────────────────────────────────────────────────┐
│ Layer 4: Application                                    │
│ - VehicleHandler (HTTP/MQTT)                            │
│ - Receives commands, calls channel methods              │
│ - Returns immediate response (command queued)           │
└────────────────────────┬────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│ Layer 3: Protocol Channels & Domains                    │
│                                                          │
│ PASSIVE (Read-only):                                    │
│ - BatteryDomain, BodyDomain, ClimateDomain, etc.        │
│ - Process CAN frames → Update VehicleState              │
│                                                          │
│ ACTIVE (Bidirectional):                                 │
│ - BatteryControlChannel (BAP Device 0x25)               │
│   - RX: Parse BAP messages → Update state               │
│   - TX: Command state machine → Send BAP frames         │
│   - loop(): Process command queue                       │
│                                                          │
│ - ChargingProfileManager (High-level API)               │
│   - Thin wrapper around BatteryControlChannel           │
│   - Manages profile data structures                     │
│   - Delegates commands to channel                       │
└────────────────────────┬────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│ Layer 2: Vehicle Protocol Manager                       │
│ VehicleManager                                           │
│                                                          │
│ RX Path (Core 0):                                       │
│ - onCanFrame() receives frames from CanManager          │
│ - Routes standard IDs → Domains                         │
│ - Routes extended BAP IDs → BapChannelRouter            │
│                                                          │
│ TX Path (Core 1):                                       │
│ - sendCanFrame() immediate send to CanManager           │
│                                                          │
│ Wake State Machine (Core 1):                            │
│ - WakeState: ASLEEP, WAKE_REQUESTED, WAKING, AWAKE      │
│ - requestWake() sets WAKE_REQUESTED flag               │
│ - loop() processes state transitions                    │
│ - Sends wake frames, manages keep-alive                 │
│                                                          │
│ Channel Management:                                      │
│ - Calls loop() on all active channels                   │
└────────────────────────┬────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│ Layer 1: CAN Transport                                  │
│ CanManager                                               │
│ - RX task (Core 0): Dispatch frames to VehicleManager   │
│ - TX: Queue frames to hardware                          │
└─────────────────────────────────────────────────────────┘
```

---

## State Machines

### VehicleManager Wake State Machine

```
States:
- ASLEEP: Vehicle CAN bus inactive
- WAKE_REQUESTED: Command wants wake, need to start sequence
- WAKING: Wake frames sent, waiting for CAN activity
- AWAKE: Vehicle responding, ready for commands

Key Concepts:
- Vehicle can wake naturally (door open, key fob, ignition)
- Vehicle goes to sleep naturally after inactivity
- Keep-alive prevents vehicle sleep ONLY while commands pending
- State transitions based on CAN activity, not our actions

Transitions:
ASLEEP → AWAKE: CAN activity detected (natural wake)
ASLEEP → WAKE_REQUESTED: requestWake() called by channel
WAKE_REQUESTED → WAKING: loop() sends wake frame + BAP init, starts keep-alive
WAKING → AWAKE: loop() detects CAN activity (state.isAwake())
WAKING → ASLEEP: Timeout (10s) without CAN activity
AWAKE → ASLEEP: CAN activity stops (vehicle slept naturally)

Keep-Alive Management:
- Started when we request wake
- Sends 0x5A7 every 500ms while active
- Stops after 5 min timeout (no commands)
- Stopping keep-alive does NOT force ASLEEP
- Vehicle decides when to sleep

API:
- bool requestWake() - Request wake (returns false if already waking/awake)
- bool isAwake() - Check if AWAKE state
- WakeState getWakeState() - Get current state
- void loop() - Process state machine
```

### BatteryControlChannel Command State Machine

```
States:
- IDLE: No pending commands
- REQUESTING_WAKE: Requested vehicle wake
- WAITING_FOR_WAKE: Polling for vehicle awake
- SENDING_COMMAND: Sending BAP frames
- DONE: Command sent successfully
- FAILED: Command failed (wake timeout, send error)

Transitions:
IDLE → REQUESTING_WAKE: startClimate/startCharging/etc called
REQUESTING_WAKE → WAITING_FOR_WAKE: Wake requested
WAITING_FOR_WAKE → SENDING_COMMAND: Vehicle awake
WAITING_FOR_WAKE → FAILED: Wake timeout (15s) or wake failed
SENDING_COMMAND → DONE: All frames sent
SENDING_COMMAND → FAILED: Send error
DONE → IDLE: Next loop cycle
FAILED → IDLE: Error event sent

Pending Commands (flags + data):
- wantStartClimate (temp, allowBattery)
- wantStopClimate
- wantStartCharging (targetSoc, maxCurrent)
- wantStopCharging
- Only ONE can be active at a time

API:
- bool startClimate(temp, allowBattery) - Queue command (returns false if busy)
- bool stopClimate() - Queue command
- bool startCharging(targetSoc, maxCurrent) - Queue command
- bool stopCharging() - Queue command
- bool isBusy() - Check if command in progress
- void loop() - Process state machine
```

---

## Command Flow Example: startClimate()

### Step 1: User Request
```
HTTP POST /vehicle/climate/start
↓
VehicleHandler::handleStartClimate()
↓
manager.batteryControl().startClimate(22.0, true)
↓
Returns: {"status": "queued"} or {"status": "busy"}
```

### Step 2: Channel State Machine (in loop)
```
Main Loop:
  VehicleManager::loop()
    ↓
  BatteryControlChannel::loop()
    
State: IDLE → REQUESTING_WAKE
  - Set wantStartClimate = true
  - Store temp, allowBattery
  - Call manager->requestWake()
  - Set commandState = REQUESTING_WAKE

State: REQUESTING_WAKE → WAITING_FOR_WAKE
  - Wake request accepted
  - Start timeout timer
  - Set commandState = WAITING_FOR_WAKE

State: WAITING_FOR_WAKE → SENDING_COMMAND
  - Check manager->isAwake()
  - If true: commandState = SENDING_COMMAND
  - If timeout: commandState = FAILED, send event

State: SENDING_COMMAND → DONE
  - Build BAP frames
  - Send profile config
  - Send operation trigger
  - Set commandState = DONE
  - Clear wantStartClimate

State: DONE → IDLE
  - Send success event to server
  - Reset state
```

### Step 3: VehicleManager Wake State Machine (parallel in loop)
```
State: ASLEEP → WAKE_REQUESTED
  - requestWake() called
  - Set wakeState = WAKE_REQUESTED

State: WAKE_REQUESTED → WAKING
  - Send wake frame (0x17330301)
  - Send BAP init (0x1B000067)
  - Start keep-alive (0x5A7 every 500ms)
  - Start timeout timer (10s)
  - Set wakeState = WAKING

State: WAKING → AWAKE
  - Monitor state.isAwake() (CAN activity)
  - When active: wakeState = AWAKE
  - If timeout: wakeState = ASLEEP

State: AWAKE → maintain
  - Keep-alive continues (500ms interval)
  - Timeout after 5 min inactivity
```

---

## Event System

### Command Events

Send to server when:
- Command succeeds: `{"event": "command_success", "command": "startClimate"}`
- Command fails: `{"event": "command_failed", "command": "startClimate", "reason": "wake_timeout"}`
- Command rejected: `{"event": "command_rejected", "command": "startClimate", "reason": "busy"}`

Failure Reasons:
- `wake_timeout` - Vehicle didn't respond to wake
- `wake_failed` - Wake request failed
- `send_failed` - CAN frame send failed
- `busy` - Another command in progress

Implementation:
- BatteryControlChannel has callback: `void (*onCommandEvent)(const char* event, const char* command, const char* reason)`
- Set by VehicleManager during initialization
- Callback sends MQTT event to server

---

## Key Benefits

1. **Truly non-blocking** - No delays anywhere, pure state machines
2. **Clear responsibility** - Each layer has defined role
3. **Single command queue** - Simple, predictable behavior
4. **Error handling** - Events notify server of failures
5. **Testable** - State machines easy to unit test
6. **Extensible** - Easy to add more BAP channels

---

## Migration Path

### Phase 1: Refactor VehicleManager
- Remove separate VehicleWakeManager class
- Add wake state machine to VehicleManager
- Keep existing API working

### Phase 2: Refactor BatteryControlChannel
- Add command state machine
- Add loop() method
- Keep ChargingProfileManager as wrapper

### Phase 3: Add Event System
- Add callback mechanism
- Wire up to MQTT/HTTP handlers
- Add error handling

### Phase 4: Testing
- Verify non-blocking behavior
- Test command queueing
- Test wake sequence
- Test error scenarios

---

## Open Questions

1. **ChargingProfileManager**: Keep as thin wrapper or merge into BatteryControlChannel?
   - Pros of keeping: Separation of concerns, profile scheduling logic separate
   - Cons: Extra layer
   - **Decision: Keep as thin wrapper initially, merge later if too simple**

2. **Multiple BAP channels**: How to handle multiple channels wanting wake?
   - Current: First come first serve
   - Alternative: Priority system
   - **Decision: YAGNI - handle when we add second channel**

3. **Command cancellation**: Should we support cancelling queued commands?
   - **Decision: Not needed for V1, add later if required**
