#include "NetworkProvider.h"
#include "../modules/MqttManager.h"

NetworkProvider::NetworkProvider(ModemManager* modemManager, MqttManager* mqttManager)
    : modemManager(modemManager), mqttManager(mqttManager) {
}

void NetworkProvider::getTelemetry(JsonObject& data) {
    if (modemManager) {
        data["modemState"] = modemStateToString(modemManager->getState());
        data["signalStrength"] = modemManager->getSignalQuality();
        data["simCCID"] = modemManager->getSimCCID();
        data["modemConnected"] = modemManager->isConnected();
    }
    
    if (mqttManager) {
        data["mqttConnected"] = mqttManager->isConnected();
        
        // Map MQTT state to string
        const char* mqttStateStr;
        switch (mqttManager->getState()) {
            case MqttState::DISCONNECTED:   mqttStateStr = "disconnected"; break;
            case MqttState::CONFIGURING:    mqttStateStr = "configuring"; break;
            case MqttState::CONNECTING:     mqttStateStr = "connecting"; break;
            case MqttState::SUBSCRIBING:    mqttStateStr = "subscribing"; break;
            case MqttState::CONNECTED:      mqttStateStr = "connected"; break;
            case MqttState::MQTT_ERROR:     mqttStateStr = "error"; break;
            default:                        mqttStateStr = "unknown"; break;
        }
        data["mqttState"] = mqttStateStr;
    }
}

TelemetryPriority NetworkProvider::getPriority() {
    // Higher priority when state changes
    if (modemManager) {
        ModemState currentState = modemManager->getState();
        if (currentState != lastModemState) {
            return TelemetryPriority::PRIORITY_HIGH;
        }
    }
    
    if (mqttManager) {
        bool currentConnected = mqttManager->isConnected();
        if (currentConnected != lastMqttConnected) {
            return TelemetryPriority::PRIORITY_HIGH;
        }
    }
    
    return TelemetryPriority::PRIORITY_NORMAL;
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
    
    // Check for MQTT state change
    if (mqttManager) {
        bool currentConnected = mqttManager->isConnected();
        if (currentConnected != lastMqttConnected) {
            return true;
        }
    }
    
    return false;
}

void NetworkProvider::onTelemetrySent() {
    initialReport = false;
    changed = false;
    
    // Update last reported values
    if (modemManager) {
        lastModemState = modemManager->getState();
        lastSignalStrength = modemManager->getSignalQuality();
    }
    
    if (mqttManager) {
        lastMqttConnected = mqttManager->isConnected();
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
