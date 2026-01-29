#include "ChargingProfileManager.h"
#include "VehicleManager.h"

using namespace ChargingProfile;
using namespace BapProtocol;

// =============================================================================
// BAP Constants (TODO: Move to BatteryControlChannel)
// =============================================================================

namespace {
    constexpr uint8_t DEVICE_BATTERY_CONTROL = 0x25;
    constexpr uint32_t CAN_ID_BATTERY_TX = 0x17332501;
    
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
// Profile Data Management
// =============================================================================

void ChargingProfileManager::updateProfileLocal(uint8_t profileIndex, const Profile& profile) {
    if (profileIndex >= PROFILE_COUNT) {
        Serial.printf("[ProfileMgr] Invalid profile index %d\r\n", profileIndex);
        return;
    }
    
    profiles[profileIndex] = profile;
    profiles[profileIndex].valid = true;
    profiles[profileIndex].lastUpdate = millis();
    
    Serial.printf("[ProfileMgr] Updated profile %d locally\r\n", profileIndex);
}

void ChargingProfileManager::clearAllProfiles() {
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        profiles[i].clear();
    }
    Serial.println("[ProfileMgr] All profiles cleared");
}

// =============================================================================
// BAP Request Methods (TODO: Move to BatteryControlChannel)
// =============================================================================

bool ChargingProfileManager::requestAllProfiles() {
    uint8_t frame[8];
    
    // Build GET request for ProfilesArray
    encodeShortMessage(frame, OpCode::GET, DEVICE_BATTERY_CONTROL,
                      Function::PROFILES_ARRAY, nullptr, 0);
    
    Serial.println("[ProfileMgr] Requesting all profiles...");
    return manager->sendCanFrame(CAN_ID_BATTERY_TX, frame, 8, true);
}

bool ChargingProfileManager::updateTimerProfile(uint8_t profileIndex, const Profile& profile) {
    if (profileIndex < 1 || profileIndex > 3) {
        Serial.println("[ProfileMgr] Invalid profile index for timer (must be 1-3)");
        return false;
    }
    
    // Store the profile locally
    profiles[profileIndex] = profile;
    profiles[profileIndex].valid = true;
    profiles[profileIndex].lastUpdate = millis();
    
    // TODO: Send full profile update to car via BAP
    // This requires building a longer BAP message with full profile data (RecordAddr=0)
    Serial.printf("[ProfileMgr] TODO: Send full profile %d update to vehicle\r\n", profileIndex);
    
    return true;
}

bool ChargingProfileManager::setTimerProfileEnabled(uint8_t profileIndex, bool enable) {
    if (profileIndex < 1 || profileIndex > 3) {
        Serial.println("[ProfileMgr] Invalid profile index for timer (must be 1-3)");
        return false;
    }
    
    // Build OPERATION_MODE command to enable/disable timer
    uint8_t frame[8];
    uint8_t timerBit = enable ? static_cast<uint8_t>(1 << profileIndex) : 0x00;
    uint8_t payload[2] = { 0x00, timerBit };
    
    encodeShortMessage(frame, OpCode::SET_GET, DEVICE_BATTERY_CONTROL,
                      Function::OPERATION_MODE, payload, 2);
    
    Serial.printf("[ProfileMgr] %s timer profile %d\r\n", 
                  enable ? "Enabling" : "Disabling", profileIndex);
    
    return manager->sendCanFrame(CAN_ID_BATTERY_TX, frame, 8, true);
}

// =============================================================================
// BAP Message Processing
// =============================================================================

