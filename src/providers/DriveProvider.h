#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * DriveProvider - Reports drive state telemetry
 * 
 * Telemetry includes:
 * - Ignition state
 * - Key inserted status
 * - Speed
 * - Odometer
 * 
 * Publishes to: smartkar/{ccid}/state/drive
 */
class DriveProvider : public ITelemetryProvider {
public:
    explicit DriveProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "drive"; }
    void getTelemetry(JsonObject& data) override;
    unsigned long getMaxInterval() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    unsigned long lastSendTime = 0;
    bool lastIgnitionOn = false;
    float lastSpeedKmh = 0.0f;
    
    // Thresholds
    static constexpr float SPEED_CHANGE_THRESHOLD = 5.0f;    // 5 km/h speed change
};
