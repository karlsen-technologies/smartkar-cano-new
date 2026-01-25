# TODO

## Current Status

The project has been refactored to a modular architecture with:
- Core interfaces (IModule, ICommandHandler, ITelemetryProvider)
- DeviceController as central coordinator
- CommandRouter for command/telemetry routing
- Working telemetry and command system

## High Priority

### CAN Bus Integration
- [ ] Research T-SIMHAT pinout for CAN TX/RX and interrupt
- [ ] Create CANManager module implementing IModule
- [ ] Implement CAN message sending/receiving
- [ ] Add wake-on-CAN capability (EXT1 wake source)
- [ ] Integrate CAN activity into sleep decision

### Vehicle Modules
- [ ] Create ChargingModule (ICommandHandler + ITelemetryProvider)
  - Commands: charging.start, charging.stop, charging.setLimit
  - Telemetry: soc, range, chargingState, chargerPower
- [ ] Create ClimateModule
  - Commands: climate.preheat, climate.setTemp
  - Telemetry: cabinTemp, hvacState
- [ ] Create VehicleModule
  - Commands: vehicle.query
  - Telemetry: odometer, 12vBattery, gps

## Medium Priority

### Scheduling System
- [ ] Departure timer - schedule charging/preheating
- [ ] Periodic telemetry scheduling (beyond current priority system)
- [ ] Scheduled wake from deep sleep

### Error Recovery
- [ ] ModemManager: Add timeout for network search
- [ ] LinkManager: Add timeout for authentication
- [ ] ModemManager: Recovery from NOSIM state on wake
- [ ] General: Watchdog timer for system hangs

### Configuration
- [ ] Extract server host/port to configuration
- [ ] Extract APN to configuration
- [ ] Consider NVS storage for runtime configuration
- [ ] OTA configuration updates

## Low Priority

### Code Quality
- [ ] Add unit tests (where feasible on embedded)
- [ ] Create configuration header file
- [ ] Add more detailed logging levels
- [ ] Memory usage optimization

### Future Features
- [ ] OTA firmware updates
- [ ] SMS command fallback
- [ ] Location tracking (if GPS enabled)
- [ ] Geofencing alerts

## Completed

### Phase 1: Architecture Refactor
- [x] Create IModule interface with lifecycle methods
- [x] Create ICommandHandler interface
- [x] Create ITelemetryProvider interface
- [x] Create CommandRouter for routing
- [x] Create DeviceController as central coordinator
- [x] Refactor PowerManager to IModule
- [x] Refactor ModemManager to IModule
- [x] Refactor LinkManager to IModule + CommandRouter integration

### Phase 2: Telemetry System
- [x] Create DeviceProvider (device telemetry)
- [x] Create NetworkProvider (network telemetry)
- [x] Implement priority-based telemetry intervals
- [x] Implement change detection (hasChanged)
- [x] Wire providers to CommandRouter

### Phase 3: Command System
- [x] Create SystemHandler (system commands)
- [x] Implement system.reboot command
- [x] Implement system.sleep command with duration
- [x] Implement system.telemetry command
- [x] Implement system.info command
- [x] Wire handlers to CommandRouter

### Phase 4: Sleep System
- [x] Activity-based sleep decisions
- [x] Module busy checks before sleep
- [x] Request sleep via command
- [x] Timer wake source support

### Documentation
- [x] Update README.md
- [x] Update architecture.md
- [x] Create protocol.md (comprehensive protocol spec)
- [x] Update TODO.md

## Design Notes

### Module Pattern

Each hardware-interfacing component implements IModule:
```cpp
class MyModule : public IModule {
    bool setup() override;
    void loop() override;
    void prepareForSleep() override;
    bool isBusy() override;
    bool isReady() override;
};
```

### Handler/Provider Pattern

Vehicle modules should implement both interfaces:
```cpp
class ChargingModule : public ICommandHandler, public ITelemetryProvider {
    // Handler interface
    const char* getDomain() override { return "charging"; }
    CommandResult handleCommand(CommandContext& ctx) override;
    
    // Provider interface  
    bool hasChanged() override;
    void collect(JsonObject& telemetry) override;
    TelemetryPriority getPriority() override;
};
```

### Command Routing

Commands use domain.action format:
- `system.reboot` → SystemHandler
- `charging.start` → ChargingModule
- `climate.preheat` → ClimateModule

### Telemetry Priority

| Priority | Interval | Use Case |
|----------|----------|----------|
| PRIORITY_LOW | 5 min | Stable metrics |
| PRIORITY_NORMAL | 2 min | Network status |
| PRIORITY_HIGH | 30 sec | Active operations |
| PRIORITY_REALTIME | 5 sec | Critical events |
