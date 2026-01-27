#include "PowerManager.h"
#include "../util.h"

#include <Arduino.h>
#include "driver/rtc_io.h"

/**
 * RTC-persistent configuration for power state.
 * Survives deep sleep.
 */
typedef struct {
    bool modemPowered;
    bool lowPowerMode;  // True when we've entered ultra-low power mode due to low battery
} PowerManagerRTCConfig;

RTC_DATA_ATTR PowerManagerRTCConfig rtcConfig = {
    .modemPowered = false,
    .lowPowerMode = false,
};

// Static volatile flag for ISR
volatile bool PowerManager::pmuIrqTriggered = false;

void IRAM_ATTR PowerManager::pmuIrqHandler() {
    pmuIrqTriggered = true;
}

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
    // (LilyGo T-SIM7080G has no NTC thermistor on the battery connector)
    pmu->disableTSPinMeasure();

    // Configure 18650 Li-ion battery charging
    // Target voltage: 4.2V (standard Li-ion)
    pmu->setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);
    
    // Charge current: 500mA (safe for most 18650 cells, ~0.2C for 2500mAh)
    pmu->setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_500MA);
    
    // Precharge current: 50mA (for deeply discharged batteries < 3.0V)
    pmu->setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
    
    // Termination current: 25mA (stop charging when current drops below this)
    pmu->setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);
    pmu->enableChargerTerminationLimit();
    
    // VBUS input current limit: 900mA (USB 3.0 standard)
    pmu->setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_900MA);
    
    // Low battery warning at 10%, shutdown at 5%
    pmu->setLowBatWarnThreshold(10);
    pmu->setLowBatShutdownThreshold(5);

    // Enable PMU IRQs for normal (awake) operation
    // These include informational events that we want to log while awake
    configureAwakeIrqs();
    
    // Setup PMU IRQ pin interrupt
    pinMode(PMU_INPUT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PMU_INPUT_PIN), pmuIrqHandler, FALLING);
    Serial.printf("[POWER] PMU IRQ enabled on GPIO%d\r\n", PMU_INPUT_PIN);

    // Disable deep sleep wakeup during normal operation
    disableDeepSleepWakeup();

    initialized = true;
    Serial.println("OK!");

    // Print charging configuration
    printBatteryStatus();

    return true;
}

void PowerManager::loop() {
    if (!initialized) return;
    
    // Check for PMU IRQ
    if (pmuIrqTriggered) {
        pmuIrqTriggered = false;
        handlePmuIrq();
    }
}

void PowerManager::prepareForSleep() {
    Serial.println("[POWER] Preparing for sleep");
    
    // Switch to minimal IRQ set for sleep - only critical events
    configureSleepIrqs();
    
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
    Serial.printf("[POWER] Modem power %s\r\n", enable ? "ON" : "OFF");
    
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
    // Configure ESP32 to wake up on:
    // - GPIO3: Modem RI (Ring Indicator) - incoming data/call
    // - GPIO6: PMU IRQ - low battery warning, USB connect, etc.
    // - GPIO21: CAN RX - CAN bus activity (vehicle wake)
    
    // Debug: print current pin states before configuring
    Serial.printf("[POWER] Pin states before sleep - GPIO3: %d, GPIO6: %d, GPIO21: %d\r\n",
        digitalRead(GPIO_NUM_3), digitalRead(PMU_INPUT_PIN), digitalRead(GPIO_NUM_21));
    
    // Setup modem RI pin (GPIO3)
    rtc_gpio_init(GPIO_NUM_3);
    rtc_gpio_set_direction(GPIO_NUM_3, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_3);
    rtc_gpio_pullup_dis(GPIO_NUM_3);
    
    // Setup PMU IRQ pin (GPIO6)
    rtc_gpio_init((gpio_num_t)PMU_INPUT_PIN);
    rtc_gpio_set_direction((gpio_num_t)PMU_INPUT_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis((gpio_num_t)PMU_INPUT_PIN);
    rtc_gpio_pullup_en((gpio_num_t)PMU_INPUT_PIN);  // PMU IRQ is active low, needs pullup
    
    // Setup CAN RX pin (GPIO21) - CAN idles high (recessive), goes low on activity
    rtc_gpio_init(GPIO_NUM_21);
    rtc_gpio_set_direction(GPIO_NUM_21, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_21);
    rtc_gpio_pullup_dis(GPIO_NUM_21);  // Transceiver handles the level
    
    // Wake on LOW level on any pin (all are active low)
    // CAN idles high (recessive), goes low when there's bus activity (dominant)
    uint64_t wakeMask = (1ULL << GPIO_NUM_3) | (1ULL << PMU_INPUT_PIN) | (1ULL << GPIO_NUM_21);
    esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_LOW);
    
    Serial.printf("[POWER] Deep sleep wakeup enabled on GPIO3 (RI), GPIO%d (PMU), GPIO21 (CAN RX)\r\n", PMU_INPUT_PIN);
}

