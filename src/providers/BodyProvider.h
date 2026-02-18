#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;

/**
 * BodyProvider - Reports body state telemetry
 * 
 * Telemetry includes:
 * - Lock state
 * - Central lock status
 * - Trunk state
 * - Individual door states
 * 
 * Publishes to: smartkar/{ccid}/state/body
 */
class BodyProvider : public ITelemetryProvider {
public:
    explicit BodyProvider(VehicleManager* vehicleManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "body"; }
    void getTelemetry(JsonObject& data) override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
private:
    VehicleManager* vehicleManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    unsigned long lastSendTime = 0;
    bool lastLocked = false;
    bool lastTrunkOpen = false;
    bool lastAnyDoorOpen = false;
};
