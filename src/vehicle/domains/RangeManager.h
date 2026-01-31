#pragma once

#include <Arduino.h>
#include "../IDomain.h"
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * RangeManager - Unified range domain manager
 * 
 * Encapsulates ALL range estimation data from instrument cluster:
 * - Standard CAN frames (11-bit): 0x5F5 (Reichweite_01), 0x5F7 (Reichweite_02)
 * 
 * Implements new domain-based architecture:
 * - Owns complete range state
 * - Provides clean public API
 * - Fast CAN frame processing (<1-2ms)
 * - Read-only domain (no commands)
 * 
 * Data Source: CAN only (instrument cluster calculations)
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex
 * - loop() called from main loop (Core 1)
 * - State access is thread-safe via VehicleManager mutex
 */
class RangeManager : public IDomain {
public:
    /**
     * Range state structure (mirrors RangeState for consistency)
     */
    struct State {
        // === Range Data (Reichweite_01 0x5F5) ===
        uint16_t totalRangeKm = 0;
        uint16_t electricRangeKm = 0;
        uint16_t consumptionKwh100km = 0;
        unsigned long rangeUpdate = 0;
        
        // === Display Data (Reichweite_02 0x5F7) ===
        uint16_t displayRangeKm = 0;
        RangeTendency tendency = RangeTendency::STABLE;
        bool reserveWarning = false;
        unsigned long displayUpdate = 0;
        
        // === Helper Methods ===
        
        bool isValid() const {
            return totalRangeKm < INVALID_RANGE && totalRangeKm > 0;
        }
        
        const char* tendencyStr() const {
            switch (tendency) {
                case RangeTendency::STABLE: return "stable";
                case RangeTendency::INCREASING: return "increasing";
                case RangeTendency::DECREASING: return "decreasing";
                default: return "unknown";
            }
        }
        
        static constexpr uint16_t INVALID_RANGE = 2045;
    };

    explicit RangeManager(VehicleManager* vehicleManager);

    // IDomain interface
    const char* getName() const override { return "RangeManager"; }
    bool setup() override;
    void loop() override;
    void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    void onWakeComplete() override;
    bool isBusy() const override;

    // Public API
    const State& getState() const { return state; }
    uint16_t getTotalRange() const { return state.totalRangeKm; }
    uint16_t getElectricRange() const { return state.electricRangeKm; }
    uint16_t getDisplayRange() const { return state.displayRangeKm; }
    bool isReserveWarning() const { return state.reserveWarning; }
    RangeTendency getTendency() const { return state.tendency; }
    bool isValid() const { return state.isValid(); }
    
    // Statistics
    void getFrameCounts(uint32_t& rw01, uint32_t& rw02) const {
        rw01 = reichweite01Count;
        rw02 = reichweite02Count;
    }

private:
    VehicleManager* vehicleManager;
    State state;
    
    static constexpr uint32_t CAN_ID_REICHWEITE_01 = 0x5F5;
    static constexpr uint32_t CAN_ID_REICHWEITE_02 = 0x5F7;
    
    volatile uint32_t reichweite01Count = 0;
    volatile uint32_t reichweite02Count = 0;
    
    void processReichweite01(const uint8_t* data);
    void processReichweite02(const uint8_t* data);
};
