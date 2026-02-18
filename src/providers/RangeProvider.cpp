#include "RangeProvider.h"
#include "../vehicle/VehicleManager.h"

RangeProvider::RangeProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void RangeProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const RangeManager::State& state = vehicleManager->range()->getState();
    
    // Only send if valid
    if (!state.isValid()) return;
    
    data["totalKm"] = state.totalRangeKm;
    data["electricKm"] = state.electricRangeKm;
    data["displayKm"] = state.displayRangeKm;
    data["consumption"] = state.consumptionKwh100km;
    data["tendency"] = state.tendencyStr();
    data["reserveWarning"] = state.reserveWarning;
}

unsigned long RangeProvider::getMaxInterval() {
    if (vehicleManager && vehicleManager->drive()->getState().ignitionOn) {
        return 30000;  // 30 seconds when driving
    }
    return 300000;  // 5 minutes default
}

bool RangeProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    // Check max interval
    if (millis() - lastSendTime >= getMaxInterval()) {
        return true;
    }
    
    const RangeManager::State& state = vehicleManager->range()->getState();
    
    // Only check if valid
    if (!state.isValid()) return false;
    
    // Range changed significantly (≥5km)
    if (abs((int)state.totalRangeKm - (int)lastRangeKm) >= 5) return true;
    
    return false;
}

void RangeProvider::onTelemetrySent() {
    initialReport = false;
    lastSendTime = millis();
    if (!vehicleManager) return;
    
    const RangeManager::State& state = vehicleManager->range()->getState();
    if (state.isValid()) {
        lastRangeKm = state.totalRangeKm;
    }
}
