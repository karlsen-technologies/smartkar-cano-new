#include "NetworkProvider.h"
#include "../modules/MqttManager.h"

NetworkProvider::NetworkProvider(ModemManager* modemManager, MqttManager* mqttManager)
    : modemManager(modemManager), mqttManager(mqttManager) {
}

void NetworkProvider::getTelemetry(JsonObject& data) {
    if (modemManager) {
        data["modemState"] = modemStateToString(modemManager->getState());
        data["signalStrength"] = modemManager->getSignalQuality();
    }
}

bool NetworkProvider::hasChanged() {
    // Always send initial report
    if (initialReport) {
        return true;
    }
    
    // Check explicit change flag
    if (changed) {
        return true;
    }
    
    // Check if max interval exceeded
    if (millis() - lastSendTime >= getMaxInterval()) {
        return true;
    }
    
    // Check for modem state change
    if (modemManager) {
        ModemState currentState = modemManager->getState();
        if (currentState != lastModemState) {
            return true;
        }
        
        // Check for significant signal change (ModemManager caches this value)
        int16_t currentSignal = modemManager->getSignalQuality();
        if (abs(currentSignal - lastSignalStrength) >= SIGNAL_CHANGE_THRESHOLD) {
            return true;
        }
    }
    
    return false;
}

void NetworkProvider::onTelemetrySent() {
    initialReport = false;
    changed = false;
    lastSendTime = millis();
    
    // Update last reported values
    if (modemManager) {
        lastModemState = modemManager->getState();
        lastSignalStrength = modemManager->getSignalQuality();
    }
}

const char* NetworkProvider::modemStateToString(ModemState state) {
    switch (state) {
        case ModemState::OFF:           return "off";
        case ModemState::STARTING:      return "starting";
        case ModemState::HOTSTART:      return "hotstart";
        case ModemState::CONFIGURING:   return "configuring";
        case ModemState::NO_SIM:        return "no_sim";
        case ModemState::SEARCHING:     return "searching";
        case ModemState::REGISTERED:    return "registered";
        case ModemState::UNREGISTERED:  return "unregistered";
        case ModemState::DENIED:        return "denied";
        case ModemState::CONNECTING:    return "connecting";
        case ModemState::CONNECTED:     return "connected";
        case ModemState::MODEM_ERROR:   return "error";
        default:                        return "unknown";
    }
}
