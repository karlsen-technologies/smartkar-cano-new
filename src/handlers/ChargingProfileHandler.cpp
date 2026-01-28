#include "ChargingProfileHandler.h"
#include "../vehicle/VehicleManager.h"
#include "../vehicle/ChargingProfile.h"
#include "../core/CommandRouter.h"

using namespace ChargingProfile;

// Static action list
const char* ChargingProfileHandler::supportedActions[] = {
    "get",
    "getProfile",
    "updateProfile",
    "setEnabled",
    "refresh"
};
const size_t ChargingProfileHandler::supportedActionCount = 5;

ChargingProfileHandler::ChargingProfileHandler(VehicleManager* vehicleManager, 
                                                CommandRouter* commandRouter)
    : vehicleManager(vehicleManager), commandRouter(commandRouter) {
}

CommandResult ChargingProfileHandler::handleCommand(CommandContext& ctx) {
    Serial.printf("[PROFILES] Command: %s\r\n", ctx.actionName.c_str());
    
    if (!vehicleManager) {
        return CommandResult::error("VehicleManager not available");
    }
    
    if (ctx.actionName == "get") {
        return handleGet(ctx);
    }
    else if (ctx.actionName == "getProfile") {
        return handleGetProfile(ctx);
    }
    else if (ctx.actionName == "updateProfile") {
        return handleUpdateProfile(ctx);
    }
    else if (ctx.actionName == "setEnabled") {
        return handleSetEnabled(ctx);
    }
    else if (ctx.actionName == "refresh") {
        return handleRefresh(ctx);
    }
    
    return CommandResult::notSupported();
}

const char** ChargingProfileHandler::getSupportedActions(size_t& count) {
    count = supportedActionCount;
    return supportedActions;
}

// ============================================================================
// Get All Profiles
// ============================================================================

CommandResult ChargingProfileHandler::handleGet(CommandContext& ctx) {
    Serial.println("[PROFILES] Getting all profiles");
    
    CommandResult result = CommandResult::ok();
    ChargingProfileManager& pm = vehicleManager->profiles();
    
    // Create array of profiles
    JsonArray profiles = result.data["profiles"].to<JsonArray>();
    
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        JsonObject profileObj = profiles.add<JsonObject>();
        serializeProfile(i, profileObj);
    }
    
    result.data["profileCount"] = PROFILE_COUNT;
    result.data["lastUpdateTime"] = pm.getLastUpdateTime();
    result.data["updateCount"] = pm.getProfileUpdateCount();
    
    return result;
}

// ============================================================================
// Get Single Profile
// ============================================================================

CommandResult ChargingProfileHandler::handleGetProfile(CommandContext& ctx) {
    // Validate index parameter
    if (!ctx.params["index"].is<int>()) {
        return CommandResult::invalidParams("Missing 'index' parameter");
    }
    
    int index = ctx.params["index"].as<int>();
    if (index < 0 || index >= PROFILE_COUNT) {
        return CommandResult::invalidParams("Index must be 0-3");
    }
    
    Serial.printf("[PROFILES] Getting profile %d\r\n", index);
    
    CommandResult result = CommandResult::ok();
    JsonObject profileObj = result.data["profile"].to<JsonObject>();
    serializeProfile(index, profileObj);
    
    return result;
}

// ============================================================================
// Update Profile
// ============================================================================

CommandResult ChargingProfileHandler::handleUpdateProfile(CommandContext& ctx) {
    // Validate index parameter (only 1-3 for timer profiles)
    if (!ctx.params["index"].is<int>()) {
        return CommandResult::invalidParams("Missing 'index' parameter");
    }
    
    int index = ctx.params["index"].as<int>();
    if (index < 1 || index > 3) {
        return CommandResult::invalidParams("Index must be 1-3 (timer profiles only)");
    }
    
    Serial.printf("[PROFILES] Updating profile %d\r\n", index);
    
    ChargingProfileManager& pm = vehicleManager->profiles();
    Profile& profile = pm.getProfileMutable(index);
    
    // Apply settings from params
    if (ctx.params["targetSoc"].is<int>()) {
        profile.setTargetSoc(ctx.params["targetSoc"].as<int>());
    }
    
    if (ctx.params["maxCurrent"].is<int>()) {
        profile.setMaxCurrent(ctx.params["maxCurrent"].as<int>());
    }
    
    if (ctx.params["temperature"].is<float>()) {
        profile.setTemperature(ctx.params["temperature"].as<float>());
    } else if (ctx.params["temperature"].is<int>()) {
        profile.setTemperature(ctx.params["temperature"].as<int>());
    }
    
    if (ctx.params["enableCharging"].is<bool>()) {
        profile.enableCharging(ctx.params["enableCharging"].as<bool>());
    }
    
    if (ctx.params["enableClimate"].is<bool>()) {
        bool allowBattery = true;
        if (ctx.params["allowBattery"].is<bool>()) {
            allowBattery = ctx.params["allowBattery"].as<bool>();
        }
        profile.enableClimate(ctx.params["enableClimate"].as<bool>(), allowBattery);
    }
    
    if (ctx.params["leadTime"].is<int>()) {
        profile.leadTime = ctx.params["leadTime"].as<int>();
    }
    
    if (ctx.params["holdTimePlug"].is<int>()) {
        profile.holdingTimePlug = ctx.params["holdTimePlug"].as<int>();
    }
    
    if (ctx.params["holdTimeBattery"].is<int>()) {
        profile.holdingTimeBattery = ctx.params["holdTimeBattery"].as<int>();
    }
    
    if (ctx.params["name"].is<const char*>()) {
        profile.setName(ctx.params["name"].as<const char*>());
    }
    
    // Send update to car
    bool success = pm.updateTimerProfile(index, profile);
    
    if (success) {
        CommandResult result = CommandResult::ok("Profile updated");
        JsonObject profileObj = result.data["profile"].to<JsonObject>();
        serializeProfile(index, profileObj);
        
        // Emit event
        if (commandRouter) {
            JsonDocument eventDoc;
            JsonObject details = eventDoc.to<JsonObject>();
            details["index"] = index;
            commandRouter->sendEvent("profiles", "profileUpdated", &details);
        }
        
        return result;
    } else {
        return CommandResult::error("Failed to update profile");
    }
}