void ChargingProfileManager::processProfilesArray(const uint8_t* payload, uint8_t len) {
    if (len < 5) {
        Serial.println("[ProfileMgr] ProfilesArray payload too short");
        return;
    }
    
    // Parse BAP array header (STATUS format from FSG)
    // STATUS header (5+ bytes):
    //   Byte 0: [ASG-ID:4][Transaction-ID:4]
    //   Byte 1: totalElementsInList
    //   Byte 2: [LargeIdx:1][PosTransmit:1][Backward:1][Shift:1][RecordAddr:4]
    //   Byte 3: startIndex (or bytes 3-4 if LargeIdx=1)
    //   Byte 4: elementCount (or bytes 5-6 if LargeIdx=1)
    //
    // Note: We receive STATUS responses, not GET requests!
    
    uint8_t totalElements = payload[1];
    uint8_t headerByte2 = payload[2];
    uint8_t recordAddr = headerByte2 & 0x0F;
    bool posTransmit = (headerByte2 & ArrayHeader::POS_TRANSMIT) != 0;
    bool largeIdx = (headerByte2 & ArrayHeader::LARGE_IDX) != 0;
    uint8_t startIndex = payload[3];
    uint8_t elementCount = payload[4];
    
    Serial.printf("[ProfileMgr] ProfilesArray: %d profiles from idx %d\r\n",
                  elementCount, startIndex);
    
    // Skip header bytes (5 bytes for STATUS format, or more if LargeIdx=1)
    uint8_t headerSize = largeIdx ? 7 : 5;  // 16-bit indexes use 2 more bytes
    const uint8_t* profileData = payload + headerSize;
    uint8_t remainingLen = len - headerSize;
    
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
    
    // Full profile received - mark as valid for read-modify-write workflow
    p.valid = true;
    p.lastUpdate = millis();
    
    Serial.printf("[ProfileMgr] Profile %d: op=0x%02X, soc=%d%%, maxA=%d [VALID]\r\n",
                  profileIndex, p.operation, p.targetChargeLevel, p.maxCurrent);
    
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
    
    // Compact updates do NOT set valid flag - we need full profile for that
    // p.valid = false;  // Don't change valid state
    p.lastUpdate = millis();
    
    Serial.printf("[ProfileMgr] Profile %d: op=0x%02X, soc=%d%%, maxA=%d [compact]\r\n",
                  profileIndex, p.operation, p.targetChargeLevel, p.maxCurrent);
    
    return true;
}

// =============================================================================
// Profile Update Loop and State Management
// =============================================================================

void ChargingProfileManager::loop() {
    updateStateMachine();
}

const char* ChargingProfileManager::getUpdateStateName() const {
    switch (updateState) {
        case ProfileUpdateState::IDLE: return "IDLE";
        case ProfileUpdateState::READING_PROFILE: return "READING_PROFILE";
        case ProfileUpdateState::UPDATING_PROFILE: return "UPDATING_PROFILE";
        case ProfileUpdateState::UPDATE_COMPLETE: return "UPDATE_COMPLETE";
        case ProfileUpdateState::UPDATE_FAILED: return "UPDATE_FAILED";
        default: return "UNKNOWN";
    }
}

void ChargingProfileManager::updateStateMachine() {
    unsigned long now = millis();
    unsigned long elapsed = now - updateStateStartTime;
    
    switch (updateState) {
        case ProfileUpdateState::IDLE:
            // Nothing to do
            break;
            
        case ProfileUpdateState::READING_PROFILE:
            // Check if profile became valid (received from CAN)
            if (profiles[pendingProfileIndex].valid) {
                Serial.printf("[ProfileMgr] Profile %d received after %lums\r\n", 
                             pendingProfileIndex, elapsed);
                setUpdateState(ProfileUpdateState::UPDATING_PROFILE);
            }
            // Check for timeout
            else if (elapsed > PROFILE_READ_TIMEOUT) {
                Serial.printf("[ProfileMgr] Profile %d read timeout\r\n", pendingProfileIndex);
                setUpdateState(ProfileUpdateState::UPDATE_FAILED);
            }
            break;
            
        case ProfileUpdateState::UPDATING_PROFILE:
            // Apply updates and send
            applyPendingUpdates();
            
            if (sendProfileUpdateRequest(pendingProfileIndex)) {
                Serial.printf("[ProfileMgr] Profile %d update sent\r\n", pendingProfileIndex);
                setUpdateState(ProfileUpdateState::UPDATE_COMPLETE);
            } else {
                Serial.printf("[ProfileMgr] Profile %d update send failed\r\n", pendingProfileIndex);
                setUpdateState(ProfileUpdateState::UPDATE_FAILED);
            }
            break;
            
        case ProfileUpdateState::UPDATE_COMPLETE:
            // Call callback with success
            completeUpdate(true);
            
            // Reset to IDLE on next loop
            setUpdateState(ProfileUpdateState::IDLE);
            break;
            
        case ProfileUpdateState::UPDATE_FAILED:
            // Call callback with failure
            completeUpdate(false);
            
            // Reset to IDLE on next loop
            setUpdateState(ProfileUpdateState::IDLE);
            break;
    }
}

