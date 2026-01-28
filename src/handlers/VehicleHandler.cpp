#include "VehicleHandler.h"
#include "../vehicle/VehicleManager.h"
#include "../core/CommandRouter.h"

// Static action list
const char* VehicleHandler::supportedActions[] = {
    "startClimate",
    "stopClimate",
    "startCharging",
    "stopCharging",
    "requestState",
    "getState"
};
const size_t VehicleHandler::supportedActionCount = 6;

VehicleHandler::VehicleHandler(VehicleManager* vehicleManager, CommandRouter* commandRouter)
    : vehicleManager(vehicleManager), commandRouter(commandRouter) {
}

CommandResult VehicleHandler::handleCommand(CommandContext& ctx) {
    Serial.printf("[VEHICLE] Command: %s\r\n", ctx.actionName.c_str());
    
    if (!vehicleManager) {
        return CommandResult::error("VehicleManager not available");
    }
    
    if (ctx.actionName == "startClimate") {
        return handleStartClimate(ctx);
    }
    else if (ctx.actionName == "stopClimate") {
        return handleStopClimate(ctx);
    }
    else if (ctx.actionName == "startCharging") {
        return handleStartCharging(ctx);
    }
    else if (ctx.actionName == "stopCharging") {
        return handleStopCharging(ctx);
    }
    else if (ctx.actionName == "requestState") {
        return handleRequestState(ctx);
    }
    else if (ctx.actionName == "getState") {
        return handleGetState(ctx);
    }
    
    return CommandResult::notSupported();
}

const char** VehicleHandler::getSupportedActions(size_t& count) {
    count = supportedActionCount;
    return supportedActions;
}

// ============================================================================
// Climate Control Commands
// ============================================================================

CommandResult VehicleHandler::handleStartClimate(CommandContext& ctx) {
    // Extract optional temperature parameter (default 21.0°C)
    float tempCelsius = 21.0f;
    if (ctx.params["temp"].is<float>()) {
        tempCelsius = ctx.params["temp"].as<float>();
    } else if (ctx.params["temperature"].is<float>()) {
        tempCelsius = ctx.params["temperature"].as<float>();
    }
    
    // Validate temperature range (BAP protocol accepts 15.5 - 30.0°C)
    if (tempCelsius < 15.5f || tempCelsius > 30.0f) {
        return CommandResult::invalidParams("Temperature must be between 15.5 and 30.0°C");
    }
    
    // Check for allowBattery parameter (default true)
    bool allowBattery = true;
    if (ctx.params["allowBattery"].is<bool>()) {
        allowBattery = ctx.params["allowBattery"].as<bool>();
    }
    
    Serial.printf("[VEHICLE] Starting climate control at %.1f°C (battery=%s)\r\n", 
                  tempCelsius, allowBattery ? "yes" : "no");
    
    // Use ChargingProfileManager for proper profile 0 configuration + trigger
    bool queued = vehicleManager->profiles().startClimateNow(tempCelsius, allowBattery);
    
    if (queued) {
        CommandResult result = CommandResult::ok("Climate start command queued");
        result.data["temperature"] = tempCelsius;
        result.data["allowBattery"] = allowBattery;
        result.data["status"] = "queued";
        
        // Emit event for climate start request
        if (commandRouter) {
            JsonDocument eventDoc;
            JsonObject details = eventDoc.to<JsonObject>();
            details["temperature"] = tempCelsius;
            details["allowBattery"] = allowBattery;
            commandRouter->sendEvent("vehicle", "climateStartRequested", &details);
        }
        
        return result;
    } else {
        return CommandResult::error("Command busy - another command in progress");
    }
}

CommandResult VehicleHandler::handleStopClimate(CommandContext& ctx) {
    Serial.println("[VEHICLE] Stopping climate control");
    
    // Use ChargingProfileManager for proper stop sequence
    bool queued = vehicleManager->profiles().stopClimateNow();
    
    if (queued) {
        // Emit event for climate stop request
        if (commandRouter) {
            commandRouter->sendEvent("vehicle", "climateStopRequested", nullptr);
        }
        
        CommandResult result = CommandResult::ok("Climate stop command queued");
        result.data["status"] = "queued";
        return result;
    } else {
        return CommandResult::error("Command busy - another command in progress");
    }
}

// ============================================================================
// Charging Control Commands
// ============================================================================

