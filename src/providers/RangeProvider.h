#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * RangeProvider - Reports range estimation telemetry
 * 
 * Telemetry includes:
 * - Total range
 * - Electric range
 * - Display range
 * - Consumption
 * - Tendency
 * - Reserve warning
 * 
 * Publishes to: smartkar/{ccid}/state/range
 */
class RangeProvider : public ITelemetryProvider {
public:
    explicit RangeProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "range"; }
    void getTelemetry(JsonObject& data) override;
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
};
