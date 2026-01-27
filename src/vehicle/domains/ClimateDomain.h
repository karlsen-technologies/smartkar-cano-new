#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * ClimateDomain - Handles climate and temperature data
 * 
 * Listens to:
 *   0x66E (Klima_03) - Inside temperature, standby heating status
 *   0x5E1 (Klima_Sensor_02) - Outside temperature
 * 
 * Note: Climate commands (start/stop heating) require BAP protocol,
 * which is not yet implemented. This domain is read-only for now.
 */
class ClimateDomain {
public:
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_KLIMA_03 = 0x66E;         // Climate status, inside temp
    static constexpr uint32_t CAN_ID_KLIMA_SENSOR_02 = 0x5E1;  // Outside temp
    
    /**
     * Construct the climate domain.
     * @param state Reference to shared vehicle state
     * @param manager Pointer to vehicle manager (for future commands)
     */
    ClimateDomain(VehicleState& state, VehicleManager* manager = nullptr);
    
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
    
    float insideTemp() const { return vehicleState.climate.insideTemp; }
    float outsideTemp() const { return vehicleState.climate.outsideTemp; }
    bool isStandbyHeatingActive() const { return vehicleState.climate.standbyHeatingActive; }
    bool isStandbyVentActive() const { return vehicleState.climate.standbyVentActive; }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;
    
    // Process specific message types
    void processKlima03(const uint8_t* data);
    void processKlimaSensor02(const uint8_t* data);
};
