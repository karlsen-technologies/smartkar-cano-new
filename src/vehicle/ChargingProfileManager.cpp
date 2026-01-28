#include "ChargingProfileManager.h"
#include "VehicleManager.h"

using namespace ChargingProfile;
using namespace BapProtocol;

// =============================================================================
// Battery Control Constants (Device 0x25)
// =============================================================================

namespace {
    constexpr uint8_t DEVICE_BATTERY_CONTROL = 0x25;
    constexpr uint32_t CAN_ID_BATTERY_TX = 0x17332501;  // Commands TO battery control
    
    namespace Function {
        constexpr uint8_t OPERATION_MODE = 0x18;
        constexpr uint8_t PROFILES_ARRAY = 0x19;
    }
}

// =============================================================================
// Constructor
// =============================================================================

ChargingProfileManager::ChargingProfileManager(VehicleManager* mgr)
    : manager(mgr)
{
    // Initialize all profiles to cleared state
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        profiles[i].clear();
    }
}

// =============================================================================
// Profile Access
// =============================================================================

const Profile& ChargingProfileManager::getProfile(uint8_t index) const {
    static Profile empty;
    if (index >= PROFILE_COUNT) {
        return empty;
    }
    return profiles[index];
}

Profile& ChargingProfileManager::getProfileMutable(uint8_t index) {
    static Profile empty;
    if (index >= PROFILE_COUNT) {
        return empty;
    }
    return profiles[index];
}

bool ChargingProfileManager::isProfileValid(uint8_t index) const {
    if (index >= PROFILE_COUNT) {
        return false;
    }
    return profiles[index].valid;
}

// =============================================================================
// High-Level Operations (Profile 0)
// =============================================================================

bool ChargingProfileManager::startClimateNow(float tempCelsius, bool allowBattery) {
    Serial.printf("[ProfileMgr] Start climate request: %.1f°C (battery=%s)\r\n", 
                  tempCelsius, allowBattery ? "yes" : "no");
    
    // Update profile 0's temperature setting locally
    profiles[PROFILE_IMMEDIATE].setTemperature(tempCelsius);
    
    // Delegate to BatteryControlChannel (non-blocking)
    return manager->batteryControl().startClimate(tempCelsius, allowBattery);
}

bool ChargingProfileManager::stopClimateNow() {
    Serial.println("[ProfileMgr] Stop climate request");
    
    // Delegate to BatteryControlChannel (non-blocking)
    return manager->batteryControl().stopClimate();
}

bool ChargingProfileManager::startChargingNow(uint8_t targetSoc, uint8_t maxCurrent) {
    Serial.printf("[ProfileMgr] Start charging request: target=%d%%, max=%dA\r\n", 
                  targetSoc, maxCurrent);
    
    // Delegate to BatteryControlChannel (non-blocking)
    return manager->batteryControl().startCharging(targetSoc, maxCurrent);
}

bool ChargingProfileManager::stopChargingNow() {
    Serial.println("[ProfileMgr] Stop charging request");
    
    // Delegate to BatteryControlChannel (non-blocking)
    return manager->batteryControl().stopCharging();
}

bool ChargingProfileManager::startChargingAndClimateNow(float tempCelsius, 
                                                         uint8_t targetSoc,
                                                         bool allowClimateOnBattery) {
    Serial.printf("[ProfileMgr] Start charging+climate request: temp=%.1f, soc=%d%%\r\n",
                  tempCelsius, targetSoc);
    
    // Update profile 0's temperature setting locally
    profiles[PROFILE_IMMEDIATE].setTemperature(tempCelsius);
    
    // Use the new combined command
    return manager->batteryControl().startChargingAndClimate(tempCelsius, targetSoc, 32, allowClimateOnBattery);
}

// =============================================================================
// Profile Management
// =============================================================================

bool ChargingProfileManager::updateTimerProfile(uint8_t profileIndex, 
                                                 const Profile& profile) {
    if (profileIndex < 1 || profileIndex > 3) {
        Serial.println("[ProfileMgr] Invalid profile index for timer (must be 1-3)");
        return false;
    }
    
    // Store the profile locally
    profiles[profileIndex] = profile;
    profiles[profileIndex].valid = true;
    profiles[profileIndex].lastUpdate = millis();
    
    // TODO: Send full profile update to car via BAP
    // This requires building a longer BAP message with full profile data
    Serial.printf("[ProfileMgr] TODO: Send full profile %d update\r\n", profileIndex);
    
    return true;
}