bool ChargingProfileManager::requestProfileUpdate(uint8_t profileIndex,
                                                   const ProfileFieldUpdate& updates,
                                                   std::function<void(bool)> callback) {
    // Reject if already busy
    if (updateState != ProfileUpdateState::IDLE) {
        Serial.println("[ProfileMgr] Update rejected: already in progress");
        return false;
    }
    
    // Validate profile index
    if (profileIndex >= PROFILE_COUNT) {
        Serial.printf("[ProfileMgr] Update rejected: invalid profile %d\r\n", profileIndex);
        return false;
    }
    
    // Store request
    pendingProfileIndex = profileIndex;
    pendingUpdates = updates;
    pendingCallback = callback;
    
    Serial.printf("[ProfileMgr] Profile %d update requested\r\n", profileIndex);
    
    // Check if we need to read profile first
    if (!profiles[profileIndex].valid) {
        Serial.printf("[ProfileMgr] Profile %d not valid, reading first\r\n", profileIndex);
        
        if (sendProfileReadRequest(profileIndex)) {
            setUpdateState(ProfileUpdateState::READING_PROFILE);
            return true;
        } else {
            Serial.printf("[ProfileMgr] Profile %d read request failed\r\n", profileIndex);
            return false;
        }
    } else {
        // Profile already valid, proceed to update
        Serial.printf("[ProfileMgr] Profile %d already valid, proceeding to update\r\n", profileIndex);
        setUpdateState(ProfileUpdateState::UPDATING_PROFILE);
        return true;
    }
}

void ChargingProfileManager::cancelProfileUpdate() {
    if (updateState != ProfileUpdateState::IDLE) {
        Serial.println("[ProfileMgr] Update cancelled");
        completeUpdate(false);
        setUpdateState(ProfileUpdateState::IDLE);
    }
}

void ChargingProfileManager::setUpdateState(ProfileUpdateState newState) {
    if (updateState != newState) {
        Serial.printf("[ProfileMgr] Update: %s -> %s\r\n",
                     getUpdateStateName(),
                     [this, newState]() {
                         ProfileUpdateState temp = updateState;
                         updateState = newState;
                         const char* name = getUpdateStateName();
                         updateState = temp;
                         return name;
                     }());
        updateState = newState;
        updateStateStartTime = millis();
    }
}

void ChargingProfileManager::applyPendingUpdates() {
    Profile& p = profiles[pendingProfileIndex];
    
    if (pendingUpdates.updateOperation) {
        Serial.printf("[ProfileMgr] Updating operation: 0x%02X -> 0x%02X\r\n",
                     p.operation, pendingUpdates.operation);
        p.operation = pendingUpdates.operation;
    }
    
    if (pendingUpdates.updateMaxCurrent) {
        Serial.printf("[ProfileMgr] Updating maxCurrent: %d -> %d\r\n",
                     p.maxCurrent, pendingUpdates.maxCurrent);
        p.maxCurrent = pendingUpdates.maxCurrent;
    }
    
    if (pendingUpdates.updateTargetSoc) {
        Serial.printf("[ProfileMgr] Updating targetSoc: %d -> %d\r\n",
                     p.targetChargeLevel, pendingUpdates.targetSoc);
        p.targetChargeLevel = pendingUpdates.targetSoc;
    }
    
    if (pendingUpdates.updateTemperature) {
        float oldTemp = p.getTemperature();
        p.setTemperature(pendingUpdates.temperature);
        Serial.printf("[ProfileMgr] Updating temperature: %.1f -> %.1f\r\n",
                     oldTemp, pendingUpdates.temperature);
    }
}

void ChargingProfileManager::completeUpdate(bool success) {
    if (pendingCallback) {
        pendingCallback(success);
        pendingCallback = nullptr;
    }
}

bool ChargingProfileManager::sendProfileReadRequest(uint8_t profileIndex) {
    // TODO: Implement GET request for profile array
    // For now, just request all profiles
    Serial.printf("[ProfileMgr] Sending GET request for profile %d\r\n", profileIndex);
    return requestAllProfiles();
}

