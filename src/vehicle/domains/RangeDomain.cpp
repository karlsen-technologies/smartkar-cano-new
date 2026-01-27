#include "RangeDomain.h"
#include "../VehicleManager.h"

RangeDomain::RangeDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool RangeDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_REICHWEITE_01 ||
           canId == CAN_ID_REICHWEITE_02;
}

bool RangeDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        return false;
    }
    
    switch (canId) {
        case CAN_ID_REICHWEITE_01:
            processReichweite01(data);
            reichweite01Frames++;
            return true;
            
        case CAN_ID_REICHWEITE_02:
            processReichweite02(data);
            reichweite02Frames++;
            return true;
            
        default:
            return false;
    }
}

void RangeDomain::processReichweite01(const uint8_t* data) {
    // Reichweite_01 (0x5F5) - Range data
    BroadcastDecoder::Reichweite01Data decoded = BroadcastDecoder::decodeReichweite01(data);
    
    RangeState& range = vehicleState.range;
    
    // Skip invalid values (2045-2047)
    if (decoded.totalRange < INVALID_RANGE) {
        range.totalRangeKm = decoded.totalRange;
    }
    if (decoded.electricRange < INVALID_RANGE) {
        range.electricRangeKm = decoded.electricRange;
    }
    if (decoded.maxDisplayRange < INVALID_RANGE) {
        range.maxDisplayRangeKm = decoded.maxDisplayRange;
    }
    
    range.consumption = decoded.consumption;
    range.consumptionUnit = decoded.consumptionUnit;
    range.reichweite01Update = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void RangeDomain::processReichweite02(const uint8_t* data) {
    // Reichweite_02 (0x5F7) - Range display data
    BroadcastDecoder::Reichweite02Data decoded = BroadcastDecoder::decodeReichweite02(data);
    
    RangeState& range = vehicleState.range;
    
    // Update display values (skip invalid)
    if (decoded.displayTotalRange < INVALID_RANGE) {
        range.displayRangeKm = decoded.displayTotalRange;
    }
    if (decoded.displayElectricRange < INVALID_RANGE) {
        range.displayElectricRangeKm = decoded.displayElectricRange;
    }
    
    range.tendency = static_cast<RangeTendency>(decoded.tendency);
    range.reserveWarning = decoded.reserveWarning;
    range.displayInMiles = decoded.displayInMiles;
    range.reichweite02Update = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}
