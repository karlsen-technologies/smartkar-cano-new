#pragma once

#include <Arduino.h>
#include <functional>
#include "ChargingProfile.h"
#include "protocols/BapProtocol.h"

// Forward declaration
class VehicleManager;

/**
 * ChargingProfileManager - Profile Data Manager
 * 
 * This class is responsible for:
 * - Storing the current state of all 4 profiles (0-3)
 * - Parsing profile data received from BAP messages
 * - Providing read/write access to profile data
 * 
 * Profile 0 is special:
 * - Used for "immediate" operations (start climate/charging now)
 * - Not shown in the infotainment UI as a "Charge Location"
 * 
 * Profiles 1-3 are user-configurable:
 * - Shown as "Timer 1", "Timer 2", "Timer 3" in infotainment
 * - Can be enabled/disabled independently
 * - Have associated schedules (departure times)
 * 
 * Note: This class does NOT send commands. Command sending is handled
 * by BatteryControlChannel. This is purely a data storage and parser.
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
    // Profile Data Management
    // =========================================================================
    
    /**
     * Update a profile's data locally (does not send to vehicle)
     * @param profileIndex Profile to update (0-3)
     * @param profile New profile settings
     */
    void updateProfileLocal(uint8_t profileIndex, const ChargingProfile::Profile& profile);
    
    /**
     * Clear all profiles (reset to invalid state)
     */
    void clearAllProfiles();
    
    // =========================================================================
    // BAP Request Methods (TODO: Move to BatteryControlChannel)
    // =========================================================================
    
    /**
     * Request all profiles from the vehicle
     * @return true if request sent successfully
     * 
     * TODO: This should be moved to BatteryControlChannel as it involves
     * sending BAP commands. Kept here temporarily for backwards compatibility.
     */
    bool requestAllProfiles();
    
    /**
     * Update a timer profile (1-3) configuration
     * @param profileIndex Profile to update (1-3)
     * @param profile New profile settings
     * @return true if update sent successfully
     * 
     * TODO: This should be moved to BatteryControlChannel. Currently this
     * only updates locally and doesn't actually send to vehicle (incomplete).
     */
    bool updateTimerProfile(uint8_t profileIndex, const ChargingProfile::Profile& profile);
    
    /**
     * Enable or disable a timer profile
     * @param profileIndex Profile to enable/disable (1-3)
     * @param enable true to enable, false to disable
     * @return true if command sent successfully
     * 
     * TODO: This should be moved to BatteryControlChannel as it sends BAP commands.
     * Kept here temporarily for backwards compatibility.
     */
    bool setTimerProfileEnabled(uint8_t profileIndex, bool enable);
    
    // =========================================================================
    // BAP Message Processing
    // =========================================================================
    
    /**
     * Process a ProfilesArray BAP message (function 0x19)
     * Called by BatteryControlChannel when it receives profile data.
     * 
     * @param payload BAP message payload (after function ID)
     * @param len Payload length
     */
    void processProfilesArray(const uint8_t* payload, uint8_t len);
    
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
    
    // =========================================================================
    // Profile Update State Machine (for read-modify-write workflow)
    // =========================================================================
    
    enum class ProfileUpdateState {
        IDLE,               // No update in progress
        READING_PROFILE,    // Waiting for profile data from GET request
        UPDATING_PROFILE,   // Waiting for update confirmation from SET_GET
        UPDATE_COMPLETE,    // Update successful
        UPDATE_FAILED       // Update failed (timeout or error)
    };
    
    /**
     * Structure to hold a profile field update
     */
    struct ProfileFieldUpdate {
        bool updateOperation = false;
        uint8_t operation = 0;
        
        bool updateMaxCurrent = false;
        uint8_t maxCurrent = 0;
        
        bool updateTargetSoc = false;
        uint8_t targetSoc = 0;
        
        bool updateTemperature = false;
        float temperature = 0.0f;
    };
    
    /**
     * Request a profile update (async, uses state machine)
     * 
     * Workflow:
     * 1. Check if profile data is valid
     *    - If NO: Send GET request, wait for response, then proceed to step 2
     *    - If YES: Skip to step 2
     * 2. Apply updates to profile data
     * 3. Send SET_GET with full profile data
     * 4. Wait for STATUS confirmation
     * 5. Call callback with result
     * 
     * @param profileIndex Profile to update (0-3)
     * @param updates Fields to update
     * @param callback Called when update completes (true=success, false=failure)
     * @return true if request queued, false if already busy
     */
    bool requestProfileUpdate(uint8_t profileIndex, 
                             const ProfileFieldUpdate& updates,
                             std::function<void(bool)> callback = nullptr);
    
    /**
     * Cancel any pending profile update
     */
    void cancelProfileUpdate();
    
    /**
     * Check if profile update is in progress
     */
    bool isUpdateInProgress() const { return updateState != ProfileUpdateState::IDLE; }
    
    /**
     * Get current update state
     */
    ProfileUpdateState getUpdateState() const { return updateState; }
    
    /**
     * Get update state as string (for logging)
     */
    const char* getUpdateStateName() const;
    
    /**
     * Process state machine (called from VehicleManager::loop())
     */
    void loop();
    
private:
    VehicleManager* manager;
    
    // Profile storage (reference to RTC memory - survives deep sleep)
    ChargingProfile::Profile (&profiles)[ChargingProfile::PROFILE_COUNT];
    
    // Statistics
    volatile uint32_t profileUpdateCount = 0;
    volatile unsigned long lastUpdateTime = 0;
    
    // =========================================================================
    // Profile Update State Machine
    // =========================================================================
    
    ProfileUpdateState updateState = ProfileUpdateState::IDLE;
    unsigned long updateStateStartTime = 0;
    
    // Pending update request data
    uint8_t pendingProfileIndex = 0;
    ProfileFieldUpdate pendingUpdates;
    std::function<void(bool)> pendingCallback = nullptr;
    
    // Timeout for profile operations
    static constexpr unsigned long PROFILE_READ_TIMEOUT = 5000;   // 5s
    static constexpr unsigned long PROFILE_UPDATE_TIMEOUT = 5000; // 5s
    
    /**
     * Update state machine tick (called from loop())
     */
    void updateStateMachine();
    
    /**
     * Set update state with logging
     */
    void setUpdateState(ProfileUpdateState newState);
    
    /**
     * Send GET request for profile data
     */
    bool sendProfileReadRequest(uint8_t profileIndex);
    
    /**
     * Send SET_GET with full profile update
     */
    bool sendProfileUpdateRequest(uint8_t profileIndex);
    
    /**
     * Apply pending updates to profile data
     */
    void applyPendingUpdates();
    
    /**
     * Call callback and reset state to IDLE
     */
    void completeUpdate(bool success);
    
    // =========================================================================
    // Internal Methods
    // =========================================================================
    
    /**
     * Parse a full profile from BAP payload (RecordAddr=0)
     */
    bool parseFullProfile(uint8_t profileIndex, const uint8_t* data, uint8_t len);
    
    /**
     * Parse a compact profile update from BAP payload (RecordAddr=6)
     */
    bool parseCompactProfile(uint8_t profileIndex, const uint8_t* data, uint8_t len);
};
