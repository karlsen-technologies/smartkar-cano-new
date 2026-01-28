#include "VehicleProvider.h"
#include "../vehicle/VehicleManager.h"
#include "../core/CommandRouter.h"

VehicleProvider::VehicleProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void VehicleProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    // Get a consistent copy of vehicle state
    VehicleState state = vehicleManager->getStateCopy();
    
    // === Battery state ===
    JsonObject battery = data["battery"].to<JsonObject>();
    battery["soc"] = state.battery.usableSoc;
    battery["socHiRes"] = state.battery.socHiRes;
    battery["voltage"] = state.battery.voltage;
    battery["current"] = state.battery.current;
    battery["powerKw"] = state.battery.powerKw;
    battery["energyWh"] = state.battery.energyWh;
    battery["maxEnergyWh"] = state.battery.maxEnergyWh;
    battery["temperature"] = state.battery.temperature;
    battery["charging"] = state.battery.chargingActive;
    battery["balancing"] = state.battery.balancingActive;
    battery["dcdc12v"] = state.battery.dcdc12v;
    battery["dcdcCurrent"] = state.battery.dcdcCurrent;
    
    // === Drive state ===
    JsonObject drive = data["drive"].to<JsonObject>();
    drive["ignition"] = static_cast<uint8_t>(state.drive.ignition);
    drive["keyInserted"] = state.drive.keyInserted;
    drive["ignitionOn"] = state.drive.ignitionOn;
    drive["speedKmh"] = state.drive.speedKmh;
    drive["odometerKm"] = state.drive.odometerKm;
    
    // === Body state ===
    JsonObject body = data["body"].to<JsonObject>();
    body["locked"] = state.body.isLocked();
    body["centralLock"] = static_cast<uint8_t>(state.body.centralLock);
    body["trunkOpen"] = state.body.trunkOpen;
    body["anyDoorOpen"] = state.body.anyDoorOpen();
    
    // Individual doors
    JsonObject doors = body["doors"].to<JsonObject>();
    doors["driverOpen"] = state.body.driverDoor.open;
    doors["passengerOpen"] = state.body.passengerDoor.open;
    doors["rearLeftOpen"] = state.body.rearLeftDoor.open;
    doors["rearRightOpen"] = state.body.rearRightDoor.open;
    
    // === Range state ===
    if (state.range.isValid()) {
        JsonObject range = data["range"].to<JsonObject>();
        range["totalKm"] = state.range.totalRangeKm;
        range["electricKm"] = state.range.electricRangeKm;
        range["displayKm"] = state.range.displayRangeKm;
        range["consumption"] = state.range.consumption;
        range["tendency"] = state.range.tendencyStr();
        range["reserveWarning"] = state.range.reserveWarning;
    }
    
    // === CAN GPS state (if valid) ===
    if (state.gps.isValid()) {
        JsonObject gps = data["canGps"].to<JsonObject>();
        gps["lat"] = state.gps.latitude;
        gps["lng"] = state.gps.longitude;
        gps["alt"] = state.gps.altitude;
        gps["heading"] = state.gps.heading;
        gps["satellites"] = state.gps.satsInUse;
        gps["fixType"] = state.gps.fixTypeStr();
        gps["hdop"] = state.gps.hdop;
    }
    
    // === Climate state ===
    JsonObject climate = data["climate"].to<JsonObject>();
    climate["insideTemp"] = state.climate.insideTemp;
    climate["outsideTemp"] = state.climate.outsideTemp;
    climate["standbyHeating"] = state.climate.standbyHeatingActive;
    climate["standbyVent"] = state.climate.standbyVentActive;
    
    // === BAP states ===
    // Plug state
    if (state.bapPlug.isValid()) {
        JsonObject plug = data["plug"].to<JsonObject>();
        plug["plugged"] = state.bapPlug.isPlugged();
        plug["hasSupply"] = state.bapPlug.hasSupply();
        plug["state"] = state.bapPlug.plugStateStr();
    }
    
    // Charge state from BAP
    if (state.bapCharge.isValid()) {
        JsonObject bapCharge = data["bapCharge"].to<JsonObject>();
        bapCharge["mode"] = state.bapCharge.chargeModeStr();
        bapCharge["status"] = state.bapCharge.chargeStatusStr();
        bapCharge["soc"] = state.bapCharge.socPercent;
        bapCharge["remainingMin"] = state.bapCharge.remainingTimeMin;
        bapCharge["targetSoc"] = state.bapCharge.targetSoc;
        bapCharge["amps"] = state.bapCharge.chargingAmps;
    }
    
    // Climate state from BAP
    if (state.bapClimate.isValid()) {
        JsonObject bapClimate = data["bapClimate"].to<JsonObject>();
        bapClimate["active"] = state.bapClimate.climateActive;
        bapClimate["heating"] = state.bapClimate.heating;
        bapClimate["cooling"] = state.bapClimate.cooling;
        bapClimate["ventilation"] = state.bapClimate.ventilation;
        bapClimate["autoDefrost"] = state.bapClimate.autoDefrost;
        bapClimate["currentTemp"] = state.bapClimate.currentTempC;
        bapClimate["remainingMin"] = state.bapClimate.climateTimeMin;
    }
    
    // === Meta ===
    data["vehicleAwake"] = state.isAwake();
    data["canFrameCount"] = state.canFrameCount;
}

