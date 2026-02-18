#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * ClimateProvider - Reports climate state telemetry
 * 
 * Telemetry includes:
 * - Inside and outside temperature
 * - Climate active status
 * - Heating/cooling/ventilation
 * - Auto defrost
 * - Remaining time
 * 
 * Publishes to: smartkar/{ccid}/state/climate
 */
class ClimateProvider : public ITelemetryProvider {
public:
    explicit ClimateProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "climate"; }
    void getTelemetry(JsonObject& data) override;
    unsigned long getMaxInterval() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    bool lastClimateActive = false;
    float lastInsideTemp = 0.0f;
    unsigned long lastSendTime = 0;
};
