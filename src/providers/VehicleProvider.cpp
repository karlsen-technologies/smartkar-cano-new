#include "VehicleProvider.h"
#include "../vehicle/VehicleManager.h"
#include "../core/CommandRouter.h"

VehicleProvider::VehicleProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void VehicleProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    // NEW ARCHITECTURE: Get state from domain managers
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    const ClimateManager::State& climState = vehicleManager->climate()->getState();
    const BodyManager::State& bodyState = vehicleManager->body()->getState();
    const DriveManager::State& driveState = vehicleManager->drive()->getState();
    const GpsManager::State& gpsState = vehicleManager->gps()->getState();
    const RangeManager::State& rangeState = vehicleManager->range()->getState();
    
    // === Battery state (unified) ===
    JsonObject battery = data["battery"].to<JsonObject>();
    
    // SOC (BAP primary, includes source tracking)
    if (battState.socSource != DataSource::NONE && battState.soc > 0.0f) {
        battery["soc"] = battState.soc;
        battery["socSource"] = battState.socSource == DataSource::BAP ? "bap" : "can";
    }
    
    // Charging status (unified from CAN + BAP)
    battery["charging"] = battState.charging;
    battery["chargingSource"] = battState.chargingSource == DataSource::BAP ? "bap" : 
                                 battState.chargingSource == DataSource::CAN_STD ? "can" : "none";
    
    // Charging details (from BAP when available)
    if (battState.chargingUpdate > 0) {
        battery["chargingMode"] = static_cast<uint8_t>(battState.chargingMode);
        battery["chargingStatus"] = static_cast<uint8_t>(battState.chargingStatus);
        battery["chargingAmps"] = battState.chargingAmps;
        battery["targetSoc"] = battState.targetSoc;
        battery["remainingMin"] = battState.remainingTimeMin;
    }
    
    // Power and energy (CAN)
    battery["powerKw"] = battState.powerKw;
    battery["energyWh"] = battState.energyWh;
    battery["maxEnergyWh"] = battState.maxEnergyWh;
    battery["temperature"] = battState.temperature;
    battery["balancing"] = battState.balancingActive;
    
    // === Drive state ===
    JsonObject drive = data["drive"].to<JsonObject>();
    drive["ignition"] = static_cast<uint8_t>(driveState.ignition);
    drive["keyInserted"] = driveState.keyInserted;
    drive["ignitionOn"] = driveState.ignitionOn;
    drive["speedKmh"] = driveState.speedKmh;
    drive["odometerKm"] = driveState.odometerKm;
    
    // === Body state ===
    JsonObject body = data["body"].to<JsonObject>();
    body["locked"] = bodyState.isLocked();
    body["centralLock"] = static_cast<uint8_t>(bodyState.centralLock);
    body["trunkOpen"] = bodyState.trunkOpen;
    body["anyDoorOpen"] = bodyState.anyDoorOpen();
    
    // Individual doors
    JsonObject doors = body["doors"].to<JsonObject>();
    doors["driverOpen"] = bodyState.driverDoor.open;
    doors["passengerOpen"] = bodyState.passengerDoor.open;
    doors["rearLeftOpen"] = bodyState.rearLeftDoor.open;
    doors["rearRightOpen"] = bodyState.rearRightDoor.open;
    
    // === Range state ===
    if (rangeState.isValid()) {
        JsonObject range = data["range"].to<JsonObject>();
        range["totalKm"] = rangeState.totalRangeKm;
        range["electricKm"] = rangeState.electricRangeKm;
        range["displayKm"] = rangeState.displayRangeKm;
        range["consumption"] = rangeState.consumptionKwh100km;
        range["tendency"] = rangeState.tendencyStr();
        range["reserveWarning"] = rangeState.reserveWarning;
    }
    
    // === CAN GPS state (if valid) ===
    if (gpsState.isValid()) {
        JsonObject gps = data["canGps"].to<JsonObject>();
        gps["lat"] = gpsState.latitude;
        gps["lng"] = gpsState.longitude;
        gps["alt"] = gpsState.altitude;
        gps["heading"] = gpsState.heading;
        gps["satellites"] = gpsState.satsInUse;
        gps["fixType"] = gpsState.fixTypeStr();
        gps["hdop"] = gpsState.hdop;
    }
    
    // === Climate state (unified) ===
    JsonObject climate = data["climate"].to<JsonObject>();
    
    // Temperature (with source tracking)
    climate["insideTemp"] = climState.insideTemp;
    climate["insideTempSource"] = climState.insideTempSource == DataSource::BAP ? "bap" : "can";
    climate["outsideTemp"] = climState.outsideTemp;
    
    // Climate control (BAP-only with source tracking)
    climate["active"] = climState.climateActive;
    if (climState.climateActiveSource == DataSource::BAP) {
        climate["activeSource"] = "bap";  // Only BAP provides climate active status (CAN unreliable)
    }
    climate["heating"] = climState.heating;       // BAP only
    climate["cooling"] = climState.cooling;       // BAP only
    climate["ventilation"] = climState.ventilation;  // BAP only
    climate["autoDefrost"] = climState.autoDefrost;  // BAP only
    climate["remainingMin"] = climState.climateTimeMin;  // BAP only
    
    // === Plug state (BAP only) ===
    if (battState.plugState.isValid()) {
        JsonObject plug = data["plug"].to<JsonObject>();
        plug["plugged"] = battState.plugState.isPlugged();
        plug["hasSupply"] = battState.plugState.hasSupply();
        plug["state"] = battState.plugState.plugStateStr();
        plug["lockState"] = battState.plugState.lockState;
    }
    
    // === Meta ===
    data["vehicleAwake"] = vehicleManager->isVehicleAwake();
    data["canFrameCount"] = vehicleManager->getFrameCount();
}

