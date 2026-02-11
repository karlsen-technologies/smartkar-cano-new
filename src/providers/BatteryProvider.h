#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * BatteryProvider - Reports battery state telemetry
 * 
 * Telemetry includes:
 * - SOC (State of Charge) with source tracking
 * - Charging status and mode
 * - Power, energy, temperature
 * - Balancing status
 * 
 * Publishes to: smartkar/{ccid}/state/battery
 */
class BatteryProvider : public ITelemetryProvider {
public:
    explicit BatteryProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "battery"; }
    void getTelemetry(JsonObject& data) override;
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    float lastSoc = 0.0f;
    float lastPowerKw = 0.0f;
    bool lastCharging = false;
    
    // Thresholds
    static constexpr float SOC_CHANGE_THRESHOLD = 1.0f;      // 1% SOC change
    static constexpr float POWER_CHANGE_THRESHOLD = 0.5f;    // 0.5 kW power change
};
