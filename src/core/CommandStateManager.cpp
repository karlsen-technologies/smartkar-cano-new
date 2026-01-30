#include "CommandStateManager.h"

// Initialize static instance
CommandStateManager* CommandStateManager::_instance = nullptr;

CommandStateManager::CommandStateManager() {
    // Private constructor
}

CommandStateManager* CommandStateManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new CommandStateManager();
    }
    return _instance;
}

bool CommandStateManager::hasActiveCommand() const {
    return currentCommandId != -1;
}

int CommandStateManager::getCurrentCommandId() const {
    return currentCommandId;
}

const String& CommandStateManager::getCurrentAction() const {
    return currentAction;
}

CommandStateManager::Stage CommandStateManager::getCurrentStage() const {
    return currentStage;
}

unsigned long CommandStateManager::getElapsedMs() const {
    if (!hasActiveCommand()) {
        return 0;
    }
    return millis() - commandStartTime;
}

void CommandStateManager::startCommand(int commandId, const String& action) {
    // Store command info
    currentCommandId = commandId;
    currentAction = action;
    currentStage = Stage::ACCEPTED;
    commandStartTime = millis();
    failureReason = "";
    
    // Log command start
    Serial.printf("[CMD] Command %d started: %s\r\n", commandId, action.c_str());
    
    // Send initial "accepted" response
    sendResponse("in_progress", true);
}

void CommandStateManager::updateStage(Stage stage) {
    if (!hasActiveCommand()) {
        Serial.println("[CMD] Warning: updateStage called with no active command");
        return;
    }
    
    // Update stage
    Stage oldStage = currentStage;
    currentStage = stage;
    
    // Log stage transition
    Serial.printf("[CMD] Command %d: %s -> %s\r\n", 
                  currentCommandId,
                  getStageString(oldStage),
                  getStageString(stage));
    
    // Send progress response
    sendResponse("in_progress", true);
}

void CommandStateManager::completeCommand(JsonDocument* data) {
    if (!hasActiveCommand()) {
        Serial.println("[CMD] Warning: completeCommand called with no active command");
        return;
    }
    
    // Log completion
    unsigned long elapsed = getElapsedMs();
    Serial.printf("[CMD] Command %d completed in %lu ms\r\n", currentCommandId, elapsed);
    
    // Send completion response
    sendResponse("completed", true, nullptr, data);
    
    // Clear state
    clearState();
}

void CommandStateManager::failCommand(const char* reason) {
    if (!hasActiveCommand()) {
        Serial.println("[CMD] Warning: failCommand called with no active command");
        return;
    }
    
    // Store failure reason
    failureReason = reason;
    
    // Log failure
    unsigned long elapsed = getElapsedMs();
    Serial.printf("[CMD] Command %d failed after %lu ms: %s\r\n", 
                  currentCommandId, elapsed, reason);
    
    // Send failure response
    sendResponse("failed", false, reason);
    
    // Clear state
    clearState();
}

void CommandStateManager::getCurrentCommandInfo(JsonObject& obj) const {
    if (!hasActiveCommand()) {
        return;
    }
    
    obj["id"] = currentCommandId;
    obj["action"] = currentAction;
    obj["stage"] = getStageString(currentStage);
    obj["elapsedMs"] = getElapsedMs();
}

const char* CommandStateManager::getStageString(Stage stage) {
    switch (stage) {
        case Stage::NONE:
            return "none";
        case Stage::ACCEPTED:
            return "accepted";
        case Stage::REQUESTING_WAKE:
            return "requesting_wake";
        case Stage::WAITING_FOR_WAKE:
            return "waiting_for_wake";
        case Stage::UPDATING_PROFILE:
            return "updating_profile";
        case Stage::SENDING_COMMAND:
            return "sending_command";
        case Stage::COMPLETED:
            return "completed";
        case Stage::FAILED:
            return "failed";
        default:
            return "unknown";
    }
}

void CommandStateManager::setResponseSender(bool (*sender)(const String& message)) {
    responseSender = sender;
}

void CommandStateManager::sendResponse(const char* status, bool ok, const char* error, JsonDocument* data) {
    if (responseSender == nullptr) {
        Serial.println("[CMD] Warning: No response sender configured");
        return;
    }
    
    // Build response message
    JsonDocument doc;
    doc["type"] = "response";
    
    JsonObject respData = doc["data"].to<JsonObject>();
    respData["id"] = currentCommandId;
    respData["ok"] = ok;
    respData["status"] = status;
    
    // Add stage for in_progress and failed responses
    if (strcmp(status, "in_progress") == 0 || strcmp(status, "failed") == 0) {
        respData["stage"] = getStageString(currentStage);
    }
    
    // Add elapsed time if available
    if (hasActiveCommand()) {
        respData["elapsedMs"] = getElapsedMs();
    }
    
    // Add error if provided
    if (error != nullptr) {
        respData["error"] = error;
    }
    
    // Add additional data if provided
    if (data != nullptr && data->size() > 0) {
        // Merge data into respData
        for (JsonPair kv : data->as<JsonObject>()) {
            respData[kv.key()] = kv.value();
        }
    }
    
    // Serialize and send
    String message;
    serializeJson(doc, message);
    
    bool sent = responseSender(message);
    if (!sent) {
        Serial.println("[CMD] Warning: Failed to send response");
    }
}

void CommandStateManager::clearState() {
    currentCommandId = -1;
    currentAction = "";
    currentStage = Stage::NONE;
    commandStartTime = 0;
    failureReason = "";
}
