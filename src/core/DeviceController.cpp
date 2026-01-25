#include "DeviceController.h"
#include "CommandRouter.h"
#include "../modules/PowerManager.h"
#include "../modules/ModemManager.h"
#include "../modules/LinkManager.h"
#include "../providers/DeviceProvider.h"
#include "../providers/NetworkProvider.h"
#include "../handlers/SystemHandler.h"

#include <Arduino.h>
#include "driver/rtc_io.h"

// Default configuration
#define DEFAULT_ACTIVITY_TIMEOUT    60000   // 1 minute (testing)
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

    logWakeupCause();

    initModules();
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

    // Setup in dependency order
    if (!powerManager->setup()) {
        Serial.println("[DEVICE] PowerManager setup failed!");
    }

    if (!modemManager->setup()) {
        Serial.println("[DEVICE] ModemManager setup failed!");
    }

    if (!linkManager->setup()) {
        Serial.println("[DEVICE] LinkManager setup failed!");
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
    Serial.println("[DEVICE] Activity reported, sleep timer reset");
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
    Serial.printf("[DEVICE] Sleep requested (duration: %lu seconds)\n", durationSeconds);
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

    // Configure wake sources
    // GPIO_NUM_3 is modem RI pin
    rtc_gpio_init(GPIO_NUM_3);
    rtc_gpio_set_direction(GPIO_NUM_3, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_3);
    rtc_gpio_pullup_dis(GPIO_NUM_3);
    esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_3, ESP_EXT1_WAKEUP_ANY_LOW);

    // If a sleep duration was specified, also enable timer wake
    if (sleepDurationSeconds > 0) {
        Serial.printf("[DEVICE] Timer wake in %lu seconds\n", sleepDurationSeconds);
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
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("[DEVICE] Wakeup cause: EXT1 (Modem RI)");
            wakeCauseString = "modem_ri";
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            Serial.println("[DEVICE] Wakeup cause: Fresh boot");
            wakeCauseString = "fresh_boot";
            break;
        default:
            Serial.printf("[DEVICE] Wakeup cause: Unknown (%d)\n", cause);
            wakeCauseString = "unknown";
            break;
    }
}
