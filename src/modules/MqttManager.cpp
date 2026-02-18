#include "MqttManager.h"
#include "../core/CommandRouter.h"
#include "../vehicle/VehicleManager.h"
#include "../util.h"

#include <Arduino.h>
#include <ArduinoJson.h>

MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager(ModemManager* modemManager, CommandRouter* commandRouter, VehicleManager* vehicleManager)
    : modemManager(modemManager), commandRouter(commandRouter), vehicleManager(vehicleManager)
{
    _instance = this;
}

MqttManager::~MqttManager()
{
    disconnect();
}

bool MqttManager::setup()
{
    Serial.println("[MQTT] Setting up MQTT manager");

    setState(MqttState::DISCONNECTED);

    // Wire up the command router to send responses through us
    if (commandRouter)
    {
        commandRouter->setResponseSender(responseSender);
    }

    Serial.println("[MQTT] Setup complete");
    return true;
}

void MqttManager::loop()
{
    // Only operate when modem is connected
    if (!modemManager->isConnected())
    {
        if (state != MqttState::DISCONNECTED)
        {
            Serial.println("[MQTT] Modem disconnected, resetting state");
            setState(MqttState::DISCONNECTED);
        }
        return;
    }

    // State machine
    switch (state)
    {
    case MqttState::DISCONNECTED:
        // On hotstart, try to adopt existing connection first
        if (!adoptedConnection && modemManager->wasHotstart())
        {
            Serial.println("[MQTT] Modem hotstart detected, checking for existing MQTT connection");
            adoptedConnection = true;
            
            if (tryAdoptConnection())
            {
                Serial.println("[MQTT] Adopted existing MQTT connection!");
                break;
            }
            Serial.println("[MQTT] No existing MQTT connection, will reconnect normally");
        }
        else if (!adoptedConnection)
        {
            adoptedConnection = true;
        }
        
        // Try to connect after delay
        if (millis() - lastConnectAttempt >= CONNECT_RETRY_DELAY)
        {
            lastConnectAttempt = millis();
            connectAttempts++;

            Serial.printf("[MQTT] Connecting (attempt %d/%d)...\r\n", connectAttempts, MAX_CONNECT_ATTEMPTS);
            
            if (connect())
            {
                connectAttempts = 0;
            }
            else if (connectAttempts >= MAX_CONNECT_ATTEMPTS)
            {
                Serial.println("[MQTT] Max connect attempts reached, backing off");
                lastConnectAttempt = millis() + 30000; // Wait 30s extra
                connectAttempts = 0;
            }
        }
        break;

    case MqttState::CONFIGURING:
        // Waiting for configuration to complete
        if (timeInState() > CONFIG_TIMEOUT)
        {
            Serial.println("[MQTT] Configuration timeout");
            setState(MqttState::MQTT_ERROR);
        }
        break;

    case MqttState::CONNECTING:
        // Waiting for connection to complete
        if (timeInState() > CONNECT_TIMEOUT)
        {
            Serial.println("[MQTT] Connection timeout");
            setState(MqttState::DISCONNECTED);
        }
        break;

    case MqttState::SUBSCRIBING:
        // Waiting for subscription to complete
        if (timeInState() > CONFIG_TIMEOUT)
        {
            Serial.println("[MQTT] Subscription timeout");
            setState(MqttState::MQTT_ERROR);
        }
        break;

    case MqttState::CONNECTED:
        // Check connection status periodically
        if (millis() - lastConnectionCheck >= 30000)
        {
            lastConnectionCheck = millis();
            int mqttState = checkMqttState();
            if (mqttState == 0)
            {
                Serial.println("[MQTT] Connection lost");
                setState(MqttState::DISCONNECTED);
            }
        }

        // Send periodic telemetry
        checkTelemetry();
        break;

    case MqttState::MQTT_ERROR:
        // Try to recover after delay
        if (timeInState() > CONNECT_RETRY_DELAY * 2)
        {
            Serial.println("[MQTT] Attempting to recover from error");
            disconnect();
            setState(MqttState::DISCONNECTED);
        }
        break;
    }
}

void MqttManager::prepareForSleep()
{
    Serial.println("[MQTT] Preparing for sleep (keeping MQTT connection alive)");
    
    // Send sleeping status - we're still online, just sleeping
    // The modem stays connected so it can receive messages and trigger RI pin to wake us
    sendStatus(true, true);
    
    // DO NOT disconnect - modem needs to stay connected to receive wake commands
}

bool MqttManager::isBusy()
{
    return state == MqttState::CONFIGURING || 
           state == MqttState::CONNECTING || 
           state == MqttState::SUBSCRIBING;
}

bool MqttManager::isReady()
{
    return state == MqttState::CONNECTED;
}

// ============================================================================
// Connection Management
// ============================================================================

