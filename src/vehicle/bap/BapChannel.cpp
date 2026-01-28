#include "BapChannel.h"
#include "../VehicleManager.h"

bool BapChannel::sendBapFrame(VehicleManager* mgr, const uint8_t* data, uint8_t len) {
    if (!mgr) {
        return false;
    }
    
    // Send as extended frame to this channel's TX CAN ID
    return mgr->sendCanFrame(getTxCanId(), data, len, true);
}
