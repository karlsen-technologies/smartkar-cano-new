#include "BodyProvider.h"
#include "../vehicle/VehicleManager.h"

BodyProvider::BodyProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void BodyProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    
    data["locked"] = state.isLocked();
    data["trunkOpen"] = state.trunkOpen;
    data["anyDoorOpen"] = state.anyDoorOpen();
    
    // Individual doors
    JsonObject doors = data["doors"].to<JsonObject>();
    doors["driverOpen"] = state.driverDoor.open;
    doors["passengerOpen"] = state.passengerDoor.open;
    doors["rearLeftOpen"] = state.rearLeftDoor.open;
    doors["rearRightOpen"] = state.rearRightDoor.open;
}

bool BodyProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    // Check max interval
    if (millis() - lastSendTime >= getMaxInterval()) {
        return true;
    }
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    
    // Lock state changed
    if (state.isLocked() != lastLocked) return true;
    
    // Trunk state changed
    if (state.trunkOpen != lastTrunkOpen) return true;
    
    // Door state changed
    if (state.anyDoorOpen() != lastAnyDoorOpen) return true;
    
    return false;
}

void BodyProvider::onTelemetrySent() {
    initialReport = false;
    lastSendTime = millis();
    if (!vehicleManager) return;
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    lastLocked = state.isLocked();
    lastTrunkOpen = state.trunkOpen;
    lastAnyDoorOpen = state.anyDoorOpen();
}