bool MqttManager::connect()
{
    Serial.println("[MQTT] Starting connection sequence");

    // First, ensure we're disconnected from any previous session
    // This prevents "clientId already connected" errors
    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (modem)
    {
        Serial.println("[MQTT] Cleaning up any existing session...");
        modem->sendAT("+SMDISC");
        modem->waitResponse(2000); // Don't care if it fails - we might not be connected
    }

    setState(MqttState::CONFIGURING);

    // Get device CCID for client ID
    deviceCCID = getDeviceCCID();
    if (deviceCCID.length() == 0)
    {
        Serial.println("[MQTT] ERROR: No CCID available");
        setState(MqttState::MQTT_ERROR);
        return false;
    }

    // Build topic strings
    stateTopicBase = "smartkar/" + deviceCCID + "/";
    commandTopic = stateTopicBase + "command";

    Serial.printf("[MQTT] Device CCID: %s\r\n", deviceCCID.c_str());
    Serial.printf("[MQTT] Command topic: %s\r\n", commandTopic.c_str());

    // Configure MQTT parameters
    if (!configureBroker())
    {
        setState(MqttState::MQTT_ERROR);
        return false;
    }

    // Configure Last Will Testament
    if (!configureLWT())
    {
        setState(MqttState::MQTT_ERROR);
        return false;
    }

    // Connect to broker
    setState(MqttState::CONNECTING);
    if (!connectToBroker())
    {
        setState(MqttState::DISCONNECTED);
        return false;
    }

    // Subscribe to command topic
    setState(MqttState::SUBSCRIBING);
    if (!subscribeToCommands())
    {
        setState(MqttState::MQTT_ERROR);
        return false;
    }

    setState(MqttState::CONNECTED);
    Serial.println("[MQTT] Connected and subscribed successfully");

    // Send online and awake status
    sendStatus(true, false);

    // Send initial telemetry
    sendTelemetryNow(false);

    return true;
}

bool MqttManager::disconnect()
{
    Serial.println("[MQTT] Disconnecting from broker");

    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        return false;
    }

    modem->sendAT("+SMDISC");
    bool ok = modem->waitResponse(5000) == 1;

    if (ok)
    {
        Serial.println("[MQTT] Disconnected successfully");
    }
    else
    {
        Serial.println("[MQTT] Disconnect failed or timed out");
    }

    setState(MqttState::DISCONNECTED);
    return ok;
}

bool MqttManager::configureBroker()
{
    Serial.println("[MQTT] Configuring broker parameters");

    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        Serial.println("[MQTT] ERROR: No modem available");
        return false;
    }

    // Set broker URL and port
    String urlCmd = String("+SMCONF=\"URL\",\"") + MQTT_BROKER_HOST + "\"," + MQTT_BROKER_PORT;
    modem->sendAT(urlCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set URL");
        return false;
    }

    // Set client ID (use CCID for uniqueness)
    String clientIdCmd = String("+SMCONF=\"CLIENTID\",\"") + deviceCCID + "\"";
    modem->sendAT(clientIdCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set client ID");
        return false;
    }

    // Set username
    String usernameCmd = String("+SMCONF=\"USERNAME\",\"") + MQTT_USERNAME + "\"";
    modem->sendAT(usernameCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set username");
        return false;
    }

    // Set password
    String passwordCmd = String("+SMCONF=\"PASSWORD\",\"") + MQTT_PASSWORD + "\"";
    modem->sendAT(passwordCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set password");
        return false;
    }

    // Set keep-alive time
    String keepaliveCmd = String("+SMCONF=\"KEEPTIME\",") + MQTT_KEEPALIVE;
    modem->sendAT(keepaliveCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set keep-alive");
        return false;
    }

    // Set clean session (0 = persistent session for queued messages)
    modem->sendAT("+SMCONF=\"CLEANSS\",0");
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set clean session");
        return false;
    }

    // Set default QoS to 1 (at least once)
    modem->sendAT("+SMCONF=\"QOS\",1");
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set QoS");
        return false;
    }

    Serial.println("[MQTT] Broker configured successfully");
    return true;
}

bool MqttManager::configureLWT()
{
    Serial.println("[MQTT] Configuring Last Will Testament");

    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        return false;
    }

    // Set LWT topic
    String lwtTopic = stateTopicBase + "status";
    String topicCmd = String("+SMCONF=\"TOPIC\",\"") + lwtTopic + "\"";
    modem->sendAT(topicCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set LWT topic");
        return false;
    }

    // Set LWT message (JSON with offline status)
    // Escape inner quotes with backslash for AT command parser
    // LWT fires on unexpected disconnect - device is offline and not sleeping (crashed/lost connection)
    String lwtMessage = "{\\\"online\\\":false,\\\"sleeping\\\":false,\\\"timestamp\\\":" + String(millis()) + "}";
    String messageCmd = String("+SMCONF=\"MESSAGE\",\"") + lwtMessage + "\"";
    modem->sendAT(messageCmd.c_str());
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set LWT message");
        return false;
    }

    // Set LWT retain flag (1 = retained)
    modem->sendAT("+SMCONF=\"RETAIN\",1");
    if (modem->waitResponse(5000) != 1)
    {
        Serial.println("[MQTT] ERROR: Failed to set LWT retain");
        return false;
    }

    Serial.println("[MQTT] LWT configured successfully");
    return true;
}

