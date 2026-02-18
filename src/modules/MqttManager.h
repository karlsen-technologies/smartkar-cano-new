#pragma once

#include <Arduino.h>
#include "ModemManager.h"
#include "../core/IModule.h"
#include <functional>

// Forward declarations
class CommandRouter;
class VehicleManager;

/**
 * MQTT broker configuration
 */
#define MQTT_BROKER_HOST "link.smartkar.no"
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME "device"
#define MQTT_PASSWORD "_pxThY8DCH7yPTWG"
#define MQTT_KEEPALIVE 300  // 5 minutes for power saving

/**
 * MQTT connection state
 */
enum class MqttState {
    DISCONNECTED,   // Not connected to broker
    CONFIGURING,    // Setting up MQTT parameters
    CONNECTING,     // Connection in progress
    SUBSCRIBING,    // Subscribing to command topic
    CONNECTED,      // Connected and subscribed
    MQTT_ERROR      // Error state
};

/**
 * MQTT Quality of Service levels
 */
enum class MqttQoS {
    AT_MOST_ONCE = 0,   // Fire and forget
    AT_LEAST_ONCE = 1,  // Acknowledged delivery
    EXACTLY_ONCE = 2    // Assured delivery
};

/**
 * MqttManager - MQTT client for SmartKar-Cano
 * 
 * Uses the SIM7080G modem's built-in MQTT client via AT commands.
 * 
 * Responsibilities:
 * - Configure and maintain MQTT broker connection
 * - Subscribe to command topic
 * - Publish state, response, event, and status messages
 * - Handle incoming MQTT messages from +SMSUB URC
 * - Route commands to CommandRouter
 */
class MqttManager : public IModule {
public:
    /**
     * Constructor
     * @param modemManager Reference to ModemManager for AT commands
     * @param commandRouter Reference to CommandRouter for command handling
     * @param vehicleManager Reference to VehicleManager for vehicle state
     */
    MqttManager(ModemManager* modemManager, CommandRouter* commandRouter, VehicleManager* vehicleManager);
    ~MqttManager();

    // IModule interface
    bool setup() override;
    void loop() override;
    void prepareForSleep() override;
    bool isBusy() override;
    bool isReady() override;
    const char* getName() override { return "MQTT"; }

    /**
     * Set the activity callback for reporting activity to DeviceController.
     */
    void setActivityCallback(ActivityCallback callback) { activityCallback = callback; }

    /**
     * Get current MQTT state.
     */
    MqttState getState() { return state; }

    /**
     * Check if connected to MQTT broker.
     */
    bool isConnected() { return state == MqttState::CONNECTED; }

    /**
     * Publish a message to a topic.
     * @param topic Topic to publish to (without device prefix, e.g., "state")
     * @param payload JSON payload
     * @param qos Quality of Service level
     * @param retain Retain flag
     * @return true if publish was successful
     */
    bool publish(const String& topic, const String& payload, MqttQoS qos = MqttQoS::AT_LEAST_ONCE, bool retain = false);

    /**
     * Publish to full topic path (for status messages with LWT).
     * @param fullTopic Complete topic path
     * @param payload JSON payload
     * @param qos Quality of Service level
     * @param retain Retain flag
     * @return true if publish was successful
     */
    bool publishFull(const String& fullTopic, const String& payload, MqttQoS qos = MqttQoS::AT_LEAST_ONCE, bool retain = false);

    /**
     * Force immediate telemetry send (bypasses interval check).
     * @param changedOnly If true, only send if data has changed
     * @return true if telemetry was sent
     */
    bool sendTelemetryNow(bool changedOnly = false);

    /**
     * Handle MQTT message reception URC from modem.
     * Called by ModemManager when +SMSUB URC is received.
     * @param topic Topic the message was received on
     * @param payload Message payload
     */
    void handleMessage(const String& topic, const String& payload);

    /**
     * Send status message.
     * @param online true for online, false for offline
     * @param sleeping true if sleeping, false if awake
     * @return true if sent successfully
     */
    bool sendStatus(bool online, bool sleeping);

    /**
     * Get the device's CCID for topic construction.
     */
    String getDeviceCCID();

    // Singleton for URC handling
    static MqttManager* instance() { return _instance; }

private:
    static MqttManager* _instance;

    ModemManager* modemManager = nullptr;
    CommandRouter* commandRouter = nullptr;
    VehicleManager* vehicleManager = nullptr;
    ActivityCallback activityCallback = nullptr;

    MqttState state = MqttState::DISCONNECTED;
    MqttState previousState = MqttState::DISCONNECTED;
    bool adoptedConnection = false;  // Track if we've tried to adopt on this boot

    // Cached device CCID for topics
    String deviceCCID;
    String commandTopic;    // smartkar/{ccid}/command
    String stateTopicBase;  // smartkar/{ccid}/

    // Timing
    unsigned long stateEntryTime = 0;
    unsigned long lastLoopTime = 0;
    unsigned long lastConnectionCheck = 0;

    // Reconnection
    unsigned long lastConnectAttempt = 0;
    int connectAttempts = 0;

    // State machine helpers
    bool stateJustChanged();
    void setState(MqttState newState);
    unsigned long timeInState();

    // Connection management
    bool connect();
    bool disconnect();
    bool configureBroker();
    bool connectToBroker();
    bool subscribeToCommands();
    bool configureLWT();
    bool tryAdoptConnection();  // Try to adopt existing MQTT connection on hotstart

    // AT command helpers
    bool sendATCommand(const String& cmd, const String& expectedResponse = "OK", uint32_t timeout = 5000);
    bool sendATCommandMultiline(const String& cmd, const String& data, const String& expectedResponse = "OK", uint32_t timeout = 5000);
    String sendATCommandWithResponse(const String& cmd, uint32_t timeout = 5000);
    bool waitForResponse(const String& expected, uint32_t timeout);
    int checkMqttState();  // Query AT+SMSTATE, returns 0=disconnected, 1=connected, 2=connected+session

    // Topic helpers
    String buildTopic(const String& suffix);

    // Telemetry
    void checkTelemetry();

    // Static response sender for CommandRouter
    static bool responseSender(const String& message);

    // Constants
    static const unsigned long CONNECT_RETRY_DELAY = 5000;          // 5 seconds between retries
    static const unsigned long CONFIG_TIMEOUT = 10000;              // 10 seconds for configuration
    static const unsigned long CONNECT_TIMEOUT = 30000;             // 30 seconds for connection
    static const int MAX_CONNECT_ATTEMPTS = 5;                      // Max retries before backoff
};