void PowerManager::disableDeepSleepWakeup() {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    rtc_gpio_deinit(GPIO_NUM_3);
    rtc_gpio_deinit((gpio_num_t)PMU_INPUT_PIN);
    rtc_gpio_deinit(GPIO_NUM_21);
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

const char* PowerManager::getChargingState() {
    if (!pmu) return "unknown";
    
    xpowers_chg_status_t status = pmu->getChargerStatus();
    switch (status) {
        case XPOWERS_AXP2101_CHG_TRI_STATE:  return "trickle";
        case XPOWERS_AXP2101_CHG_PRE_STATE:  return "precharge";
        case XPOWERS_AXP2101_CHG_CC_STATE:   return "cc";      // constant current
        case XPOWERS_AXP2101_CHG_CV_STATE:   return "cv";      // constant voltage
        case XPOWERS_AXP2101_CHG_DONE_STATE: return "done";
        case XPOWERS_AXP2101_CHG_STOP_STATE: return "stopped";
        default:                             return "unknown";
    }
}

uint16_t PowerManager::getChargeCurrentSetting() {
    if (!pmu) return 0;
    
    // Map the enum value to actual mA
    uint8_t setting = pmu->getChargerConstantCurr();
    switch (setting) {
        case XPOWERS_AXP2101_CHG_CUR_0MA:    return 0;
        case XPOWERS_AXP2101_CHG_CUR_100MA:  return 100;
        case XPOWERS_AXP2101_CHG_CUR_125MA:  return 125;
        case XPOWERS_AXP2101_CHG_CUR_150MA:  return 150;
        case XPOWERS_AXP2101_CHG_CUR_175MA:  return 175;
        case XPOWERS_AXP2101_CHG_CUR_200MA:  return 200;
        case XPOWERS_AXP2101_CHG_CUR_300MA:  return 300;
        case XPOWERS_AXP2101_CHG_CUR_400MA:  return 400;
        case XPOWERS_AXP2101_CHG_CUR_500MA:  return 500;
        case XPOWERS_AXP2101_CHG_CUR_600MA:  return 600;
        case XPOWERS_AXP2101_CHG_CUR_700MA:  return 700;
        case XPOWERS_AXP2101_CHG_CUR_800MA:  return 800;
        case XPOWERS_AXP2101_CHG_CUR_900MA:  return 900;
        case XPOWERS_AXP2101_CHG_CUR_1000MA: return 1000;
        default:                             return 0;
    }
}

void PowerManager::printBatteryStatus() {
    if (!pmu) {
        Serial.println("[POWER] PMU not initialized");
        return;
    }

    Serial.println("[POWER] ===== Battery & Charging Status =====");
    
    // Power source status
    bool vbusGood = pmu->isVbusGood();
    bool vbusIn = pmu->isVbusIn();
    Serial.printf("[POWER] VBUS: %s (good=%s)\r\n", 
        vbusIn ? "connected" : "disconnected",
        vbusGood ? "yes" : "no");
    
    // Battery status
    bool battConnected = pmu->isBatteryConnect();
    uint16_t battVoltage = pmu->getBattVoltage();
    uint8_t battPercent = pmu->getBatteryPercent();
    Serial.printf("[POWER] Battery: %s\r\n", battConnected ? "connected" : "NOT DETECTED");
    Serial.printf("[POWER]   Voltage: %u mV\r\n", battVoltage);
    Serial.printf("[POWER]   Percent: %u%%\r\n", battPercent);
    
    // Charging status
    bool isChg = pmu->isCharging();
    const char* chgState = getChargingState();
    Serial.printf("[POWER] Charging: %s (state=%s)\r\n", 
        isChg ? "YES" : "no", chgState);
    
    // Charging configuration
    Serial.println("[POWER] Charging config:");
    Serial.printf("[POWER]   Target voltage: 4.2V\r\n");
    Serial.printf("[POWER]   Charge current: %u mA\r\n", getChargeCurrentSetting());
    Serial.printf("[POWER]   VBUS current limit: 900 mA\r\n");
    Serial.printf("[POWER]   Low batt warn: %u%%\r\n", pmu->getLowBatWarnThreshold() + 5);
    Serial.printf("[POWER]   Low batt shutdown: %u%%\r\n", pmu->getLowBatShutdownThreshold());
    
    Serial.println("[POWER] ==========================================");
}

void PowerManager::handlePmuIrq() {
    if (!pmu) return;
    
    // Read and clear IRQ status
    uint64_t irqStatus = pmu->getIrqStatus();
    pmu->clearIrqStatus();
    
    Serial.printf("[POWER] PMU IRQ triggered (status: 0x%08X)\r\n", (uint32_t)irqStatus);
    
    // Check for low battery warning level 1 (10%)
    if (irqStatus & XPOWERS_AXP2101_WARNING_LEVEL1_IRQ) {
        uint8_t percent = pmu->getBatteryPercent();
        uint16_t voltage = pmu->getBattVoltage();
        Serial.printf("[POWER] LOW BATTERY WARNING: %u%% (%umV)\r\n", percent, voltage);
        
        if (lowBatteryCallback) {
            lowBatteryCallback(1);  // Level 1 = warning
        }
    }
    
    // Check for low battery warning level 2 (5% - critical)
    if (irqStatus & XPOWERS_AXP2101_WARNING_LEVEL2_IRQ) {
        uint8_t percent = pmu->getBatteryPercent();
        uint16_t voltage = pmu->getBattVoltage();
        Serial.printf("[POWER] CRITICAL BATTERY: %u%% (%umV) - SHUTDOWN IMMINENT\r\n", percent, voltage);
        
        if (lowBatteryCallback) {
            lowBatteryCallback(2);  // Level 2 = critical
        }
    }
    
    // Battery inserted
    if (irqStatus & XPOWERS_AXP2101_BAT_INSERT_IRQ) {
        Serial.println("[POWER] Battery inserted");
        printBatteryStatus();
    }
    
    // Battery removed
    if (irqStatus & XPOWERS_AXP2101_BAT_REMOVE_IRQ) {
        Serial.println("[POWER] Battery removed!");
    }
    
    // USB/VBUS connected
    if (irqStatus & XPOWERS_AXP2101_VBUS_INSERT_IRQ) {
        Serial.println("[POWER] USB power connected");
    }
    
    // USB/VBUS disconnected
    if (irqStatus & XPOWERS_AXP2101_VBUS_REMOVE_IRQ) {
        Serial.println("[POWER] USB power disconnected");
    }
    
    // Charging events (informational)
    if (irqStatus & XPOWERS_AXP2101_BAT_CHG_START_IRQ) {
        Serial.println("[POWER] Charging started");
    }
    
    if (irqStatus & XPOWERS_AXP2101_BAT_CHG_DONE_IRQ) {
        Serial.println("[POWER] Charging complete");
    }
}

// ============== IRQ Configuration ==============

void PowerManager::configureAwakeIrqs() {
    if (!pmu) return;
    
    // While awake, enable all useful IRQs for logging and monitoring
    pmu->disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    pmu->enableIRQ(XPOWERS_AXP2101_WARNING_LEVEL1_IRQ);  // 10% warning - critical
    pmu->enableIRQ(XPOWERS_AXP2101_WARNING_LEVEL2_IRQ);  // 5% critical - critical
    pmu->enableIRQ(XPOWERS_AXP2101_BAT_INSERT_IRQ);      // Battery inserted - informational
    pmu->enableIRQ(XPOWERS_AXP2101_BAT_REMOVE_IRQ);      // Battery removed - informational
    pmu->enableIRQ(XPOWERS_AXP2101_VBUS_INSERT_IRQ);     // USB connected - informational
    pmu->enableIRQ(XPOWERS_AXP2101_VBUS_REMOVE_IRQ);     // USB disconnected - informational
    pmu->enableIRQ(XPOWERS_AXP2101_BAT_CHG_START_IRQ);   // Charging started - informational
    pmu->enableIRQ(XPOWERS_AXP2101_BAT_CHG_DONE_IRQ);    // Charging complete - informational
    pmu->clearIrqStatus();
    
    Serial.println("[POWER] Configured awake IRQs (full set)");
}

void PowerManager::configureSleepIrqs() {
    if (!pmu) return;
    
    // During sleep, only enable critical IRQs that should wake the device:
    // - Low battery warnings (need to handle power saving)
    // - VBUS insert (car power restored - useful in low power mode)
    // 
    // Do NOT enable during sleep:
    // - BAT_INSERT/REMOVE (won't happen in deployed car)
    // - VBUS_REMOVE (don't need to wake when power is cut, just keep sleeping)
    // - CHG_START/DONE (will fire repeatedly as battery tops off, wasting power)
    
    pmu->disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    pmu->enableIRQ(XPOWERS_AXP2101_WARNING_LEVEL1_IRQ);  // 10% warning
    pmu->enableIRQ(XPOWERS_AXP2101_WARNING_LEVEL2_IRQ);  // 5% critical
    pmu->enableIRQ(XPOWERS_AXP2101_VBUS_INSERT_IRQ);     // Power restored (for low power mode recovery)
    pmu->clearIrqStatus();
    
    Serial.println("[POWER] Configured sleep IRQs (minimal set)");
}

// ============== Low Power Mode Methods ==============

bool PowerManager::isLowPowerMode() {
    return rtcConfig.lowPowerMode;
}

void PowerManager::enterLowPowerMode() {
    Serial.println("[POWER] Entering LOW POWER MODE (modem will stay off)");
    rtcConfig.lowPowerMode = true;
}

void PowerManager::exitLowPowerMode() {
    Serial.println("[POWER] Exiting low power mode - resuming normal operation");
    rtcConfig.lowPowerMode = false;
}

int PowerManager::getWakeupPin() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    
    if (cause != ESP_SLEEP_WAKEUP_EXT1) {
        return -1;  // Not an EXT1 wake
    }
    
    // Get the GPIO mask that triggered the wake
    uint64_t wakeupBits = esp_sleep_get_ext1_wakeup_status();
    
    if (wakeupBits == 0) {
        return -1;  // No GPIO triggered (shouldn't happen)
    }
    
    // Find which GPIO triggered the wake
    // We only care about GPIO3 (modem RI) and GPIO6 (PMU IRQ)
    if (wakeupBits & (1ULL << GPIO_NUM_3)) {
        return 3;
    }
    if (wakeupBits & (1ULL << PMU_INPUT_PIN)) {
        return PMU_INPUT_PIN;
    }
    
    // Some other GPIO
    for (int i = 0; i < 22; i++) {  // RTC GPIOs are 0-21
        if (wakeupBits & (1ULL << i)) {
            return i;
        }
    }
    
    return -1;
}

