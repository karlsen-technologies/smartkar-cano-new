#pragma once

#include <Arduino.h>
#include "../VehicleState.h"
#include "../protocols/BroadcastDecoder.h"

// Forward declaration
class VehicleManager;

/**
 * GpsDomain - Handles GPS data from infotainment (routed via gateway)
 * 
 * Listens to:
 *   0x486 (NavPos_01) - Lat/Long position, satellites, fix type
 *   0x485 (NavData_02) - Altitude, UTC timestamp, satellite counts
 *   0x484 (NavData_01) - Heading, DOP values
 * 
 * GPS data is broadcast by the infotainment system and routed to the 
 * comfort CAN bus via the gateway. This provides an alternative to
 * modem-based GPS with potentially better accuracy in some situations.
 */
class GpsDomain {
public:
    // CAN IDs this domain handles
    static constexpr uint32_t CAN_ID_NAV_POS_01 = 0x486;     // Position
    static constexpr uint32_t CAN_ID_NAV_DATA_02 = 0x485;    // Altitude, UTC
    static constexpr uint32_t CAN_ID_NAV_DATA_01 = 0x484;    // Heading, DOP
    
    /**
     * Construct the GPS domain.
     * @param state Reference to shared vehicle state
     * @param manager Pointer to vehicle manager (for future use)
     */
    GpsDomain(VehicleState& state, VehicleManager* manager = nullptr);
    
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
    
    double latitude() const { return vehicleState.gps.latitude; }
    double longitude() const { return vehicleState.gps.longitude; }
    float altitude() const { return vehicleState.gps.altitude; }
    float heading() const { return vehicleState.gps.heading; }
    uint8_t satellites() const { return vehicleState.gps.satellites; }
    bool hasFix() const { return vehicleState.gps.hasFix(); }
    bool isValid() const { return vehicleState.gps.isValid(); }
    
    // Frame counters for diagnostics
    void getFrameCounts(uint32_t& pos, uint32_t& data02, uint32_t& data01) const {
        pos = navPos01Frames;
        data02 = navData02Frames;
        data01 = navData01Frames;
    }
    
private:
    VehicleState& vehicleState;
    VehicleManager* vehicleManager;
    
    // Frame counters
    uint32_t navPos01Frames = 0;
    uint32_t navData02Frames = 0;
    uint32_t navData01Frames = 0;
    
    // Process specific message types
    void processNavPos01(const uint8_t* data);
    void processNavData02(const uint8_t* data);
    void processNavData01(const uint8_t* data);
};
