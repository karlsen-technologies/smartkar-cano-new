#include "BodyDomain.h"
#include "../VehicleManager.h"

BodyDomain::BodyDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool BodyDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_DRIVER_DOOR ||
           canId == CAN_ID_PASSENGER_DOOR ||
           canId == CAN_ID_LOCK_STATUS;
}

bool BodyDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    if (dlc < 8) {
        return false;  // Need full 8 bytes
    }
    
    switch (canId) {
        case CAN_ID_DRIVER_DOOR:
            processDriverDoor(data);
            return true;
            
        case CAN_ID_PASSENGER_DOOR:
            processPassengerDoor(data);
            return true;
            
        case CAN_ID_LOCK_STATUS:
            processLockStatus(data);
            return true;
            
        default:
            return false;
    }
}

void BodyDomain::processDriverDoor(const uint8_t* data) {
    auto decoded = BroadcastDecoder::decodeDriverDoor(data);
    
    DoorState& door = vehicleState.body.driverDoor;
    door.open = decoded.doorOpen;
    door.locked = decoded.doorLocked;
    door.windowPosition = decoded.windowPos;
    door.lastUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BodyDomain::processPassengerDoor(const uint8_t* data) {
    auto decoded = BroadcastDecoder::decodePassengerDoor(data);
    
    DoorState& door = vehicleState.body.passengerDoor;
    door.open = decoded.doorOpen;
    door.locked = decoded.doorLocked;
    door.windowPosition = decoded.windowPos;
    door.lastUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void BodyDomain::processLockStatus(const uint8_t* data) {
    auto decoded = BroadcastDecoder::decodeLockStatus(data);
    
    BodyState& body = vehicleState.body;
    
    // Store raw bytes for debugging
    body.zv02_byte2 = decoded.byte2;
    body.zv02_byte7 = decoded.byte7;
    
    // Update lock state
    LockState newState = decoded.isLocked ? LockState::LOCKED : LockState::UNLOCKED;
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
    
    body.centralLock = newState;
    body.centralLockUpdate = millis();
}

// ============================================================================
// Commands
// ============================================================================

bool BodyDomain::sendTm01Command(Tm01Commands::Command cmd) {
    if (!vehicleManager) {
        Serial.println("[BodyDomain] No vehicle manager - cannot send command");
        return false;
    }
    
    uint8_t data[8];
    Tm01Commands::buildCommand(cmd, data);
    
    Serial.printf("[BodyDomain] Sending %s command\r\n", Tm01Commands::getCommandName(cmd));
    
    return vehicleManager->sendCanFrame(Tm01Commands::getCanId(), data, 8, false);
}

bool BodyDomain::horn() {
    return sendTm01Command(Tm01Commands::Command::HORN);
}

bool BodyDomain::flash() {
    return sendTm01Command(Tm01Commands::Command::FLASH);
}

bool BodyDomain::lock() {
    return sendTm01Command(Tm01Commands::Command::LOCK);
}

bool BodyDomain::unlock() {
    return sendTm01Command(Tm01Commands::Command::UNLOCK);
}

bool BodyDomain::panic() {
    return sendTm01Command(Tm01Commands::Command::PANIC);
}