TelemetryPriority VehicleProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    // NEW ARCHITECTURE: Get state from domain managers
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    const DriveManager::State& driveState = vehicleManager->drive()->getState();
    
    // High priority for significant events
    if (driveState.ignitionOn != lastIgnitionOn) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    if (battState.charging != lastCharging) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    if (battState.plugState.isPlugged() != lastPlugged) {
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
    
    // NEW ARCHITECTURE: Get state from domain managers
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    const DriveManager::State& driveState = vehicleManager->drive()->getState();
    const BodyManager::State& bodyState = vehicleManager->body()->getState();
    
    // Determine interval based on vehicle state
    unsigned long reportInterval = vehicleManager->isVehicleAwake() ? 
        REPORT_INTERVAL_AWAKE : REPORT_INTERVAL_ASLEEP;
    
    // Check if interval has elapsed
    if (millis() - lastReportTime >= reportInterval) {
        return true;
    }
    
    // Check for significant changes
    
    // Ignition changed
    if (driveState.ignitionOn != lastIgnitionOn) {
        return true;
    }
    
    // Charging state changed
    if (battState.charging != lastCharging) {
        return true;
    }
    
    // Lock state changed
    if (bodyState.isLocked() != lastLocked) {
        return true;
    }
    
    // Plug state changed
    if (battState.plugState.isPlugged() != lastPlugged) {
        return true;
    }
    
    // SOC changed significantly (use unified soc field)
    float socDiff = abs(battState.soc - lastSoc);
    if (socDiff >= SOC_CHANGE_THRESHOLD) {
        return true;
    }
    
    // Power changed significantly (useful during charging)
    float powerDiff = abs(battState.powerKw - lastPowerKw);
    if (powerDiff >= POWER_CHANGE_THRESHOLD) {
        return true;
    }
    
    // Speed changed significantly (useful when driving)
    float speedDiff = abs(driveState.speedKmh - lastSpeedKmh);
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
        // NEW ARCHITECTURE: Get state from domain managers
        const BatteryManager::State& battState = vehicleManager->battery()->getState();
        const DriveManager::State& driveState = vehicleManager->drive()->getState();
        const BodyManager::State& bodyState = vehicleManager->body()->getState();
        
        lastSoc = battState.soc;
        lastPowerKw = battState.powerKw;
        lastIgnitionOn = driveState.ignitionOn;
        lastCharging = battState.charging;
        lastLocked = bodyState.isLocked();
        lastPlugged = battState.plugState.isPlugged();
        lastSpeedKmh = driveState.speedKmh;
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
    
    // NEW ARCHITECTURE: Get state from domain managers
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    const ClimateManager::State& climState = vehicleManager->climate()->getState();
    const BodyManager::State& bodyState = vehicleManager->body()->getState();
    const DriveManager::State& driveState = vehicleManager->drive()->getState();
    
    // Initialize event tracking on first run (don't emit events for initial state)
    if (!eventsInitialized) {
        eventIgnitionOn = driveState.ignitionOn;
        eventCharging = battState.charging;
        eventLocked = bodyState.isLocked();
        eventPlugged = battState.plugState.isPlugged();
        eventDriverDoorOpen = bodyState.driverDoor.open;
        eventPassengerDoorOpen = bodyState.passengerDoor.open;
        eventRearLeftDoorOpen = bodyState.rearLeftDoor.open;
        eventRearRightDoorOpen = bodyState.rearRightDoor.open;
        eventTrunkOpen = bodyState.trunkOpen;
        eventClimateActive = climState.climateActive;
        eventLastSoc = battState.soc;
        eventsInitialized = true;
        return;
    }
    
    // === Ignition Events ===
    if (driveState.ignitionOn != eventIgnitionOn) {
        eventIgnitionOn = driveState.ignitionOn;
        if (eventIgnitionOn) {
            Serial.println("[VEHICLE] Event: ignitionOn");
            emitEvent("ignitionOn", nullptr);
        } else {
            Serial.println("[VEHICLE] Event: ignitionOff");
            emitEvent("ignitionOff", nullptr);
        }
    }
    
    // === Charging Events ===
    if (battState.charging != eventCharging) {
        eventCharging = battState.charging;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["soc"] = battState.soc;
        details["powerKw"] = battState.powerKw;
        
        if (eventCharging) {
            Serial.println("[VEHICLE] Event: chargingStarted");
            emitEvent("chargingStarted", &details);
        } else {
            Serial.println("[VEHICLE] Event: chargingStopped");
            emitEvent("chargingStopped", &details);
        }
    }
    
    // === Lock Events ===
    if (bodyState.isLocked() != eventLocked) {
        eventLocked = bodyState.isLocked();
        if (eventLocked) {
            Serial.println("[VEHICLE] Event: locked");
            emitEvent("locked", nullptr);
        } else {
            Serial.println("[VEHICLE] Event: unlocked");
            emitEvent("unlocked", nullptr);
        }
    }
    
    // === Plug Events ===
    if (battState.plugState.isPlugged() != eventPlugged) {
        eventPlugged = battState.plugState.isPlugged();
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["hasSupply"] = battState.plugState.hasSupply();
        
        if (eventPlugged) {
            Serial.println("[VEHICLE] Event: plugged");
            emitEvent("plugged", &details);
        } else {
            Serial.println("[VEHICLE] Event: unplugged");
            emitEvent("unplugged", &details);
        }
    }
    
    // === Door Events ===
    if (bodyState.driverDoor.open != eventDriverDoorOpen) {
        eventDriverDoorOpen = bodyState.driverDoor.open;
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
    
    if (bodyState.passengerDoor.open != eventPassengerDoorOpen) {
        eventPassengerDoorOpen = bodyState.passengerDoor.open;
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
    
    if (bodyState.rearLeftDoor.open != eventRearLeftDoorOpen) {
        eventRearLeftDoorOpen = bodyState.rearLeftDoor.open;
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
    
    if (bodyState.rearRightDoor.open != eventRearRightDoorOpen) {
        eventRearRightDoorOpen = bodyState.rearRightDoor.open;
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
    if (bodyState.trunkOpen != eventTrunkOpen) {
        eventTrunkOpen = bodyState.trunkOpen;
        if (eventTrunkOpen) {
            Serial.println("[VEHICLE] Event: trunkOpened");
            emitEvent("trunkOpened", nullptr);
        } else {
            Serial.println("[VEHICLE] Event: trunkClosed");
            emitEvent("trunkClosed", nullptr);
        }
    }
    
    // === Climate Events ===
    if (climState.climateActive != eventClimateActive) {
        eventClimateActive = climState.climateActive;
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["heating"] = climState.heating;
        details["cooling"] = climState.cooling;
        details["temp"] = climState.insideTemp;
        
        if (eventClimateActive) {
            Serial.println("[VEHICLE] Event: climateStarted");
            emitEvent("climateStarted", &details);
        } else {
            Serial.println("[VEHICLE] Event: climateStopped");
            emitEvent("climateStopped", &details);
        }
    }
    
    // === SOC Threshold Events (20%, 50%, 80%, 100%) ===
    float currentSoc = battState.soc;
    if (battState.socSource != DataSource::NONE && battState.soc > 0.0f && eventLastSoc > 0) {
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
    if (battState.socSource != DataSource::NONE && battState.soc > 0.0f && currentSoc < 20 && eventLastSoc >= 20) {
        JsonDocument doc;
        JsonObject details = doc.to<JsonObject>();
        details["soc"] = currentSoc;
        Serial.println("[VEHICLE] Event: lowBattery");
        emitEvent("lowBattery", &details);
    }
}
