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
    
    // === Battery state (unified) ===
    JsonObject battery = data["battery"].to<JsonObject>();
    
    // SOC (BAP primary, includes source tracking)
    if (state.battery.hasSoc()) {
        battery["soc"] = state.battery.soc;
        battery["socSource"] = state.battery.socSource == DataSource::BAP ? "bap" : "can";
    }
    
    // Charging status (unified from CAN + BAP)
    battery["charging"] = state.battery.charging;
    battery["chargingSource"] = state.battery.chargingSource == DataSource::BAP ? "bap" : 
                                 state.battery.chargingSource == DataSource::CAN_STD ? "can" : "none";
    
    // Charging details (from BAP when available)
    if (state.battery.chargingDetailsUpdate > 0) {
        battery["chargingMode"] = static_cast<uint8_t>(state.battery.chargingMode);
        battery["chargingStatus"] = static_cast<uint8_t>(state.battery.chargingStatus);
        battery["chargingAmps"] = state.battery.chargingAmps;
        battery["targetSoc"] = state.battery.targetSoc;
        battery["remainingMin"] = state.battery.remainingTimeMin;
    }
    
    // Power and energy (CAN)
    battery["powerKw"] = state.battery.powerKw;
    battery["energyWh"] = state.battery.energyWh;
    battery["maxEnergyWh"] = state.battery.maxEnergyWh;
    battery["temperature"] = state.battery.temperature;
    battery["balancing"] = state.battery.balancingActive;
    
    // Voltage/current (NOT AVAILABLE on comfort CAN)
    if (state.battery.voltageUpdate > 0) {
        battery["voltage"] = state.battery.voltage;
        battery["current"] = state.battery.current;
    }
    
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
    
    // === Climate state (unified) ===
    JsonObject climate = data["climate"].to<JsonObject>();
    
    // Temperature (with source tracking)
    climate["insideTemp"] = state.climate.insideTemp;
    climate["insideTempSource"] = state.climate.insideTempSource == DataSource::BAP ? "bap" : "can";
    climate["outsideTemp"] = state.climate.outsideTemp;
    
    // Standby modes (CAN only)
    climate["standbyHeating"] = state.climate.standbyHeatingActive;
    climate["standbyVent"] = state.climate.standbyVentActive;
    
    // Active climate control (BAP)
    climate["active"] = state.climate.climateActive;
    climate["heating"] = state.climate.heating;
    climate["cooling"] = state.climate.cooling;
    climate["ventilation"] = state.climate.ventilation;
    climate["autoDefrost"] = state.climate.autoDefrost;
    climate["remainingMin"] = state.climate.climateTimeMin;
    
    // === Plug state (BAP only) ===
    if (state.plug.isValid()) {
        JsonObject plug = data["plug"].to<JsonObject>();
        plug["plugged"] = state.plug.isPlugged();
        plug["hasSupply"] = state.plug.hasSupply();
        plug["state"] = state.plug.plugStateStr();
        plug["lockState"] = state.plug.lockState;
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
    if (state.battery.charging != lastCharging) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    if (state.plug.isPlugged() != lastPlugged) {
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
    if (state.battery.charging != lastCharging) {
        return true;
    }
    
    // Lock state changed
    if (state.body.isLocked() != lastLocked) {
        return true;
    }
    
    // Plug state changed
    if (state.plug.isPlugged() != lastPlugged) {
        return true;
    }
    
    // SOC changed significantly (use unified soc field)
    float socDiff = abs(state.battery.soc - lastSoc);
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
        lastSoc = state.battery.soc;
        lastPowerKw = state.battery.powerKw;
        lastIgnitionOn = state.drive.ignitionOn;
        lastCharging = state.battery.charging;
        lastLocked = state.body.isLocked();
        lastPlugged = state.plug.isPlugged();
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
        eventCharging = state.battery.charging;
        eventLocked = state.body.isLocked();
        eventPlugged = state.plug.isPlugged();
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
    
    // Check charging change (unified field)
    if (state.battery.charging != eventCharging) {
        eventCharging = state.battery.charging;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["soc"] = state.battery.soc;
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
    if (state.plug.isPlugged() != eventPlugged) {
        eventPlugged = state.plug.isPlugged();
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["hasSupply"] = state.plug.hasSupply();
        
        if (eventPlugged) {
            Serial.println("[VEHICLE] Event: plugged");
            emitEvent("plugged", &details);
        } else {
            Serial.println("[VEHICLE] Event: unplugged");
            emitEvent("unplugged", &details);
        }
    }
}
