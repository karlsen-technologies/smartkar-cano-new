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

unsigned long GpsProvider::getMaxInterval() {
    if (vehicleManager && vehicleManager->drive()->getState().ignitionOn) {
        return 10000;  // 10 seconds when driving
    }
    return 300000;  // 5 minutes default
}

bool GpsProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    // Check max interval (time-based only, no position delta)
    if (millis() - lastSendTime >= getMaxInterval()) {
        return true;
    }
    
    return false;
}

void GpsProvider::onTelemetrySent() {
    initialReport = false;
    lastSendTime = millis();
}
