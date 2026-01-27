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
    
    // Check for significant range change (> 5 km)
    bool rangeChanged = (abs((int)decoded.totalRange - (int)range.totalRangeKm) > 5);
    
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
    
    // Log on significant change
    if (rangeChanged && decoded.totalRange < INVALID_RANGE) {
        Serial.printf("[RangeDomain] Range: %d km (electric: %d km, consumption: %.1f)\r\n",
            decoded.totalRange, decoded.electricRange, decoded.consumption);
    }
}

void RangeDomain::processReichweite02(const uint8_t* data) {
    // Reichweite_02 (0x5F7) - Range display data
    BroadcastDecoder::Reichweite02Data decoded = BroadcastDecoder::decodeReichweite02(data);
    
    RangeState& range = vehicleState.range;
    
    // Track state changes
    bool reserveChanged = (decoded.reserveWarning != range.reserveWarning);
    RangeTendency newTendency = static_cast<RangeTendency>(decoded.tendency);
    bool tendencyChanged = (newTendency != range.tendency);
    
    // Update display values (skip invalid)
    if (decoded.displayTotalRange < INVALID_RANGE) {
        range.displayRangeKm = decoded.displayTotalRange;
    }
    if (decoded.displayElectricRange < INVALID_RANGE) {
        range.displayElectricRangeKm = decoded.displayElectricRange;
    }
    
    range.tendency = newTendency;
    range.reserveWarning = decoded.reserveWarning;
    range.displayInMiles = decoded.displayInMiles;
    range.reichweite02Update = millis();
    
    // Log state changes
    if (reserveChanged) {
        Serial.printf("[RangeDomain] Reserve warning: %s\r\n",
            decoded.reserveWarning ? "ACTIVE" : "off");
    }
    if (tendencyChanged) {
        Serial.printf("[RangeDomain] Range tendency: %s\r\n",
            range.tendencyStr());
    }
}
