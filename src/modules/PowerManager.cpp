#include "PowerManager.h"
#include "../util.h"

#include <Arduino.h>
#include "driver/rtc_io.h"

/**
 * RTC-persistent configuration for modem power state.
 * Survives deep sleep.
 */
typedef struct {
    bool modemPowered;
} PowerManagerRTCConfig;

RTC_DATA_ATTR PowerManagerRTCConfig rtcConfig = {
    .modemPowered = false,
};

PowerManager::PowerManager() {
    pmu = new XPowersPMU();
}

PowerManager::~PowerManager() {
    if (pmu) {
        delete pmu;
        pmu = nullptr;
    }
}

bool PowerManager::setup() {
    Serial.print("[POWER] Initializing PMU ");

    // Initialize I2C communication with AXP2101
    if (!pmu->begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("FAIL!");
        return false;
    }

    // Configure DC3 for modem power (SIM7080 main power: 2700-3400mV)
    pmu->setDC3Voltage(3000);

    // Restore modem power state from RTC memory
    if (rtcConfig.modemPowered) {
        pmu->enableDC3();
    } else {
        pmu->disableDC3();
    }

    // GPS antenna power (BLDO2) - disabled, not using GPS
    pmu->setBLDO2Voltage(3300);
    pmu->disableBLDO2();

    // TS Pin detection must be disabled for battery charging to work
    pmu->disableTSPinMeasure();

    // Disable deep sleep wakeup during normal operation
    disableDeepSleepWakeup();

    initialized = true;
    Serial.println("OK!");

    return true;
}

void PowerManager::loop() {
    // PowerManager doesn't need regular polling
    // Battery monitoring can be done on-demand via getter methods
}

void PowerManager::prepareForSleep() {
    Serial.println("[POWER] Preparing for sleep");
    
    // Enable wakeup source before sleep
    enableDeepSleepWakeup();
}

bool PowerManager::isBusy() {
    // PowerManager is never busy - it's always ready for sleep
    return false;
}

bool PowerManager::isReady() {
    return initialized;
}

bool PowerManager::setModemPower(bool enable) {
    Serial.printf("[POWER] Modem power %s\n", enable ? "ON" : "OFF");
    
    rtcConfig.modemPowered = enable;
    
    if (enable) {
        return pmu->enableDC3();
    } else {
        return pmu->disableDC3();
    }
}

bool PowerManager::isModemPowered() {
    return rtcConfig.modemPowered;
}

void PowerManager::enableDeepSleepWakeup() {
    // Configure ESP32 to wake up on modem RI (Ring Indicator) pin
    // RI goes LOW when there's incoming data/call
    rtc_gpio_init(GPIO_NUM_3);
    rtc_gpio_set_direction(GPIO_NUM_3, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_3);
    rtc_gpio_pullup_dis(GPIO_NUM_3);
    
    // Wake on LOW level (RI is active low)
    esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_3, ESP_EXT1_WAKEUP_ANY_LOW);
    
    Serial.println("[POWER] Deep sleep wakeup enabled on RI pin");
}

void PowerManager::disableDeepSleepWakeup() {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    rtc_gpio_deinit(GPIO_NUM_3);
}

uint16_t PowerManager::getBatteryVoltage() {
    if (!pmu) return 0;
    return pmu->getBattVoltage();
}

uint8_t PowerManager::getBatteryPercent() {
    if (!pmu) return 0;
    return pmu->getBatteryPercent();
}

bool PowerManager::isCharging() {
    if (!pmu) return false;
    return pmu->isCharging();
}

bool PowerManager::isVbusConnected() {
    if (!pmu) return false;
    return pmu->isVbusIn();
}
