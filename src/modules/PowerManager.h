#pragma once

#include <Arduino.h>

// Define AXP2101 chip before including XPowersLib
#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

#include "../core/IModule.h"

// Callback type for low battery warning
typedef void (*LowBatteryCallback)(uint8_t level);

/**
 * PowerManager - AXP2101 PMU control module
 * 
 * Responsibilities:
 * - Initialize and configure the AXP2101 PMU
 * - Control modem power rail (DC3)
 * - Configure deep sleep wakeup sources
 * - Monitor battery status
 * - Handle low battery warnings via IRQ
 */
class PowerManager : public IModule {
public:
    PowerManager();
    ~PowerManager();

    // IModule interface
    bool setup() override;
    void loop() override;
    void prepareForSleep() override;
    bool isBusy() override;
    bool isReady() override;
    const char* getName() override { return "POWER"; }

    /**
     * Set the activity callback for reporting activity to DeviceController.
     * PowerManager doesn't typically generate activity, but we keep this
     * for interface consistency.
     */
    void setActivityCallback(ActivityCallback callback) { activityCallback = callback; }

    /**
     * Set callback for low battery warnings.
     * @param callback Function called with warning level (1 = warning, 2 = critical)
     */
    void setLowBatteryCallback(LowBatteryCallback callback) { lowBatteryCallback = callback; }

    // Power control
    /**
     * Enable or disable modem power (DC3 rail).
     * @param enable true to power on, false to power off
     * @return true if operation succeeded
     */
    bool setModemPower(bool enable);

    /**
     * Check if modem power is currently enabled.
     * @return true if modem is powered
     */
    bool isModemPowered();

    /**
     * Enable deep sleep wakeup source (modem RI pin).
     * Call this before entering deep sleep.
     */
    void enableDeepSleepWakeup();

    /**
     * Disable deep sleep wakeup sources.
     * Called during normal operation.
     */
    void disableDeepSleepWakeup();

    // Battery monitoring
    /**
     * Get battery voltage in millivolts.
     */
    uint16_t getBatteryVoltage();

    /**
     * Get battery percentage (0-100).
     */
    uint8_t getBatteryPercent();

    /**
     * Check if device is charging.
     */
    bool isCharging();

    /**
     * Check if external power is connected.
     */
    bool isVbusConnected();

    /**
     * Get detailed charging state.
     * @return One of: "idle", "trickle", "precharge", "cc", "cv", "done", "stopped"
     */
    const char* getChargingState();

    /**
     * Get the configured charge current setting in mA.
     */
    uint16_t getChargeCurrentSetting();

    /**
     * Print detailed battery and charging status to Serial.
     * Useful for debugging.
     */
    void printBatteryStatus();

    // Low power mode (modem off, minimal wake sources)
    /**
     * Check if device is in low power mode due to low battery.
     */
    bool isLowPowerMode();
    
    /**
     * Enter low power mode - should be called after sending final message.
     * Sets flag that survives deep sleep.
     */
    void enterLowPowerMode();
    
    /**
     * Exit low power mode - called when external power is restored.
     */
    void exitLowPowerMode();
    
    /**
     * Check which pin caused the EXT1 wakeup.
     * @return GPIO number that triggered wake, or -1 if not EXT1 wake
     */
    int getWakeupPin();
    
    /**
     * Check and clear PMU IRQ status after wakeup.
     * Should be called early in setup if woken by PMU IRQ.
     * @return true if there are pending PMU IRQs to handle
     */
    bool checkPmuWakeupCause();

private:
    XPowersPMU* pmu = nullptr;
    ActivityCallback activityCallback = nullptr;
    LowBatteryCallback lowBatteryCallback = nullptr;
    bool initialized = false;
    
    // Periodic logging
    static constexpr uint32_t BATTERY_LOG_INTERVAL = 30000; // 30 seconds
    uint32_t lastBatteryLogTime = 0;
    
    // PMU IRQ handling
    static volatile bool pmuIrqTriggered;
    static void IRAM_ATTR pmuIrqHandler();
    void handlePmuIrq();
};