bool ChargingProfileManager::setTimerProfileEnabled(uint8_t profileIndex, bool enable) {
    if (profileIndex < 1 || profileIndex > 3) {
        Serial.println("[ProfileMgr] Invalid profile index for timer (must be 1-3)");
        return false;
    }
    
    // The timer enable is controlled via the OPERATION_MODE function
    // Each timer has its own bit in the payload
    uint8_t frame[8];
    
    // Payload byte 1: timer bitmask
    // Bit 0: Profile 0 (immediately)
    // Bit 1: Profile 1 (Timer 1)
    // Bit 2: Profile 2 (Timer 2)
    // Bit 3: Profile 3 (Timer 3)
    uint8_t timerBit = enable ? static_cast<uint8_t>(1 << profileIndex) : 0x00;
    uint8_t payload[2] = { 0x00, timerBit };
    
    encodeShortMessage(frame, OpCode::SET_GET, DEVICE_BATTERY_CONTROL,
                       Function::OPERATION_MODE, payload, 2);
    
    Serial.printf("[ProfileMgr] %s timer profile %d\r\n", 
                  enable ? "Enabling" : "Disabling", profileIndex);
    
    return sendFrame(frame, 8);
}

bool ChargingProfileManager::requestAllProfiles() {
    uint8_t frame[8];
    buildProfilesRequest(frame);
    
    Serial.println("[ProfileMgr] Requesting all profiles...");
    return sendFrame(frame, 8);
}

// =============================================================================
// BAP Message Processing
// =============================================================================

void ChargingProfileManager::processProfilesArray(const uint8_t* payload, uint8_t len) {
    if (len < 4) {
        Serial.println("[ProfileMgr] ProfilesArray payload too short");
        return;
    }
    
    // Parse BAP array header
    // Byte 0: [ASG-ID:4][Transaction-ID:4]
    // Byte 1: [LargeIdx:1][PosTransmit:1][Backward:1][Shift:1][RecordAddr:4]
    // Byte 2: startIndex
    // Byte 3: elementCount
    
    uint8_t headerByte1 = payload[1];
    uint8_t recordAddr = headerByte1 & 0x0F;
    bool posTransmit = (headerByte1 & ArrayHeader::POS_TRANSMIT) != 0;
    uint8_t startIndex = payload[2];
    uint8_t elementCount = payload[3];
    
    Serial.printf("[ProfileMgr] ProfilesArray: recordAddr=%d, start=%d, count=%d, posTransmit=%d\r\n",
                  recordAddr, startIndex, elementCount, posTransmit);
    
    // Skip header bytes
    const uint8_t* profileData = payload + 4;
    uint8_t remainingLen = len - 4;
    
    // Parse based on record address format
    if (recordAddr == ArrayHeader::RECORD_ADDR_COMPACT) {
        // Compact format (4 bytes per profile)
        for (uint8_t i = 0; i < elementCount && remainingLen >= 4; i++) {
            uint8_t profileIndex = posTransmit ? profileData[0] : (startIndex + i);
            const uint8_t* data = posTransmit ? profileData + 1 : profileData;
            
            if (profileIndex < PROFILE_COUNT) {
                parseCompactProfile(profileIndex, data, 4);
            }
            
            uint8_t consumed = posTransmit ? 5 : 4;
            profileData += consumed;
            remainingLen -= consumed;
        }
    } else if (recordAddr == ArrayHeader::RECORD_ADDR_FULL) {
        // Full format (20+ bytes per profile)
        for (uint8_t i = 0; i < elementCount && remainingLen >= 19; i++) {
            uint8_t profileIndex = posTransmit ? profileData[0] : (startIndex + i);
            const uint8_t* data = posTransmit ? profileData + 1 : profileData;
            
            if (profileIndex < PROFILE_COUNT) {
                parseFullProfile(profileIndex, data, remainingLen);
            }
            
            // Profile size varies due to name field
            // For now assume minimum 19 bytes + name
            uint8_t nameLen = (remainingLen > 19) ? data[19] : 0;
            uint8_t profileSize = 20 + nameLen;
            if (posTransmit) profileSize += 1;
            
            profileData += profileSize;
            if (profileSize > remainingLen) break;
            remainingLen -= profileSize;
        }
    }
    
    profileUpdateCount++;
    lastUpdateTime = millis();
}

