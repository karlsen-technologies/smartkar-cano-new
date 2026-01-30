# Command Handling System Refactoring Plan

**Author**: Planning Session  
**Date**: 2026-01-30  
**Status**: APPROVED - Ready for Implementation  
**Protocol Version**: v2.1 → v2.2

---

## Executive Summary

This document describes a comprehensive refactoring of the command handling system to provide full lifecycle tracking, progress visibility, and better error handling. The current "fire and forget" approach with disconnected events will be replaced with a stateful command tracking system that sends linked responses throughout the command lifecycle.

---

## Table of Contents

1. [Background](#background)
2. [Goals](#goals)
3. [Architecture Overview](#architecture-overview)
4. [Command Flow](#command-flow)
5. [Protocol Changes](#protocol-changes)
6. [Implementation Components](#implementation-components)
7. [Design Decisions](#design-decisions)
8. [Benefits](#benefits)
9. [Implementation Phases](#implementation-phases)
10. [Examples](#examples)

---

## Background

### Current System Problems

The current command handling system (Protocol v2.1) has several limitations discovered during real-world car testing:

1. **No Command Correlation**: Commands return immediately with `status: "queued"`, then later send events like `commandCompleted` without including the original command ID. The server has to guess which command completed.

2. **Hidden Command Lifecycle**: The device has a sophisticated internal state machine (IDLE → REQUESTING_WAKE → WAITING_FOR_WAKE → UPDATING_PROFILE → SENDING_COMMAND → DONE/FAILED) but the server only sees "queued" and then later a disconnected event.

3. **No Progress Tracking**: Long-running commands (wake vehicle → update profile → send command) give no visibility into progress. The server doesn't know if we're starting or nearly done.

4. **Event Ambiguity**: Events are fire-and-forget. If the server misses an event or receives events out of order, it loses track of command status.

5. **Poor Error Context**: When a command fails, the server receives a generic `commandFailed` event but doesn't know at which stage it failed or why.

### Current Protocol Flow

```json
// Server sends command
→ {"type":"command","data":{"id":12,"action":"vehicle.startClimate","temp":21.0}}

// Device responds immediately
← {"type":"response","data":{"id":12,"ok":true,"status":"queued"}}

// Later: Event with no command ID link
← {"type":"event","data":{"domain":"vehicle","name":"climateStartRequested","temperature":21.0}}

// Even later: Completion event (also no command ID!)
← {"type":"event","data":{"domain":"vehicle","name":"commandCompleted","action":"startClimate"}}
```

**Problems:**
- No way to correlate events to command ID 12
- If server issues commands 12 and 13 quickly, events get confused
- No progress visibility
- Command state hidden from server

### What Works Well

The current system has good foundations to build upon:

- **Single Command Enforcement**: `BatteryControlChannel` already enforces "only one command at a time" via `isBusy()` checks
- **Internal State Machine**: Well-structured state transitions (CommandState enum)
- **Non-blocking Design**: Commands are queued and processed asynchronously in loop()

---

## Goals

### Primary Objectives

1. **Full Lifecycle Tracking**: Server can track a command from acceptance → in-progress → completed/failed with visibility into each stage

2. **Command Correlation**: All progress updates and final status linked to the original command ID

3. **Progress Visibility**: Server knows what stage the command is in (requesting_wake, updating_profile, sending_command)

4. **Global State Management**: Centralized "current command" state that can be queried from anywhere in the system

5. **Better Busy Responses**: When rejecting a command because busy, tell the server what command is currently running and its status

6. **Consistency**: ALL commands go through the same tracking system, regardless of how fast they execute

### Non-Goals

- Command queueing (still only one command at a time)
- Cancellation of in-progress commands
- Command priority or pre-emption
- Persistent command history across reboots

---

## Architecture Overview

### High-Level Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                         Server                               │
└─────────────────────┬───────────────────────────────────────┘
                      │ TCP (JSON)
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  LinkManager                                                 │
│  - Receives command JSON                                     │
│  - Parses and extracts id, action, params                    │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  CommandRouter                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 1. Check: hasActiveCommand()?                          │ │
│  │    YES → Send busy response, RETURN                    │ │
│  │    NO  → Continue                                      │ │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 2. Start tracking: startCommand(id, action)            │ │
│  │    CommandStateManager sends first response            │ │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 3. Route to appropriate handler                        │ │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 4. Handle result:                                      │ │
│  │    - PENDING: command continues in background          │ │
│  │    - OK: completeCommand()                             │ │
│  │    - ERROR: failCommand(reason)                        │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  Handler (VehicleHandler, SystemHandler, etc)                │
│  - Validate parameters                                       │
│  - Call appropriate channel/manager                          │
│  - Return result (PENDING, OK, ERROR, INVALID_PARAMS)        │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  BatteryControlChannel / ChargingProfileManager              │
│  - Execute state machine                                     │
│  - Update CommandStateManager at each stage transition       │
│  - Complete or fail command when done                        │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  CommandStateManager (NEW - Singleton)                       │
│  - Tracks single active command (ID, action, stage)          │
│  - Sends response updates on stage changes                   │
│  - Sends final completion/failure response                   │
│  - Provides current command info for busy responses          │
└──────────────────────────────────────────────────────────────┘
```

### Key Principle: Single Source of Truth

`CommandStateManager` is the ONLY place that tracks which command is currently active. All components check it before accepting new commands.

---

## Command Flow

### Complete Flow: vehicle.startClimate

Let's trace a climate start command from server to completion.

#### Step 1: Server Sends Command

```json
→ {"type":"command","data":{"id":12,"action":"vehicle.startClimate","temp":21.0}}
```

#### Step 2: LinkManager Receives & Parses

- Receives JSON over TCP
- Parses message, extracts: `id=12`, `action="vehicle.startClimate"`, `params={"temp":21.0}`
- Calls `CommandRouter::handleCommand(action, id, params)`

#### Step 3: CommandRouter - Busy Check

```
1. Check if another command is active:
   if (CommandStateManager::getInstance()->hasActiveCommand()) {
       // Build busy response with current command info
       sendBusyResponse(id);
       return;
   }
```

If another command (e.g., ID 10) is active, server receives:

```json
← {
    "type":"response",
    "data":{
      "id":12,
      "ok":false,
      "status":"busy",
      "error":"Another command is in progress",
      "currentCommand":{
        "id":10,
        "action":"vehicle.stopCharging",
        "stage":"sending_command",
        "elapsedMs":2300
      }
    }
  }
```

Command rejected early - handler never called!

#### Step 4: CommandRouter - Start Tracking

```
2. No active command - start tracking this one:
   CommandStateManager::getInstance()->startCommand(12, "vehicle.startClimate");
```

CommandStateManager immediately sends first response:

```json
← {"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}
```

Server now knows: Command 12 accepted and is being processed.

#### Step 5: CommandRouter - Route to Handler

```
3. Parse action: "vehicle.startClimate"
   - domain = "vehicle"
   - actionName = "startClimate"

4. Find handler for domain "vehicle" → VehicleHandler

5. Create CommandContext:
   - ctx.id = 12
   - ctx.actionName = "startClimate"
   - ctx.params = {"temp": 21.0}

6. Call: result = VehicleHandler::handleCommand(ctx)
```

#### Step 6: VehicleHandler - Validate & Execute

```
VehicleHandler::handleStartClimate(ctx):

1. Extract temperature parameter: temp = 21.0
2. Validate range (15.5-30.0°C): ✓
3. Extract allowBattery parameter: allowBattery = true

4. Call channel:
   bool accepted = vehicleManager->batteryControl().startClimate(12, 21.0, true)

5. If accepted:
   return CommandResult::pending()
   
6. If rejected (shouldn't happen):
   return CommandResult::error("Internal error")
```

**Note**: No busy check in handler! CommandRouter already did that.

#### Step 7: CommandRouter - Handle PENDING Result

```
result = handler->handleCommand(ctx);

if (result.status == PENDING) {
    // Command accepted and is now executing
    // CommandStateManager will send further updates
    // Nothing more to do here
    return;
}
```

#### Step 8: BatteryControlChannel - State Machine Execution

The channel's state machine runs in `loop()`:

**State: REQUESTING_WAKE**
```
1. Call manager->requestWake()
2. Transition to WAITING_FOR_WAKE
3. Update CommandStateManager:
   CommandStateManager::getInstance()->updateStage(REQUESTING_WAKE)
```

Server receives:
```json
← {"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"requesting_wake"}}
```

**State: WAITING_FOR_WAKE**
```
1. Check if vehicle awake (CAN frames received?)
2. If YES: transition to UPDATING_PROFILE
3. If NO and timeout (15s): transition to FAILED
4. Update CommandStateManager:
   CommandStateManager::getInstance()->updateStage(WAITING_FOR_WAKE)
```

Server receives:
```json
← {"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}
```

**State: UPDATING_PROFILE**
```
1. Check if Profile 0 needs update (temperature, operation mode)
2. If YES: request profile update
3. When complete: transition to SENDING_COMMAND
4. Update CommandStateManager:
   CommandStateManager::getInstance()->updateStage(UPDATING_PROFILE)
```

Server receives:
```json
← {"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}
```

**State: SENDING_COMMAND**
```
1. Build BAP frame with climate start command
2. Send CAN frame to vehicle
3. If successful: transition to DONE
4. If failed: transition to FAILED
5. Update CommandStateManager:
   CommandStateManager::getInstance()->updateStage(SENDING_COMMAND)
```

Server receives:
```json
← {"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}
```

**State: DONE**
```
1. Notify CommandStateManager:
   CommandStateManager::getInstance()->completeCommand()
   
2. Clear pending command:
   pendingCommand = NONE
   
3. Transition to IDLE
```

Server receives:
```json
← {"type":"response","data":{"id":12,"ok":true,"status":"completed"}}
```

Command complete! CommandStateManager clears state. Device ready for next command.

#### Summary: Complete Message Sequence

```
Server → Device: {"id":12, "action":"vehicle.startClimate", "temp":21.0}

Device → Server: {"id":12, "status":"in_progress", "stage":"accepted"}
Device → Server: {"id":12, "status":"in_progress", "stage":"requesting_wake"}
Device → Server: {"id":12, "status":"in_progress", "stage":"waiting_for_wake"}
Device → Server: {"id":12, "status":"in_progress", "stage":"updating_profile"}
Device → Server: {"id":12, "status":"in_progress", "stage":"sending_command"}
Device → Server: {"id":12, "status":"completed"}
```

All responses linked by ID 12. Server has full visibility.

---

### Alternative Flow: Validation Failure

What if temperature is invalid?

```
Server → Device: {"id":12, "action":"vehicle.startClimate", "temp":50.0}

1. CommandRouter: startCommand(12, ...)
   Device → Server: {"id":12, "status":"in_progress", "stage":"accepted"}

2. VehicleHandler validates: temp=50.0 out of range (15.5-30.0)
   Returns: CommandResult::invalidParams("Temperature must be 15.5-30.0°C")

3. CommandRouter handles INVALID_PARAMS:
   CommandStateManager::getInstance()->failCommand("Temperature must be 15.5-30.0°C")
   
   Device → Server: {
     "id":12,
     "ok":false,
     "status":"failed",
     "stage":"accepted",
     "error":"Temperature must be 15.5-30.0°C"
   }

4. CommandStateManager clears state (no active command)
```

**Server perspective:**
- Received "accepted" → device got the command
- Immediately received "failed" → validation error
- Error message tells user what's wrong
- Can debug: "Did server send 50.0? Or did device receive it wrong?"

---

### Alternative Flow: Execution Failure

What if vehicle never wakes up?

```
Server → Device: {"id":12, "action":"vehicle.startClimate", "temp":21.0}

Device → Server: {"id":12, "status":"in_progress", "stage":"accepted"}
Device → Server: {"id":12, "status":"in_progress", "stage":"requesting_wake"}
Device → Server: {"id":12, "status":"in_progress", "stage":"waiting_for_wake"}

... 15 seconds pass, no response from vehicle ...

BatteryControlChannel detects timeout:
  - setCommandState(FAILED)
  - CommandStateManager::getInstance()->failCommand("Wake timeout after 15000ms")

Device → Server: {
  "id":12,
  "ok":false,
  "status":"failed",
  "stage":"waiting_for_wake",
  "error":"Wake timeout after 15000ms",
  "elapsedMs":15100
}
```

**Server perspective:**
- Command progressed through stages normally
- Stuck in "waiting_for_wake" stage
- Failed after 15 seconds with clear reason
- Can show user: "Vehicle didn't wake up - may be out of range or in deep sleep"

---

### Alternative Flow: Instant Synchronous Command

What about fast commands like `vehicle.getState`?

```
Server → Device: {"id":13, "action":"vehicle.getState"}

1. CommandRouter: startCommand(13, "vehicle.getState")
   Device → Server: {"id":13, "status":"in_progress", "stage":"accepted"}

2. VehicleHandler executes immediately (< 10ms):
   - Reads current VehicleState
   - Returns CommandResult::ok() with data

3. CommandRouter handles OK:
   CommandStateManager::getInstance()->completeCommand()
   
   Device → Server: {
     "id":13,
     "ok":true,
     "status":"completed",
     "battery":{...},
     "drive":{...},
     "climate":{...}
   }
```

**Two responses** (accepted + completed) but they arrive within milliseconds of each other. Server processes them as one logical operation.

**Benefit**: Even for instant commands, server knows device received it. If only "accepted" arrives and no "completed", server knows something went wrong.

---

## Protocol Changes

### Response Status Values

#### Current Status Values (v2.1)

| Status | Description |
|--------|-------------|
| *(ok: true)* | Command executed successfully |
| `error` | Command failed |
| `pending` | Command accepted, will complete async |
| `not_supported` | Unknown command |

#### New Status Values (v2.2)

| Status | ok | Description | Use Case |
|--------|-----|-------------|----------|
| *(no status field)* | true | Immediate success (rare, legacy) | Very simple commands |
| `in_progress` | true | Command executing, see stage | All tracked commands during execution |
| `completed` | true | Command finished successfully | Final response for successful commands |
| `failed` | false | Command failed, see error & stage | Any failure (validation or execution) |
| `busy` | false | Another command in progress | Command rejected due to active command |
| `error` | false | General error | Generic errors |
| `not_supported` | false | Unknown command/domain | Unknown action |

**Key Changes:**
- **NEW**: `in_progress` - replaces ambiguous `pending`, includes stage information
- **NEW**: `completed` - clear final status for successful async commands
- **NEW**: `busy` - distinct status for rejection due to active command (not generic `error`)

### Stage Names

Stages reported in `in_progress` responses:

| Stage | Description |
|-------|-------------|
| `accepted` | Command accepted, about to start |
| `requesting_wake` | Requesting vehicle wake |
| `waiting_for_wake` | Waiting for vehicle to respond |
| `updating_profile` | Updating charging/climate profile |
| `sending_command` | Sending BAP frames to vehicle |

**Format**: `snake_case` (matches existing protocol conventions)

**Note**: Not all commands go through all stages. Fast commands may only show `accepted` → `completed`.

### Response Message Schemas

#### In-Progress Response

```typescript
{
  type: "response",
  data: {
    id: number,              // Command ID
    ok: true,
    status: "in_progress",
    stage: string,           // Current stage name
    elapsedMs?: number       // Optional: milliseconds since command started
  }
}
```

Example:
```json
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}
```

#### Completed Response

```typescript
{
  type: "response",
  data: {
    id: number,              // Command ID
    ok: true,
    status: "completed",
    elapsedMs?: number,      // Optional: total time taken
    ...additionalData        // Command-specific response data
  }
}
```

Example:
```json
{"type":"response","data":{"id":12,"ok":true,"status":"completed","elapsedMs":5234}}
```

#### Failed Response

```typescript
{
  type: "response",
  data: {
    id: number,              // Command ID
    ok: false,
    status: "failed",
    stage: string,           // Stage where failure occurred
    error: string,           // Human-readable error message
    elapsedMs?: number       // Optional: time until failure
  }
}
```

Example:
```json
{
  "type":"response",
  "data":{
    "id":12,
    "ok":false,
    "status":"failed",
    "stage":"waiting_for_wake",
    "error":"Wake timeout after 15000ms",
    "elapsedMs":15100
  }
}
```

#### Busy Response

```typescript
{
  type: "response",
  data: {
    id: number,              // Rejected command ID
    ok: false,
    status: "busy",
    error: string,           // "Another command is in progress"
    currentCommand: {
      id: number,            // Active command ID
      action: string,        // Active command action
      stage: string,         // Current stage
      elapsedMs: number      // Time active
    }
  }
}
```

Example:
```json
{
  "type":"response",
  "data":{
    "id":15,
    "ok":false,
    "status":"busy",
    "error":"Another command is in progress",
    "currentCommand":{
      "id":12,
      "action":"vehicle.startClimate",
      "stage":"updating_profile",
      "elapsedMs":3200
    }
  }
}
```

---

## Implementation Components

### 1. CommandStateManager (NEW)

**File**: `src/core/CommandStateManager.h`, `src/core/CommandStateManager.cpp`

**Purpose**: Global singleton that tracks the single active command and sends response updates.

**State Variables**:
```cpp
int currentCommandId = -1;           // -1 = no active command
String currentAction = "";           // e.g., "vehicle.startClimate"
Stage currentStage = Stage::NONE;    // Current execution stage
unsigned long commandStartTime = 0;  // millis() when command started
unsigned long lastUpdateSent = 0;    // millis() when last update sent
String failureReason = "";           // Set when command fails
```

**Stage Enum**:
```cpp
enum class Stage {
    NONE,              // No active command
    ACCEPTED,          // Command accepted
    REQUESTING_WAKE,   // Requesting vehicle wake
    WAITING_FOR_WAKE,  // Waiting for wake confirmation
    UPDATING_PROFILE,  // Updating charging/climate profile
    SENDING_COMMAND,   // Sending BAP frames
    COMPLETED,         // Success (cleared immediately)
    FAILED             // Failure (cleared immediately)
};
```

**Key Methods**:

```cpp
class CommandStateManager {
public:
    // Singleton access
    static CommandStateManager* getInstance();
    
    // Check if a command is currently active
    bool hasActiveCommand() const;
    
    // Get current command ID (-1 if none)
    int getCurrentCommandId() const;
    
    // Start tracking a new command
    // Stores command info and sends first "accepted" response
    void startCommand(int commandId, const String& action);
    
    // Update current stage
    // Sends "in_progress" response with new stage
    void updateStage(Stage stage);
    
    // Complete command successfully
    // Sends "completed" response and clears state
    void completeCommand();
    
    // Fail command with reason
    // Sends "failed" response and clears state
    void failCommand(const char* reason);
    
    // Get current command info (for busy responses)
    void getCurrentCommandInfo(JsonObject& obj) const;
    
    // Get stage as string (for responses)
    const char* getStageString(Stage stage) const;
    
private:
    static CommandStateManager* _instance;
    
    // Send response via CommandRouter
    void sendResponse(const char* status, bool ok, const char* error = nullptr);
    
    // State variables (as above)
    int currentCommandId;
    String currentAction;
    Stage currentStage;
    unsigned long commandStartTime;
    // ...
};
```

**Configuration**:
- No periodic updates (only on stage change)
- No command timeout at this level (handled by BatteryControlChannel state machine)

---

### 2. CommandRouter (MODIFIED)

**File**: `src/core/CommandRouter.h`, `src/core/CommandRouter.cpp`

**Changes Required**:

#### In `handleCommand()` method:

**BEFORE routing to handler:**
```cpp
void CommandRouter::handleCommand(const String& action, int id, JsonObject& params) {
    // NEW: Check if another command is active
    CommandStateManager* csm = CommandStateManager::getInstance();
    if (csm->hasActiveCommand()) {
        // Build busy response with current command info
        JsonDocument busyDoc;
        JsonObject busyData = busyDoc["data"].to<JsonObject>();
        busyData["id"] = id;
        busyData["ok"] = false;
        busyData["status"] = "busy";
        busyData["error"] = "Another command is in progress";
        
        JsonObject currentCmd = busyData["currentCommand"].to<JsonObject>();
        csm->getCurrentCommandInfo(currentCmd);
        
        // Send and return
        responseSender(busyDoc);
        return;
    }
    
    // NEW: Start tracking this command
    csm->startCommand(id, action);
    
    // Existing: Parse action, find handler, create context
    String domain, actionName;
    parseAction(action, domain, actionName);
    ICommandHandler* handler = findHandler(domain);
    // ...
    CommandContext ctx(id, action, domain, actionName, params);
    
    // Execute handler
    CommandResult result = handler->handleCommand(ctx);
    
    // NEW: Handle different result statuses
    if (result.status == CommandStatus::PENDING) {
        // Command accepted and executing in background
        // CommandStateManager will send further updates
        return;
    }
    
    if (result.status == CommandStatus::OK) {
        // Synchronous command completed immediately
        csm->completeCommand();
        
        // Send response with any additional data
        sendResponse(id, CommandStatus::OK, result.message.c_str(), 
                     result.data.size() > 0 ? &result.data : nullptr);
        return;
    }
    
    if (result.status == CommandStatus::INVALID_PARAMS ||
        result.status == CommandStatus::NOT_SUPPORTED ||
        result.status == CommandStatus::CMD_ERROR) {
        // Validation or execution error
        csm->failCommand(result.message.c_str());
        return;
    }
}
```

**Key Points**:
- Busy check happens BEFORE routing to handler (early rejection)
- `startCommand()` called for ALL commands (consistency)
- Handler never sees busy commands
- CommandStateManager handles all response sending for tracked commands

---

### 3. VehicleHandler (MODIFIED)

**File**: `src/handlers/VehicleHandler.h`, `src/handlers/VehicleHandler.cpp`

**Changes Required**:

#### Remove Busy Checks

Handlers no longer check if busy - CommandRouter does that.

#### Pass Command ID to Channels

All command methods need to pass `ctx.id` to channels:

**BEFORE:**
```cpp
bool queued = vehicleManager->batteryControl().startClimate(tempCelsius, allowBattery);
```

**AFTER:**
```cpp
bool accepted = vehicleManager->batteryControl().startClimate(ctx.id, tempCelsius, allowBattery);
```

#### Remove Manual Event Emissions

Delete code that manually emits events like:
```cpp
// DELETE THIS:
commandRouter->sendEvent("vehicle", "climateStartRequested", &details);
```

CommandStateManager now handles all command lifecycle communication.

#### Keep Vehicle State Events

**DO NOT DELETE** events for vehicle state changes:
- `chargingStarted` / `chargingStopped` - when vehicle starts/stops charging (not in response to our command)
- `doorOpened` / `doorClosed` - when doors open/close
- `plugged` / `unplugged` - when charger plugged/unplugged
- `ignitionOn` / `ignitionOff` - when ignition state changes
- etc.

These are **vehicle state events**, not **command lifecycle events**. They stay.

#### Example: Updated handleStartClimate()

```cpp
CommandResult VehicleHandler::handleStartClimate(CommandContext& ctx) {
    // Extract and validate parameters
    float tempCelsius = 21.0f;
    if (ctx.params["temp"].is<float>()) {
        tempCelsius = ctx.params["temp"].as<float>();
    }
    
    // Validate range
    if (tempCelsius < 15.5f || tempCelsius > 30.0f) {
        return CommandResult::invalidParams("Temperature must be 15.5-30.0°C");
    }
    
    bool allowBattery = ctx.params["allowBattery"] | true;
    
    // Call channel - pass command ID!
    bool accepted = vehicleManager->batteryControl().startClimate(
        ctx.id,           // NEW: Pass command ID
        tempCelsius,
        allowBattery
    );
    
    if (accepted) {
        // Command accepted and will execute in background
        return CommandResult::pending();
    } else {
        // Shouldn't happen (CommandRouter checks busy), but handle anyway
        return CommandResult::error("Internal error - command rejected");
    }
}
```

---

### 4. BatteryControlChannel (MODIFIED)

**File**: `src/vehicle/bap/channels/BatteryControlChannel.h`, `src/vehicle/bap/channels/BatteryControlChannel.cpp`

**Changes Required**:

#### Add Command ID Parameter

All command methods need to accept `commandId`:

**BEFORE:**
```cpp
bool startClimate(float tempCelsius = 21.0f, bool allowBattery = false);
bool stopClimate();
bool startCharging(uint8_t targetSoc = 80, uint8_t maxCurrent = 32);
bool stopCharging();
```

**AFTER:**
```cpp
bool startClimate(int commandId, float tempCelsius = 21.0f, bool allowBattery = false);
bool stopClimate(int commandId);
bool startCharging(int commandId, uint8_t targetSoc = 80, uint8_t maxCurrent = 32);
bool stopCharging(int commandId);
```

#### Remove Local Busy Check

Delete local `isBusy()` check - CommandRouter already checked:

**DELETE:**
```cpp
if (isBusy()) {
    Serial.println("[BatteryControl] Busy - command rejected");
    return false;
}
```

#### Confirm Command with CommandStateManager

At start of command method, just store the command ID for debugging:

```cpp
bool BatteryControlChannel::startClimate(int commandId, float tempCelsius, bool allowBattery) {
    // Store command info for logging
    currentCommandId = commandId;  // NEW: Add this member variable
    
    // Store parameters
    pendingCommand = START_CLIMATE;
    pendingTempCelsius = tempCelsius;
    pendingAllowBattery = allowBattery;
    
    // Begin execution
    setCommandState(REQUESTING_WAKE);
    
    return true;
}
```

#### Update CommandStateManager on Stage Transitions

In `setCommandState()` method:

```cpp
void BatteryControlChannel::setCommandState(CommandState newState) {
    commandState = newState;
    commandStateStartTime = millis();
    
    // NEW: Update CommandStateManager
    CommandStateManager* csm = CommandStateManager::getInstance();
    
    switch (newState) {
        case CommandState::REQUESTING_WAKE:
            csm->updateStage(CommandStateManager::Stage::REQUESTING_WAKE);
            break;
            
        case CommandState::WAITING_FOR_WAKE:
            csm->updateStage(CommandStateManager::Stage::WAITING_FOR_WAKE);
            break;
            
        case CommandState::UPDATING_PROFILE:
            csm->updateStage(CommandStateManager::Stage::UPDATING_PROFILE);
            break;
            
        case CommandState::SENDING_COMMAND:
            csm->updateStage(CommandStateManager::Stage::SENDING_COMMAND);
            break;
            
        case CommandState::DONE:
            csm->completeCommand();
            break;
            
        case CommandState::FAILED:
            // Get failure reason from last error or default
            const char* reason = "Command execution failed";
            if (/* timeout detected */) {
                reason = "Wake timeout after 15000ms";
            } else if (/* profile update failed */) {
                reason = "Profile update failed";
            } else if (/* send failed */) {
                reason = "Failed to send command to vehicle";
            }
            csm->failCommand(reason);
            break;
            
        case CommandState::IDLE:
            // No update needed
            break;
    }
}
```

#### Remove Manual Event Emissions

Delete all calls to `emitCommandEvent()` - these are now handled by CommandStateManager:

**DELETE:**
```cpp
emitCommandEvent("commandCompleted", nullptr);
emitCommandEvent("commandFailed", "wake_timeout");
```

---

### 5. ChargingProfileHandler (MODIFIED)

**File**: `src/handlers/ChargingProfileHandler.h`, `src/handlers/ChargingProfileHandler.cpp`

**Changes Required**:

Same pattern as VehicleHandler:
- Remove busy checks
- Pass command ID to ChargingProfileManager
- Remove manual event emissions
- Return PENDING for async operations

Profile updates can also be tracked through the same system for consistency.

---

### 6. SystemHandler (NO CHANGES)

**File**: `src/handlers/SystemHandler.h`, `src/handlers/SystemHandler.cpp`

System commands like `system.reboot` and `system.sleep` are immediate and don't need progress tracking. They can continue to return `OK` immediately.

However, they will still go through CommandStateManager (send "accepted" then immediately "completed") for consistency. This is acceptable since the extra message overhead is negligible for these rare commands.

---

## Design Decisions

### Decision 1: Update Frequency ✓ APPROVED

**Question**: How often should updates be sent?

**Decision**: **Option A - On stage change only**

Updates sent only when command transitions to a new stage:
- `accepted` → `requesting_wake` → `waiting_for_wake` → `updating_profile` → `sending_command` → `completed`

Typically 3-6 messages per command.

**Rationale**:
- Minimal network traffic
- Clear stage boundaries
- Server gets useful information at each update
- No spam during long-running stages

**Rejected Alternatives**:
- Periodic updates (every 2s) - unnecessary traffic
- Heartbeat in addition to stage changes - overcomplicated

---

### Decision 2: Event System Migration ✓ APPROVED

**Question**: Should we keep existing command events or replace them?

**Decision**: **Option A - Replace command events entirely**

- **REMOVE** all command lifecycle events:
  - `climateStartRequested`
  - `chargingStartRequested`
  - `commandCompleted`
  - `commandFailed`
  
- **KEEP** all vehicle state change events:
  - `chargingStarted` / `chargingStopped`
  - `plugged` / `unplugged`
  - `doorOpened` / `doorClosed`
  - `ignitionOn` / `ignitionOff`
  - `locked` / `unlocked`
  - `socThreshold`
  - `chargingComplete`
  - etc.

**Rationale**:
- Clean separation: stateful responses for commands, events for vehicle state
- No redundancy
- Better long-term design
- Server must be updated, but that's acceptable

**Note**: This is a breaking change. Server implementations must be updated before deploying new firmware.

---

### Decision 3: Command Timeout ✓ APPROVED

**Question**: Should commands auto-fail after a timeout?

**Decision**: **Option A - Auto-fail after 60 seconds**

CommandStateManager will automatically fail any command that runs longer than 60 seconds.

**Rationale**:
- Prevents stuck commands blocking forever
- 60s is very conservative (vehicle wake typically 5-15s, profile update 2-5s, total ~20s typical)
- Provides safety net for unexpected failures
- Can be made configurable later if needed

**Implementation**: CommandStateManager checks elapsed time in `loop()` (if we add one) or state machine checks in BatteryControlChannel already have per-stage timeouts.

**Note**: Individual stages already have timeouts (e.g., wake timeout = 15s). The 60s is a global safety net.

---

### Decision 4: Progress Granularity ✓ APPROVED

**Question**: Should we include numeric progress percentages?

**Decision**: **Option C - Stage names only, no percentages**

Responses include stage name only:
```json
{"id":12, "status":"in_progress", "stage":"waiting_for_wake"}
```

No `progress` field with percentage.

**Rationale**:
- Avoids false precision (we don't actually know if we're 20% or 40% done during wake)
- Stage names are clear and meaningful
- Server can implement its own progress UI if desired (e.g., "stage 2 of 5")
- Simpler protocol
- Can add percentages later if truly needed

**Rejected Alternative**:
- 4-stage progress (0%, 25%, 50%, 75%, 100%) - gives false sense of precision

---

### Decision 5: All Commands Tracked ✓ APPROVED

**Question**: Should fast synchronous commands also be tracked?

**Decision**: **Yes - ALL commands go through CommandStateManager**

Even instant commands like `vehicle.getState` will receive:
1. `{"id":13, "status":"in_progress", "stage":"accepted"}`
2. `{"id":13, "status":"completed", ...data}`

**Rationale**:
- **Consistency**: All commands work the same way
- **Debugging**: Server always knows device received command (even if it completes instantly)
- **Simplicity**: No special cases in code
- **Minimal cost**: Two small messages arrive within milliseconds

**Alternative considered**: Skip tracking for synchronous commands - rejected because it adds complexity and special cases.

---

## Benefits

### For the Device Firmware

✅ **Single source of truth**: CommandStateManager is the one place that knows command state  
✅ **Cleaner separation**: CommandRouter handles routing, CommandStateManager handles state  
✅ **Easier debugging**: Can query current command from anywhere in code  
✅ **Automatic timeout**: Commands can't get stuck forever  
✅ **Consistent behavior**: All commands handled the same way  

### For the Server

✅ **Full lifecycle visibility**: Track command from start to finish  
✅ **Progress UI**: Can show "Waking vehicle...", "Updating profile...", etc  
✅ **Better error messages**: "Failed to wake vehicle after 15s" instead of generic "command failed"  
✅ **Command correlation**: Never lose track of which command is which  
✅ **Smarter decisions**: Don't send new commands when device is busy  
✅ **Diagnostics**: Can see exactly where commands are getting stuck  

### For End Users

✅ **Better feedback**: UI shows actual progress instead of generic spinner  
✅ **Clearer errors**: "Vehicle didn't wake up" vs "Error occurred"  
✅ **Less confusion**: UI always shows current state  
✅ **Better reliability**: Stuck commands detected and reported  

---

## Implementation Phases

### Phase 1: Core Infrastructure (Week 1)

**Deliverables**:
- Create `CommandStateManager.h` and `CommandStateManager.cpp`
- Implement singleton pattern
- Implement stage tracking and response sending
- Write unit tests (if test framework available)

**Time Estimate**: 8 hours

---

### Phase 2: CommandRouter Integration (Week 2)

**Deliverables**:
- Modify `CommandRouter::handleCommand()` to check busy state
- Add `startCommand()` call before routing
- Handle handler result statuses (PENDING, OK, ERROR, etc)
- Implement enhanced busy response with current command info

**Time Estimate**: 6 hours

---

### Phase 3: Handler Updates (Week 2)

**Deliverables**:
- Update `VehicleHandler` to pass command IDs
- Remove manual event emissions from handlers
- Update return values (use PENDING appropriately)
- Update `ChargingProfileHandler` similarly

**Time Estimate**: 4 hours

---

### Phase 4: Channel Integration (Week 3)

**Deliverables**:
- Update `BatteryControlChannel` command methods to accept command ID
- Remove local busy checks
- Add `updateStage()` calls in state machine
- Update failure handling to call `failCommand()` with reasons

**Time Estimate**: 6 hours

---

### Phase 5: Protocol Documentation (Week 4)

**Deliverables**:
- Update `docs/protocol.md` with new response formats
- Document new status values and stages
- Add examples of progress tracking flows
- Document busy response format
- Increment protocol version to 2.2
- Add migration guide for server implementations

**Time Estimate**: 4 hours

---

### Phase 6: Testing & Refinement (Week 4-5)

**Deliverables**:
- Test basic command flow (startClimate, stopClimate, etc)
- Test busy rejection
- Test validation failures
- Test execution failures (wake timeout, etc)
- Test rapid commands
- Real vehicle testing in car
- Fix any issues discovered

**Time Estimate**: 12 hours

---

**Total Estimated Time**: ~40 hours (assuming single developer, part-time work)

---

## Examples

### Example 1: Successful Climate Start (Vehicle Awake)

```
[T+0ms] Server → Device
{"type":"command","data":{"id":12,"action":"vehicle.startClimate","temp":21.0}}

[T+10ms] Device → Server (Accepted)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+50ms] Device → Server (Updating Profile)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}

[T+2200ms] Device → Server (Sending Command)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}

[T+2400ms] Device → Server (Completed)
{"type":"response","data":{"id":12,"ok":true,"status":"completed"}}
```

**Total time**: 2.4 seconds  
**Messages**: 4

---

### Example 2: Successful Climate Start (Vehicle Sleeping)

```
[T+0ms] Server → Device
{"type":"command","data":{"id":12,"action":"vehicle.startClimate","temp":21.0}}

[T+10ms] Device → Server (Accepted)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+50ms] Device → Server (Requesting Wake)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

[T+100ms] Device → Server (Waiting for Wake)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

[T+3000ms] Device → Server (Updating Profile - vehicle woke up!)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}

[T+5200ms] Device → Server (Sending Command)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}

[T+5400ms] Device → Server (Completed)
{"type":"response","data":{"id":12,"ok":true,"status":"completed"}}
```

**Total time**: 5.4 seconds  
**Messages**: 6

---

### Example 3: Wake Timeout Failure

```
[T+0ms] Server → Device
{"type":"command","data":{"id":13,"action":"vehicle.startCharging","targetSoc":80}}

[T+10ms] Device → Server (Accepted)
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+50ms] Device → Server (Requesting Wake)
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

[T+100ms] Device → Server (Waiting for Wake)
{"type":"response","data":{"id":13,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

... vehicle never responds ...

[T+15100ms] Device → Server (Failed)
{
  "type":"response",
  "data":{
    "id":13,
    "ok":false,
    "status":"failed",
    "stage":"waiting_for_wake",
    "error":"Wake timeout after 15000ms",
    "elapsedMs":15100
  }
}
```

**Total time**: 15.1 seconds (timeout)  
**Messages**: 4  
**Result**: Clear failure at wake stage

---

### Example 4: Validation Failure

```
[T+0ms] Server → Device
{"type":"command","data":{"id":14,"action":"vehicle.startClimate","temp":50.0}}

[T+10ms] Device → Server (Accepted)
{"type":"response","data":{"id":14,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+15ms] Device → Server (Failed - Validation)
{
  "type":"response",
  "data":{
    "id":14,
    "ok":false,
    "status":"failed",
    "stage":"accepted",
    "error":"Temperature must be 15.5-30.0°C",
    "elapsedMs":5
  }
}
```

**Total time**: 15 milliseconds  
**Messages**: 2  
**Result**: Fast validation failure

---

### Example 5: Busy Rejection

```
[T+0ms] Server → Device (Command A)
{"type":"command","data":{"id":12,"action":"vehicle.startClimate","temp":21.0}}

[T+10ms] Device → Server (A Accepted)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+50ms] Device → Server (A Requesting Wake)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"requesting_wake"}}

[T+1000ms] Server → Device (Command B - while A is running)
{"type":"command","data":{"id":13,"action":"vehicle.stopClimate"}}

[T+1010ms] Device → Server (B Rejected - Busy)
{
  "type":"response",
  "data":{
    "id":13,
    "ok":false,
    "status":"busy",
    "error":"Another command is in progress",
    "currentCommand":{
      "id":12,
      "action":"vehicle.startClimate",
      "stage":"requesting_wake",
      "elapsedMs":1000
    }
  }
}

... Command A continues normally ...

[T+3000ms] Device → Server (A Waiting for Wake)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"waiting_for_wake"}}

[T+5000ms] Device → Server (A Updating Profile)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"updating_profile"}}

[T+7000ms] Device → Server (A Sending Command)
{"type":"response","data":{"id":12,"ok":true,"status":"in_progress","stage":"sending_command"}}

[T+7200ms] Device → Server (A Completed)
{"type":"response","data":{"id":12,"ok":true,"status":"completed"}}

[T+7300ms] Server → Device (Retry Command B)
{"type":"command","data":{"id":15,"action":"vehicle.stopClimate"}}

[T+7310ms] Device → Server (B Accepted)
{"type":"response","data":{"id":15,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+7320ms] Device → Server (B Sending Command - vehicle already awake, no profile needed)
{"type":"response","data":{"id":15,"ok":true,"status":"in_progress","stage":"sending_command"}}

[T+7500ms] Device → Server (B Completed)
{"type":"response","data":{"id":15,"ok":true,"status":"completed"}}
```

**Key Points**:
- Command B rejected immediately (fast)
- Server knows exactly why (command A still running)
- Server knows command A's progress (requesting_wake stage)
- Server can retry command B after A completes

---

### Example 6: Fast Synchronous Command

```
[T+0ms] Server → Device
{"type":"command","data":{"id":16,"action":"vehicle.getState"}}

[T+10ms] Device → Server (Accepted)
{"type":"response","data":{"id":16,"ok":true,"status":"in_progress","stage":"accepted"}}

[T+15ms] Device → Server (Completed with data)
{
  "type":"response",
  "data":{
    "id":16,
    "ok":true,
    "status":"completed",
    "battery":{...},
    "drive":{...},
    "climate":{...},
    "plug":{...}
  }
}
```

**Total time**: 15 milliseconds  
**Messages**: 2 (arrive nearly simultaneously)  
**Benefit**: Server knows device received command even for instant operations

---

## Migration Guide (for Server Implementations)

### Breaking Changes

1. **Command responses changed from single "queued" to multiple "in_progress" messages**
   - Old: `{"id":12, "ok":true, "status":"queued"}`
   - New: Multiple `{"id":12, "status":"in_progress", "stage":"..."}` messages

2. **Command lifecycle events removed**
   - Removed: `climateStartRequested`, `chargingStartRequested`, `commandCompleted`, `commandFailed`
   - Replaced by: Stateful responses linked by command ID

3. **Vehicle state events unchanged**
   - Kept: `chargingStarted`, `plugged`, `doorOpened`, `ignitionOn`, etc.
   - These still work as before

### Server Update Checklist

- [ ] Update command response handler to process multiple responses per command
- [ ] Track command state by ID (in_progress → completed/failed)
- [ ] Remove handlers for `climateStartRequested`, `commandCompleted`, etc
- [ ] Keep handlers for vehicle state events (charging, doors, etc)
- [ ] Update UI to show progress stages
- [ ] Handle new `busy` status with currentCommand info
- [ ] Handle `failed` status with stage and error details
- [ ] Test with new firmware before production deployment

### Backward Compatibility

**None.** This is a breaking protocol change (v2.1 → v2.2).

Old servers will not correctly handle new device firmware. Server must be updated first, then devices can be updated.

---

## State Machine Diagram

```
                         Server sends command
                                  │
                                  ▼
                    ┌─────────────────────────┐
                    │   CommandRouter         │
                    │   - Check busy?         │
                    │   - Start tracking      │
                    └───────────┬─────────────┘
                                │
                     ┌──────────┴──────────┐
                     │                     │
                NO BUSY                 BUSY
                     │                     │
                     ▼                     ▼
        ┌─────────────────────┐  ┌────────────────┐
        │  ACCEPTED (stage)   │  │  Send busy     │
        │  Send response      │  │  response      │
        └──────────┬──────────┘  └────────────────┘
                   │
                   ▼
        ┌─────────────────────┐
        │  Route to Handler   │
        │  - Validate params  │
        │  - Call channel     │
        └──────────┬──────────┘
                   │
         ┌─────────┴──────────┐
         │                    │
    INVALID/ERROR          PENDING
         │                    │
         ▼                    ▼
    ┌─────────┐    ┌──────────────────────┐
    │ FAILED  │    │  Channel executes    │
    │ stage:  │    │  state machine       │
    │accepted │    └──────────┬───────────┘
    └─────────┘               │
                              │
                    ┌─────────┴─────────┐
                    │                   │
               Vehicle Awake      Vehicle Sleeping
                    │                   │
                    ▼                   ▼
         ┌──────────────────┐  ┌────────────────────┐
         │ UPDATING_PROFILE │  │ REQUESTING_WAKE    │
         │ (if needed)      │  │                    │
         └────────┬─────────┘  └─────────┬──────────┘
                  │                       │
                  │                       ▼
                  │            ┌────────────────────┐
                  │            │ WAITING_FOR_WAKE   │
                  │            │                    │
                  │            └─────────┬──────────┘
                  │                      │
                  │              ┌───────┴────────┐
                  │              │                │
                  │           Success         Timeout
                  │              │                │
                  │              ▼                ▼
                  │   ┌──────────────────┐  ┌─────────┐
                  │   │ UPDATING_PROFILE │  │ FAILED  │
                  │   │ (if needed)      │  │ stage:  │
                  │   └────────┬─────────┘  │waiting  │
                  │            │             └─────────┘
                  └────────────┤
                               │
                               ▼
                    ┌──────────────────────┐
                    │ SENDING_COMMAND      │
                    │                      │
                    └──────────┬───────────┘
                               │
                      ┌────────┴────────┐
                      │                 │
                  Success           Failure
                      │                 │
                      ▼                 ▼
                ┌──────────┐      ┌─────────┐
                │COMPLETED │      │ FAILED  │
                │          │      │ stage:  │
                │          │      │sending  │
                └──────────┘      └─────────┘
```

---

## Risks and Mitigations

### Risk 1: Breaking Change for Server

**Impact**: Server must be updated to handle new protocol

**Mitigation**: 
- Document migration guide clearly (see above)
- Test server changes with new firmware before production
- Deploy server updates first, then device firmware
- Version protocol explicitly (v2.2)

### Risk 2: More Network Traffic

**Impact**: Multiple progress updates instead of single "queued" response

**Analysis**:
- Current: ~200 bytes per command (1 response + events)
- New: ~100 bytes × 3-6 messages = 300-600 bytes per command
- Increase: 1.5x - 3x traffic

**Mitigation**:
- Traffic increase is acceptable for much better visibility
- Updates only on stage changes (no spam)
- Typical command: 4-5 messages = ~400 bytes
- Commands are relatively rare (not continuous)
- Modern cellular data plans can easily handle this

### Risk 3: Implementation Complexity

**Impact**: New component (CommandStateManager) adds complexity

**Mitigation**:
- Well-defined interface with clear responsibilities
- Singleton pattern keeps it simple
- Comprehensive documentation
- Thorough testing before deployment
- State machine already exists (just exposed to server now)

### Risk 4: Extra Message for Fast Commands

**Impact**: Instant commands get 2 responses (accepted + completed)

**Analysis**:
- Time difference: < 50ms between messages
- Server processing: Messages arrive nearly simultaneously
- User experience: No visible delay

**Mitigation**:
- Accept the cost for consistency and debugging benefits
- Server knows device received command even if completion fails
- Consistent behavior for all commands simplifies code

---

## Testing Strategy

### Unit Tests (if framework available)

- CommandStateManager state transitions
- Stage string conversions
- Response message formatting
- Busy detection logic

### Integration Tests

- Command acceptance and rejection
- Stage progression
- Multiple commands in sequence
- Rapid command attempts (busy rejection)
- Validation failures
- Timeout handling

### Real Vehicle Tests

- Vehicle awake scenarios
- Vehicle sleeping scenarios (wake required)
- Profile updates
- Failure scenarios (vehicle doesn't respond)
- All command types (climate, charging, profiles)

### Edge Cases

- Commands during vehicle state transitions
- Connection loss during command execution
- Device reboot during command execution
- Rapid command flood from server
- Invalid command IDs
- Command timeout edge cases

---

## Open Questions

### Q1: What if connection drops during command execution?

**Answer**: Command continues executing on device, but responses are lost. When connection re-establishes:
- CommandStateManager state is NOT persisted
- Server sees device as "idle" again
- Acceptable: transient state, commands finish quickly anyway

### Q2: Should command state persist across reboots?

**Answer**: No. Commands are transient operations. If device reboots:
- All command state is lost
- Server will timeout waiting for response
- Acceptable: reboots are rare and indicate serious issue

### Q3: What happens if stage update fails to send (buffer full, etc)?

**Answer**: 
- Current stage stored in CommandStateManager
- Next update will show current stage
- Server may miss intermediate stages but will see final state
- Acceptable: final state (completed/failed) is most important

### Q4: Should we log all command state transitions?

**Answer**: Yes, for debugging. Add Serial.printf() in CommandStateManager:
```cpp
Serial.printf("[CMD] Command %d: %s → %s\r\n", 
              currentCommandId, 
              oldStage, 
              newStage);
```

### Q5: Can CommandStateManager be extended for other features?

**Answer**: Yes, potential future enhancements:
- Command history (last N commands)
- Command statistics (success rate, average duration)
- Command cancellation support
- Multiple concurrent commands (if needed later)

---

## Future Enhancements (Not in Scope)

These ideas are NOT part of this refactoring but could be considered later:

1. **Command Queue**: Allow server to queue multiple commands (device executes in order)
2. **Command Cancellation**: Allow server to cancel in-progress commands
3. **Command History**: Track last 10 commands with timestamps and results
4. **Command Priorities**: High-priority commands can pre-empt low-priority ones
5. **Persistent State**: Save command state to flash for recovery after reboot
6. **Progress Percentages**: Add numeric progress based on real timing data
7. **Estimated Time Remaining**: Tell server "~5 seconds remaining"
8. **Sub-stages**: More granular progress (e.g., "waiting_for_wake" could have "polling_can", "checking_frames", etc)

These can be evaluated separately if business needs arise.

---

## Conclusion

This refactoring transforms the command handling system from a "fire and forget" approach to a fully stateful, trackable system. The server gains complete visibility into command execution, enabling better UX, clearer error messages, and easier debugging.

The implementation is straightforward, building on existing state machine infrastructure. The key innovation is exposing internal device state to the server through linked responses, giving both parties a shared understanding of command progress.

Total effort is estimated at ~40 hours, with the bulk of work in creating CommandStateManager and integrating it with existing channels. The protocol change is breaking but necessary for the quality improvements we need.

---

**Next Steps:**
1. ✅ Review and approve this plan
2. ⬜ Resolve any questions or concerns
3. ⬜ Begin Phase 1 implementation (CommandStateManager)
4. ⬜ Iterate through phases 2-6
5. ⬜ Deploy to test device
6. ⬜ Real-world car testing
7. ⬜ Deploy to production

---

**Document Version**: 1.0  
**Last Updated**: 2026-01-30  
**Status**: APPROVED - Ready for Implementation
