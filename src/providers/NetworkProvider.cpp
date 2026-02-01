#include "NetworkProvider.h"
#include "../modules/LinkManager.h"

NetworkProvider::NetworkProvider(ModemManager* modemManager, LinkManager* linkManager)
    : modemManager(modemManager), linkManager(linkManager) {
}

void NetworkProvider::getTelemetry(JsonObject& data) {
    if (modemManager) {
        data["modemState"] = modemStateToString(modemManager->getState());
        data["signalStrength"] = modemManager->getSignalQuality();
        data["simCCID"] = modemManager->getSimCCID();
        data["modemConnected"] = modemManager->isConnected();
    }
    
    if (linkManager) {
        data["linkConnected"] = linkManager->isConnected();
        
        // Map link state to string
        const char* linkStateStr;
        switch (linkManager->getState()) {
            case LinkState::DISCONNECTED:   linkStateStr = "disconnected"; break;
            case LinkState::CONNECTING:     linkStateStr = "connecting"; break;
            case LinkState::AUTHENTICATING: linkStateStr = "authenticating"; break;
            case LinkState::CONNECTED:      linkStateStr = "connected"; break;
            case LinkState::REJECTED:       linkStateStr = "rejected"; break;
            case LinkState::LINK_ERROR:     linkStateStr = "error"; break;
            default:                        linkStateStr = "unknown"; break;
        }
        data["linkState"] = linkStateStr;
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
    
    if (linkManager) {
        bool currentConnected = linkManager->isConnected();
        if (currentConnected != lastLinkConnected) {
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
    
    // Check for link state change
    if (linkManager) {
        bool currentConnected = linkManager->isConnected();
        if (currentConnected != lastLinkConnected) {
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
    
    if (linkManager) {
        lastLinkConnected = linkManager->isConnected();
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
