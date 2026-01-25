#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class PowerManager;

/**
 * DeviceProvider - Reports device status telemetry
 * 
 * Telemetry includes:
 * - Uptime (millis)
 * - Free heap memory
 * - Wake cause (after deep sleep)
 * - Battery voltage, percent, charging status
 * 
 * Sends on:
 * - Device wake (initial report)
 * - Long interval (5 minutes)
 * - Battery percent change (>5%)
 * - Charging state change
 */
class DeviceProvider : public ITelemetryProvider {
public:
    DeviceProvider(PowerManager* powerManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "device"; }
    void getTelemetry(JsonObject& data) override;
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
    /**
     * Mark data as changed, forcing a send on next collection.
     * Call this when something significant happens.
     */
    void markChanged() { changed = true; }
    
    /**
     * Set the wake cause string (called by DeviceController on boot).
     */
    void setWakeCause(const char* cause) { wakeCause = cause; }

private:
    PowerManager* powerManager = nullptr;
    
    // Change tracking
    bool initialReport = true;      // First report after boot
    bool changed = false;           // Explicit change flag
    unsigned long lastReportTime = 0;
    
    // Last reported values for change detection
    uint8_t lastBatteryPercent = 0;
    bool lastChargingState = false;
    
    // Wake cause (set once on boot)
    const char* wakeCause = "unknown";
    
    // Reporting interval (5 minutes)
    static const unsigned long REPORT_INTERVAL = 5 * 60 * 1000;
    
    // Change threshold for battery percent
    static const uint8_t BATTERY_CHANGE_THRESHOLD = 5;
};
