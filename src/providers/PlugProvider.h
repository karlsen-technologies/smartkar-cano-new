#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * PlugProvider - Reports plug/connector state telemetry
 * 
 * Telemetry includes:
 * - Plugged status
 * - Has supply (AC power available)
 * - Plug state
 * - Lock state
 * 
 * Publishes to: smartkar/{ccid}/state/plug
 */
class PlugProvider : public ITelemetryProvider {
public:
    explicit PlugProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "plug"; }
    void getTelemetry(JsonObject& data) override;
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    bool lastPlugged = false;
};
