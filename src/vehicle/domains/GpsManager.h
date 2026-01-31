#pragma once

#include <Arduino.h>
#include "../IDomain.h"
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * GpsManager - Unified GPS domain manager
 * 
 * Encapsulates ALL GPS data sources from infotainment:
 * - Standard CAN frames (11-bit): 0x486 (NavPos_01), 0x485 (NavData_02), 0x484 (NavData_01)
 * 
 * Implements new domain-based architecture:
 * - Owns complete GPS state (position, altitude, satellites, DOP)
 * - Provides clean public API
 * - Fast CAN frame processing (<1-2ms)
 * - Read-only domain (no commands)
 * 
 * Data Source: CAN only (infotainment GPS routed via gateway)
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex
 * - loop() called from main loop (Core 1)
 * - State access is thread-safe via VehicleManager mutex
 */
class GpsManager : public IDomain {
public:
    /**
     * GPS state structure (mirrors CanGpsState for consistency)
     */
    struct State {
        // === Position (NavPos_01 0x486) ===
        double latitude = 0.0;
        double longitude = 0.0;
        uint8_t satellites = 0;
        uint8_t fixType = 0;  // 0=none, 1=2D, 2=3D, 3=DGPS
        unsigned long positionUpdate = 0;
        
        // === Altitude (NavData_02 0x485) ===
        float altitude = 0.0f;
        uint32_t utcTime = 0;
        uint8_t satsInUse = 0;
        uint8_t satsInView = 0;
        uint8_t accuracy = 0;
        unsigned long altitudeUpdate = 0;
        
        // === Heading & DOP (NavData_01 0x484) ===
        float heading = 0.0f;
        float hdop = 0.0f;
        float vdop = 0.0f;
        float pdop = 0.0f;
        bool gpsInit = false;
        unsigned long headingUpdate = 0;
        
        // === Helper Methods ===
        
        bool hasFix() const { return fixType >= 2; }
        
        bool isValid() const {
            return hasFix() && (millis() - positionUpdate) < 30000;
        }
        
        const char* fixTypeStr() const {
            switch (fixType) {
                case 0: return "None";
                case 1: return "2D";
                case 2: return "3D";
                case 3: return "DGPS";
                default: return "Unknown";
            }
        }
    };

    explicit GpsManager(VehicleManager* vehicleManager);

    // IDomain interface
    const char* getName() const override { return "GpsManager"; }
    bool setup() override;
    void loop() override;
    void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) override;
    void onWakeComplete() override;
    bool isBusy() const override;

    // Public API
    const State& getState() const { return state; }
    double getLatitude() const { return state.latitude; }
    double getLongitude() const { return state.longitude; }
    float getAltitude() const { return state.altitude; }
    float getHeading() const { return state.heading; }
    uint8_t getSatellites() const { return state.satellites; }
    bool hasFix() const { return state.hasFix(); }
    bool isValid() const { return state.isValid(); }
    
    // Statistics
    void getFrameCounts(uint32_t& pos, uint32_t& data02, uint32_t& data01) const {
        pos = navPos01Count;
        data02 = navData02Count;
        data01 = navData01Count;
    }

private:
    VehicleManager* vehicleManager;
    State state;
    
    static constexpr uint32_t CAN_ID_NAV_POS_01 = 0x486;
    static constexpr uint32_t CAN_ID_NAV_DATA_02 = 0x485;
    static constexpr uint32_t CAN_ID_NAV_DATA_01 = 0x484;
    
    volatile uint32_t navPos01Count = 0;
    volatile uint32_t navData02Count = 0;
    volatile uint32_t navData01Count = 0;
    
    void processNavPos01(const uint8_t* data);
    void processNavData02(const uint8_t* data);
    void processNavData01(const uint8_t* data);
};
