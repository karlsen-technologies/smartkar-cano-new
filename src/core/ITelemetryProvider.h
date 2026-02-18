#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>



/**
 * ITelemetryProvider - Interface for modules that report telemetry data
 * 
 * Modules that have data to report to the server implement this interface.
 * The DeviceController or a TelemetryManager queries providers periodically
 * and aggregates their data into telemetry messages.
 * 
 * Example implementation:
 * 
 *   class ChargingModule : public IModule, public ITelemetryProvider {
 *   public:
 *       const char* getTelemetryDomain() override { return "charging"; }
 *       
 *       void getTelemetry(JsonObject& data) override {
 *           data["state"] = getChargeStateString();
 *           data["batteryPercent"] = getBatteryPercent();
 *           data["batteryKwh"] = getBatteryKwh();
 *           data["chargeLimit"] = getChargeLimit();
 *           data["isPluggedIn"] = isPluggedIn();
 *       }
 *       
 *       bool hasChanged() override {
 *           return batteryPercent != lastReportedPercent || 
 *                  chargeState != lastReportedState;
 *       }
 *       
 *       TelemetryPriority getPriority() override {
 *           return isCharging() ? TelemetryPriority::HIGH : TelemetryPriority::NORMAL;
 *       }
 *   };
 * 
 * Telemetry is aggregated into messages like:
 *   {
 *     "type": "telemetry",
 *     "data": {
 *       "charging": { ... },
 *       "climate": { ... },
 *       "vehicle": { ... }
 *     }
 *   }
 */
class ITelemetryProvider {
public:
    virtual ~ITelemetryProvider() = default;
    
    /**
     * Get the domain name for this telemetry.
     * Used as the key in the telemetry message.
     * 
     * @return Domain string (e.g., "charging", "climate", "vehicle", "security")
     */
    virtual const char* getTelemetryDomain() = 0;
    
    /**
     * Populate telemetry data.
     * Called when telemetry is being collected for transmission.
     * 
     * @param data JsonObject to populate with telemetry values
     */
    virtual void getTelemetry(JsonObject& data) = 0;
    
    /**
     * Get the maximum interval between sends (milliseconds).
     * Provider should send at least this often even if hasChanged() is false.
     * Can return different values based on active state (e.g., faster when charging).
     * Default: 5 minutes (300000ms)
     * 
     * @return Maximum milliseconds between telemetry sends
     */
    virtual unsigned long getMaxInterval() {
        return 300000;  // 5 minutes default
    }
    
    /**
     * Check if telemetry has changed since last report.
     * Used to avoid sending redundant data.
     * 
     * @return true if data has changed and should be sent
     */
    virtual bool hasChanged() {
        return true;  // Default: always report
    }
    
    /**
     * Called after telemetry has been sent.
     * Use this to update "last reported" state for change detection.
     */
    virtual void onTelemetrySent() {
        // Default: do nothing
    }
};

/**
 * EventCallback - Callback for sending events immediately
 * 
 * Modules can use this to send events that happen between telemetry intervals.
 * Example: door opened, charging stopped, fault detected
 * 
 * @param domain Event domain (e.g., "security", "charging")
 * @param event Event name (e.g., "doorOpened", "chargeStopped")
 * @param details Optional JsonObject with event details
 */
typedef void (*EventCallback)(const char* domain, const char* event, JsonObject* details);
