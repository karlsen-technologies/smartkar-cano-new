#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ICommandHandler.h"
#include "ITelemetryProvider.h"
#include "CommandStateManager.h"

// Maximum number of handlers/providers that can be registered
#define MAX_COMMAND_HANDLERS 8
#define MAX_TELEMETRY_PROVIDERS 8

/**
 * Response sender callback type.
 * Used by CommandRouter to send responses back through LinkManager.
 */
typedef bool (*ResponseSender)(const String& message);

/**
 * CommandRouter - Central hub for command routing and telemetry collection
 * 
 * Singleton design allows any module to emit events without passing pointers.
 * 
 * Responsibilities:
 * - Register and manage command handlers by domain
 * - Route incoming commands to appropriate handlers
 * - Collect telemetry from all registered providers
 * - Handle async response callbacks for long-running operations
 * - Manage built-in system commands (ping, status, etc.)
 * 
 * Usage:
 *   CommandRouter* router = new CommandRouter();
 *   router->registerHandler(&chargingModule);
 *   router->registerProvider(&chargingModule);
 *   router->setResponseSender([](const String& msg) { return linkManager.send(msg); });
 *   
 *   // In LinkManager::handleMessage():
 *   router->handleCommand(action, id, params);
 *   
 *   // From any module:
 *   CommandRouter::getInstance()->sendEvent("vehicle", "commandCompleted", &details);
 */
class CommandRouter {
public:
    CommandRouter();
    
    /**
     * Get the singleton instance.
     * Returns nullptr if instance not yet created.
     */
    static CommandRouter* getInstance() { return _instance; }
    
    /**
     * Register a command handler.
     * Handler will receive commands matching its domain.
     * 
     * @param handler Handler implementing ICommandHandler
     * @return true if registered successfully
     */
    bool registerHandler(ICommandHandler* handler);
    
    /**
     * Register a telemetry provider.
     * Provider will be queried when collecting telemetry.
     * 
     * @param provider Provider implementing ITelemetryProvider
     * @return true if registered successfully
     */
    bool registerProvider(ITelemetryProvider* provider);
    
    /**
     * Set the response sender callback.
     * Used to send command responses and telemetry.
     * 
     * @param sender Callback function that sends messages to server
     */
    void setResponseSender(ResponseSender sender) { responseSender = sender; }
    
    /**
     * Handle an incoming command.
     * Parses action string, finds handler, executes command, sends response.
     * 
     * @param action Full action string (e.g., "charging.setLimit")
     * @param id Command ID for response correlation
     * @param params JsonObject containing command parameters
     */
    void handleCommand(const String& action, int id, JsonObject& params);
    
    /**
     * Collect telemetry from all providers.
     * Queries each provider and builds aggregated telemetry message.
     * 
     * @param onlyChanged If true, only include providers with changed data
     * @return JSON string with telemetry message, or empty if nothing to send
     */
    String collectTelemetry(bool onlyChanged = true);
    
    /**
     * Check if any provider has high-priority data.
     * Used to decide if telemetry should be sent immediately.
     * 
     * @return Highest priority level among providers with changed data
     */
    TelemetryPriority getHighestPriority();
    
    /**
     * Get list of all registered domains and their actions.
     * Used for capability discovery.
     * 
     * @param doc JsonDocument to populate with capabilities
     */
    void getCapabilities(JsonDocument& doc);

    /**
     * Send an event immediately.
     * Used by modules to send events between telemetry intervals.
     * 
     * @param domain Event domain
     * @param event Event name
     * @param details Optional event details
     */
    void sendEvent(const char* domain, const char* event, JsonObject* details = nullptr);

private:
    ICommandHandler* handlers[MAX_COMMAND_HANDLERS];
    ITelemetryProvider* providers[MAX_TELEMETRY_PROVIDERS];
    size_t handlerCount = 0;
    size_t providerCount = 0;
    
    ResponseSender responseSender = nullptr;
    
    /**
     * Find handler for a given domain.
     */
    ICommandHandler* findHandler(const String& domain);
    
    /**
     * Parse action string into domain and action name.
     * "charging.setLimit" -> domain="charging", actionName="setLimit"
     * 
     * @param action Full action string
     * @param domain Output: domain part
     * @param actionName Output: action name part
     * @return true if parsed successfully
     */
    bool parseAction(const String& action, String& domain, String& actionName);
    
    /**
     * Send a command response.
     */
    void sendResponse(int id, CommandStatus status, const char* message = nullptr, JsonDocument* data = nullptr);
    
    /**
     * Handle built-in system commands.
     * @return true if command was handled
     */
    bool handleSystemCommand(const String& action, int id, JsonObject& params);
    
    /**
     * Static callback for async responses.
     */
    static void asyncResponseCallback(int id, CommandResult result);
    static CommandRouter* _instance;
};
