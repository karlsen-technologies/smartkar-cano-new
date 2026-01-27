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
    IgnitionState previousState = drive.ignition;
    
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
    
    // Log state changes
    if (drive.ignition != previousState) {
        const char* stateStr;
        switch (drive.ignition) {
            case IgnitionState::OFF: stateStr = "OFF"; break;
            case IgnitionState::ACCESSORY: stateStr = "ACCESSORY"; break;
            case IgnitionState::ON: stateStr = "ON"; break;
            case IgnitionState::START: stateStr = "START"; break;
            default: stateStr = "UNKNOWN"; break;
        }
        Serial.printf("[DriveDomain] Ignition: %s\r\n", stateStr);
    }
}

void DriveDomain::processSpeed(const uint8_t* data) {
    // ESP_21 (0x0FD) - Vehicle speed
    float speed = BroadcastDecoder::decodeSpeed(data);
    
    DriveState& drive = vehicleState.drive;
    bool wasMoving = drive.isMoving();
    
    drive.speedKmh = speed;
    drive.speedUpdate = millis();
    
    // Log when vehicle starts/stops moving
    bool nowMoving = drive.isMoving();
    if (nowMoving != wasMoving) {
        if (nowMoving) {
            Serial.printf("[DriveDomain] Vehicle moving: %.1f km/h\r\n", speed);
        } else {
            Serial.println("[DriveDomain] Vehicle stopped");
        }
    }
}

void DriveDomain::processDiagnose(const uint8_t* data) {
    // Diagnose_01 (0x6B2) - Odometer and vehicle time
    BroadcastDecoder::DiagnoseData decoded = BroadcastDecoder::decodeDiagnose(data);
    
    DriveState& drive = vehicleState.drive;
    uint32_t previousOdometer = drive.odometerKm;
    
    drive.odometerKm = decoded.odometerKm;
    drive.odometerUpdate = millis();
    
    drive.year = decoded.year;
    drive.month = decoded.month;
    drive.day = decoded.day;
    drive.hour = decoded.hour;
    drive.minute = decoded.minute;
    drive.second = decoded.second;
    drive.timeUpdate = millis();
    
    // Log odometer occasionally (on change or every 60 seconds)
    static unsigned long lastLog = 0;
    if (drive.odometerKm != previousOdometer || millis() - lastLog > 60000) {
        Serial.printf("[DriveDomain] Odometer: %lu km, Time: %04u-%02u-%02u %02u:%02u:%02u\r\n",
            drive.odometerKm, drive.year, drive.month, drive.day,
            drive.hour, drive.minute, drive.second);
        lastLog = millis();
    }
}
