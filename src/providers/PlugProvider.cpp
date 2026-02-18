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

bool PlugProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    // Check max interval
    if (millis() - lastSendTime >= getMaxInterval()) {
        return true;
    }
    
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    
    // Plug state changed
    if (battState.plugState.isPlugged() != lastPlugged) return true;
    
    // Supply state changed
    if (battState.plugState.hasSupply() != lastHasSupply) return true;
    
    return false;
}

void PlugProvider::onTelemetrySent() {
    initialReport = false;
    lastSendTime = millis();
    if (!vehicleManager) return;
    
    const BatteryManager::State& battState = vehicleManager->battery()->getState();
    lastPlugged = battState.plugState.isPlugged();
    lastHasSupply = battState.plugState.hasSupply();
}
