#pragma once

#include <Arduino.h>

// Define AXP2101 chip before including XPowersLib
#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

#include "../core/IModule.h"

/**
 * PowerManager - AXP2101 PMU control module
 * 
 * Responsibilities:
 * - Initialize and configure the AXP2101 PMU
 * - Control modem power rail (DC3)
 * - Configure deep sleep wakeup sources
 * - Monitor battery status
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

private:
    XPowersPMU* pmu = nullptr;
    ActivityCallback activityCallback = nullptr;
    bool initialized = false;
};
