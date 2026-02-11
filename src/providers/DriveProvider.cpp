#include "DriveProvider.h"
#include "../vehicle/VehicleManager.h"

DriveProvider::DriveProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void DriveProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const DriveManager::State& state = vehicleManager->drive()->getState();
    
    data["ignition"] = static_cast<uint8_t>(state.ignition);
    data["keyInserted"] = state.keyInserted;
    data["ignitionOn"] = state.ignitionOn;
    data["speedKmh"] = state.speedKmh;
    data["odometerKm"] = state.odometerKm;
}

TelemetryPriority DriveProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    const DriveManager::State& state = vehicleManager->drive()->getState();
    
    // High priority for ignition changes
    if (state.ignitionOn != lastIgnitionOn) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool DriveProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    const DriveManager::State& state = vehicleManager->drive()->getState();
    
    // Ignition changed
    if (state.ignitionOn != lastIgnitionOn) return true;
    
    // Speed changed significantly
    if (abs(state.speedKmh - lastSpeedKmh) >= SPEED_CHANGE_THRESHOLD) return true;
    
    return false;
}

void DriveProvider::onTelemetrySent() {
    initialReport = false;
    if (!vehicleManager) return;
    
    const DriveManager::State& state = vehicleManager->drive()->getState();
    lastIgnitionOn = state.ignitionOn;
    lastSpeedKmh = state.speedKmh;
}
