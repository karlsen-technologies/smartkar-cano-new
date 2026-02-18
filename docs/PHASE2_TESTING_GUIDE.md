# Phase 2 Testing Guide: BatteryManager Parallel Operation

## Overview

This guide covers testing the new BatteryManager domain running in parallel with the existing BatteryDomain. Both systems process the same CAN frames and BAP callbacks, allowing us to compare outputs and verify the new architecture works correctly.

## Prerequisites

- ESP32 hardware with CAN transceiver connected to VW e-Golf
- PlatformIO installation
- Serial monitor (115200 baud)
- Vehicle must be accessible (ideally with charging cable)

## Build and Flash

```bash
# From project root
platformio run --target upload
platformio device monitor --baud 115200
```

## What to Expect on Startup

### 1. Initialization Messages

Look for these startup messages in serial output:

```
[VehicleManager] Initializing vehicle domains...
[VehicleManager] === NEW ARCHITECTURE (Phase 2) ===
[BatteryManager] Initializing...
[BatteryManager] Registering BAP callbacks...
[BatteryManager] Initialized:
[BatteryManager]   - CAN IDs: 0x5CA (BMS_07), 0x59E (BMS_06), 0x483 (Motor_Hybrid_06)
[BatteryManager]   - BAP callbacks: PlugState, ChargeState
[BatteryManager]   - Data source priority: BAP > CAN > Computed
[VehicleManager] === END NEW ARCHITECTURE ===
```

**✅ SUCCESS:** All initialization messages appear without errors  
**❌ FAILURE:** Error messages about callback registration or missing channel

### 2. Frame Processing (Every 10 Seconds)

VehicleManager logs statistics every 10 seconds. Compare OLD vs NEW outputs:

```
[VehicleManager] === Vehicle Status ===
...
[VehicleManager] Battery frames: 0x5CA:1234 0x59E:1234 0x483:1234
[VehicleManager] Battery: energy=18500/24200Wh (76%) temp=18.5°C power=0.00kW
[VehicleManager] Charging: no (source:CAN), Balancing: no

[VehicleManager] NEW BatteryManager: frames=0x5CA:1234 0x59E:1234 0x483:1234 callbacks=plug:45 charge:45
[VehicleManager] NEW BatteryManager: SOC=76% (source:BAP) energy=18500/24200Wh plugged:no charging:no
```

## Test Scenarios

### Scenario 1: Idle Vehicle (Not Plugged In)

**Expected Behavior:**
- Both old and new receive same CAN frames (0x5CA, 0x59E, 0x483)
- Frame counts match: `0x5CA:X == frames=0x5CA:X`
- Energy values match (±1 Wh tolerance)
- Temperature matches (±0.5°C tolerance)
- Power is 0.00 kW
- OLD: `Charging: no (source:CAN)`
- NEW: `Charging: no`
- NEW: `callbacks=plug:0 charge:0` (no BAP updates when not plugged)

