#include "DeviceController.h"
#include "CommandRouter.h"
#include "../modules/PowerManager.h"
#include "../modules/ModemManager.h"
#include "../modules/LinkManager.h"
#include "../modules/CanManager.h"
#include "../vehicle/VehicleManager.h"
#include "../providers/DeviceProvider.h"
#include "../providers/NetworkProvider.h"
#include "../providers/VehicleProvider.h"
#include "../handlers/SystemHandler.h"
#include "../handlers/VehicleHandler.h"
#include "../handlers/ChargingProfileHandler.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include "esp_sleep.h"
#include "esp_system.h"

// Default configuration
#define DEFAULT_ACTIVITY_TIMEOUT    60000   // 1 minute (for testing)
#define DEFAULT_MIN_AWAKE_TIME      10000   // 10 seconds

// PMU retry configuration
#define PMU_INIT_RETRIES            3       // Number of PMU init attempts before reboot
#define PMU_RETRY_DELAY_MS          500     // Delay between PMU init attempts

// Static instance for callback
DeviceController* DeviceController::instance = nullptr;

DeviceController::DeviceController() {
    instance = this;
    
    config.activityTimeout = DEFAULT_ACTIVITY_TIMEOUT;
    config.minAwakeTime = DEFAULT_MIN_AWAKE_TIME;
}

void DeviceController::setup() {
    state = DeviceState::INITIALIZING;
    bootTime = millis();
    lastActivityTime = bootTime;

    Serial.begin(115200);
    Serial.println("[DEVICE] Starting up...");

    // Initialize modules first (need PowerManager for wake detection)
    initModules();
    
    // Now log wakeup cause (needs PowerManager to detect which pin)
    logWakeupCause();
    
    // Handle low power mode wake - check if we should stay asleep
    if (powerManager->isLowPowerMode()) {
        handleLowPowerModeWake();
        // If we return from handleLowPowerModeWake, power has been restored
    }

    initProvidersAndHandlers();

    state = DeviceState::RUNNING;
    Serial.println("[DEVICE] Initialization complete, entering RUNNING state");
}

void DeviceController::initModules() {
    Serial.println("[DEVICE] Initializing modules...");

    // Create command router first (used by LinkManager)
    commandRouter = new CommandRouter();

    // Create modules in dependency order
    powerManager = new PowerManager();
    modemManager = new ModemManager(powerManager);
    linkManager = new LinkManager(modemManager, commandRouter);
    canManager = new CanManager();

    // Set activity callbacks on all modules
    ActivityCallback activityCb = getActivityCallback();
    powerManager->setActivityCallback(activityCb);
    modemManager->setActivityCallback(activityCb);
    linkManager->setActivityCallback(activityCb);
    canManager->setActivityCallback(activityCb);  // Re-enabled - we check vehicle state separately now
    
    // Set low battery callback
    powerManager->setLowBatteryCallback(&DeviceController::lowBatteryCallbackWrapper);

    // Setup in dependency order
    // PMU is critical - retry a few times, then reboot if it keeps failing
    bool pmuOk = false;
    for (int attempt = 1; attempt <= PMU_INIT_RETRIES; attempt++) {
        if (powerManager->setup()) {
            pmuOk = true;
            break;
        }
        Serial.printf("[DEVICE] PMU init failed (attempt %d/%d)\r\n", attempt, PMU_INIT_RETRIES);
        if (attempt < PMU_INIT_RETRIES) {
            delay(PMU_RETRY_DELAY_MS);
        }
    }
    
    if (!pmuOk) {
        Serial.println("[DEVICE] PMU init failed after all retries - rebooting!");
        Serial.flush();
        delay(100);
        esp_restart();
        // Won't reach here
    }

    // Don't setup modem if we're in low power mode - we'll check VBUS first
    if (powerManager->isLowPowerMode()) {
        Serial.println("[DEVICE] In low power mode - deferring modem setup");
    } else {
        if (!modemManager->setup()) {
            Serial.println("[DEVICE] ModemManager setup failed!");
        }

        if (!linkManager->setup()) {
            Serial.println("[DEVICE] LinkManager setup failed!");
        }

        // Start the modem (unless it's already running from hotstart)
        if (!modemManager->isReady() && !modemManager->isBusy()) {
            Serial.println("[DEVICE] Starting modem...");
            modemManager->enable();
        }
    }
    
    // Setup CAN manager
    if (!canManager->setup()) {
        Serial.println("[DEVICE] CanManager setup failed!");
    }
    
    // Create and setup VehicleManager
    vehicleManager = new VehicleManager(canManager);
    if (!vehicleManager->setup()) {
        Serial.println("[DEVICE] VehicleManager setup failed!");
    }
    
    // Connect CAN frame callback to VehicleManager
    canManager->setFrameCallback([this](uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended) {
        vehicleManager->onCanFrame(canId, data, dlc, extended);
    });
    
    // Now start CAN
    canManager->start();

    Serial.println("[DEVICE] All modules initialized");
}