bool ChargingProfileManager::sendProfileUpdateRequest(uint8_t profileIndex) {
    // Encode full profile (RecordAddr=0, 20+ bytes) and send via SET_GET
    Profile& p = profiles[profileIndex];
    
    // Build array header (4 bytes for SET_GET)
    // Byte 0: [ASG-ID:4][Transaction-ID:4] - using 0x00
    // Byte 1: [LargeIdx:1][PosTransmit:1][Backward:1][Shift:1][RecordAddr:4]
    // Byte 2: startIndex
    // Byte 3: elementCount
    
    uint8_t arrayHeader[4] = {
        0x00,                              // ASG-ID=0, Transaction=0
        0x10,                              // LargeIdx=0, PosTransmit=1, Backward=0, Shift=0, RecordAddr=0 (full)
        profileIndex,                      // Start at this profile index
        0x01                               // Update 1 element
    };
    
    // Build profile data (20 bytes minimum, excluding name)
    uint8_t profileData[20];
    profileData[0] = p.operation;
    profileData[1] = p.operation2;
    profileData[2] = p.maxCurrent;
    profileData[3] = p.minChargeLevel;
    profileData[4] = p.minRange & 0xFF;            // LE low byte
    profileData[5] = (p.minRange >> 8) & 0xFF;     // LE high byte
    profileData[6] = p.targetChargeLevel;
    profileData[7] = p.targetChargeDuration;
    profileData[8] = p.targetChargeRange & 0xFF;   // LE low byte
    profileData[9] = (p.targetChargeRange >> 8) & 0xFF;  // LE high byte
    profileData[10] = p.unitRange;
    profileData[11] = p.rangeCalculationSetup;
    profileData[12] = p.temperatureRaw;
    profileData[13] = p.temperatureUnit;
    profileData[14] = p.leadTime;
    profileData[15] = p.holdingTimePlug;
    profileData[16] = p.holdingTimeBattery;
    profileData[17] = p.providerDataId & 0xFF;     // LE low byte
    profileData[18] = (p.providerDataId >> 8) & 0xFF;  // LE high byte
    profileData[19] = p.nameLength;
    
    // Total payload: array header (4) + position byte (1) + profile data (20+)
    // If PosTransmit=1, each element has [pos][data]
    uint8_t totalPayloadLen = 4 + 1 + 20 + p.nameLength;
    
    // Build complete payload
    uint8_t payload[64];  // Max BAP payload
    if (totalPayloadLen > sizeof(payload)) {
        Serial.printf("[ProfileMgr] ERROR: Profile %d payload too large: %d bytes\r\n", 
                     profileIndex, totalPayloadLen);
        return false;
    }
    
    // Copy array header
    memcpy(payload, arrayHeader, 4);
    
    // Add position byte (since PosTransmit=1)
    payload[4] = profileIndex;
    
    // Copy profile data
    memcpy(payload + 5, profileData, 20);
    
    // Copy name if present
    if (p.nameLength > 0) {
        memcpy(payload + 25, p.name, p.nameLength);
    }
    
    Serial.printf("[ProfileMgr] Sending profile %d update: %d bytes, temp=%.1fC\r\n",
                 profileIndex, totalPayloadLen, p.getTemperature());
    
    // Use high-level sendBapMessage helper - it handles framing automatically
    auto sendFrame = [this](const uint8_t* data, uint8_t len) {
        return manager->sendCanFrame(CAN_ID_BATTERY_TX, data, len, true);
    };
    
    uint8_t frameCount = BapProtocol::sendBapMessage(
        sendFrame,
        OpCode::SET_GET,
        DEVICE_BATTERY_CONTROL,
        Function::PROFILES_ARRAY,
        payload,
        totalPayloadLen,
        0  // group
    );
    
    if (frameCount == 0) {
        Serial.printf("[ProfileMgr] ERROR: Failed to send profile %d update\r\n", profileIndex);
        return false;
    }
    
    Serial.printf("[ProfileMgr] Sent %d frames for profile %d update\r\n", frameCount, profileIndex);
    return true;
}
