#include "BodyProvider.h"
#include "../vehicle/VehicleManager.h"

BodyProvider::BodyProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void BodyProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    
    data["locked"] = state.isLocked();
    data["centralLock"] = static_cast<uint8_t>(state.centralLock);
    data["trunkOpen"] = state.trunkOpen;
    data["anyDoorOpen"] = state.anyDoorOpen();
    
    // Individual doors
    JsonObject doors = data["doors"].to<JsonObject>();
    doors["driverOpen"] = state.driverDoor.open;
    doors["passengerOpen"] = state.passengerDoor.open;
    doors["rearLeftOpen"] = state.rearLeftDoor.open;
    doors["rearRightOpen"] = state.rearRightDoor.open;
}

TelemetryPriority BodyProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    
    // High priority for lock state changes
    if (state.isLocked() != lastLocked) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool BodyProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    
    // Lock state changed
    if (state.isLocked() != lastLocked) return true;
    
    return false;
}

void BodyProvider::onTelemetrySent() {
    initialReport = false;
    if (!vehicleManager) return;
    
    const BodyManager::State& state = vehicleManager->body()->getState();
    lastLocked = state.isLocked();
}
