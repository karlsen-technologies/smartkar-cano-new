#include "BatteryProvider.h"
#include "../vehicle/VehicleManager.h"

BatteryProvider::BatteryProvider(VehicleManager* vehicleManager)
    : vehicleManager(vehicleManager) {
}

void BatteryProvider::getTelemetry(JsonObject& data) {
    if (!vehicleManager) return;
    
    const BatteryManager::State& state = vehicleManager->battery()->getState();
    
    // SOC (BAP primary, includes source tracking)
    if (state.socSource != DataSource::NONE && state.soc > 0.0f) {
        data["soc"] = state.soc;
        data["socSource"] = state.socSource == DataSource::BAP ? "bap" : "can";
    }
    
    // Charging status (unified from CAN + BAP)
    data["charging"] = state.charging;
    data["chargingSource"] = state.chargingSource == DataSource::BAP ? "bap" : 
                             state.chargingSource == DataSource::CAN_STD ? "can" : "none";
    
    // Charging details (from BAP when available)
    if (state.chargingUpdate > 0) {
        data["chargingMode"] = static_cast<uint8_t>(state.chargingMode);
        data["chargingStatus"] = static_cast<uint8_t>(state.chargingStatus);
        data["chargingAmps"] = state.chargingAmps;
        data["targetSoc"] = state.targetSoc;
        data["remainingMin"] = state.remainingTimeMin;
    }
    
    // Power and energy (CAN)
    data["powerKw"] = state.powerKw;
    data["energyWh"] = state.energyWh;
    data["maxEnergyWh"] = state.maxEnergyWh;
    data["temperature"] = state.temperature;
    data["balancing"] = state.balancingActive;
}

TelemetryPriority BatteryProvider::getPriority() {
    if (!vehicleManager) return TelemetryPriority::PRIORITY_LOW;
    
    const BatteryManager::State& state = vehicleManager->battery()->getState();
    
    // High priority for charging state changes
    if (state.charging != lastCharging) {
        return TelemetryPriority::PRIORITY_HIGH;
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
}

bool BatteryProvider::hasChanged() {
    if (initialReport) return true;
    if (!vehicleManager) return false;
    
    const BatteryManager::State& state = vehicleManager->battery()->getState();
    
    // Charging state changed
    if (state.charging != lastCharging) return true;
    
    // SOC changed significantly
    if (abs(state.soc - lastSoc) >= SOC_CHANGE_THRESHOLD) return true;
    
    // Power changed significantly (useful during charging)
    if (abs(state.powerKw - lastPowerKw) >= POWER_CHANGE_THRESHOLD) return true;
    
    return false;
}

void BatteryProvider::onTelemetrySent() {
    initialReport = false;
    if (!vehicleManager) return;
    
    const BatteryManager::State& state = vehicleManager->battery()->getState();
    lastSoc = state.soc;
    lastPowerKw = state.powerKw;
    lastCharging = state.charging;
}
