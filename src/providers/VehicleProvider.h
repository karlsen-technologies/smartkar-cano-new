#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"

// Forward declarations
class VehicleManager;
class CommandRouter;

/**
 * VehicleProvider - Reports vehicle state telemetry
 * 
 * Telemetry includes:
 * - Battery: SOC, voltage, current, power, energy, temperature, charging state
 * - Drive: ignition state, speed, odometer
 * - Body: lock state, doors, trunk
 * - Range: estimated range, consumption
 * - GPS: position from CAN (if available)
 * - Climate: inside/outside temp, climate active
 * - BAP: plug state, charge state from BAP protocol
 * 
 * Sends on:
 * - Significant state changes (SOC, ignition, charging, etc.)
 * - Long interval (30 seconds when awake)
 * 
 * Also emits events for significant state changes:
 * - vehicle.ignitionOn / vehicle.ignitionOff
 * - vehicle.chargingStarted / vehicle.chargingStopped
 * - vehicle.plugged / vehicle.unplugged
 * - vehicle.locked / vehicle.unlocked
 */
class VehicleProvider : public ITelemetryProvider {
public:
    explicit VehicleProvider(VehicleManager* vehicleManager);
    
    /**
     * Set the CommandRouter for event emission.
     * Must be called after construction (since CommandRouter is created first).
     */
    void setCommandRouter(CommandRouter* router) { commandRouter = router; }
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "vehicle"; }
    void getTelemetry(JsonObject& data) override;
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
    /**
     * Mark data as changed, forcing a send on next collection.
     */
    void markChanged() { changed = true; }
    
    /**
     * Check for state changes and emit events.
     * Called periodically from main loop.
     */
    void checkAndEmitEvents();

private:
    VehicleManager* vehicleManager = nullptr;
    CommandRouter* commandRouter = nullptr;
    
    // Change tracking
    bool initialReport = true;
    bool changed = false;
    unsigned long lastReportTime = 0;
    unsigned long lastEventCheckTime = 0;
    
    // Last reported values for change detection
    float lastSoc = 0.0f;
    float lastPowerKw = 0.0f;
    bool lastIgnitionOn = false;
    bool lastCharging = false;
    bool lastLocked = false;
    bool lastPlugged = false;
    float lastSpeedKmh = 0.0f;
    
    // Event tracking (separate from telemetry change detection)
    bool eventIgnitionOn = false;
    bool eventCharging = false;
    bool eventLocked = false;
    bool eventPlugged = false;
    bool eventsInitialized = false;
    
    // Thresholds for change detection
    static constexpr float SOC_CHANGE_THRESHOLD = 1.0f;      // 1% SOC change
    static constexpr float POWER_CHANGE_THRESHOLD = 0.5f;    // 0.5 kW power change
    static constexpr float SPEED_CHANGE_THRESHOLD = 5.0f;    // 5 km/h speed change
    
    // Reporting intervals
    static constexpr unsigned long REPORT_INTERVAL_AWAKE = 30 * 1000;  // 30s when vehicle awake
    static constexpr unsigned long REPORT_INTERVAL_ASLEEP = 5 * 60 * 1000;  // 5m when vehicle asleep
    static constexpr unsigned long EVENT_CHECK_INTERVAL = 1000;  // Check for events every 1s
    
    // Event emission helpers
    void emitEvent(const char* eventName, JsonObject* details = nullptr);
};