bool MqttManager::connectToBroker()
{
    Serial.println("[MQTT] Connecting to broker...");

    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        return false;
    }

    modem->sendAT("+SMCONN");
    int result = modem->waitResponse(30000); // 30 second timeout for connection

    if (result == 1)
    {
        Serial.println("[MQTT] Connected to broker");
        return true;
    }
    else
    {
        Serial.printf("[MQTT] Connection failed (result: %d)\r\n", result);
        return false;
    }
}

bool MqttManager::subscribeToCommands()
{
    Serial.printf("[MQTT] Subscribing to: %s\r\n", commandTopic.c_str());

    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        return false;
    }

    // First, unsubscribe in case we're already subscribed (persistent session with CLEANSS=0)
    // This prevents "already subscribed" errors on reconnect
    Serial.println("[MQTT] Unsubscribing first (in case already subscribed)...");
    String unsubCmd = String("+SMUNSUB=\"") + commandTopic + "\"";
    modem->sendAT(unsubCmd.c_str());
    modem->waitResponse(2000); // Ignore result - it's OK if we weren't subscribed

    // Now subscribe with QoS 1
    String subCmd = String("+SMSUB=\"") + commandTopic + "\",1";
    modem->sendAT(subCmd.c_str());
    
    if (modem->waitResponse(10000) != 1)
    {
        Serial.println("[MQTT] ERROR: Subscription failed");
        return false;
    }

    Serial.println("[MQTT] Subscribed successfully");
    return true;
}

int MqttManager::checkMqttState()
{
    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        return 0;
    }

    modem->sendAT("+SMSTATE?");
    
    // Parse response: +SMSTATE: <state>
    // 0 = disconnected, 1 = connected, 2 = connected with session
    if (modem->waitResponse(1000, "+SMSTATE:") == 1)
    {
        String response = modem->stream.readStringUntil('\n');
        int state = response.toInt();
        modem->waitResponse();  // Consume the trailing "OK" to prevent buffer pollution
        return state;
    }

    return 0;
}

bool MqttManager::tryAdoptConnection()
{
    Serial.println("[MQTT] Checking for existing MQTT connection...");
    
    int mqttState = checkMqttState();  // Uses AT+SMSTATE?
    
    if (mqttState > 0)  // 1 = connected, 2 = connected with session
    {
        Serial.printf("[MQTT] Hotstart: MQTT connection still active (state=%d)\r\n", mqttState);
        
        // Re-initialize internal state (lost during deep sleep)
        deviceCCID = getDeviceCCID();
        stateTopicBase = "smartkar/" + deviceCCID + "/";
        commandTopic = stateTopicBase + "command";
        
        setState(MqttState::CONNECTED);
        sendStatus(true, false);  // Awake status
        sendTelemetryNow(false);
        
        return true;
    }
    
    Serial.printf("[MQTT] MQTT not connected (state=%d), will reconnect\r\n", mqttState);
    return false;
}

// ============================================================================
// Publishing
// ============================================================================

bool MqttManager::publish(const String& topic, const String& payload, MqttQoS qos, bool retain)
{
    String fullTopic = stateTopicBase + topic;
    return publishFull(fullTopic, payload, qos, retain);
}

bool MqttManager::publishFull(const String& fullTopic, const String& payload, MqttQoS qos, bool retain)
{
    if (!isConnected())
    {
        Serial.println("[MQTT] ERROR: Not connected, cannot publish");
        return false;
    }

    TinyGsmSim7080Extended* modem = modemManager->getModem();
    if (!modem)
    {
        return false;
    }

    // Report activity
    if (activityCallback)
    {
        activityCallback();
    }

    int payloadLen = payload.length();
    int qosInt = static_cast<int>(qos);
    int retainInt = retain ? 1 : 0;

    // AT+SMPUB="<topic>",<length>,<qos>,<retain>
    String pubCmd = String("+SMPUB=\"") + fullTopic + "\"," + payloadLen + "," + qosInt + "," + retainInt;
    
    Serial.printf("[MQTT] Publishing to %s (%d bytes, QoS %d)\r\n", fullTopic.c_str(), payloadLen, qosInt);

    modem->sendAT(pubCmd.c_str());
    
    // Wait for ">" prompt
    if (modem->waitResponse(5000, ">") != 1)
    {
        Serial.println("[MQTT] ERROR: No prompt for data");
        return false;
    }

    // Send payload
    modem->stream.print(payload);
    
    // Wait for OK
    if (modem->waitResponse(10000) != 1)
    {
        Serial.println("[MQTT] ERROR: Publish failed");
        return false;
    }

    Serial.println("[MQTT] Published successfully");
    return true;
}

