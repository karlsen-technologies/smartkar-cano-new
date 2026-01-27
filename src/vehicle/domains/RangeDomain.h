#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * RangeDomain - Handles range estimation data from instrument cluster
 * 
 * Listens to:
 *   0x5F5 (Reichweite_01) - Total/electric range, consumption
 *   0x5F7 (Reichweite_02) - Display range, tendency, reserve warning
 * 
 * Provides the car's calculated range estimate directly, eliminating
 * the need to calculate range from SOC and efficiency.
 * 
 * Note: Values 2045-2047 (0x7FD-0x7FF) indicate invalid/initializing data.
 */
class RangeDomain {
public:
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_REICHWEITE_01 = 0x5F5;  // Range data
    static constexpr uint32_t CAN_ID_REICHWEITE_02 = 0x5F7;  // Range display
    
    // Invalid value marker
    static constexpr uint16_t INVALID_RANGE = 2045;
    
    /**
     * Construct the range domain.
     * @param state Reference to shared vehicle state
     * @param manager Pointer to vehicle manager (for future use)
     */
    RangeDomain(VehicleState& state, VehicleManager* manager = nullptr);
    
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
    
    uint16_t totalRange() const { return vehicleState.range.totalRangeKm; }
    uint16_t electricRange() const { return vehicleState.range.electricRangeKm; }
    uint16_t displayRange() const { return vehicleState.range.displayRangeKm; }
    bool isReserveWarning() const { return vehicleState.range.reserveWarning; }
    RangeTendency tendency() const { return vehicleState.range.tendency; }
    bool isValid() const { return vehicleState.range.isValid(); }
    
    // Frame counters for diagnostics
    void getFrameCounts(uint32_t& rw01, uint32_t& rw02) const {
        rw01 = reichweite01Frames;
        rw02 = reichweite02Frames;
    }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;
    
    // Frame counters
    uint32_t reichweite01Frames = 0;
    uint32_t reichweite02Frames = 0;
    
    // Process specific message types
    void processReichweite01(const uint8_t* data);
    void processReichweite02(const uint8_t* data);
};
