#pragma once

#include <Arduino.h>
#include "ModemManager.h" // Includes TinyGsmClient.h
#include "../core/IModule.h"
#include "../core/CommandRouter.h"

// Forward declarations
class VehicleManager;

/**
 * Server connection configuration
 */
#define LINK_SERVER_HOST "link.smartkar.no"
#define LINK_SERVER_PORT 4589

/**
 * Link state machine states
 */
enum class LinkState
{
    DISCONNECTED,   // Not connected to server
    CONNECTING,     // TCP connection in progress
    AUTHENTICATING, // Connected, sending auth message
    CONNECTED,      // Authenticated and ready
    REJECTED,       // Server rejected authentication
    LINK_ERROR      // Connection error (renamed from ERROR to avoid macro conflicts)
};

/**
 * LinkManager - Server connection management module
 *
 * Responsibilities:
 * - Establish and maintain TCP connection to server
 * - Handle authentication protocol
 * - Parse incoming JSON messages
 * - Route commands to CommandRouter
 * - Send telemetry and events to server
 */
class LinkManager : public IModule
{
public:
    /**
     * Constructor
     * @param modemManager Reference to ModemManager for TCP client
     * @param commandRouter Reference to CommandRouter for command handling
     * @param vehicleManager Reference to VehicleManager for vehicle state
     */
    LinkManager(ModemManager *modemManager, CommandRouter *commandRouter, VehicleManager *vehicleManager);
    ~LinkManager();

    // IModule interface
    bool setup() override;
    void loop() override;
    void prepareForSleep() override;
    bool isBusy() override;
    bool isReady() override;
    const char *getName() override { return "LINK"; }

    /**
     * Set the activity callback for reporting activity to DeviceController.
     */
    void setActivityCallback(ActivityCallback callback) { activityCallback = callback; }

    /**
     * Get current link state.
     */
    LinkState getState() { return state; }

    /**
     * Check if connected and authenticated with server.
     */
    bool isConnected() { return state == LinkState::CONNECTED; }

    /**
     * Send a message to the server.
     * @param message JSON string to send
     * @return true if sent successfully
     */
    bool send(const String &message);

    /**
     * Send a bye message to the server before disconnecting.
     * @param reason The reason for disconnecting (e.g., "sleep", "shutdown", "reboot")
     * @return true if sent successfully
     */
    bool sendBye(const String &reason);

    /**
     * Force immediate telemetry send (bypasses interval check).
     * @param changedOnly If true, only send if data has changed
     * @return true if telemetry was sent
     */
    bool sendTelemetryNow(bool changedOnly = false);

    /**
     * Handle TCP-layer interrupt from modem.
     * Called by ModemManager when +CA URC is received.
     */
    void handleTCPInterrupt();

    // Singleton for interrupt access and response sending
    static LinkManager *instance() { return _instance; }

private:
    static LinkManager *_instance;

    ModemManager *modemManager = nullptr;
    CommandRouter *commandRouter = nullptr;
    VehicleManager *vehicleManager = nullptr;
    TinyGsmClient *client = nullptr;
    ActivityCallback activityCallback = nullptr;

    LinkState state = LinkState::DISCONNECTED;
    LinkState previousState = LinkState::DISCONNECTED;

    // Timing
    unsigned long stateEntryTime = 0;
    unsigned long lastLoopTime = 0;
    unsigned long lastTelemetryTime = 0;
    unsigned long lastConnectionCheck = 0;

    // Reconnection
    unsigned long lastConnectAttempt = 0;
    int connectAttempts = 0;
    bool adoptedConnection = false;  // Track if we've adopted existing connection after wake

    // State machine helpers
    bool stateJustChanged();
    void setState(LinkState newState);
    unsigned long timeInState();

    // Connection management
    bool connect();
    void disconnect();
    bool sendAuth();

    // Message handling
    void processIncomingData();
    void handleMessage(const String &json);
    void handleAuthResponse(bool ok, const String &reason);

    // Telemetry
    void checkTelemetry();

    // Static response sender for CommandRouter
    static bool responseSender(const String &message);

    // Constants
    static const unsigned long CONNECT_RETRY_DELAY = 5000;          // 5 seconds between retries
    static const unsigned long AUTH_TIMEOUT = 10000;                // 10 seconds for auth response
    static const unsigned long KEEPALIVE_INTERVAL = 30000;          // 30 seconds keepalive
    static const unsigned long TELEMETRY_INTERVAL_AWAKE = 30000;    // 30 seconds when vehicle awake
    static const unsigned long TELEMETRY_INTERVAL_ASLEEP = 300000;  // 5 minutes when vehicle asleep
    static const unsigned long TELEMETRY_HIGH_INTERVAL = 5000;      // 5 seconds for high priority
    static const int MAX_CONNECT_ATTEMPTS = 5;                      // Max retries before backoff
};
