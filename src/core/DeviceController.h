#pragma once

#include <Arduino.h>
#include "IModule.h"

// Forward declarations
class PowerManager;
class ModemManager;
class LinkManager;
class CanManager;
class VehicleManager;
class CommandRouter;
class DeviceProvider;
class NetworkProvider;
class SystemHandler;

/**
 * Device states for the main state machine
 */
enum class DeviceState {
    INITIALIZING,   // Setting up modules
    RUNNING,        // Normal operation
    PREPARING_SLEEP,// Preparing modules for sleep
    SLEEPING        // Entering deep sleep
};

/**
 * Configuration for activity timeout and sleep behavior
 */
struct DeviceConfig {
    unsigned long activityTimeout;  // Time without activity before sleep (ms)
    unsigned long minAwakeTime;     // Minimum time to stay awake after boot (ms)
};

/**
 * DeviceController - Central coordinator for all modules
 * 
 * Responsibilities:
 * - Module lifecycle management (setup, loop, prepareForSleep)
 * - Activity tracking and sleep decisions
 * - Device state machine
 */
class DeviceController {
public:
    DeviceController();

    /**
     * Initialize the device and all modules.
     * Called once from setup().
     */
    void setup();

    /**
     * Main loop tick.
     * Called repeatedly from loop().
     */
    void loop();

    /**
     * Report activity from a module.
     * Resets the sleep timer.
     * Modules should call this when meaningful activity occurs.
     */
    void reportActivity();

    /**
     * Get the activity callback for modules to use.
     */
    ActivityCallback getActivityCallback();

    /**
     * Check if the device can sleep.
     * Returns true if:
     * - No modules are busy
     * - Activity timeout has elapsed OR sleep was requested
     * - Minimum awake time has passed
     */
    bool canSleep();

    /**
     * Request the device to enter sleep mode.
     * Sleep will occur on the next loop iteration after all modules are ready.
     * @param durationSeconds Optional sleep duration (0 = wake on interrupt only)
     */
    void requestSleep(unsigned long durationSeconds = 0);

    /**
     * Check if sleep has been requested.
     */
    bool isSleepRequested() { return sleepRequested; }

    /**
     * Get current device state.
     */
    DeviceState getState() { return state; }

    /**
     * Get module references (for inter-module dependencies)
     */
    PowerManager* getPowerManager() { return powerManager; }
    ModemManager* getModemManager() { return modemManager; }
    LinkManager* getLinkManager() { return linkManager; }
    CanManager* getCanManager() { return canManager; }
    VehicleManager* getVehicleManager() { return vehicleManager; }
    CommandRouter* getCommandRouter() { return commandRouter; }

private:
    DeviceState state = DeviceState::INITIALIZING;
    DeviceConfig config;

    // Core services
    CommandRouter* commandRouter = nullptr;

    // Modules
    PowerManager* powerManager = nullptr;
    ModemManager* modemManager = nullptr;
    LinkManager* linkManager = nullptr;
    CanManager* canManager = nullptr;
    VehicleManager* vehicleManager = nullptr;
    
    // Telemetry providers
    DeviceProvider* deviceProvider = nullptr;
    NetworkProvider* networkProvider = nullptr;
    
    // Command handlers
    SystemHandler* systemHandler = nullptr;
    
    // Wake cause string (set once on boot)
    const char* wakeCauseString = "unknown";

    // Timing
    unsigned long bootTime = 0;
    unsigned long lastActivityTime = 0;
    
    // Sleep control
    bool sleepRequested = false;
    unsigned long sleepDurationSeconds = 0;  // 0 = wake on interrupt only

    // Internal methods
    void initModules();
    void initProvidersAndHandlers();
    void loopModules();
    void prepareForSleep();
    void enterSleep();
    void logWakeupCause();
    void handleLowPowerModeWake();
    void handleLowBattery(uint8_t level);

    // Static callback wrapper for activity reporting
    static DeviceController* instance;
    static void activityCallbackWrapper();
    static void lowBatteryCallbackWrapper(uint8_t level);
};
