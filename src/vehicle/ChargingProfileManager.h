#pragma once

#include <Arduino.h>
#include "ChargingProfile.h"
#include "protocols/BapProtocol.h"

// Forward declaration
class VehicleManager;

/**
 * ChargingProfileManager - Manages Battery Control Profiles
 * 
 * This class handles:
 * - Storing the current state of all 4 profiles (0-3)
 * - Parsing profile data received from the car via BAP
 * - Building BAP messages to configure profiles
 * - Providing a high-level API for climate/charging operations
 * 
 * Profile 0 is special:
 * - Used for "immediate" operations (start climate/charging now)
 * - Not shown in the infotainment UI as a "Charge Location"
 * - Always modified before executing immediate start commands
 * 
 * Profiles 1-3 are user-configurable:
 * - Shown as "Timer 1", "Timer 2", "Timer 3" in infotainment
 * - Can be enabled/disabled independently
 * - Have associated schedules (departure times)
 */
class ChargingProfileManager {
public:
    ChargingProfileManager(VehicleManager* mgr);
    
    // =========================================================================
    // Profile Access
    // =========================================================================
    
    /**
     * Get a profile by index (0-3)
     */
    const ChargingProfile::Profile& getProfile(uint8_t index) const;
    
    /**
     * Get mutable profile reference for updates
     */
    ChargingProfile::Profile& getProfileMutable(uint8_t index);
    
    /**
     * Check if a profile is valid (has been read from car)
     */
    bool isProfileValid(uint8_t index) const;
    
    // =========================================================================
    // High-Level Operations (uses Profile 0)
    // =========================================================================
    
    /**
     * Start climate control immediately
     * 
     * This configures Profile 0 for climate mode and triggers it.
     * @param tempCelsius Target temperature (15.5 - 30.0)
     * @param allowBattery Allow running on battery (no plug required)
     * @return true if commands were queued successfully
     */
    bool startClimateNow(float tempCelsius = 22.0f, bool allowBattery = true);
    
    /**
     * Stop climate control
     * @return true if command was queued successfully
     */
    bool stopClimateNow();
    
    /**
     * Start charging immediately
     * 
     * This configures Profile 0 for charging mode and triggers it.
     * @param targetSoc Target state of charge (0-100%)
     * @param maxCurrent Maximum charging current (0-32A)
     * @return true if commands were queued successfully
     */
    bool startChargingNow(uint8_t targetSoc = 80, uint8_t maxCurrent = 32);
    
    /**
     * Stop charging
     * @return true if command was queued successfully
     */
    bool stopChargingNow();
    
    /**
     * Start both charging and climate
     * @return true if commands were queued successfully
     */
    bool startChargingAndClimateNow(float tempCelsius = 22.0f, 
                                     uint8_t targetSoc = 80,
                                     bool allowClimateOnBattery = true);
    
    // =========================================================================
    // Profile Management (for Profiles 1-3)
    // =========================================================================
    
    /**
     * Update a timer profile (1-3) with new settings
     * @param profileIndex Profile to update (1-3)
     * @param profile New profile settings
     * @return true if update command was sent
     */
    bool updateTimerProfile(uint8_t profileIndex, const ChargingProfile::Profile& profile);
    
    /**
     * Enable or disable a timer profile
     * @param profileIndex Profile to enable/disable (1-3)
     * @param enable true to enable, false to disable
     * @return true if command was sent
     */
    bool setTimerProfileEnabled(uint8_t profileIndex, bool enable);
    
    /**
     * Request all profiles from the car
     * @return true if request was sent
     */
    bool requestAllProfiles();
    
    // =========================================================================
    // BAP Message Processing
    // =========================================================================
    
    /**
     * Process a ProfilesArray BAP message (function 0x19)
     * Called by BapDomain when it receives profile data.
     * 
     * @param payload BAP message payload (after function ID)
     * @param len Payload length
     */
    void processProfilesArray(const uint8_t* payload, uint8_t len);
    
    // =========================================================================
    // Command Building (for sending via BapDomain)
    // =========================================================================
    
    /**
     * Build frames to configure Profile 0 for a specific operation mode
     * 
     * This creates the 2-frame sequence needed to update Profile 0:
     * Frame 1: Long BAP start frame
     * Frame 2: Long BAP continuation frame
     * 
     * @param startFrame Output buffer for start frame (8 bytes)
     * @param contFrame Output buffer for continuation frame (8 bytes)
     * @param mode Operation mode (see ChargingProfile::OperationMode)
     * @param maxCurrent Max charging current
     * @param targetSoc Target SOC
     * @return Number of frames (always 2)
     */
    uint8_t buildProfile0Config(uint8_t* startFrame, uint8_t* contFrame,
                                 uint8_t mode, uint8_t maxCurrent = 32, 
                                 uint8_t targetSoc = 80);
    
    /**
     * Build frame to trigger Profile 0 (start operations)
     * @param frame Output buffer (8 bytes)
     * @return Frame length
     */
    uint8_t buildOperationStart(uint8_t* frame);
    
    /**
     * Build frame to stop all operations
     * @param frame Output buffer (8 bytes)
     * @return Frame length
     */
    uint8_t buildOperationStop(uint8_t* frame);
    
    /**
     * Build frame to request all profiles
     * @param frame Output buffer (8 bytes)
     * @return Frame length
     */
    uint8_t buildProfilesRequest(uint8_t* frame);
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    /**
     * Get number of profile updates received
     */
    uint32_t getProfileUpdateCount() const { return profileUpdateCount; }
    
    /**
     * Get last profile update timestamp
     */
    unsigned long getLastUpdateTime() const { return lastUpdateTime; }
    
private:
    VehicleManager* manager;
    
    // Profile storage
    ChargingProfile::Profile profiles[ChargingProfile::PROFILE_COUNT];
    
    // Statistics
    volatile uint32_t profileUpdateCount = 0;
    volatile unsigned long lastUpdateTime = 0;
    
    // =========================================================================
    // Internal Methods
    // =========================================================================
    
    /**
     * Parse a full profile from BAP payload
     */
    bool parseFullProfile(uint8_t profileIndex, const uint8_t* data, uint8_t len);
    
    /**
     * Parse a compact profile update from BAP payload
     */
    bool parseCompactProfile(uint8_t profileIndex, const uint8_t* data, uint8_t len);
    
    /**
     * Send a CAN frame via the vehicle manager
     */
    bool sendFrame(const uint8_t* data, uint8_t len);
    
    /**
     * Send the 2-frame profile configuration sequence
     */
    bool sendProfileConfig(uint8_t mode, uint8_t maxCurrent, uint8_t targetSoc);
    
    /**
     * Send the operation mode trigger
     */
    bool sendOperationTrigger(bool start);
};