bool MqttManager::sendStatus(bool online, bool sleeping)
{
    String statusTopic = stateTopicBase + "status";
    String payload = String("{\"online\":") + (online ? "true" : "false") + 
                     ",\"sleeping\":" + (sleeping ? "true" : "false") +
                     ",\"timestamp\":" + millis() + "}";
    
    return publishFull(statusTopic, payload, MqttQoS::AT_LEAST_ONCE, true);
}

bool MqttManager::sendTelemetryNow(bool changedOnly)
{
    if (!isConnected())
    {
        return false;
    }

    if (!commandRouter)
    {
        Serial.println("[MQTT] No CommandRouter - cannot send telemetry");
        return false;
    }

    Serial.println("[MQTT] Sending telemetry");

    bool anyPublished = false;
    
    // Iterate through all registered telemetry providers
    for (size_t i = 0; i < commandRouter->getProviderCount(); i++)
    {
        ITelemetryProvider* provider = commandRouter->getProvider(i);
        if (!provider)
        {
            continue;
        }

        // Skip if changedOnly mode and provider hasn't changed
        if (changedOnly && !provider->hasChanged())
        {
            continue;
        }

        // Build telemetry JSON for this provider
        JsonDocument doc;
        JsonObject data = doc.to<JsonObject>();
        provider->getTelemetry(data);

        // Serialize to string
        String payload;
        serializeJson(doc, payload);

        // Build topic: state/{domain} (e.g., state/battery)
        String topic = String("state/") + provider->getTelemetryDomain();

        // Publish to MQTT
        if (publish(topic, payload, MqttQoS::AT_LEAST_ONCE, false))
        {
            // Notify provider that telemetry was sent
            provider->onTelemetrySent();
            anyPublished = true;
        }
        else
        {
            Serial.printf("[MQTT] Failed to publish %s telemetry\r\n", provider->getTelemetryDomain());
        }
    }

    return anyPublished;
}

// ============================================================================
// Message Handling
// ============================================================================

void MqttManager::handleMessage(const String& topic, const String& payload)
{
    Serial.printf("[MQTT] Received message on %s: %s\r\n", topic.c_str(), payload.c_str());

    // Report activity
    if (activityCallback)
    {
        activityCallback();
    }

    // Check if this is a command message
    if (topic == commandTopic)
    {
        // Parse JSON command
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error)
        {
            Serial.printf("[MQTT] ERROR: JSON parse error: %s\r\n", error.c_str());
            return;
        }

        // Extract command fields
        String action = doc["action"] | "";
        int id = doc["id"] | 0;
        JsonObject params = doc["params"].as<JsonObject>();

        if (action.length() == 0)
        {
            Serial.println("[MQTT] ERROR: Command missing 'action' field");
            return;
        }

        // Route to command router
        if (commandRouter)
        {
            commandRouter->handleCommand(action, id, params);
        }
    }
}

// ============================================================================
// Telemetry
// ============================================================================

void MqttManager::checkTelemetry()
{
    if (!vehicleManager)
    {
        return;
    }

    // No global interval - just check each provider
    // Providers manage their own intervals and change detection
    sendTelemetryNow(true);  // changedOnly = true
}

// ============================================================================
// Helpers
// ============================================================================

String MqttManager::getDeviceCCID()
{
    if (deviceCCID.length() > 0)
    {
        return deviceCCID;
    }

    if (modemManager)
    {
        deviceCCID = modemManager->getSimCCID();
    }

    return deviceCCID;
}

String MqttManager::buildTopic(const String& suffix)
{
    return stateTopicBase + suffix;
}

bool MqttManager::responseSender(const String& message)
{
    if (!_instance)
    {
        return false;
    }

    // Publish to response topic
    return _instance->publish("response", message, MqttQoS::AT_LEAST_ONCE, false);
}

// ============================================================================
// State Machine Helpers
// ============================================================================

bool MqttManager::stateJustChanged()
{
    return state != previousState;
}

void MqttManager::setState(MqttState newState)
{
    if (state != newState)
    {
        previousState = state;
        state = newState;
        stateEntryTime = millis();

        Serial.printf("[MQTT] State: %d -> %d\r\n", (int)previousState, (int)state);
    }
}

unsigned long MqttManager::timeInState()
{
    return millis() - stateEntryTime;
}