bool PowerManager::checkPmuWakeupCause() {
    if (!pmu) return false;
    
    // Read the PMU IRQ status (without clearing yet)
    // This tells us what caused the PMU to trigger the wake
    uint64_t irqStatus = pmu->getIrqStatus();
    
    if (irqStatus == 0) {
        Serial.println("[POWER] PMU woke us but no IRQ status (already cleared?)");
        return false;
    }
    
    Serial.printf("[POWER] PMU wake cause IRQ status: 0x%08X\r\n", (uint32_t)irqStatus);
    
    bool hasEvents = false;
    
    // Log the specific causes
    if (irqStatus & XPOWERS_AXP2101_WARNING_LEVEL1_IRQ) {
        Serial.println("[POWER] PMU wake: Low battery warning (10%)");
        hasEvents = true;
    }
    if (irqStatus & XPOWERS_AXP2101_WARNING_LEVEL2_IRQ) {
        Serial.println("[POWER] PMU wake: Critical battery (5%)");
        hasEvents = true;
    }
    if (irqStatus & XPOWERS_AXP2101_VBUS_INSERT_IRQ) {
        Serial.println("[POWER] PMU wake: USB power connected");
        hasEvents = true;
    }
    if (irqStatus & XPOWERS_AXP2101_VBUS_REMOVE_IRQ) {
        Serial.println("[POWER] PMU wake: USB power disconnected");
        hasEvents = true;
    }
    if (irqStatus & XPOWERS_AXP2101_BAT_INSERT_IRQ) {
        Serial.println("[POWER] PMU wake: Battery inserted");
        hasEvents = true;
    }
    if (irqStatus & XPOWERS_AXP2101_BAT_REMOVE_IRQ) {
        Serial.println("[POWER] PMU wake: Battery removed");
        hasEvents = true;
    }
    
    // Clear the IRQ status
    pmu->clearIrqStatus();
    
    return hasEvents;
}
