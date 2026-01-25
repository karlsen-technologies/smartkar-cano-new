#pragma once

#include <Arduino.h>
#include "../core/ICommandHandler.h"

// Forward declarations
class DeviceController;
class CommandRouter;

/**
 * SystemHandler - Handles system/debug commands
 * 
 * Supported commands:
 * - system.reboot     - Restart the device
 * - system.sleep      - Force device to enter deep sleep
 * - system.telemetry  - Force immediate telemetry send
 * - system.info       - Return detailed device info
 * - system.ping       - Simple ping/pong (alias for built-in)
 */
class SystemHandler : public ICommandHandler {
public:
    /**
     * Constructor
     * @param deviceController Reference for sleep/reboot control
     * @param commandRouter Reference for forcing telemetry
     */
    SystemHandler(DeviceController* deviceController, CommandRouter* commandRouter);
    
    // ICommandHandler interface
    const char* getDomain() override { return "system"; }
    CommandResult handleCommand(CommandContext& ctx) override;
    const char** getSupportedActions(size_t& count) override;

private:
    DeviceController* deviceController = nullptr;
    CommandRouter* commandRouter = nullptr;
    
    // Command handlers
    CommandResult handleReboot(CommandContext& ctx);
    CommandResult handleSleep(CommandContext& ctx);
    CommandResult handleTelemetry(CommandContext& ctx);
    CommandResult handleInfo(CommandContext& ctx);
    
    // Supported actions list
    static const char* supportedActions[];
    static const size_t supportedActionCount;
};
