#include "GpsProvider.h"
#include "../vehicle/VehicleManager.h"

GpsProvider::GpsProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void GpsProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const GpsManager::State& state = vehicleManager->gps()->getState();
    
    // Only send if valid
    if (!state.isValid()) return;
    
    data["lat"] = state.latitude;
    data["lng"] = state.longitude;
    data["alt"] = state.altitude;
    data["heading"] = state.heading;
    data["satellites"] = state.satsInUse;
    data["fixType"] = state.fixTypeStr();
    data["hdop"] = state.hdop;
}

TelemetryPriority GpsProvider::getPriority() {
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool GpsProvider::hasChanged() {
    if (initialReport) return true;
    // GPS changes when driving, let interval handle it
    return false;
}

void GpsProvider::onTelemetrySent() {
    initialReport = false;
}
