#include "LinkManager.h"
#include "../util.h"

#include <Arduino.h>
#include <ArduinoJson.h>

LinkManager* LinkManager::_instance = nullptr;

LinkManager::LinkManager(ModemManager* modemManager, CommandRouter* commandRouter)
    : modemManager(modemManager), commandRouter(commandRouter) {
    _instance = this;
}

LinkManager::~LinkManager() {
    if (client) {
        delete client;
        client = nullptr;
    }
}

bool LinkManager::setup() {
    Serial.println("[LINK] Setting up link manager");
    
    // Client will be created when modem is ready
    setState(LinkState::DISCONNECTED);
    
    // Wire up the command router to send responses through us
    if (commandRouter) {
        commandRouter->setResponseSender(responseSender);
    }
    
    Serial.println("[LINK] Setup complete");
    return true;
}

void LinkManager::loop() {
    // Only operate when modem is connected
    if (!modemManager->isConnected()) {
        if (state != LinkState::DISCONNECTED) {
            Serial.println("[LINK] Modem disconnected, resetting state");
            setState(LinkState::DISCONNECTED);
        }
        return;
    }

    // Ensure we have a TCP client
    if (!client) {
        client = modemManager->createClient();
        if (!client) {
            return;
        }
    }

    // NOTE: Data processing happens via interrupt (handleTCPInterrupt)
    // We do NOT poll client->available() here to avoid AT command spam

    // State machine
    switch (state) {
        case LinkState::DISCONNECTED:
            // Attempt to connect with backoff
            if (millis_since(lastConnectAttempt) > CONNECT_RETRY_DELAY) {
                if (connect()) {
                    // connect() returned true = connection established
                    Serial.println("[LINK] TCP connected, authenticating");
                    setState(LinkState::AUTHENTICATING);
                    sendAuth();
                }
                lastConnectAttempt = millis();
            }
            break;

        case LinkState::CONNECTING:
            // This state is no longer used for normal flow
            // connect() is synchronous - success goes directly to AUTHENTICATING
            // This handles edge cases where we might end up here
            if (timeInState() > 10000) {
                Serial.println("[LINK] Connection timeout");
                setState(LinkState::LINK_ERROR);
            }
            break;

        case LinkState::AUTHENTICATING:
            // Wait for auth response
            if (timeInState() > AUTH_TIMEOUT) {
                Serial.println("[LINK] Auth timeout");
                disconnect();
                setState(LinkState::DISCONNECTED);
            }
            break;

        case LinkState::CONNECTED:
            // Normal operation - data processing happens via interrupt
            checkTelemetry();
            
            // Periodic connection health check (fallback for missed interrupts)
            if (millis_since(lastConnectionCheck) > 60000) {
                if (!client->connected()) {
                    Serial.println("[LINK] Connection lost (periodic check)");
                    setState(LinkState::DISCONNECTED);
                }
                lastConnectionCheck = millis();
            }
            break;

        case LinkState::REJECTED:
            // Server rejected us - wait before retrying
            if (timeInState() > 60000) {  // 1 minute
                setState(LinkState::DISCONNECTED);
            }
            break;

        case LinkState::LINK_ERROR:
            // Error state - reset after delay
            if (timeInState() > CONNECT_RETRY_DELAY) {
                disconnect();
                setState(LinkState::DISCONNECTED);
                connectAttempts++;
            }
            break;
    }

    previousState = state;
}

void LinkManager::prepareForSleep() {
    Serial.println("[LINK] Preparing for sleep");
    
    // Notify server we're going to sleep with bye message
    if (state == LinkState::CONNECTED && client && client->connected()) {
        sendBye("sleep");
        delay(100);  // Give time for message to send
        client->stop();
    }
    
    setState(LinkState::DISCONNECTED);
}

bool LinkManager::isBusy() {
    // Busy during connection and authentication
    return state == LinkState::CONNECTING || state == LinkState::AUTHENTICATING;
}

bool LinkManager::isReady() {
    return state == LinkState::CONNECTED;
}

bool LinkManager::send(const String& message) {
    if (!client || !client->connected() || state != LinkState::CONNECTED) {
        return false;
    }
    
    client->println(message);
    
    // NOTE: We intentionally do NOT call activityCallback here.
    // Outbound messages (telemetry) should not reset the sleep timer.
    // Only incoming messages from server should keep the device awake.
    
    return true;
}

bool LinkManager::sendBye(const String& reason) {
    // Protocol v2: {"type":"bye","data":{"reason":"sleep"}}
    String msg = "{\"type\":\"bye\",\"data\":{\"reason\":\"";
    msg += reason;
    msg += "\"}}";
    
    if (!client || !client->connected()) {
        return false;
    }
    
    client->println(msg);
    
    // NOTE: We intentionally do NOT call activityCallback here.
    // Sending bye is the last thing before sleep, should not reset timer.
    
    return true;
}

bool LinkManager::sendTelemetryNow(bool changedOnly) {
    if (!commandRouter) {
        Serial.println("[LINK] No command router for telemetry");
        return false;
    }
    
    if (state != LinkState::CONNECTED || !client || !client->connected()) {
        Serial.println("[LINK] Cannot send telemetry - not connected");
        return false;
    }
    
    String telemetry = commandRouter->collectTelemetry(changedOnly);
    if (telemetry.length() == 0) {
        Serial.println("[LINK] No telemetry to send");
        return false;
    }
    
    Serial.println("[LINK] Sending telemetry now");
    bool sent = send(telemetry);
    
    if (sent) {
        lastTelemetryTime = millis();
    }
    
    return sent;
}

