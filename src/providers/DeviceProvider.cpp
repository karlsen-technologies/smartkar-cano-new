#include "DeviceProvider.h"
#include "../modules/PowerManager.h"

DeviceProvider::DeviceProvider(PowerManager* powerManager)
    : powerManager(powerManager) {
}

void DeviceProvider::getTelemetry(JsonObject& data) {
    // Device info
    data["uptime"] = millis();
    data["freeHeap"] = ESP.getFreeHeap();
    data["wakeCause"] = wakeCause;
    
    // Battery info (if PowerManager available)
    if (powerManager) {
        data["batteryVoltage"] = powerManager->getBatteryVoltage();
        data["batteryPercent"] = powerManager->getBatteryPercent();
        data["charging"] = powerManager->isCharging();
        data["vbusConnected"] = powerManager->isVbusConnected();
    }
    
    // ESP32 chip info
    data["chipModel"] = ESP.getChipModel();
    data["chipRevision"] = ESP.getChipRevision();
    data["cpuFreqMHz"] = ESP.getCpuFreqMHz();
}

TelemetryPriority DeviceProvider::getPriority() {
    // Higher priority if charging state just changed
    if (powerManager) {
        bool currentCharging = powerManager->isCharging();
        if (currentCharging != lastChargingState) {
            return TelemetryPriority::PRIORITY_HIGH;
        }
    }
    return TelemetryPriority::PRIORITY_LOW;
}

bool DeviceProvider::hasChanged() {
    // Always send initial report after boot
    if (initialReport) {
        return true;
    }
    
    // Check explicit change flag
    if (changed) {
        return true;
    }
    
    // Check if interval has elapsed
    if (millis() - lastReportTime >= REPORT_INTERVAL) {
        return true;
    }
    
    // Check for battery/charging changes
    if (powerManager) {
        uint8_t currentPercent = powerManager->getBatteryPercent();
        bool currentCharging = powerManager->isCharging();
        
        // Charging state changed
        if (currentCharging != lastChargingState) {
            return true;
        }
        
        // Battery percent changed significantly
        int diff = abs((int)currentPercent - (int)lastBatteryPercent);
        if (diff >= BATTERY_CHANGE_THRESHOLD) {
            return true;
        }
    }
    
    return false;
}

void DeviceProvider::onTelemetrySent() {
    initialReport = false;
    changed = false;
    lastReportTime = millis();
    
    // Update last reported values
    if (powerManager) {
        lastBatteryPercent = powerManager->getBatteryPercent();
        lastChargingState = powerManager->isCharging();
    }
}
