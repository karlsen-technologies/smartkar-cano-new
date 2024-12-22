#include "PowerManager.h"
#include "util.h"

#include <Arduino.h>
#include "driver/rtc_io.h"

typedef struct {
    bool modemPowered;
} PowerManagerConfig;

RTC_DATA_ATTR PowerManagerConfig config = {
    .modemPowered = false,
};

PowerManager::PowerManager() {
    this->PMU = new XPowersPMU();
}

PowerManager::~PowerManager() {
}

bool PowerManager::setup() {

    Serial.print("[PM] Initializing PMU ");

    // We need to start by initializing the power chip
    if(! this->PMU->begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("FAIL!");
        return false;
    }

    // Configure the PMU
    // Modem power channel
    this->PMU->setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V

    // Depending on the configuration, we may want to power the modem on or off
    if(config.modemPowered) {
        this->PMU->enableDC3();
    } else {
        this->PMU->disableDC3();
    }

    //Modem GPS antenna power channel
    this->PMU->setBLDO2Voltage(3300);
    this->PMU->disableBLDO2();     //Turn off the GPS power supply, we will not be using it

    // TS Pin detection must be disabled, otherwise battery cannot be charged
    this->PMU->disableTSPinMeasure();

    // Disable deep sleep
    this->disableDeepSleep();

    Serial.println("OK!");

    return true;
}

void PowerManager::enableDeepSleep() {
    // Configure the ESP wakeup source
    // We want to wakeup on the RTC GPIO 3 pin, this is the modem RI pin
    rtc_gpio_init(GPIO_NUM_3);
    rtc_gpio_set_direction(GPIO_NUM_3, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_3);
    rtc_gpio_pullup_dis(GPIO_NUM_3);
    esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_3, ESP_EXT1_WAKEUP_ANY_LOW);
}

void PowerManager::disableDeepSleep() {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    rtc_gpio_deinit(GPIO_NUM_3);
}

void PowerManager::loop() {
}

bool PowerManager::isModemPowered() {
    return config.modemPowered;
}

bool PowerManager::modemPower(bool enable) {
    config.modemPowered = enable;
    return (enable) ? this->PMU->enableDC3() : this->PMU->disableDC3();
}