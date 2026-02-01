#pragma once

#include <Arduino.h>
#include "../core/ITelemetryProvider.h"
#include "../modules/ModemManager.h"

// Forward declarations
class LinkManager;

/**
 * NetworkProvider - Reports network status telemetry
 * 
 * Telemetry includes:
 * - Modem state (off, searching, connected, etc.)
 * - Network registration status
 * - Signal strength (RSSI in dBm)
 * - Link state (disconnected, connected, etc.)
 * - SIM CCID
 * 
 * Sends on:
 * - Device wake (initial report)
 * - State changes (modem state, link state)
 * - Significant signal changes (>10 dBm)
 * 
 * Note: Interval-based sending is controlled by LinkManager, not this provider.
 */
class NetworkProvider : public ITelemetryProvider {
public:
    NetworkProvider(ModemManager* modemManager, LinkManager* linkManager);
    
    // ITelemetryProvider interface
    const char* getTelemetryDomain() override { return "network"; }
    void getTelemetry(JsonObject& data) override;
    TelemetryPriority getPriority() override;
    bool hasChanged() override;
    void onTelemetrySent() override;
    
    /**
     * Mark data as changed, forcing a send on next collection.
     */
    void markChanged() { changed = true; }

private:
    ModemManager* modemManager = nullptr;
    LinkManager* linkManager = nullptr;
    
    // Change tracking
    bool initialReport = true;
    bool changed = false;
    
    // Last reported values for change detection
    ModemState lastModemState = ModemState::OFF;
    int16_t lastSignalStrength = 0;
    bool lastLinkConnected = false;
    
    // Change threshold for signal strength (dBm)
    static const int16_t SIGNAL_CHANGE_THRESHOLD = 10;
    
    // Helper to convert ModemState to string
    static const char* modemStateToString(ModemState state);
};
