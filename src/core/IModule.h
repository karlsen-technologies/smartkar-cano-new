#pragma once

/**
 * IModule - Base interface for all device modules
 * 
 * All modules (PowerManager, ModemManager, LinkManager, CANManager)
 * implement this interface to provide consistent lifecycle management
 * and sleep coordination.
 */
class IModule {
public:
    virtual ~IModule() = default;

    /**
     * Initialize the module.
     * Called once during device startup.
     * 
     * @return true if initialization successful
     */
    virtual bool setup() = 0;

    /**
     * Main loop tick.
     * Called repeatedly from the main loop.
     * Should be non-blocking.
     */
    virtual void loop() = 0;

    /**
     * Prepare for deep sleep.
     * Called before entering deep sleep mode.
     * Module should save state, close connections, etc.
     */
    virtual void prepareForSleep() = 0;

    /**
     * Check if module is busy.
     * A busy module blocks the device from sleeping.
     * 
     * Examples of "busy":
     * - ModemManager: connecting to network, configuring
     * - LinkManager: authenticating with server
     * 
     * @return true if module is actively working and should not be interrupted
     */
    virtual bool isBusy() = 0;

    /**
     * Check if module is ready for dependent modules.
     * 
     * Examples:
     * - PowerManager: always ready after setup
     * - ModemManager: ready when connected to internet
     * - LinkManager: ready when authenticated with server
     * 
     * @return true if module is in a ready/stable state
     */
    virtual bool isReady() = 0;

    /**
     * Get module name for logging.
     * 
     * @return Short name identifier (e.g., "POWER", "MODEM", "LINK")
     */
    virtual const char* getName() = 0;
};

/**
 * Activity callback type.
 * Modules call this to report activity to the DeviceController.
 * This resets the sleep timer.
 */
typedef void (*ActivityCallback)();