void DeviceController::initProvidersAndHandlers() {
    Serial.println("[DEVICE] Setting up providers and handlers...");
    
    // Create telemetry providers
    deviceProvider = new DeviceProvider(powerManager);
    deviceProvider->setWakeCause(wakeCauseString);
    
    networkProvider = new NetworkProvider(modemManager, linkManager);
    
    vehicleProvider = new VehicleProvider(vehicleManager);
    vehicleProvider->setCommandRouter(commandRouter);  // Enable event emission
    
    // Create command handlers
    systemHandler = new SystemHandler(this, commandRouter);
    vehicleHandler = new VehicleHandler(vehicleManager, commandRouter);
    chargingProfileHandler = new ChargingProfileHandler(vehicleManager, commandRouter);
    
    // Register providers with CommandRouter
    commandRouter->registerProvider(deviceProvider);
    commandRouter->registerProvider(networkProvider);
    commandRouter->registerProvider(vehicleProvider);
    
    // Register handlers with CommandRouter
    commandRouter->registerHandler(systemHandler);
    commandRouter->registerHandler(vehicleHandler);
    commandRouter->registerHandler(chargingProfileHandler);
    
    Serial.println("[DEVICE] Providers and handlers initialized");
}

void DeviceController::loop() {
    if (state == DeviceState::RUNNING) {
        loopModules();

        // Check if we should sleep
        if (canSleep()) {
            state = DeviceState::PREPARING_SLEEP;
        }
    }
    else if (state == DeviceState::PREPARING_SLEEP) {
        Serial.println("[DEVICE] Preparing for sleep...");
        prepareForSleep();
        state = DeviceState::SLEEPING;
    }
    else if (state == DeviceState::SLEEPING) {
        enterSleep();
    }
}

void DeviceController::loopModules() {
    // Loop in dependency order
    powerManager->loop();
    modemManager->loop();
    linkManager->loop();
    canManager->loop();
    vehicleManager->loop();
    
    // Check for vehicle state changes and emit events
    if (vehicleProvider) {
        vehicleProvider->checkAndEmitEvents();
    }
}

void DeviceController::reportActivity() {
    lastActivityTime = millis();
    // Note: Not logging here to avoid spam - activity is reported frequently
}

void DeviceController::activityCallbackWrapper() {
    if (instance) {
        instance->reportActivity();
    }
}

ActivityCallback DeviceController::getActivityCallback() {
    return &DeviceController::activityCallbackWrapper;
}

void DeviceController::requestSleep(unsigned long durationSeconds) {
    Serial.printf("[DEVICE] Sleep requested (duration: %lu seconds)\r\n", durationSeconds);
    sleepRequested = true;
    sleepDurationSeconds = durationSeconds;
}