**✅ PASS Criteria:**
- Frame counts increase together
- Energy/temp values match
- No BAP callbacks (vehicle doesn't send when unplugged)

**❌ FAIL Indicators:**
- Frame count mismatch (possible frame loss)
- Energy values differ by >100Wh (parsing error)
- Temperature differs by >2°C (parsing error)

### Scenario 2: Plug In Charging Cable

**Expected Behavior:**

1. **Immediately after plugging in:**
   ```
   NEW BatteryManager: plugged:YES charging:no
   callbacks=plug:1 charge:0
   ```
   - Plug callback count increases to 1+
   - OLD system doesn't show plug state (not in CAN frames)

2. **If vehicle starts charging automatically:**
   ```
   NEW BatteryManager: plugged:YES charging:YES
   callbacks=plug:1+ charge:1+
   ```
   - Charge callback count increases
   - OLD: `Charging: YES (source:CAN)` (from 0x5CA broadcast)
   - NEW: `Charging: YES` with detailed info

**✅ PASS Criteria:**
- Plug callback fires within 1-2 seconds of plugging in
- `plugged:YES` appears in NEW output
- Both OLD and NEW show charging state (if charging starts)
- SOC source changes from CAN to BAP: `(source:BAP)`

**❌ FAIL Indicators:**
- No plug callback after 5 seconds (callback not registered)
- `plugged:no` even when cable connected (wrong parsing)
- Charging states mismatch between OLD/NEW

### Scenario 3: Start Charging via BAP Command

**Test command (from HTTP API or serial):**
```cpp
vehicleManager->battery()->startCharging(1, 80, 16);
// commandId=1, targetSoc=80%, maxCurrent=16A
```

**Expected Behavior:**
```
[BatteryManager] Starting charging: target=80%, maxCurrent=16A
[BatteryControlChannel] Sending BAP command: start charging
...
NEW BatteryManager: charging:YES callbacks=charge:2+
Battery: targetSoc=80% chargingAmps=16 remainingTime=XXXmin
```

**✅ PASS Criteria:**
- Command accepted (returns true)
- Charge callback fires within 2-3 seconds
- `charging:YES` appears
- Target SOC and current limits reflected in state

**❌ FAIL Indicators:**
- Command rejected (isBusy() or no channel)
- No charge callback after 5 seconds
- Charging doesn't start (vehicle may require key fob nearby)

### Scenario 4: Stop Charging via BAP Command

**Test command:**
```cpp
vehicleManager->battery()->stopCharging(2);
```

**Expected Behavior:**
```
[BatteryManager] Stopping charging
[BatteryControlChannel] Sending BAP command: stop charging
...
NEW BatteryManager: charging:no
```

**✅ PASS Criteria:**
- Command accepted
- Charge callback fires
- `charging:no` appears
- OLD system also reflects stopped charging (from CAN broadcast)

**❌ FAIL Indicators:**
- Command rejected
- Charging continues (vehicle may override user commands)

## Detailed Comparison Checklist

For each 10-second statistics log:

| Metric | OLD Location | NEW Location | Expected Match | Tolerance |
|--------|-------------|-------------|----------------|-----------|
| 0x5CA frames | `0x5CA:1234` | `frames=0x5CA:1234` | ✅ Exact | 0 |
| 0x59E frames | `0x59E:1234` | `frames=0x59E:1234` | ✅ Exact | 0 |
| 0x483 frames | `0x483:1234` | `frames=0x483:1234` | ✅ Exact | 0 |
| Energy (Wh) | `energy=18500` | `energy=18500` | ✅ Close | ±100Wh |
| Max Energy | `24200Wh` | `24200Wh` | ✅ Exact | 0 |
| Temperature | `temp=18.5°C` | Not shown | ⚠️ OLD only | N/A |
| Power | `power=0.00kW` | Not shown | ⚠️ OLD only | N/A |
| SOC | Not in OLD | `SOC=76%` | ⚠️ NEW only (BAP) | N/A |
| Plugged | Not in OLD | `plugged:YES/no` | ⚠️ NEW only (BAP) | N/A |
| Charging | `Charging: YES/no` | `charging:YES/no` | ✅ Match | 0 |

**NOTE:** Some fields are exclusive to one system:
- OLD doesn't show SOC or plug state (not available from CAN)
- NEW doesn't log temp/power in stats (but available via API)

## Performance Validation

### CAN Thread Timing

Watch for timing warnings in CanManager:
```
[CanManager] WARNING: CAN task took XXms (target <2ms)
```

**✅ PASS:** CAN task stays under 2ms  
**❌ FAIL:** Frequent warnings >2ms (callback too slow, frame drops likely)

### Frame Loss Detection

Compare CanManager received vs VehicleManager processed:
```
[VehicleManager] CanManager received: 12345 (TWAI missed: 0)
[VehicleManager] VehicleManager processed: 12345 (domains: 12345)
```

**✅ PASS:** `received == processed`, TWAI missed = 0  
**❌ FAIL:** `received > processed` (mutex timeout, frames lost)

### Memory/Stability

Monitor for:
- Mutex timeout messages
- Heap fragmentation (if available)
- System crashes/reboots
- Watchdog timer resets

**✅ PASS:** Runs stable for 5+ minutes without issues  
**❌ FAIL:** Crashes, hangs, or frequent mutex timeouts

## Common Issues and Solutions

### Issue: No BAP Callbacks

**Symptoms:** `callbacks=plug:0 charge:0` even when plugged in

**Possible Causes:**
1. Vehicle in deep sleep (BAP not responding)
2. Callback registration failed
3. BatteryControlChannel not receiving BAP frames

**Debug Steps:**
1. Check vehicle is awake: `Vehicle awake: YES`
2. Check BAP frame count: `bap:X` should increase
3. Add debug logging in BatteryControlChannel callback notification methods

### Issue: Frame Count Mismatch

**Symptoms:** OLD shows more frames than NEW

**Possible Causes:**
1. NEW domain not added to routing in VehicleManager::processCanFrame()
2. DLC check too strict in NEW domain

**Debug Steps:**
1. Verify routing code sends frames to BOTH old and new
2. Check if `dlc < 8` is rejecting valid frames

### Issue: Data Values Don't Match

**Symptoms:** Energy, temp, or power values significantly different

**Possible Causes:**
1. Different decoding logic (byte order, scaling)
2. Timing issue (old/new reading different frame instances)

**Debug Steps:**
1. Compare with BroadcastDecoder reference implementation
2. Add hex dump of raw frame data
3. Verify both use same BroadcastDecoder methods

## Success Criteria Summary

**Phase 2 is SUCCESSFUL when:**

✅ Code compiles without errors  
✅ Both old and new initialize without errors  
✅ Frame counts match exactly (no frame loss)  
✅ Energy values match within ±100Wh  
✅ Temperature values match within ±0.5°C  
✅ Charging states match (when sourced from CAN)  
✅ BAP callbacks fire when plugged in  
✅ SOC appears with `(source:BAP)` when plugged  
✅ Commands work: `startCharging()`, `stopCharging()`  
✅ No mutex timeouts or frame drops  
✅ System runs stable for 5+ minutes  

**When all criteria pass, proceed to Phase 3: ClimateManager**

## Next Steps After Successful Testing

1. Document any discrepancies found (expected vs actual)
2. Update architecture docs if design changes needed
3. Create Phase 3 implementation plan for ClimateManager
4. Test ClimateManager alongside BatteryManager (shared BAP channel test)

## Emergency Rollback

If new architecture causes issues:

1. Comment out in VehicleManager.cpp:
   ```cpp
   // batteryManager.setup();  // DISABLED - rollback to old architecture
   ```

2. Comment out dual routing:
   ```cpp
   // batteryManager.processCanFrame(canId, data, dlc);  // DISABLED
   ```

3. Recompile and flash - old architecture continues working

## Logging Tips

**Increase logging for debugging:**
```cpp
// In BatteryManager callback handlers, temporarily add:
Serial.printf("[BatteryManager] Plug callback: state=0x%02X\n", plug.plugState);
```

**Reduce noise:**
```cpp
// In VehicleManager.cpp, increase LOG_INTERVAL:
static constexpr unsigned long LOG_INTERVAL = 30000;  // 30 seconds instead of 10
```

---

**Last Updated:** Phase 2 implementation complete, ready for hardware testing
**Status:** BatteryManager parallel operation implemented and code-reviewed
**Bug Fixed:** Changed `isConnected()` to `isPlugged()` in BatteryManager.h:137
