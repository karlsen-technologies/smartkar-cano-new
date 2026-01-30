#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * CommandStateManager - Singleton for tracking active command state
 * 
 * This class is the single source of truth for which command is currently
 * executing on the device. It tracks the command lifecycle from acceptance
 * through execution stages to completion or failure.
 * 
 * All command progress updates are sent through this manager to ensure
 * the server has complete visibility into command execution.
 * 
 * Protocol Version: 2.2
 * 
 * Usage:
 *   CommandStateManager* csm = CommandStateManager::getInstance();
 *   
 *   // Check if device is busy
 *   if (csm->hasActiveCommand()) {
 *       // Reject new command
 *   }
 *   
 *   // Start tracking a command
 *   csm->startCommand(12, "vehicle.startClimate");
 *   
 *   // Update progress
 *   csm->updateStage(Stage::WAITING_FOR_WAKE);
 *   
 *   // Complete or fail
 *   csm->completeCommand();
 *   csm->failCommand("Wake timeout after 15000ms");
 */
class CommandStateManager {
public:
    /**
     * Stage of command execution
     */
    enum class Stage {
        NONE,              // No active command
        ACCEPTED,          // Command accepted, about to start
        REQUESTING_WAKE,   // Requesting vehicle wake
        WAITING_FOR_WAKE,  // Waiting for wake confirmation
        UPDATING_PROFILE,  // Updating charging/climate profile
        SENDING_COMMAND,   // Sending BAP frames to vehicle
        COMPLETED,         // Success (cleared immediately)
        FAILED             // Failure (cleared immediately)
    };
    
    /**
     * Get the singleton instance.
     * Creates instance on first call.
     */
    static CommandStateManager* getInstance();
    
    /**
     * Check if a command is currently active.
     * @return true if a command is in progress
     */
    bool hasActiveCommand() const;
    
    /**
     * Get current command ID.
     * @return Command ID or -1 if no active command
     */
    int getCurrentCommandId() const;
    
    /**
     * Get current command action.
     * @return Action string or empty if no active command
     */
    const String& getCurrentAction() const;
    
    /**
     * Get current stage.
     * @return Current execution stage
     */
    Stage getCurrentStage() const;
    
    /**
     * Get elapsed time since command started.
     * @return Milliseconds elapsed, or 0 if no active command
     */
    unsigned long getElapsedMs() const;
    
    /**
     * Start tracking a new command.
     * Stores command info and sends first "accepted" response.
     * 
     * @param commandId Command ID from server
     * @param action Full action string (e.g., "vehicle.startClimate")
     */
    void startCommand(int commandId, const String& action);
    
    /**
     * Update current stage.
     * Sends "in_progress" response with new stage.
     * 
     * @param stage New stage
     */
    void updateStage(Stage stage);
    
    /**
     * Complete command successfully.
     * Sends "completed" response and clears state.
     * 
     * @param data Optional response data to include
     */
    void completeCommand(JsonDocument* data = nullptr);
    
    /**
     * Fail command with reason.
     * Sends "failed" response and clears state.
     * 
     * @param reason Human-readable error message
     */
    void failCommand(const char* reason);
    
    /**
     * Get current command info (for busy responses).
     * Populates JsonObject with current command details.
     * 
     * @param obj JsonObject to populate
     */
    void getCurrentCommandInfo(JsonObject& obj) const;
    
    /**
     * Get stage as string (for responses).
     * 
     * @param stage Stage enum value
     * @return Stage name in snake_case
     */
    static const char* getStageString(Stage stage);
    
    /**
     * Set the response sender callback.
     * Used to send responses through LinkManager.
     * 
     * @param sender Callback function that sends JSON messages
     */
    void setResponseSender(bool (*sender)(const String& message));
    
private:
    // Private constructor for singleton
    CommandStateManager();
    
    // Singleton instance
    static CommandStateManager* _instance;
    
    // Response sender callback
    bool (*responseSender)(const String& message) = nullptr;
    
    // State variables
    int currentCommandId = -1;           // -1 = no active command
    String currentAction = "";           // e.g., "vehicle.startClimate"
    Stage currentStage = Stage::NONE;    // Current execution stage
    unsigned long commandStartTime = 0;  // millis() when command started
    String failureReason = "";           // Set when command fails
    
    /**
     * Send response via CommandRouter.
     * 
     * @param status Status string ("in_progress", "completed", "failed")
     * @param ok Success flag
     * @param error Optional error message
     * @param data Optional response data
     */
    void sendResponse(const char* status, bool ok, const char* error = nullptr, JsonDocument* data = nullptr);
    
    /**
     * Clear command state.
     * Called after completion or failure.
     */
    void clearState();
};