bool DeviceController::canSleep() {
    unsigned long now = millis();
    
    // Rate limit diagnostic logging to once per 5 seconds
    bool shouldLog = (now - lastSleepLogTime) > 5000;

    // Must be awake for minimum time (unless sleep was explicitly requested)
    if (!sleepRequested && (now - bootTime) < config.minAwakeTime) {
        if (shouldLog) {
            Serial.printf("[DEVICE] Cannot sleep: boot time %lums ago (need %lums)\r\n",
                         now - bootTime, config.minAwakeTime);
            lastSleepLogTime = now;
        }
        return false;
    }

    // If vehicle CAN bus is active, stay awake (skip if sleep was explicitly requested)
    if (!sleepRequested && vehicleManager && vehicleManager->isVehicleAwake()) {
        if (shouldLog) {
            Serial.println("[DEVICE] Cannot sleep: Vehicle CAN bus is active");
            lastSleepLogTime = now;
        }
        return false;
    }

    // Check activity timeout for non-vehicle activity (skip if sleep was explicitly requested)
    if (!sleepRequested && (now - lastActivityTime) < config.activityTimeout) {
        if (shouldLog) {
            Serial.printf("[DEVICE] Cannot sleep: activity %lums ago (need %lums)\r\n",
                         now - lastActivityTime, config.activityTimeout);
            lastSleepLogTime = now;
        }
        return false;
    }

    // Check if any module is busy
    if (powerManager->isBusy()) {
        if (shouldLog) {
            Serial.println("[DEVICE] Cannot sleep: PowerManager is busy");
            lastSleepLogTime = now;
        }
        return false;
    }

    if (modemManager->isBusy()) {
        if (shouldLog) {
            Serial.println("[DEVICE] Cannot sleep: ModemManager is busy");
            lastSleepLogTime = now;
        }
        return false;
    }

    if (linkManager->isBusy()) {
        if (shouldLog) {
            Serial.println("[DEVICE] Cannot sleep: LinkManager is busy");
            lastSleepLogTime = now;
        }
        return false;
    }

    if (canManager->isBusy()) {
        if (shouldLog) {
            Serial.println("[DEVICE] Cannot sleep: CanManager is busy");
            lastSleepLogTime = now;
        }
        return false;
    }

    Serial.println("[DEVICE] Sleep conditions met, can sleep");
    return true;
}

void DeviceController::prepareForSleep() {
    Serial.println("[DEVICE] Notifying modules of impending sleep...");
    
    // Stop VehicleManager from processing CAN frames FIRST (before CAN task is killed)
    if (vehicleManager) {
        vehicleManager->prepareForSleep();
    }
    
    // Now stop modules (CAN task will be deleted here)
    canManager->prepareForSleep();
    linkManager->prepareForSleep();
    modemManager->prepareForSleep();
    powerManager->prepareForSleep();

    Serial.println("[DEVICE] All modules prepared for sleep");
}

void DeviceController::enterSleep() {
    Serial.println("[DEVICE] Entering deep sleep...");
    Serial.flush();

    // PowerManager handles wake source configuration in prepareForSleep()
    // which was already called. It sets up GPIO3 (modem RI) and GPIO6 (PMU IRQ)

    // If a sleep duration was specified, also enable timer wake
    if (sleepDurationSeconds > 0) {
        Serial.printf("[DEVICE] Timer wake in %lu seconds\r\n", sleepDurationSeconds);
        esp_sleep_enable_timer_wakeup(sleepDurationSeconds * 1000000ULL);
    }

    Serial.flush();
    esp_deep_sleep_start();
}

