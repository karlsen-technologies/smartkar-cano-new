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
    
    // Queue command via BatteryControlChannel (non-blocking) - pass command ID
    bool accepted = vehicleManager->batteryControl().startClimate(ctx.id, tempCelsius, allowBattery);
    
    if (accepted) {
        // Command accepted and will execute in background
        return CommandResult::pending();
    } else {
        // Shouldn't happen (CommandRouter checks busy), but handle anyway
        return CommandResult::error("Internal error - command rejected");
    }
}

CommandResult VehicleHandler::handleStopClimate(CommandContext& ctx) {
    Serial.println("[VEHICLE] Stopping climate control");
    
    // Queue stop command via BatteryControlChannel (non-blocking) - pass command ID
    bool accepted = vehicleManager->batteryControl().stopClimate(ctx.id);
    
    if (accepted) {
        // Command accepted and will execute in background
        return CommandResult::pending();
    } else {
        // Shouldn't happen (CommandRouter checks busy), but handle anyway
        return CommandResult::error("Internal error - command rejected");
    }
}

// ============================================================================
// Charging Control Commands
// ============================================================================

CommandResult VehicleHandler::handleStartCharging(CommandContext& ctx) {
    Serial.println("[VEHICLE] Starting charging");
    
    // Note: We don't check plug state here because:
    // 1. Our state may be stale if device was asleep
    // 2. The vehicle will respond with an error if not plugged in
    // 3. Server/app can prevent command based on last known state
    
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
    
    // Queue command via BatteryControlChannel (non-blocking) - pass command ID
    bool accepted = vehicleManager->batteryControl().startCharging(ctx.id, targetSoc, maxCurrent);
    
    if (accepted) {
        // Command accepted and will execute in background
        return CommandResult::pending();
    } else {
        // Shouldn't happen (CommandRouter checks busy), but handle anyway
        return CommandResult::error("Internal error - command rejected");
    }
}

CommandResult VehicleHandler::handleStopCharging(CommandContext& ctx) {
    Serial.println("[VEHICLE] Stopping charging");
    
    // Queue stop command via BatteryControlChannel (non-blocking) - pass command ID
    bool accepted = vehicleManager->batteryControl().stopCharging(ctx.id);
    
    if (accepted) {
        // Command accepted and will execute in background
        return CommandResult::pending();
    } else {
        // Shouldn't happen (CommandRouter checks busy), but handle anyway
        return CommandResult::error("Internal error - command rejected");
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
    
    // NEW ARCHITECTURE: Get state from domain managers
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    const ClimateManager::State& climState = vehicleManager->climate()->getState();
    const BodyManager::State& bodyState = vehicleManager->body()->getState();
    const DriveManager::State& driveState = vehicleManager->drive()->getState();
    
    CommandResult result = CommandResult::ok();
    
    // Battery state (unified)
    JsonObject battery = result.data["battery"].to<JsonObject>();
    battery["soc"] = battState.soc;
    battery["socSource"] = battState.socSource == DataSource::BAP ? "bap" : "can";
    battery["powerKw"] = battState.powerKw;
    battery["temperature"] = battState.temperature;
    battery["charging"] = battState.charging;
    battery["chargingSource"] = battState.chargingSource == DataSource::BAP ? "bap" : "can";
    
    // Charging details (from BAP) - available when chargingUpdate is non-zero
    if (battState.chargingUpdate > 0) {
        battery["chargingMode"] = battState.chargingMode;
        battery["chargingStatus"] = battState.chargingStatus;
        battery["chargingAmps"] = battState.chargingAmps;
        battery["targetSoc"] = battState.targetSoc;
        battery["remainingMin"] = battState.remainingTimeMin;
    }
    
    // Drive state
    JsonObject drive = result.data["drive"].to<JsonObject>();
    drive["ignitionOn"] = driveState.ignitionOn;
    drive["speedKmh"] = driveState.speedKmh;
    drive["odometerKm"] = driveState.odometerKm;
    
    // Body state
    JsonObject body = result.data["body"].to<JsonObject>();
    body["locked"] = bodyState.isLocked();
    body["anyDoorOpen"] = bodyState.anyDoorOpen();
    body["trunkOpen"] = bodyState.trunkOpen;
    
    // Plug state (BAP) - from battery manager
    if (battState.plugState.isValid()) {
        JsonObject plug = result.data["plug"].to<JsonObject>();
        plug["plugged"] = battState.plugState.isPlugged();
        plug["hasSupply"] = battState.plugState.hasSupply();
        plug["state"] = battState.plugState.plugStateStr();
    }
    
    // Climate state (unified)
    JsonObject climate = result.data["climate"].to<JsonObject>();
    climate["insideTemp"] = climState.insideTemp;
    climate["outsideTemp"] = climState.outsideTemp;
    climate["active"] = climState.climateActive;
    climate["heating"] = climState.heating;
    climate["cooling"] = climState.cooling;
    climate["ventilation"] = climState.ventilation;
    climate["autoDefrost"] = climState.autoDefrost;
    climate["remainingMin"] = climState.climateTimeMin;
    
    // Meta info - keep using VehicleManager for global state
    result.data["vehicleAwake"] = vehicleManager->isVehicleAwake();
    result.data["canFrameCount"] = vehicleManager->getFrameCount();
    
    return result;
}
