#include "CommandRouter.h"
#include "CommandStateManager.h"

CommandRouter* CommandRouter::_instance = nullptr;

CommandRouter::CommandRouter() {
    _instance = this;
    
    // Initialize arrays
    for (size_t i = 0; i < MAX_COMMAND_HANDLERS; i++) {
        handlers[i] = nullptr;
    }
    for (size_t i = 0; i < MAX_TELEMETRY_PROVIDERS; i++) {
        providers[i] = nullptr;
    }
    
    // Set response sender for CommandStateManager
    CommandStateManager::getInstance()->setResponseSender([](const String& message) -> bool {
        if (_instance && _instance->responseSender) {
            return _instance->responseSender(message);
        }
        return false;
    });
}

bool CommandRouter::registerHandler(ICommandHandler* handler) {
    if (!handler || handlerCount >= MAX_COMMAND_HANDLERS) {
        return false;
    }
    
    // Check for duplicate domain
    for (size_t i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i]->getDomain(), handler->getDomain()) == 0) {
            Serial.printf("[ROUTER] Handler for domain '%s' already registered\r\n", handler->getDomain());
            return false;
        }
    }
    
    handlers[handlerCount++] = handler;
    Serial.printf("[ROUTER] Registered handler for domain '%s'\r\n", handler->getDomain());
    return true;
}

bool CommandRouter::registerProvider(ITelemetryProvider* provider) {
    if (!provider || providerCount >= MAX_TELEMETRY_PROVIDERS) {
        return false;
    }
    
    providers[providerCount++] = provider;
    Serial.printf("[ROUTER] Registered telemetry provider '%s'\r\n", provider->getTelemetryDomain());
    return true;
}

void CommandRouter::handleCommand(const String& action, int id, JsonObject& params) {
    Serial.printf("[ROUTER] Command: %s (id=%d)\r\n", action.c_str(), id);
    
    // Try built-in system commands first (they bypass busy check)
    if (handleSystemCommand(action, id, params)) {
        return;
    }
    
    // Check if another command is active (BUSY CHECK)
    CommandStateManager* csm = CommandStateManager::getInstance();
    if (csm->hasActiveCommand()) {
        // Build busy response with current command info
        Serial.printf("[ROUTER] Busy - rejecting command %d (current: %d)\r\n", 
                      id, csm->getCurrentCommandId());
        
        JsonDocument busyDoc;
        JsonObject busyData = busyDoc["data"].to<JsonObject>();
        busyData["id"] = id;
        busyData["ok"] = false;
        busyData["status"] = "busy";
        busyData["error"] = "Another command is in progress";
        
        JsonObject currentCmd = busyData["currentCommand"].to<JsonObject>();
        csm->getCurrentCommandInfo(currentCmd);
        
        // Send and return
        String busyMsg;
        serializeJson(busyDoc, busyMsg);
        responseSender(busyMsg);
        return;
    }
    
    // Parse action into domain.actionName
    String domain, actionName;
    if (!parseAction(action, domain, actionName)) {
        // No domain prefix - unknown command
        sendResponse(id, CommandStatus::NOT_SUPPORTED, "Unknown command format");
        return;
    }
    
    // Find handler for domain
    ICommandHandler* handler = findHandler(domain);
    if (!handler) {
        sendResponse(id, CommandStatus::NOT_SUPPORTED, "Unknown domain");
        return;
    }
    
    // Start tracking this command
    csm->startCommand(id, action);
    
    // Create command context
    CommandContext ctx(id, action, domain, actionName, params);
    ctx.sendAsyncResponse = asyncResponseCallback;
    
    // Execute command
    CommandResult result = handler->handleCommand(ctx);
    
    // Handle different result statuses
    if (result.status == CommandStatus::PENDING) {
        // Command accepted and executing in background
        // CommandStateManager will send further updates
        return;
    }
    
    if (result.status == CommandStatus::OK) {
        // Synchronous command completed immediately
        csm->completeCommand(result.data.size() > 0 ? &result.data : nullptr);
        return;
    }
    
    // Validation or execution error
    if (result.status == CommandStatus::INVALID_PARAMS ||
        result.status == CommandStatus::NOT_SUPPORTED ||
        result.status == CommandStatus::CMD_ERROR) {
        csm->failCommand(result.message.c_str());
        return;
    }
}

