#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * GpsProvider - Reports GPS position from CAN
 * 
 * Telemetry includes:
 * - Latitude and longitude
 * - Altitude
 * - Heading
 * - Satellites in use
 * - Fix type
 * - HDOP
 * 
 * Publishes to: smartkar/{ccid}/state/gps
 */
class GpsProvider : public ITelemetryProvider {
public:
    explicit GpsProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "gps"; }
    void getTelemetry(JsonObject& data) override;
    unsigned long getMaxInterval() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    unsigned long lastSendTime = 0;
};
