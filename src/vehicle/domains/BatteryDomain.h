#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * BatteryDomain - Handles HV battery data available on comfort CAN
 * 
 * Listens to:
 *   0x5CA (BMS_07)  - Charging status, balancing, energy content
 *   0x59E (BMS_06)  - Battery temperature
 * 
 * NOTE: Core battery signals (0x191, 0x509, 0x2AE) are NOT on comfort CAN!
 * They are on the powertrain CAN bus. For SOC%, use BAP protocol instead.
 * 
 * No commands - this is a read-only domain.
 */
class BatteryDomain {
public:
    // CAN IDs this domain handles (comfort CAN only)
    static constexpr uint32_t CAN_ID_BMS_07 = 0x5CA;    // Charging status, energy
    static constexpr uint32_t CAN_ID_BMS_06 = 0x59E;    // Battery temperature
    
    /**
     * Construct the battery domain.
     * @param state Reference to shared vehicle state
     * @param manager Pointer to vehicle manager (not used, no commands)
     */
    BatteryDomain(VehicleState& state, VehicleManager* manager = nullptr);
    
    /**
     * Check if this domain handles a CAN ID.
     */
    bool handlesCanId(uint32_t canId) const;
    
    /**
     * Process a CAN frame. Called by VehicleManager when a relevant frame arrives.
     * @param canId The CAN ID
     * @param data The frame data (8 bytes)
     * @param dlc Data length code
     * @return true if frame was processed
     */
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    // =========================================================================
    // Status getters (convenience methods)
    // =========================================================================
    
    // From BMS_07 (0x5CA)
    bool isCharging() const { return vehicleState.battery.chargingActive; }
    bool isBalancing() const { return vehicleState.battery.balancingActive; }
    float energyWh() const { return vehicleState.battery.energyWh; }
    float maxEnergyWh() const { return vehicleState.battery.maxEnergyWh; }
    
    // From BMS_06 (0x59E)
    float temperature() const { return vehicleState.battery.temperature; }
    
    /**
     * Calculate energy percentage (current/max).
     */
    float energyPercent() const {
        if (vehicleState.battery.maxEnergyWh > 0) {
            return (vehicleState.battery.energyWh / vehicleState.battery.maxEnergyWh) * 100.0f;
        }
        return 0.0f;
    }
    
    // Frame counters for debugging
    uint32_t bms07Count = 0;
    uint32_t bms06Count = 0;
    
public:
    // Debug: get frame counts
    void getFrameCounts(uint32_t& bms07, uint32_t& bms06) const {
        bms07 = bms07Count; bms06 = bms06Count;
    }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;  // Not used currently, but kept for consistency
    
    // Process specific message types
    void processBMS07(const uint8_t* data);
    void processBMS06(const uint8_t* data);
};
