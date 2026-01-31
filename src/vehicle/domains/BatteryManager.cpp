#include "BatteryManager.h"
#include "../VehicleManager.h"
#include "../bap/channels/BatteryControlChannel.h"

// =============================================================================
// RTC Memory Storage - Survives Deep Sleep
// =============================================================================

// Store battery state in RTC memory so it persists across deep sleep
RTC_DATA_ATTR BatteryManager::State rtcBatteryState = {};

// =============================================================================
// Constructor
// =============================================================================

BatteryManager::BatteryManager(VehicleManager* mgr)
    : vehicleManager(mgr)
    , bapChannel(nullptr)
    , state(rtcBatteryState) {  // Initialize reference to RTC memory
}

bool BatteryManager::setup() {
    Serial.println("[BatteryManager] Initializing...");
    
    if (!vehicleManager) {
        Serial.println("[BatteryManager] ERROR: No VehicleManager!");
        return false;
    }
    
    // Get reference to BatteryControlChannel
    bapChannel = &vehicleManager->batteryControl();
    
    // Register callbacks for BAP updates
    Serial.println("[BatteryManager] Registering BAP callbacks...");
    
    // Plug state callback (function 0x10)
    bapChannel->onPlugState([this](const PlugState& plug) {
        this->onPlugStateUpdate(plug);
    });
    
    // Charge state callback (function 0x11)
    bapChannel->onChargeState([this](const BatteryState& battery) {
        this->onChargeStateUpdate(battery);
    });
    
    Serial.println("[BatteryManager] Initialized:");
    Serial.println("[BatteryManager]   - CAN IDs: 0x5CA (BMS_07), 0x59E (BMS_06), 0x483 (Motor_Hybrid_06)");
    Serial.println("[BatteryManager]   - BAP callbacks: PlugState, ChargeState");
    Serial.println("[BatteryManager]   - Data source priority: BAP > CAN > Computed");
    
    return true;
}

void BatteryManager::loop() {
    // No periodic tasks needed for now
    // All processing happens in CAN frame callbacks and BAP callbacks
}

void BatteryManager::processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // Called from CAN task (Core 0) with mutex held
    // MUST be fast (<1-2ms)
    
    if (dlc < 8) {
        return;  // Need full frame
    }
    
    switch (canId) {
        case CAN_ID_BMS_07:
            processBMS07(data);
            break;
            
        case CAN_ID_BMS_06:
            processBMS06(data);
            break;
            
        case CAN_ID_MOTOR_HYBRID_06:
            processMotorHybrid06(data);
            break;
            
        default:
            // Not our frame
            break;
    }
}

void BatteryManager::onWakeComplete() {
    // Optional: Request initial BAP state after wake
    // For now, BAP channel will send updates automatically
    Serial.println("[BatteryManager] Vehicle awake, waiting for BAP updates");
}

bool BatteryManager::isBusy() const {
    // Delegate to BAP channel
    return bapChannel ? bapChannel->isBusy() : false;
}

// =============================================================================
// CAN Frame Processors
// =============================================================================

void BatteryManager::processBMS07(const uint8_t* data) {
    // BMS_07 (0x5CA) - Charging status and energy content
    bms07Count++;
    
    BroadcastDecoder::BMS07Data decoded = BroadcastDecoder::decodeBMS07(data);
    
    // Update CAN-sourced fields
    state.energyWh = decoded.energyWh;
    state.maxEnergyWh = decoded.maxEnergyWh;
    state.chargingActive = decoded.chargingActive;
    state.balancingActive = decoded.balancingActive;
    state.energyUpdate = millis();
    state.balancingUpdate = millis();
    
    // Update unified charging field (CAN source - BAP will override if available)
    if (state.chargingSource != DataSource::BAP) {
        state.charging = decoded.chargingActive;
        state.chargingSource = DataSource::CAN_STD;
        state.chargingUpdate = millis();
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryManager::processBMS06(const uint8_t* data) {
    // BMS_06 (0x59E) - Battery temperature
    bms06Count++;
    
    float temp = BroadcastDecoder::decodeBMS06Temperature(data);
    
    state.temperature = temp;
    state.tempUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryManager::processMotorHybrid06(const uint8_t* data) {
    // Motor_Hybrid_06 (0x483) - Power meter for charging/climate
    motorHybrid06Count++;
    
    BroadcastDecoder::MotorHybrid06Data decoded = BroadcastDecoder::decodeMotorHybrid06(data);
    
    state.powerKw = decoded.powerKw;
    state.powerUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// BAP Callback Handlers
// =============================================================================

void BatteryManager::onPlugStateUpdate(const PlugState& plug) {
    // Called from CAN thread via BatteryControlChannel callback
    // Keep FAST - just copy data
    plugCallbackCount++;
    
    state.plugState = plug;
    state.plugStateSource = DataSource::BAP;
    state.plugStateUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BatteryManager::onChargeStateUpdate(const BatteryState& battery) {
    // Called from CAN thread via BatteryControlChannel callback
    // Keep FAST - just copy data
    chargeCallbackCount++;
    
    // BAP SOC takes priority over CAN
    state.soc = battery.soc;
    state.socSource = DataSource::BAP;
    state.socUpdate = millis();
    
    // BAP charging info is more detailed than CAN
    state.charging = battery.charging;
    state.chargingSource = DataSource::BAP;
    state.chargingMode = battery.chargingMode;
    state.chargingStatus = battery.chargingStatus;
    state.chargingAmps = battery.chargingAmps;
    state.targetSoc = battery.targetSoc;
    state.remainingTimeMin = battery.remainingTimeMin;
    state.chargingUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

// =============================================================================
// Command Interface
// =============================================================================

bool BatteryManager::startCharging(uint8_t commandId, uint8_t targetSoc, uint8_t maxCurrent) {
    if (!bapChannel) {
        Serial.println("[BatteryManager] ERROR: No BAP channel!");
        return false;
    }
    
    Serial.printf("[BatteryManager] Starting charging: target=%d%%, maxCurrent=%dA\r\n", 
                  targetSoc, maxCurrent);
    
    return bapChannel->startCharging(commandId, targetSoc, maxCurrent);
}

bool BatteryManager::stopCharging(uint8_t commandId) {
    if (!bapChannel) {
        Serial.println("[BatteryManager] ERROR: No BAP channel!");
        return false;
    }
    
    Serial.println("[BatteryManager] Stopping charging");
    
    return bapChannel->stopCharging(commandId);
}