TelemetryPriority VehicleProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    VehicleState state = vehicleManager->getStateCopy();
    
    // High priority for significant events
    if (state.drive.ignitionOn != lastIgnitionOn) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    if (state.battery.chargingActive != lastCharging) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    if (state.bapPlug.isPlugged() != lastPlugged) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool VehicleProvider::hasChanged() {
    // Always send initial report after boot
    if (initialReport) {
        return true;
    }
    
    // Check explicit change flag
    if (changed) {
        return true;
    }
    
    if (!vehicleManager) return false;
    
    VehicleState state = vehicleManager->getStateCopy();
    
    // Determine interval based on vehicle state
    unsigned long reportInterval = state.isAwake() ? 
        REPORT_INTERVAL_AWAKE : REPORT_INTERVAL_ASLEEP;
    
    // Check if interval has elapsed
    if (millis() - lastReportTime >= reportInterval) {
        return true;
    }
    
    // Check for significant changes
    
    // Ignition changed
    if (state.drive.ignitionOn != lastIgnitionOn) {
        return true;
    }
    
    // Charging state changed
    if (state.battery.chargingActive != lastCharging) {
        return true;
    }
    
    // Lock state changed
    if (state.body.isLocked() != lastLocked) {
        return true;
    }
    
    // Plug state changed
    if (state.bapPlug.isPlugged() != lastPlugged) {
        return true;
    }
    
    // SOC changed significantly
    float socDiff = abs(state.battery.usableSoc - lastSoc);
    if (socDiff >= SOC_CHANGE_THRESHOLD) {
        return true;
    }
    
    // Power changed significantly (useful during charging)
    float powerDiff = abs(state.battery.powerKw - lastPowerKw);
    if (powerDiff >= POWER_CHANGE_THRESHOLD) {
        return true;
    }
    
    // Speed changed significantly (useful when driving)
    float speedDiff = abs(state.drive.speedKmh - lastSpeedKmh);
    if (speedDiff >= SPEED_CHANGE_THRESHOLD) {
        return true;
    }
    
    return false;
}

void VehicleProvider::onTelemetrySent() {
    initialReport = false;
    changed = false;
    lastReportTime = millis();
    
    // Update last reported values
    if (vehicleManager) {
        VehicleState state = vehicleManager->getStateCopy();
        lastSoc = state.battery.usableSoc;
        lastPowerKw = state.battery.powerKw;
        lastIgnitionOn = state.drive.ignitionOn;
        lastCharging = state.battery.chargingActive;
        lastLocked = state.body.isLocked();
        lastPlugged = state.bapPlug.isPlugged();
        lastSpeedKmh = state.drive.speedKmh;
    }
}

// ============================================================================
// Event Emission
// ============================================================================

void VehicleProvider::emitEvent(const char* eventName, JsonObject* details) {
    if (commandRouter) {
        commandRouter->sendEvent("vehicle", eventName, details);
    }
}

void VehicleProvider::checkAndEmitEvents() {
    if (!vehicleManager || !commandRouter) return;
    
    // Rate limit event checks
    unsigned long now = millis();
    if (now - lastEventCheckTime < EVENT_CHECK_INTERVAL) {
        return;
    }
    lastEventCheckTime = now;
    
    VehicleState state = vehicleManager->getStateCopy();
    
    // Initialize event tracking on first run (don't emit events for initial state)
    if (!eventsInitialized) {
        eventIgnitionOn = state.drive.ignitionOn;
        eventCharging = state.battery.chargingActive;
        eventLocked = state.body.isLocked();
        eventPlugged = state.bapPlug.isPlugged();
        eventsInitialized = true;
        return;
    }
    
    // Check ignition change
    if (state.drive.ignitionOn != eventIgnitionOn) {
        eventIgnitionOn = state.drive.ignitionOn;
        if (eventIgnitionOn) {
            Serial.println("[VEHICLE] Event: ignitionOn");
            emitEvent("ignitionOn", nullptr);
        } else {
            Serial.println("[VEHICLE] Event: ignitionOff");
            emitEvent("ignitionOff", nullptr);
        }
    }
    
    // Check charging change
    if (state.battery.chargingActive != eventCharging) {
        eventCharging = state.battery.chargingActive;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["soc"] = state.battery.usableSoc;
        details["powerKw"] = state.battery.powerKw;
        
        if (eventCharging) {
            Serial.println("[VEHICLE] Event: chargingStarted");
            emitEvent("chargingStarted", &details);
        } else {
            Serial.println("[VEHICLE] Event: chargingStopped");
            emitEvent("chargingStopped", &details);
        }
    }
    
    // Check lock change
    if (state.body.isLocked() != eventLocked) {
        eventLocked = state.body.isLocked();
        if (eventLocked) {
            Serial.println("[VEHICLE] Event: locked");
            emitEvent("locked", nullptr);
        } else {
            Serial.println("[VEHICLE] Event: unlocked");
            emitEvent("unlocked", nullptr);
        }
    }
    
    // Check plug change
    if (state.bapPlug.isPlugged() != eventPlugged) {
        eventPlugged = state.bapPlug.isPlugged();
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["hasSupply"] = state.bapPlug.hasSupply();
        
        if (eventPlugged) {
            Serial.println("[VEHICLE] Event: plugged");
            emitEvent("plugged", &details);
        } else {
            Serial.println("[VEHICLE] Event: unplugged");
            emitEvent("unplugged", &details);
        }
    }
}
