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
    
    // Climate control (BAP-only with source tracking)
    climate["active"] = state.climate.climateActive;
    if (state.climate.climateActiveSource == DataSource::BAP) {
        climate["activeSource"] = "bap";  // Only BAP provides climate active status (CAN unreliable)
    }
    climate["heating"] = state.climate.heating;       // BAP only
    climate["cooling"] = state.climate.cooling;       // BAP only
    climate["ventilation"] = state.climate.ventilation;  // BAP only
    climate["autoDefrost"] = state.climate.autoDefrost;  // BAP only
    climate["remainingMin"] = state.climate.climateTimeMin;  // BAP only
    
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
        eventDriverDoorOpen = state.body.driverDoor.open;
        eventPassengerDoorOpen = state.body.passengerDoor.open;
        eventRearLeftDoorOpen = state.body.rearLeftDoor.open;
        eventRearRightDoorOpen = state.body.rearRightDoor.open;
        eventTrunkOpen = state.body.trunkOpen;
        eventClimateActive = state.climate.climateActive;
        eventLastSoc = state.battery.soc;
        eventsInitialized = true;
        return;
    }
    
    // === Ignition Events ===
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
    
    // === Charging Events ===
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
    
    // === Lock Events ===
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
    
    // === Plug Events ===
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
    
    // === Door Events ===
    if (state.body.driverDoor.open != eventDriverDoorOpen) {
        eventDriverDoorOpen = state.body.driverDoor.open;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["door"] = "driver";
        if (eventDriverDoorOpen) {
            Serial.println("[VEHICLE] Event: doorOpened (driver)");
            emitEvent("doorOpened", &details);
        } else {
            Serial.println("[VEHICLE] Event: doorClosed (driver)");
            emitEvent("doorClosed", &details);
        }
    }
    
    if (state.body.passengerDoor.open != eventPassengerDoorOpen) {
        eventPassengerDoorOpen = state.body.passengerDoor.open;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["door"] = "passenger";
        if (eventPassengerDoorOpen) {
            Serial.println("[VEHICLE] Event: doorOpened (passenger)");
            emitEvent("doorOpened", &details);
        } else {
            Serial.println("[VEHICLE] Event: doorClosed (passenger)");
            emitEvent("doorClosed", &details);
        }
    }
    
    if (state.body.rearLeftDoor.open != eventRearLeftDoorOpen) {
        eventRearLeftDoorOpen = state.body.rearLeftDoor.open;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["door"] = "rearLeft";
        if (eventRearLeftDoorOpen) {
            Serial.println("[VEHICLE] Event: doorOpened (rearLeft)");
            emitEvent("doorOpened", &details);
        } else {
            Serial.println("[VEHICLE] Event: doorClosed (rearLeft)");
            emitEvent("doorClosed", &details);
        }
    }
    
    if (state.body.rearRightDoor.open != eventRearRightDoorOpen) {
        eventRearRightDoorOpen = state.body.rearRightDoor.open;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["door"] = "rearRight";
        if (eventRearRightDoorOpen) {
            Serial.println("[VEHICLE] Event: doorOpened (rearRight)");
            emitEvent("doorOpened", &details);
        } else {
            Serial.println("[VEHICLE] Event: doorClosed (rearRight)");
            emitEvent("doorClosed", &details);
        }
    }
    
    // === Trunk Events ===
    if (state.body.trunkOpen != eventTrunkOpen) {
        eventTrunkOpen = state.body.trunkOpen;
        if (eventTrunkOpen) {
            Serial.println("[VEHICLE] Event: trunkOpened");
            emitEvent("trunkOpened", nullptr);
        } else {
            Serial.println("[VEHICLE] Event: trunkClosed");
            emitEvent("trunkClosed", nullptr);
        }
    }
    
    // === Climate Events ===
    if (state.climate.climateActive != eventClimateActive) {
        eventClimateActive = state.climate.climateActive;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["heating"] = state.climate.heating;
        details["cooling"] = state.climate.cooling;
        details["temp"] = state.climate.insideTemp;
        
        if (eventClimateActive) {
            Serial.println("[VEHICLE] Event: climateStarted");
            emitEvent("climateStarted", &details);
        } else {
            Serial.println("[VEHICLE] Event: climateStopped");
            emitEvent("climateStopped", &details);
        }
    }
    
    // === SOC Threshold Events (20%, 50%, 80%, 100%) ===
    float currentSoc = state.battery.soc;
    if (state.battery.hasSoc() && eventLastSoc > 0) {
        // Check if we crossed any threshold
        int lastThreshold = (int)(eventLastSoc / 20) * 20;  // Round down to nearest 20%
        int currentThreshold = (int)(currentSoc / 20) * 20;
        
        if (currentThreshold != lastThreshold && currentThreshold > 0) {
            // Crossed a threshold
            JsonDocument doc;
            JsonObject details = doc.to<JsonObject>();
            details["soc"] = currentSoc;
            
            if (currentThreshold == 20) {
                details["threshold"] = "20%";
                Serial.println("[VEHICLE] Event: socThreshold (20%)");
                emitEvent("socThreshold", &details);
            } else if (currentThreshold == 40) {
                details["threshold"] = "50%";
                Serial.println("[VEHICLE] Event: socThreshold (50%)");
                emitEvent("socThreshold", &details);
            } else if (currentThreshold == 60) {
                details["threshold"] = "80%";
                Serial.println("[VEHICLE] Event: socThreshold (80%)");
                emitEvent("socThreshold", &details);
            } else if (currentThreshold >= 100) {
                details["threshold"] = "100%";
                Serial.println("[VEHICLE] Event: socThreshold (100%) / chargingComplete");
                emitEvent("socThreshold", &details);
                emitEvent("chargingComplete", &details);
            }
        }
    }
    eventLastSoc = currentSoc;
    
    // === Low Battery Event (below 20%) ===
    if (state.battery.hasSoc() && currentSoc < 20 && eventLastSoc >= 20) {
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["soc"] = currentSoc;
        Serial.println("[VEHICLE] Event: lowBattery");
        emitEvent("lowBattery", &details);
    }
}
