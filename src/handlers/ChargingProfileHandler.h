#pragma once

#include <Arduino.h>
#include "../core/ICommandHandler.h"

// Forward declarations
class VehicleManager;
class CommandRouter;

/**
 * ChargingProfileHandler - Handles charging profile management commands
 * 
 * This handler manages the user-configurable timer profiles (1-3).
 * Profile 0 (immediate operations) is handled by VehicleHandler.
 * 
 * Supported commands:
 * - profiles.get           - Get all profiles
 * - profiles.getProfile    - Get a specific profile (params: index)
 * - profiles.updateProfile - Update a profile (params: index, settings)
 * - profiles.setEnabled    - Enable/disable a timer profile (params: index, enabled)
 * - profiles.refresh       - Request fresh profile data from vehicle
 * 
 * Profile settings that can be updated:
 * - targetSoc: Target state of charge (0-100%)
 * - maxCurrent: Maximum charging current (0-32A)
 * - temperature: Climate temperature (15.5-30.0°C)
 * - enableCharging: Enable charging for this profile
 * - enableClimate: Enable climate for this profile
 * - allowBattery: Allow climate to run on battery
 * - leadTime: Minutes before departure to start (0-120)
 * - holdTimePlug: Climate duration when plugged (minutes)
 * - holdTimeBattery: Climate duration on battery (minutes)
 */
class ChargingProfileHandler : public ICommandHandler {
public:
    /**
     * Constructor
     * @param vehicleManager Reference to VehicleManager for profile access
     * @param commandRouter Reference to CommandRouter for event emission
     */
    ChargingProfileHandler(VehicleManager* vehicleManager, CommandRouter* commandRouter);
    
    // ICommandHandler interface
    const char* getDomain() override { return "profiles"; }
    CommandResult handleCommand(CommandContext& ctx) override;
    const char** getSupportedActions(size_t& count) override;

private:
    VehicleManager* vehicleManager = nullptr;
    CommandRouter* commandRouter = nullptr;
    
    // Command handlers
    CommandResult handleGet(CommandContext& ctx);
    CommandResult handleGetProfile(CommandContext& ctx);
    CommandResult handleUpdateProfile(CommandContext& ctx);
    CommandResult handleSetEnabled(CommandContext& ctx);
    CommandResult handleRefresh(CommandContext& ctx);
    
    // Helper to serialize a profile to JSON
    void serializeProfile(uint8_t index, JsonObject& obj);
    
    // Supported actions list
    static const char* supportedActions[];
    static const size_t supportedActionCount;
};
