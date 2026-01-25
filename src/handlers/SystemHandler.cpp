#include "SystemHandler.h"
#include "../core/DeviceController.h"
#include "../core/CommandRouter.h"

// Static action list
const char* SystemHandler::supportedActions[] = {
    "reboot",
    "sleep",
    "telemetry",
    "info"
};
const size_t SystemHandler::supportedActionCount = 4;

SystemHandler::SystemHandler(DeviceController* deviceController, CommandRouter* commandRouter)
    : deviceController(deviceController), commandRouter(commandRouter) {
}

CommandResult SystemHandler::handleCommand(CommandContext& ctx) {
    Serial.printf("[SYSTEM] Command: %s\n", ctx.actionName.c_str());
    
    if (ctx.actionName == "reboot") {
        return handleReboot(ctx);
    }
    else if (ctx.actionName == "sleep") {
        return handleSleep(ctx);
    }
    else if (ctx.actionName == "telemetry") {
        return handleTelemetry(ctx);
    }
    else if (ctx.actionName == "info") {
        return handleInfo(ctx);
    }
    
    return CommandResult::notSupported();
}

const char** SystemHandler::getSupportedActions(size_t& count) {
    count = supportedActionCount;
    return supportedActions;
}

CommandResult SystemHandler::handleReboot(CommandContext& ctx) {
    Serial.println("[SYSTEM] Rebooting device...");
    Serial.flush();
    
    // Small delay to allow response to be sent
    delay(100);
    
    ESP.restart();
    
    // Won't reach here, but return OK anyway
    return CommandResult::ok("Rebooting");
}

CommandResult SystemHandler::handleSleep(CommandContext& ctx) {
    Serial.println("[SYSTEM] Sleep requested via command...");
    
    if (!deviceController) {
        return CommandResult::error("DeviceController not available");
    }
    
    // Extract optional duration parameter (in seconds)
    // Default to 0 = wake on interrupt only
    unsigned long durationSeconds = 0;
    if (ctx.params["duration"].is<unsigned long>()) {
        durationSeconds = ctx.params["duration"].as<unsigned long>();
    }
    
    // Request sleep - it will happen on next loop iteration
    // after the response is sent and modules are ready
    deviceController->requestSleep(durationSeconds);
    
    CommandResult result = CommandResult::ok("Sleep requested");
    result.data["duration"] = durationSeconds;
    if (durationSeconds > 0) {
        result.data["wakeMode"] = "timer_or_interrupt";
    } else {
        result.data["wakeMode"] = "interrupt_only";
    }
    return result;
}

CommandResult SystemHandler::handleTelemetry(CommandContext& ctx) {
    Serial.println("[SYSTEM] Forcing telemetry send...");
    
    if (!commandRouter) {
        return CommandResult::error("CommandRouter not available");
    }
    
    // Collect and send telemetry immediately (include unchanged data)
    String telemetry = commandRouter->collectTelemetry(false);
    
    if (telemetry.length() == 0) {
        return CommandResult::ok("No telemetry providers registered");
    }
    
    // The telemetry is collected but not sent here - that happens
    // through the normal LinkManager flow. However, collectTelemetry(false)
    // forces collection of all data regardless of hasChanged().
    
    // We could also trigger the LinkManager to send it, but for now
    // the response confirms the collection happened.
    
    CommandResult result = CommandResult::ok("Telemetry collected");
    result.data["collected"] = true;
    return result;
}

CommandResult SystemHandler::handleInfo(CommandContext& ctx) {
    Serial.println("[SYSTEM] Returning device info...");
    
    CommandResult result = CommandResult::ok();
    
    // Chip info
    result.data["chipModel"] = ESP.getChipModel();
    result.data["chipRevision"] = ESP.getChipRevision();
    result.data["chipCores"] = ESP.getChipCores();
    result.data["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    
    // Memory
    result.data["freeHeap"] = ESP.getFreeHeap();
    result.data["minFreeHeap"] = ESP.getMinFreeHeap();
    result.data["heapSize"] = ESP.getHeapSize();
    
    // Flash
    result.data["flashSize"] = ESP.getFlashChipSize();
    result.data["flashSpeed"] = ESP.getFlashChipSpeed();
    
    // SDK/firmware
    result.data["sdkVersion"] = ESP.getSdkVersion();
    
    // Runtime
    result.data["uptime"] = millis();
    
    return result;
}
