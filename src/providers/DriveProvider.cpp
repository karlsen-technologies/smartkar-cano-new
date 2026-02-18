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

unsigned long DriveProvider::getMaxInterval() {
    if (vehicleManager && vehicleManager->drive()->getState().ignitionOn) {
        return 10000;  // 10 sec when driving
    }
    return 300000;  // 5 min default
}

bool DriveProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    // Check if max interval exceeded
    if (millis() - lastSendTime >= getMaxInterval()) {
        return true;
    }
    
    const DriveManager::State& state = vehicleManager->drive()->getState();
    
    // Ignition changed
    if (state.ignitionOn != lastIgnitionOn) return true;
    
    // Speed changed significantly
    if (abs(state.speedKmh - lastSpeedKmh) >= SPEED_CHANGE_THRESHOLD) return true;
    
    return false;
}

void DriveProvider::onTelemetrySent() {
    initialReport = false;
    lastSendTime = millis();
    if (!vehicleManager) return;
    
    const DriveManager::State& state = vehicleManager->drive()->getState();
    lastIgnitionOn = state.ignitionOn;
    lastSpeedKmh = state.speedKmh;
}
