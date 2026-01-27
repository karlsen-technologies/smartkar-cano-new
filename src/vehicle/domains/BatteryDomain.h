#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * BatteryDomain - Handles HV battery and DCDC converter data
 * 
 * Listens to:
 *   0x191 (BMS_01)  - Core battery data: voltage, current, hi-res SOC
 *   0x509 (BMS_10)  - Usable SOC, energy content
 *   0x5CA (BMS_07)  - Charging status, balancing
 *   0x59E (BMS_06)  - Battery temperature
 *   0x2AE (DCDC_01) - DC-DC converter: 12V voltage, current
 * 
 * No commands - this is a read-only domain.
 */
class BatteryDomain {
public:
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_BMS_01 = 0x191;    // Core battery data
    static constexpr uint32_t CAN_ID_BMS_10 = 0x509;    // Usable SOC/energy
    static constexpr uint32_t CAN_ID_BMS_07 = 0x5CA;    // Charging status
    static constexpr uint32_t CAN_ID_BMS_06 = 0x59E;    // Battery temperature
    static constexpr uint32_t CAN_ID_DCDC_01 = 0x2AE;   // DC-DC converter
    
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
    
    // From BMS_01
    float voltage() const { return vehicleState.battery.voltage; }
    float current() const { return vehicleState.battery.current; }
    float socHiRes() const { return vehicleState.battery.socHiRes; }
    
    // From BMS_10
    float usableSoc() const { return vehicleState.battery.usableSoc; }
    float energyWh() const { return vehicleState.battery.energyWh; }
    float maxEnergyWh() const { return vehicleState.battery.maxEnergyWh; }
    
    // From BMS_07
    bool isCharging() const { return vehicleState.battery.chargingActive; }
    bool isBalancing() const { return vehicleState.battery.balancingActive; }
    
    // From BMS_06
    float temperature() const { return vehicleState.battery.temperature; }
    
    // From DCDC_01
    float dcdc12v() const { return vehicleState.battery.dcdc12v; }
    float dcdcCurrent() const { return vehicleState.battery.dcdcCurrent; }
    
    // Computed values
    float powerKw() const { return vehicleState.battery.power() / 1000.0f; }
    
    /**
     * Calculate energy percentage (current/max).
     */
    float energyPercent() const {
        if (vehicleState.battery.maxEnergyWh > 0) {
            return (vehicleState.battery.energyWh / vehicleState.battery.maxEnergyWh) * 100.0f;
        }
        return 0.0f;
    }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;  // Not used currently, but kept for consistency
    
    // Process specific message types
    void processBMS01(const uint8_t* data);
    void processBMS10(const uint8_t* data);
    void processBMS07(const uint8_t* data);
    void processBMS06(const uint8_t* data);
    void processDCDC01(const uint8_t* data);
};