// =============================================================================
// Command Building
// =============================================================================

uint8_t ChargingProfileManager::buildProfile0Config(uint8_t* startFrame, uint8_t* contFrame,
                                                     uint8_t mode, uint8_t maxCurrent, 
                                                     uint8_t targetSoc) {
    // Build the 2-frame profile configuration sequence
    // 
    // This updates Profile 0 using RecordAddress=6 (compact format)
    //
    // From trace analysis (start_climate.csv):
    // Frame 1: 80 08 29 59 22 06 00 01
    //   80 = Long message start, group 0
    //   08 = Total length (8 bytes: 2 BAP header + 6 payload)
    //   29 59 = BAP header: OpCode=SetGet(2), Device=0x25, Function=0x19
    //   22 = Array header byte 1: PosTransmit=1, RecordAddr=6
    //   06 = startIndex (profile 0 encoded? or part of header?)
    //   00 = elementCount
    //   01 = first byte of profile position marker
    //
    // Frame 2: C0 06 00 20 00 00 00 00
    //   C0 = Continuation, group 0, index 0
    //   06 = operation mode (climate+battery = 0x06)
    //   00 = operation2
    //   20 = maxCurrent (32A = 0x20)
    //   00 = targetSoc (0% for climate-only)
    
    // The payload for compact profile update:
    // [arrayHeader0][arrayHeader1][startIndex][elementCount][posMarker][op][op2][maxA][soc]
    
    uint8_t arrayHeader0 = 0x22;  // Some ASG/Transaction ID
    uint8_t arrayHeader1 = ArrayHeader::POS_TRANSMIT | ArrayHeader::RECORD_ADDR_COMPACT;  // 0x46
    
    // Actually looking at the trace, byte values are:
    // 22 06 00 01 | 06 00 20 00
    // Let me match the exact trace format for now:
    
    uint8_t payload[8] = {
        0x22,           // Array header byte 0 (from trace)
        0x06,           // Array header byte 1: RecordAddr=6
        0x00,           // startIndex or padding
        0x01,           // elementCount or position marker
        mode,           // operation mode
        0x00,           // operation2
        maxCurrent,     // max charging current  
        targetSoc       // target SOC
    };
    
    // Encode long message start frame
    // Note: We're sending 8 bytes of payload
    encodeLongStart(startFrame, OpCode::SET_GET, DEVICE_BATTERY_CONTROL,
                    Function::PROFILES_ARRAY, 8, payload, 0);
    
    // Continuation frame gets the remaining 4 bytes
    // Start frame carries first 4 bytes of payload in bytes 4-7
    // So continuation gets payload[4:7]
    uint8_t contPayload[7] = {
        mode,           // operation mode
        0x00,           // operation2
        maxCurrent,     // max current
        targetSoc,      // target soc
        0x00, 0x00, 0x00  // padding
    };
    
    encodeLongContinuation(contFrame, contPayload, 7, 0, 0);
    
    return 2;  // Always 2 frames
}