// ============================================================================
// Enable/Disable Timer Profile
// ============================================================================

CommandResult ChargingProfileHandler::handleSetEnabled(CommandContext& ctx) {
    // Validate parameters
    if (!ctx.params["index"].is<int>()) {
        return CommandResult::invalidParams("Missing 'index' parameter");
    }
    if (!ctx.params["enabled"].is<bool>()) {
        return CommandResult::invalidParams("Missing 'enabled' parameter");
    }
    
    int index = ctx.params["index"].as<int>();
    if (index < 1 || index > 3) {
        return CommandResult::invalidParams("Index must be 1-3 (timer profiles only)");
    }
    
    bool enabled = ctx.params["enabled"].as<bool>();
    
    Serial.printf("[PROFILES] %s profile %d\r\n", enabled ? "Enabling" : "Disabling", index);
    
    ChargingProfileManager& pm = vehicleManager->profiles();
    bool success = pm.setTimerProfileEnabled(index, enabled);
    
    if (success) {
        CommandResult result = CommandResult::ok(enabled ? "Timer enabled" : "Timer disabled");
        result.data["index"] = index;
        result.data["enabled"] = enabled;
        
        // Emit event
        if (commandRouter) {
            JsonDocument eventDoc;
            JsonObject details = eventDoc.to<JsonObject>();
            details["index"] = index;
            details["enabled"] = enabled;
            commandRouter->sendEvent("profiles", "timerStateChanged", &details);
        }
        
        return result;
    } else {
        return CommandResult::error("Failed to update timer state");
    }
}

// ============================================================================
// Refresh Profiles from Vehicle
// ============================================================================

CommandResult ChargingProfileHandler::handleRefresh(CommandContext& ctx) {
    Serial.println("[PROFILES] Requesting profile refresh from vehicle");
    
    ChargingProfileManager& pm = vehicleManager->profiles();
    bool success = pm.requestAllProfiles();
    
    if (success) {
        return CommandResult::ok("Profile refresh requested");
    } else {
        return CommandResult::error("Failed to request profile refresh");
    }
}

// ============================================================================
// Helper Methods
// ============================================================================

void ChargingProfileHandler::serializeProfile(uint8_t index, JsonObject& obj) {
    const Profile& profile = vehicleManager->profiles().getProfile(index);
    
    obj["index"] = index;
    obj["valid"] = profile.valid;
    obj["lastUpdate"] = profile.lastUpdate;
    
    if (!profile.valid) {
        return;
    }
    
    // Operation flags
    obj["operation"] = profile.operation;
    obj["operation2"] = profile.operation2;
    obj["chargingEnabled"] = profile.isChargingEnabled();
    obj["climateEnabled"] = profile.isClimateEnabled();
    obj["climateAllowBattery"] = profile.isClimateAllowedOnBattery();
    
    // Charging settings
    obj["maxCurrent"] = profile.maxCurrent;
    obj["minChargeLevel"] = profile.minChargeLevel;
    obj["targetChargeLevel"] = profile.targetChargeLevel;
    
    // Climate settings
    obj["temperature"] = profile.getTemperature();
    obj["temperatureUnit"] = profile.temperatureUnit == 0 ? "celsius" : "fahrenheit";
    
    // Timing
    obj["leadTime"] = profile.leadTime;
    obj["holdTimePlug"] = profile.holdingTimePlug;
    obj["holdTimeBattery"] = profile.holdingTimeBattery;
    
    // Name (if present)
    if (profile.nameLength > 0) {
        obj["name"] = profile.name;
    }
    
    // Profile type label
    if (index == PROFILE_IMMEDIATE) {
        obj["type"] = "immediate";
        obj["description"] = "Used for 'start now' operations";
    } else {
        obj["type"] = "timer";
        char desc[32];
        snprintf(desc, sizeof(desc), "Timer %d", index);
        obj["description"] = desc;
    }
}
