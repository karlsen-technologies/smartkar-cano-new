#include "ClimateManager.h"
#include "../VehicleManager.h"
#include "../bap/channels/BatteryControlChannel.h"

ClimateManager::ClimateManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , bapChannel(nullptr) {
}

bool ClimateManager::setup() {
    Serial.println("[ClimateManager] Initializing...");
    
    if (!vehicleManager) {
        Serial.println("[ClimateManager] ERROR: No VehicleManager!");
        return false;
    }
    
    // Get reference to BatteryControlChannel (SHARED with BatteryManager)
    bapChannel = &vehicleManager->batteryControl();
    
    // Register callback for BAP updates
    Serial.println("[ClimateManager] Registering BAP callback (SHARED channel)...");
    
    // Climate state callback (function 0x12)
    bapChannel->onClimateState([this](const ClimateState& climate) {
        this->onClimateStateUpdate(climate);
    });
    
    Serial.println("[ClimateManager] Initialized:");
    Serial.println("[ClimateManager]   - CAN IDs: 0x66E (Klima_03), 0x5E1 (Klima_Sensor_02)");
    Serial.println("[ClimateManager]   - BAP callbacks: ClimateState (function 0x12)");
    Serial.println("[ClimateManager]   - Data source priority: BAP > CAN");
    Serial.println("[ClimateManager]   - SHARED channel: BatteryControlChannel");
    
    return true;
}

void ClimateManager::loop() {
    // No periodic tasks needed for now
    // All processing happens in CAN frame callbacks and BAP callbacks
}

void ClimateManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // Called from CAN task (Core 0) with mutex held
    // MUST be fast (<1-2ms)
    
    if (dlc < 8) {
        return;  // Need full frame
    }
    
    switch (canId) {
        case CAN_ID_KLIMA_03:
            processKlima03(data);
            break;
            
        case CAN_ID_KLIMA_SENSOR_02:
            processKlimaSensor02(data);
            break;
            
        default:
            // Not our frame
            break;
    }
}

void ClimateManager::onWakeComplete() {
    // Optional: Request initial BAP state after wake
    // For now, BAP channel will send updates automatically
    Serial.println("[ClimateManager] Vehicle awake, waiting for BAP updates");
}

bool ClimateManager::isBusy() const {
    // Delegate to BAP channel
    return bapChannel ? bapChannel->isBusy() : false;
}

// =============================================================================
// CAN Frame Processors
// =============================================================================

void ClimateManager::processKlima03(const uint8_t* data) {
    // Klima_03 (0x66E) - Inside temperature
    // Note: CAN climate active flags are unreliable - use BAP only for active status
    klima03Count++;
    
    BroadcastDecoder::KlimaData decoded = BroadcastDecoder::decodeKlima03(data);
    
    // Update inside temperature (CAN source - only if BAP hasn't updated recently)
    // BAP takes priority when climate is actively controlled
    if (state.insideTempSource != DataSource::BAP || 
        (millis() - state.insideTempUpdate) > 5000) {
        state.insideTemp = decoded.insideTemp;
        state.insideTempSource = DataSource::CAN_STD;
        state.insideTempUpdate = millis();
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void ClimateManager::processKlimaSensor02(const uint8_t* data) {
    // Klima_Sensor_02 (0x5E1) - Outside temperature
    // BCM1_Aussen_Temp_ungef: Byte 0, scale 0.5, offset -50
    klimaSensor02Count++;
    
    uint8_t rawTemp = data[0];
    float outsideTemp = rawTemp * 0.5f - 50.0f;
    
    state.outsideTemp = outsideTemp;
    state.outsideTempUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// BAP Callback Handler
// =============================================================================

void ClimateManager::onClimateStateUpdate(const ClimateState& climate) {
    // Called from CAN thread via BatteryControlChannel callback
    // Keep FAST - just copy data
    climateCallbackCount++;
    
    // BAP climate state is authoritative
    state.climateActive = climate.climateActive;
    state.climateActiveSource = DataSource::BAP;
    state.heating = climate.heating;
    state.cooling = climate.cooling;
    state.ventilation = climate.ventilation;
    state.autoDefrost = climate.autoDefrost;
    state.climateTimeMin = climate.climateTimeMin;
    state.climateActiveUpdate = millis();
    
    // Also update inside temp if provided by BAP (more accurate during active climate)
    if (climate.insideTemp > 0.0f) {
        state.insideTemp = climate.insideTemp;
        state.insideTempSource = DataSource::BAP;
        state.insideTempUpdate = millis();
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// Command Interface
// =============================================================================

bool ClimateManager::requestState() {
    if (!bapChannel) {
        Serial.println("[ClimateManager] ERROR: No BAP channel!");
        return false;
    }
    
    Serial.println("[ClimateManager] Requesting climate state from vehicle");
    
    return bapChannel->requestClimateState();
}

bool ClimateManager::startClimate(int commandId, float tempCelsius, bool allowBattery) {
    if (!bapChannel) {
        Serial.println("[ClimateManager] ERROR: No BAP channel!");
        return false;
    }
    
    Serial.printf("[ClimateManager] Starting climate: temp=%.1f°C, allowBattery=%s\r\n", 
                  tempCelsius, allowBattery ? "YES" : "no");
    
    return bapChannel->startClimate(commandId, tempCelsius, allowBattery);
}

bool ClimateManager::stopClimate(int commandId) {
    if (!bapChannel) {
        Serial.println("[ClimateManager] ERROR: No BAP channel!");
        return false;
    }
    
    Serial.println("[ClimateManager] Stopping climate");
    
    return bapChannel->stopClimate(commandId);
}
