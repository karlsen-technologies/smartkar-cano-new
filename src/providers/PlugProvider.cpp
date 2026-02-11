#include "PlugProvider.h"
#include "../vehicle/VehicleManager.h"

PlugProvider::PlugProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void PlugProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    
    // Only send if valid (BAP only)
    if (!battState.plugState.isValid()) return;
    
    data["plugged"] = battState.plugState.isPlugged();
    data["hasSupply"] = battState.plugState.hasSupply();
    data["state"] = battState.plugState.plugStateStr();
    data["lockState"] = battState.plugState.lockState;
}

TelemetryPriority PlugProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    
    // High priority for plug events
    if (battState.plugState.isPlugged() != lastPlugged) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool PlugProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    
    // Plug state changed
    if (battState.plugState.isPlugged() != lastPlugged) return true;
    
    return false;
}

void PlugProvider::onTelemetrySent() {
    initialReport = false;
    if (!vehicleManager) return;
    
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    lastPlugged = battState.plugState.isPlugged();
}