CommandResult VehicleHandler::handleStartCharging(CommandContext& ctx) {
    Serial.println("[VEHICLE] Starting charging");
    
    // Check if vehicle is plugged in first
    VehicleState state = vehicleManager->getStateCopy();
    if (!state.bapPlug.isPlugged()) {
        return CommandResult::error("Vehicle is not plugged in");
    }
    
    // Extract optional parameters
    uint8_t targetSoc = 80;  // Default to 80%
    uint8_t maxCurrent = 32; // Default to max
    
    if (ctx.params["targetSoc"].is<int>()) {
        targetSoc = ctx.params["targetSoc"].as<int>();
        if (targetSoc > 100) targetSoc = 100;
    }
    if (ctx.params["maxCurrent"].is<int>()) {
        maxCurrent = ctx.params["maxCurrent"].as<int>();
        if (maxCurrent > 32) maxCurrent = 32;
    }
    
    Serial.printf("[VEHICLE] Charging params: targetSoc=%d%%, maxCurrent=%dA\r\n", targetSoc, maxCurrent);
    
    // Use ChargingProfileManager for proper profile 0 configuration + trigger
    bool queued = vehicleManager->profiles().startChargingNow(targetSoc, maxCurrent);
    
    if (queued) {
        CommandResult result = CommandResult::ok("Charging start command queued");
        result.data["targetSoc"] = targetSoc;
        result.data["maxCurrent"] = maxCurrent;
        result.data["status"] = "queued";
        
        // Emit event for charging start request
        if (commandRouter) {
            JsonDocument eventDoc;
            JsonObject details = eventDoc.to<JsonObject>();
            details["targetSoc"] = targetSoc;
            details["maxCurrent"] = maxCurrent;
            commandRouter->sendEvent("vehicle", "chargingStartRequested", &details);
        }
        
        return result;
    } else {
        return CommandResult::error("Command busy - another command in progress");
    }
}

CommandResult VehicleHandler::handleStopCharging(CommandContext& ctx) {
    Serial.println("[VEHICLE] Stopping charging");
    
    // Use ChargingProfileManager for proper stop sequence
    bool queued = vehicleManager->profiles().stopChargingNow();
    
    if (queued) {
        // Emit event for charging stop request
        if (commandRouter) {
            commandRouter->sendEvent("vehicle", "chargingStopRequested", nullptr);
        }
        
        CommandResult result = CommandResult::ok("Charging stop command queued");
        result.data["status"] = "queued";
        return result;
    } else {
        return CommandResult::error("Command busy - another command in progress");
    }
}

// ============================================================================
// State Request Commands
// ============================================================================

CommandResult VehicleHandler::handleRequestState(CommandContext& ctx) {
    Serial.println("[VEHICLE] Requesting BAP states...");
    
    // Request all BAP states from the vehicle
    bool plugOk = vehicleManager->batteryControl().requestPlugState();
    bool chargeOk = vehicleManager->batteryControl().requestChargeState();
    bool climateOk = vehicleManager->batteryControl().requestClimateState();
    
    CommandResult result = CommandResult::ok("State requests sent");
    result.data["plugRequested"] = plugOk;
    result.data["chargeRequested"] = chargeOk;
    result.data["climateRequested"] = climateOk;
    
    return result;
}

CommandResult VehicleHandler::handleGetState(CommandContext& ctx) {
    Serial.println("[VEHICLE] Getting current vehicle state...");
    
    // Get current state snapshot
    VehicleState state = vehicleManager->getStateCopy();
    
    CommandResult result = CommandResult::ok();
    
    // Battery state
    JsonObject battery = result.data["battery"].to<JsonObject>();
    battery["soc"] = state.battery.usableSoc;
    battery["voltage"] = state.battery.voltage;
    battery["current"] = state.battery.current;
    battery["powerKw"] = state.battery.powerKw;
    battery["temperature"] = state.battery.temperature;
    battery["charging"] = state.battery.chargingActive;
    
    // Drive state
    JsonObject drive = result.data["drive"].to<JsonObject>();
    drive["ignitionOn"] = state.drive.ignitionOn;
    drive["speedKmh"] = state.drive.speedKmh;
    drive["odometerKm"] = state.drive.odometerKm;
    
    // Body state
    JsonObject body = result.data["body"].to<JsonObject>();
    body["locked"] = state.body.isLocked();
    body["anyDoorOpen"] = state.body.anyDoorOpen();
    body["trunkOpen"] = state.body.trunkOpen;
    
    // Plug state (BAP)
    if (state.bapPlug.isValid()) {
        JsonObject plug = result.data["plug"].to<JsonObject>();
        plug["plugged"] = state.bapPlug.isPlugged();
        plug["hasSupply"] = state.bapPlug.hasSupply();
        plug["state"] = state.bapPlug.plugStateStr();
    }
    
    // Charge state (BAP)
    if (state.bapCharge.isValid()) {
        JsonObject charge = result.data["charge"].to<JsonObject>();
        charge["mode"] = state.bapCharge.chargeModeStr();
        charge["status"] = state.bapCharge.chargeStatusStr();
        charge["soc"] = state.bapCharge.socPercent;
        charge["remainingMin"] = state.bapCharge.remainingTimeMin;
    }
    
    // Climate state (BAP)
    if (state.bapClimate.isValid()) {
        JsonObject climate = result.data["climate"].to<JsonObject>();
        climate["active"] = state.bapClimate.climateActive;
        climate["heating"] = state.bapClimate.heating;
        climate["cooling"] = state.bapClimate.cooling;
        climate["currentTemp"] = state.bapClimate.currentTempC;
    }
    
    // Meta info
    result.data["vehicleAwake"] = state.isAwake();
    result.data["canFrameCount"] = state.canFrameCount;
    
    return result;
}
