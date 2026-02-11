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

TelemetryPriority RangeProvider::getPriority() {
    return TelemetryPriority::PRIORITY_LOW;  // Changes slowly
}

bool RangeProvider::hasChanged() {
    if (initialReport) return true;
    // Range changes slowly, always send on interval
    return false;
}

void RangeProvider::onTelemetrySent() {
    initialReport = false;
}
