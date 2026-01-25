#include "DeviceController.h"
#include "CommandRouter.h"
#include "../modules/PowerManager.h"
#include "../modules/ModemManager.h"
#include "../modules/LinkManager.h"
#include "../providers/DeviceProvider.h"
#include "../providers/NetworkProvider.h"
#include "../handlers/SystemHandler.h"

#include <Arduino.h>
#include "esp_sleep.h"

// Default configuration
#define DEFAULT_ACTIVITY_TIMEOUT    300000  // 5 minutes
#define DEFAULT_MIN_AWAKE_TIME      10000   // 10 seconds

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

    // Set activity callbacks on all modules
    ActivityCallback activityCb = getActivityCallback();
    powerManager->setActivityCallback(activityCb);
    modemManager->setActivityCallback(activityCb);
    linkManager->setActivityCallback(activityCb);
    
    // Set low battery callback
    powerManager->setLowBatteryCallback(&DeviceController::lowBatteryCallbackWrapper);

    // Setup in dependency order
    if (!powerManager->setup()) {
        Serial.println("[DEVICE] PowerManager setup failed!");
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

    Serial.println("[DEVICE] All modules initialized");
}

void DeviceController::initProvidersAndHandlers() {
    Serial.println("[DEVICE] Setting up providers and handlers...");
    
    // Create telemetry providers
    deviceProvider = new DeviceProvider(powerManager);
    deviceProvider->setWakeCause(wakeCauseString);
    
    networkProvider = new NetworkProvider(modemManager, linkManager);
    
    // Create command handlers
    systemHandler = new SystemHandler(this, commandRouter);
    
    // Register providers with CommandRouter
    commandRouter->registerProvider(deviceProvider);
    commandRouter->registerProvider(networkProvider);
    
    // Register handlers with CommandRouter
    commandRouter->registerHandler(systemHandler);
    
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

    // Must be awake for minimum time (unless sleep was explicitly requested)
    if (!sleepRequested && (now - bootTime) < config.minAwakeTime) {
        return false;
    }

    // Check activity timeout (skip if sleep was explicitly requested)
    if (!sleepRequested && (now - lastActivityTime) < config.activityTimeout) {
        return false;
    }

    // Check if any module is busy
    if (powerManager->isBusy()) {
        Serial.println("[DEVICE] Cannot sleep: PowerManager is busy");
        return false;
    }

    if (modemManager->isBusy()) {
        Serial.println("[DEVICE] Cannot sleep: ModemManager is busy");
        return false;
    }

    if (linkManager->isBusy()) {
        Serial.println("[DEVICE] Cannot sleep: LinkManager is busy");
        return false;
    }

    return true;
}

void DeviceController::prepareForSleep() {
    Serial.println("[DEVICE] Notifying modules of impending sleep...");

    // Prepare in reverse dependency order
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
        
        // TODO: Send low_battery event to server via LinkManager
        // For now, just log and prepare for shutdown
        // linkManager->sendEvent("low_battery", ...);
        
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
