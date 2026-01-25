#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * Command execution result status
 */
enum class CommandStatus {
    OK,             // Command executed successfully
    PENDING,        // Command accepted, response will be sent async
    INVALID_PARAMS, // Invalid or missing parameters
    NOT_SUPPORTED,  // Action not supported by this handler
    BUSY,           // Handler is busy, try again later
    CMD_ERROR       // General error (renamed from ERROR to avoid macro conflicts)
};

/**
 * CommandResult - Result of command execution
 * 
 * Note: JsonDocument is not copyable in ArduinoJson v7, so we use
 * factory methods that create the result directly rather than
 * brace initialization.
 */
struct CommandResult {
    CommandStatus status = CommandStatus::OK;
    String message;         // Human-readable status message (optional)
    JsonDocument data;      // Response data to include in reply
    
    // Default constructor
    CommandResult() = default;
    
    // Move constructor (JsonDocument is movable but not copyable)
    CommandResult(CommandResult&& other) = default;
    CommandResult& operator=(CommandResult&& other) = default;
    
    // Delete copy operations (JsonDocument is not copyable)
    CommandResult(const CommandResult&) = delete;
    CommandResult& operator=(const CommandResult&) = delete;
    
    // Factory methods - these create results directly
    static CommandResult ok() {
        CommandResult r;
        r.status = CommandStatus::OK;
        return r;
    }
    
    static CommandResult ok(const char* msg) {
        CommandResult r;
        r.status = CommandStatus::OK;
        r.message = msg;
        return r;
    }
    
    static CommandResult pending() {
        CommandResult r;
        r.status = CommandStatus::PENDING;
        return r;
    }
    
    static CommandResult error(const char* msg) {
        CommandResult r;
        r.status = CommandStatus::CMD_ERROR;
        r.message = msg;
        return r;
    }
    
    static CommandResult invalidParams(const char* msg) {
        CommandResult r;
        r.status = CommandStatus::INVALID_PARAMS;
        r.message = msg;
        return r;
    }
    
    static CommandResult notSupported() {
        CommandResult r;
        r.status = CommandStatus::NOT_SUPPORTED;
        return r;
    }
    
    static CommandResult busy() {
        CommandResult r;
        r.status = CommandStatus::BUSY;
        r.message = "Handler is busy";
        return r;
    }
};

/**
 * CommandContext - Context for command execution
 * 
 * Provides access to command parameters and a way to send
 * async responses for long-running operations.
 */
class CommandContext {
public:
    const int id;               // Command ID for response correlation
    const String action;        // Full action string (e.g., "charging.setLimit")
    const String domain;        // Domain part (e.g., "charging")
    const String actionName;    // Action part (e.g., "setLimit")
    const JsonObject params;    // Command parameters
    
    /**
     * Callback for sending async response.
     * Use this for commands that complete asynchronously.
     */
    typedef void (*AsyncResponseCallback)(int id, CommandResult result);
    AsyncResponseCallback sendAsyncResponse = nullptr;
    
    CommandContext(int id, const String& action, const String& domain, 
                   const String& actionName, const JsonObject& params)
        : id(id), action(action), domain(domain), actionName(actionName), params(params) {}
};

/**
 * ICommandHandler - Interface for modules that handle commands
 * 
 * Modules that want to receive commands from the server implement this
 * interface. Commands are routed based on domain (e.g., "charging", "climate").
 * 
 * Example implementation:
 * 
 *   class ChargingModule : public IModule, public ICommandHandler {
 *   public:
 *       const char* getDomain() override { return "charging"; }
 *       
 *       CommandResult handleCommand(CommandContext& ctx) override {
 *           if (ctx.actionName == "setLimit") {
 *               int limit = ctx.params["percent"] | 80;
 *               // Set charge limit...
 *               return CommandResult::ok();
 *           }
 *           return CommandResult::notSupported();
 *       }
 *   };
 * 
 * Commands are formatted as "domain.action" (e.g., "charging.setLimit").
 * The router extracts the domain and routes to the appropriate handler.
 */
class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    
    /**
     * Get the domain this handler manages.
     * Commands with this domain prefix will be routed here.
     * 
     * @return Domain string (e.g., "charging", "climate", "vehicle", "security")
     */
    virtual const char* getDomain() = 0;
    
    /**
     * Handle a command.
     * 
     * @param ctx Command context with action, params, and async response callback
     * @return Result indicating success, error, or pending async response
     */
    virtual CommandResult handleCommand(CommandContext& ctx) = 0;
    
    /**
     * Get list of supported actions.
     * Used for capability discovery.
     * 
     * @param count Output: number of actions in returned array
     * @return Array of action names (without domain prefix)
     */
    virtual const char** getSupportedActions(size_t& count) {
        count = 0;
        return nullptr;
    }
};
