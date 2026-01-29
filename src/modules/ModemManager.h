#pragma once

// TinyGSM configuration - MUST be defined before including TinyGsmClient.h
#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_MODEM_SIM7080

#include <Arduino.h>
#include <TinyGsmClient.h>
#include "../modem/TinyGsmSim7080Extended.h"
#include "../core/IModule.h"

// Forward declarations
class PowerManager;

/**
 * Modem state machine states
 */
enum class ModemState {
    OFF,            // Modem is powered off
    STARTING,       // Modem is powering on and initializing
    HOTSTART,       // Modem was already powered (wake from sleep)
    CONFIGURING,    // Sending initial AT commands
    NO_SIM,         // No SIM card detected
    SEARCHING,      // Searching for network
    REGISTERED,     // Registered to network, not yet connected to internet
    UNREGISTERED,   // Lost network registration
    DENIED,         // Network registration denied
    CONNECTING,     // Establishing data connection
    CONNECTED,      // Connected to internet (GPRS/LTE)
    MODEM_ERROR     // Error state (renamed from ERROR to avoid macro conflicts)
};

/**
 * Network registration status from +CEREG
 */
enum class RegStatus {
    NOT_REGISTERED = 0,
    REGISTERED_HOME = 1,
    SEARCHING = 2,
    DENIED = 3,
    UNKNOWN = 4,
    REGISTERED_ROAMING = 5
};

/**
 * ModemManager - SIM7080G cellular modem control module
 * 
 * Responsibilities:
 * - Initialize and configure the SIM7080G modem
 * - Handle modem power sequencing
 * - Manage network registration state machine
 * - Handle modem interrupts (RI pin)
 * - Provide TCP client for LinkManager
 */
class ModemManager : public IModule {
public:
    /**
     * Constructor
     * @param powerManager Reference to PowerManager for modem power control
     */
    ModemManager(PowerManager* powerManager);
    ~ModemManager();

    // IModule interface
    bool setup() override;
    void loop() override;
    void prepareForSleep() override;
    bool isBusy() override;
    bool isReady() override;
    const char* getName() override { return "MODEM"; }

    /**
     * Set the activity callback for reporting activity to DeviceController.
     */
    void setActivityCallback(ActivityCallback callback) { activityCallback = callback; }

    // Modem control
    /**
     * Enable/power on the modem and start initialization.
     * @return true if the enable sequence was started
     */
    bool enable();

    /**
     * Disable/power off the modem.
     * @return true if the disable sequence was started
     */
    bool disable();

    /**
     * Get current modem state.
     */
    ModemState getState() { return state; }

    /**
     * Check if modem is connected to internet.
     */
    bool isConnected() { return state == ModemState::CONNECTED; }

    /**
     * Check if modem did a hotstart (was already powered on boot).
     * This indicates we may have persistent TCP connections to adopt.
     */
    bool wasHotstart() { return didHotstart; }

    /**
     * Get the TinyGsm modem instance for TCP client creation.
     * Only valid when modem is connected.
     */
    TinyGsmSim7080Extended* getModem() { return modem; }

    /**
     * Create a TCP client for the LinkManager.
     * @return New TinyGsmClient instance (caller owns memory)
     */
    TinyGsmClient* createClient();

    /**
     * Get SIM CCID (cached after first read).
     */
    String getSimCCID();

    /**
     * Get signal quality (RSSI).
     * @return Signal strength in dBm, or 0 if unavailable
     */
    int16_t getSignalQuality();

    // Interrupt handling
    /**
     * Call this from the main loop to process pending interrupts.
     * Must be called from the main loop context, not ISR.
     */
    void handleInterrupt();

    /**
     * ISR-safe flag setter - called from ISR.
     */
    void IRAM_ATTR onInterrupt();

    // Singleton for ISR access
    static ModemManager* instance() { return _instance; }

private:
    static ModemManager* _instance;
    
    PowerManager* powerManager = nullptr;
    TinyGsmSim7080Extended* modem = nullptr;
    ActivityCallback activityCallback = nullptr;

    ModemState state = ModemState::OFF;
    ModemState previousState = ModemState::OFF;
    bool didHotstart = false;  // Track if modem did hotstart (for TCP connection adoption)
    
    // Timing
    unsigned long stateEntryTime = 0;
    unsigned long lastLoopTime = 0;

    // ISR flag
    volatile bool hasInterrupt = false;

    // Cached data
    String simCCID;
    int16_t cachedSignalQuality = 0;

    // State machine helpers
    bool stateJustChanged();
    void setState(ModemState newState);
    unsigned long timeInState();

    // State handlers
    void handleDisabledState();
    void handleStartingState();
    void handleHotstartState();
    void handleConfiguringState();
    void handleSearchingState();
    void handleRegisteredState();
    void handleConnectedState();

    // Modem operations
    bool powerOnSequence();
    bool sendInitCommands();
    RegStatus checkRegistration();
    bool activateDataConnection();

    // Interrupt management
    void enableInterrupt();
    void disableInterrupt();

    // Delay constants (ms)
    static const unsigned long SEARCH_CHECK_INTERVAL = 1000;
    static const unsigned long REGISTERED_CHECK_INTERVAL = 1000;
    static const unsigned long CONNECTED_CHECK_INTERVAL = 10000;
    static const unsigned long DENIED_RETRY_INTERVAL = 30000;
    static const unsigned long UNREGISTERED_RETRY_INTERVAL = 5000;
};