void DeviceController::logWakeupCause() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("[DEVICE] Wakeup cause: Timer");
            wakeCauseString = "timer";
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            Serial.println("[DEVICE] Wakeup cause: GPIO");
            wakeCauseString = "gpio";
            break;
        case ESP_SLEEP_WAKEUP_EXT1: {
            // Determine which pin triggered the wake
            int wakePin = powerManager ? powerManager->getWakeupPin() : -1;
            if (wakePin == 3) {
                Serial.println("[DEVICE] Wakeup cause: EXT1 (Modem RI)");
                wakeCauseString = "modem_ri";
            } else if (wakePin == 6) {  // PMU_INPUT_PIN
                Serial.println("[DEVICE] Wakeup cause: EXT1 (PMU IRQ)");
                wakeCauseString = "pmu_irq";
                // Check and log what PMU event occurred
                powerManager->checkPmuWakeupCause();
            } else if (wakePin == 21) {  // CAN RX pin
                Serial.println("[DEVICE] Wakeup cause: EXT1 (CAN bus activity)");
                wakeCauseString = "can_activity";
            } else {
                Serial.printf("[DEVICE] Wakeup cause: EXT1 (GPIO%d)\r\n", wakePin);
                wakeCauseString = "ext1";
            }
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            Serial.println("[DEVICE] Wakeup cause: Fresh boot");
            wakeCauseString = "fresh_boot";
            break;
        default:
            Serial.printf("[DEVICE] Wakeup cause: Unknown (%d)\r\n", cause);
            wakeCauseString = "unknown";
            break;
    }
}

void DeviceController::handleLowPowerModeWake() {
    Serial.println("[DEVICE] Woke in low power mode - checking if power restored...");
    
    // Check if VBUS (USB/external power) is now connected
    if (powerManager->isVbusConnected()) {
        Serial.println("[DEVICE] External power restored! Exiting low power mode.");
        powerManager->exitLowPowerMode();
        
        // Now we need to setup and enable the modem since we skipped it
        if (!modemManager->setup()) {
            Serial.println("[DEVICE] ModemManager setup failed!");
        }
        if (!linkManager->setup()) {
            Serial.println("[DEVICE] LinkManager setup failed!");
        }
        if (!modemManager->isReady() && !modemManager->isBusy()) {
            Serial.println("[DEVICE] Starting modem...");
            modemManager->enable();
        }
        return;  // Continue normal operation
    }
    
    // Power not restored - go back to sleep immediately
    Serial.println("[DEVICE] No external power - returning to low power sleep");
    Serial.flush();
    
    // Re-enable wake sources and sleep
    powerManager->enableDeepSleepWakeup();
    esp_deep_sleep_start();
    // We won't return from here
}

void DeviceController::handleLowBattery(uint8_t level) {
    Serial.printf("[DEVICE] Low battery callback triggered (level %u)\r\n", level);
    
    if (level == 1) {
        // Level 1 = 10% warning - initiate graceful shutdown
        Serial.println("[DEVICE] Initiating low battery shutdown sequence...");
        
        // Send low_battery event to server
        if (commandRouter) {
            JsonDocument doc;
            JsonObject details = doc.to<JsonObject>();
            details["level"] = level;
            details["percentage"] = 10;
            details["action"] = "shutdown_initiated";
            commandRouter->sendEvent("device", "lowBattery", &details);
        }
        
        // Disconnect from server gracefully
        linkManager->prepareForSleep();
        
        // Disable the modem to save power
        Serial.println("[DEVICE] Disabling modem to conserve power");
        modemManager->disable();
        
        // Enter low power mode
        powerManager->enterLowPowerMode();
        
        // Request immediate sleep (0 = wake on interrupt only)
        requestSleep(0);
        
    } else if (level == 2) {
        // Level 2 = 5% critical - we might not have sent level 1 if it happened during sleep
        // Do emergency shutdown
        Serial.println("[DEVICE] CRITICAL battery - emergency shutdown!");
        
        // Try to send critical event (may not complete if modem disconnected)
        if (commandRouter) {
            JsonDocument doc;
            JsonObject details = doc.to<JsonObject>();
            details["level"] = level;
            details["percentage"] = 5;
            details["action"] = "emergency_shutdown";
            commandRouter->sendEvent("device", "lowBattery", &details);
        }
        
        // Just kill the modem and sleep immediately
        powerManager->setModemPower(false);
        powerManager->enterLowPowerMode();
        powerManager->enableDeepSleepWakeup();
        
        Serial.flush();
        esp_deep_sleep_start();
    }
}

void DeviceController::lowBatteryCallbackWrapper(uint8_t level) {
    if (instance) {
        instance->handleLowBattery(level);
    }
}
