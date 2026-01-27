#include "DriveDomain.h"
#include "../VehicleManager.h"

DriveDomain::DriveDomain(VehicleState& state, VehicleManager* manager)
    : vehicleState(state)
    , vehicleManager(manager)
{
}

bool DriveDomain::handlesCanId(uint32_t canId) const {
    return canId == CAN_ID_IGNITION ||
           canId == CAN_ID_SPEED ||
           canId == CAN_ID_DIAGNOSE;
}

bool DriveDomain::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    switch (canId) {
        case CAN_ID_IGNITION:
            if (dlc >= 4) {  // 0x3C0 is only 4 bytes
                processIgnition(data);
                return true;
            }
            return false;
            
        case CAN_ID_SPEED:
            if (dlc >= 8) {
                processSpeed(data);
                return true;
            }
            return false;
            
        case CAN_ID_DIAGNOSE:
            if (dlc >= 8) {
                processDiagnose(data);
                return true;
            }
            return false;
            
        default:
            return false;
    }
}

void DriveDomain::processIgnition(const uint8_t* data) {
    // Klemmen_Status_01 (0x3C0) - Ignition/terminal status
    BroadcastDecoder::IgnitionData decoded = BroadcastDecoder::decodeIgnition(data);
    
    DriveState& drive = vehicleState.drive;
    
    drive.keyInserted = decoded.keyInserted;
    drive.ignitionOn = decoded.ignitionOn;
    drive.startRequested = decoded.startRequested;
    drive.ignitionUpdate = millis();
    
    // Determine overall ignition state
    if (decoded.startRequested) {
        drive.ignition = IgnitionState::START;
    } else if (decoded.ignitionOn) {
        drive.ignition = IgnitionState::ON;
    } else if (decoded.keyInserted) {
        drive.ignition = IgnitionState::ACCESSORY;
    } else {
        drive.ignition = IgnitionState::OFF;
    }
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void DriveDomain::processSpeed(const uint8_t* data) {
    // ESP_21 (0x0FD) - Vehicle speed
    float speed = BroadcastDecoder::decodeSpeed(data);
    
    DriveState& drive = vehicleState.drive;
    
    drive.speedKmh = speed;
    drive.speedUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}

void DriveDomain::processDiagnose(const uint8_t* data) {
    // Diagnose_01 (0x6B2) - Odometer and vehicle time
    BroadcastDecoder::DiagnoseData decoded = BroadcastDecoder::decodeDiagnose(data);
    
    DriveState& drive = vehicleState.drive;
    
    drive.odometerKm = decoded.odometerKm;
    drive.odometerUpdate = millis();
    
    drive.year = decoded.year;
    drive.month = decoded.month;
    drive.day = decoded.day;
    drive.hour = decoded.hour;
    drive.minute = decoded.minute;
    drive.second = decoded.second;
    drive.timeUpdate = millis();
    
    // NO SERIAL OUTPUT - This runs on CAN task (Core 0)
}
