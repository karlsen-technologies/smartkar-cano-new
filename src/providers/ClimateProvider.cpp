#include "ClimateProvider.h"
#include "../vehicle/VehicleManager.h"

ClimateProvider::ClimateProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void ClimateProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const ClimateManager::State& state = vehicleManager->climate()->getState();
    
    // Temperature (with source tracking)
    data["insideTemp"] = state.insideTemp;
    data["insideTempSource"] = state.insideTempSource == DataSource::BAP ? "bap" : "can";
    data["outsideTemp"] = state.outsideTemp;
    
    // Climate control (BAP-only with source tracking)
    data["active"] = state.climateActive;
    if (state.climateActiveSource == DataSource::BAP) {
        data["activeSource"] = "bap";  // Only BAP provides climate active status
    }
    data["heating"] = state.heating;       // BAP only
    data["cooling"] = state.cooling;       // BAP only
    data["ventilation"] = state.ventilation;  // BAP only
    data["autoDefrost"] = state.autoDefrost;  // BAP only
    data["remainingMin"] = state.climateTimeMin;  // BAP only
}

TelemetryPriority ClimateProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    const ClimateManager::State& state = vehicleManager->climate()->getState();
    
    // High priority for climate state changes
    if (state.climateActive != lastClimateActive) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool ClimateProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    const ClimateManager::State& state = vehicleManager->climate()->getState();
    
    // Climate active state changed
    if (state.climateActive != lastClimateActive) return true;
    
    return false;
}

void ClimateProvider::onTelemetrySent() {
    initialReport = false;
    if (!vehicleManager) return;
    
    const ClimateManager::State& state = vehicleManager->climate()->getState();
    lastClimateActive = state.climateActive;
}