uint8_t ChargingProfileManager::buildOperationStart(uint8_t* frame) {
    // Short BAP message to OPERATION_MODE (0x18)
    // Payload: [0x00, 0x01] = immediately active
    uint8_t payload[2] = { 0x00, 0x01 };
    
    return encodeShortMessage(frame, OpCode::SET_GET, DEVICE_BATTERY_CONTROL,
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t ChargingProfileManager::buildOperationStop(uint8_t* frame) {
    // Short BAP message to OPERATION_MODE (0x18)
    // Payload: [0x00, 0x00] = immediately off
    uint8_t payload[2] = { 0x00, 0x00 };
    
    return encodeShortMessage(frame, OpCode::SET_GET, DEVICE_BATTERY_CONTROL,
                              Function::OPERATION_MODE, payload, 2);
}

uint8_t ChargingProfileManager::buildProfilesRequest(uint8_t* frame) {
    // GET request for ProfilesArray (function 0x19)
    return encodeShortMessage(frame, OpCode::GET, DEVICE_BATTERY_CONTROL,
                              Function::PROFILES_ARRAY, nullptr, 0);
}

// =============================================================================
// Internal Methods
// =============================================================================

bool ChargingProfileManager::parseFullProfile(uint8_t profileIndex, 
                                               const uint8_t* data, uint8_t len) {
    if (profileIndex >= PROFILE_COUNT || len < 19) {
        return false;
    }
    
    Profile& p = profiles[profileIndex];
    
    p.operation = data[0];
    p.operation2 = data[1];
    p.maxCurrent = data[2];
    p.minChargeLevel = data[3];
    p.minRange = data[4] | (data[5] << 8);
    p.targetChargeLevel = data[6];
    p.targetChargeDuration = data[7];
    p.targetChargeRange = data[8] | (data[9] << 8);
    p.unitRange = data[10];
    p.rangeCalculationSetup = data[11];
    p.temperatureRaw = data[12];
    p.temperatureUnit = data[13];
    p.leadTime = data[14];
    p.holdingTimePlug = data[15];
    p.holdingTimeBattery = data[16];
    p.providerDataId = data[17] | (data[18] << 8);
    
    // Parse name if present
    if (len > 19) {
        p.nameLength = data[19];
        if (p.nameLength > 0 && len > 20) {
            uint8_t copyLen = min(p.nameLength, (uint8_t)(sizeof(p.name) - 1));
            memcpy(p.name, data + 20, copyLen);
            p.name[copyLen] = '\0';
        }
    }
    
    p.valid = true;
    p.lastUpdate = millis();
    
    Serial.printf("[ProfileMgr] Parsed profile %d: mode=0x%02X, temp=%.1f, soc=%d%%\r\n",
                  profileIndex, p.operation, p.getTemperature(), p.targetChargeLevel);
    
    return true;
}

bool ChargingProfileManager::parseCompactProfile(uint8_t profileIndex, 
                                                  const uint8_t* data, uint8_t len) {
    if (profileIndex >= PROFILE_COUNT || len < 4) {
        return false;
    }
    
    Profile& p = profiles[profileIndex];
    
    p.operation = data[0];
    p.operation2 = data[1];
    p.maxCurrent = data[2];
    p.targetChargeLevel = data[3];
    
    p.valid = true;
    p.lastUpdate = millis();
    
    Serial.printf("[ProfileMgr] Parsed compact profile %d: mode=0x%02X, maxA=%d, soc=%d%%\r\n",
                  profileIndex, p.operation, p.maxCurrent, p.targetChargeLevel);
    
    return true;
}

bool ChargingProfileManager::sendFrame(const uint8_t* data, uint8_t len) {
    if (!manager) {
        Serial.println("[ProfileMgr] No manager - cannot send");
        return false;
    }
    
    return manager->sendCanFrame(CAN_ID_BATTERY_TX, data, len, true);
}

bool ChargingProfileManager::sendProfileConfig(uint8_t mode, uint8_t maxCurrent, 
                                                uint8_t targetSoc) {
    uint8_t startFrame[8];
    uint8_t contFrame[8];
    
    buildProfile0Config(startFrame, contFrame, mode, maxCurrent, targetSoc);
    
    // Debug output
    Serial.printf("[ProfileMgr] Sending profile config: mode=0x%02X, maxA=%d, soc=%d\r\n",
                  mode, maxCurrent, targetSoc);
    Serial.printf("[ProfileMgr] Frame1: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                  startFrame[0], startFrame[1], startFrame[2], startFrame[3],
                  startFrame[4], startFrame[5], startFrame[6], startFrame[7]);
    Serial.printf("[ProfileMgr] Frame2: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                  contFrame[0], contFrame[1], contFrame[2], contFrame[3],
                  contFrame[4], contFrame[5], contFrame[6], contFrame[7]);
    
    // Send start frame
    if (!sendFrame(startFrame, 8)) {
        Serial.println("[ProfileMgr] Failed to send start frame");
        return false;
    }
    
    // Small delay between frames
    delay(10);
    
    // Send continuation frame
    if (!sendFrame(contFrame, 8)) {
        Serial.println("[ProfileMgr] Failed to send continuation frame");
        return false;
    }
    
    return true;
}

bool ChargingProfileManager::sendOperationTrigger(bool start) {
    uint8_t frame[8];
    
    if (start) {
        buildOperationStart(frame);
        Serial.println("[ProfileMgr] Sending operation START");
    } else {
        buildOperationStop(frame);
        Serial.println("[ProfileMgr] Sending operation STOP");
    }
    
    Serial.printf("[ProfileMgr] Frame: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                  frame[0], frame[1], frame[2], frame[3],
                  frame[4], frame[5], frame[6], frame[7]);
    
    return sendFrame(frame, 8);
}