void LinkManager::handleTCPInterrupt() {
    if (!client) return;
    
    // TinyGSM's handleURCs() already processed the URC and set:
    // - got_data = true (for +CADATAIND or +CARECV)
    // - sock_connected = false (for +CASTATE with state != 1)
    
    // Check if connection was lost
    if (!client->connected()) {
        Serial.println("[LINK] TCP disconnected via interrupt");
        setState(LinkState::DISCONNECTED);
        if (activityCallback) activityCallback();
        return;
    }
    
    // Process any incoming data
    // client->available() will now work because got_data was set by handleURCs()
    processIncomingData();
    
    if (activityCallback) activityCallback();
}

// Static response sender for CommandRouter
bool LinkManager::responseSender(const String& message) {
    if (_instance) {
        return _instance->send(message);
    }
    return false;
}

// State machine helpers

bool LinkManager::stateJustChanged() {
    return previousState != state;
}

void LinkManager::setState(LinkState newState) {
    if (state != newState) {
        Serial.printf("[LINK] State: %d -> %d\r\n", (int)state, (int)newState);
        previousState = state;
        state = newState;
        stateEntryTime = millis();
        lastLoopTime = millis();
    }
}

unsigned long LinkManager::timeInState() {
    return millis_since(stateEntryTime);
}

// Connection management

bool LinkManager::connect() {
    if (!client) {
        Serial.println("[LINK] No TCP client available");
        return false;
    }

    Serial.printf("[LINK] Connecting to %s:%d\r\n", LINK_SERVER_HOST, LINK_SERVER_PORT);
    
    if (client->connect(LINK_SERVER_HOST, LINK_SERVER_PORT)) {
        Serial.println("[LINK] TCP connect initiated");
        connectAttempts = 0;
        return true;
    } else {
        Serial.println("[LINK] TCP connect failed");
        connectAttempts++;
        return false;
    }
}

void LinkManager::disconnect() {
    if (client) {
        client->stop();
    }
}

bool LinkManager::sendAuth() {
    if (!client || !client->connected()) {
        return false;
    }

    String ccid = modemManager->getSimCCID();
    
    // Protocol v2: {"type":"auth","data":{"ccid":"..."}}
    String auth = "{\"type\":\"auth\",\"data\":{\"ccid\":\"";
    auth += ccid;
    auth += "\"}}";
    
    Serial.println("[LINK] Sending auth");
    client->println(auth);
    
    if (activityCallback) activityCallback();
    
    return true;
}

// Message handling

void LinkManager::processIncomingData() {
    if (!client) return;
    
    while (client->available()) {
        String line = client->readStringUntil('\n');
        line.trim();
        
        if (line.length() > 0) {
            Serial.println("[LINK] Received: " + line);
            handleMessage(line);
        }
    }
}

void LinkManager::handleMessage(const String& json) {
    JsonDocument doc;
    
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print("[LINK] JSON parse error: ");
        Serial.println(error.f_str());
        return;
    }
    
    // Use ArduinoJson v7 pattern instead of deprecated containsKey()
    if (!doc["type"].is<const char*>()) {
        Serial.println("[LINK] Message missing type field");
        return;
    }
    
    String type = doc["type"].as<String>();
    
    // Protocol v2: auth response
    if (type == "auth") {
        JsonObject data = doc["data"];
        if (data && data["ok"].is<bool>()) {
            bool ok = data["ok"].as<bool>();
            String reason = data["reason"] | "";
            handleAuthResponse(ok, reason);
        }
    } 
    // Protocol v2: command (params flattened into data)
    else if (type == "command") {
        JsonObject data = doc["data"];
        if (data && data["action"].is<const char*>()) {
            String action = data["action"].as<String>();
            int id = data["id"] | 0;
            
            // In v2, params are flattened into data itself
            // Route to CommandRouter with the whole data object
            if (commandRouter) {
                commandRouter->handleCommand(action, id, data);
            }
        }
    } else {
        Serial.println("[LINK] Unknown message type: " + type);
    }
    
    if (activityCallback) activityCallback();
}

void LinkManager::handleAuthResponse(bool ok, const String& reason) {
    if (ok) {
        Serial.println("[LINK] Authentication accepted");
        setState(LinkState::CONNECTED);
        
        // Reset telemetry timer - auth success implies device is awake
        lastTelemetryTime = millis();
    } else {
        Serial.println("[LINK] Authentication rejected: " + reason);
        setState(LinkState::REJECTED);
    }
}

// Telemetry

void LinkManager::checkTelemetry() {
    if (!commandRouter) return;
    
    // Check priority to determine interval
    TelemetryPriority priority = commandRouter->getHighestPriority();
    
    unsigned long interval = TELEMETRY_INTERVAL;
    if (priority >= TelemetryPriority::PRIORITY_HIGH) {
        interval = TELEMETRY_HIGH_INTERVAL;
    } else if (priority == TelemetryPriority::PRIORITY_REALTIME) {
        interval = 0;  // Send immediately
    }
    
    if (millis_since(lastTelemetryTime) >= interval) {
        String telemetry = commandRouter->collectTelemetry(true);
        if (telemetry.length() > 0) {
            send(telemetry);
        }
        lastTelemetryTime = millis();
    }
}