String CommandRouter::collectTelemetry(bool onlyChanged) {
    if (providerCount == 0) {
        return "";
    }
    
    JsonDocument doc;
    doc["type"] = "state";
    JsonObject data = doc["data"].to<JsonObject>();
    
    bool hasData = false;
    
    for (size_t i = 0; i < providerCount; i++) {
        ITelemetryProvider* provider = providers[i];
        
        if (onlyChanged && !provider->hasChanged()) {
            continue;
        }
        
        JsonObject domainData = data[provider->getTelemetryDomain()].to<JsonObject>();
        provider->getTelemetry(domainData);
        provider->onTelemetrySent();
        hasData = true;
    }
    
    if (!hasData) {
        return "";
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

TelemetryPriority CommandRouter::getHighestPriority() {
    TelemetryPriority highest = TelemetryPriority::PRIORITY_LOW;
    
    for (size_t i = 0; i < providerCount; i++) {
        if (providers[i]->hasChanged()) {
            TelemetryPriority p = providers[i]->getPriority();
            if (p > highest) {
                highest = p;
            }
        }
    }
    
    return highest;
}

void CommandRouter::getCapabilities(JsonDocument& doc) {
    JsonObject domains = doc["domains"].to<JsonObject>();
    
    for (size_t i = 0; i < handlerCount; i++) {
        ICommandHandler* handler = handlers[i];
        size_t actionCount = 0;
        const char** actions = handler->getSupportedActions(actionCount);
        
        JsonArray domainActions = domains[handler->getDomain()].to<JsonArray>();
        for (size_t j = 0; j < actionCount; j++) {
            domainActions.add(actions[j]);
        }
    }
    
    JsonArray telemetry = doc["telemetry"].to<JsonArray>();
    for (size_t i = 0; i < providerCount; i++) {
        telemetry.add(providers[i]->getTelemetryDomain());
    }
}

void CommandRouter::sendEvent(const char* domain, const char* event, JsonObject* details) {
    if (!responseSender) return;
    
    JsonDocument doc;
    doc["type"] = "event";
    JsonObject data = doc["data"].to<JsonObject>();
    data["domain"] = domain;
    data["name"] = event;  // Protocol v2: use "name" instead of "event"
    
    // Flatten details into data (Protocol v2)
    if (details) {
        for (JsonPair kv : *details) {
            data[kv.key()] = kv.value();
        }
    }
    
    String output;
    serializeJson(doc, output);
    responseSender(output);
}

// Private methods

ICommandHandler* CommandRouter::findHandler(const String& domain) {
    for (size_t i = 0; i < handlerCount; i++) {
        if (domain.equals(handlers[i]->getDomain())) {
            return handlers[i];
        }
    }
    return nullptr;
}

bool CommandRouter::parseAction(const String& action, String& domain, String& actionName) {
    int dotIndex = action.indexOf('.');
    if (dotIndex <= 0 || dotIndex >= (int)action.length() - 1) {
        return false;
    }
    
    domain = action.substring(0, dotIndex);
    actionName = action.substring(dotIndex + 1);
    return true;
}

void CommandRouter::sendResponse(int id, CommandStatus status, const char* message, JsonDocument* data) {
    if (!responseSender) {
        Serial.println("[ROUTER] No response sender configured");
        return;
    }
    
    JsonDocument doc;
    doc["type"] = "response";
    JsonObject respData = doc["data"].to<JsonObject>();
    respData["id"] = id;
    
    // Protocol v2: ok boolean + status string for non-ok responses
    bool ok = (status == CommandStatus::OK);
    respData["ok"] = ok;
    
    if (!ok) {
        // Map status to string for error cases
        const char* statusStr;
        switch (status) {
            case CommandStatus::PENDING:        statusStr = "pending"; break;
            case CommandStatus::INVALID_PARAMS: statusStr = "error"; break;
            case CommandStatus::NOT_SUPPORTED:  statusStr = "not_supported"; break;
            case CommandStatus::BUSY:           statusStr = "error"; break;
            case CommandStatus::CMD_ERROR:
            default:                            statusStr = "error"; break;
        }
        respData["status"] = statusStr;
    }
    
    if (message && strlen(message) > 0) {
        if (ok) {
            respData["message"] = message;
        } else {
            respData["error"] = message;
        }
    }
    
    // Flatten response data into respData (Protocol v2)
    if (data && data->size() > 0) {
        JsonObject dataObj = data->as<JsonObject>();
        for (JsonPair kv : dataObj) {
            respData[kv.key()] = kv.value();
        }
    }
    
    String output;
    serializeJson(doc, output);
    responseSender(output);
}

bool CommandRouter::handleSystemCommand(const String& action, int id, JsonObject& params) {
    if (action == "ping") {
        JsonDocument data;
        data["pong"] = true;
        data["time"] = millis();
        sendResponse(id, CommandStatus::OK, nullptr, &data);
        return true;
    }
    
    if (action == "status") {
        JsonDocument data;
        data["uptime"] = millis();
        data["freeHeap"] = ESP.getFreeHeap();
        sendResponse(id, CommandStatus::OK, nullptr, &data);
        return true;
    }
    
    if (action == "capabilities") {
        JsonDocument data;
        getCapabilities(data);
        sendResponse(id, CommandStatus::OK, nullptr, &data);
        return true;
    }
    
    if (action == "telemetry") {
        // Force immediate telemetry send
        String telemetry = collectTelemetry(false);
        if (telemetry.length() > 0 && responseSender) {
            responseSender(telemetry);
        }
        sendResponse(id, CommandStatus::OK);
        return true;
    }
    
    return false;
}

void CommandRouter::asyncResponseCallback(int id, CommandResult result) {
    CommandStateManager* csm = CommandStateManager::getInstance();
    
    if (result.status == CommandStatus::OK) {
        // Async command completed successfully
        csm->completeCommand(result.data.size() > 0 ? &result.data : nullptr);
    } else {
        // Async command failed
        csm->failCommand(result.message.c_str());
    }
}
