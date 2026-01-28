#pragma once

#include <Arduino.h>
#include "../core/ICommandHandler.h"

// Forward declarations
class VehicleManager;
class CommandRouter;

/**
 * VehicleHandler - Handles vehicle control commands via BAP protocol
 * 
 * Supported commands:
 * - vehicle.startClimate    - Start climate control (optional: temp parameter)
 * - vehicle.stopClimate     - Stop climate control
 * - vehicle.startCharging   - Start charging
 * - vehicle.stopCharging    - Stop charging
 * - vehicle.requestState    - Request current BAP states (plug, charge, climate)
 * - vehicle.getState        - Get current vehicle state snapshot
 * 
 * These commands are sent to the vehicle via the BAP (Bedien- und Anzeigeprotokoll)
 * protocol over the CAN bus.
 */
class VehicleHandler : public ICommandHandler {
public:
    /**
     * Constructor
     * @param vehicleManager Reference to VehicleManager for BAP domain access
     * @param commandRouter Reference to CommandRouter for event emission
     */
    VehicleHandler(VehicleManager* vehicleManager, CommandRouter* commandRouter);
    
    // ICommandHandler interface
    const char* getDomain() override { return "vehicle"; }
    CommandResult handleCommand(CommandContext& ctx) override;
    const char** getSupportedActions(size_t& count) override;

private:
    VehicleManager* vehicleManager = nullptr;
    CommandRouter* commandRouter = nullptr;
    
    // Command handlers
    CommandResult handleStartClimate(CommandContext& ctx);
    CommandResult handleStopClimate(CommandContext& ctx);
    CommandResult handleStartCharging(CommandContext& ctx);
    CommandResult handleStopCharging(CommandContext& ctx);
    CommandResult handleRequestState(CommandContext& ctx);
    CommandResult handleGetState(CommandContext& ctx);
    
    // Supported actions list
    static const char* supportedActions[];
    static const size_t supportedActionCount;
};
