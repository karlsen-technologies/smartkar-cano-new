#pragma once

#include <Arduino.h>

/**
 * Battery Control Profile (BCP) Data Structures
 * 
 * BCPs are used by the VW e-Golf to control charging and climate operations.
 * Each profile stores settings for charging (target SOC, max current), climate
 * (temperature, seat heaters), and timing (lead time, hold times).
 * 
 * Profile slots:
 * - Profile 0: Special "immediate" profile - used for "start now" operations
 * - Profiles 1-3: User-configurable timer profiles (shown as Charge Locations in UI)
 */

namespace ChargingProfile {

// =============================================================================
// Profile Constants
// =============================================================================

constexpr uint8_t PROFILE_COUNT = 4;           // 4 profile slots (0-3)
constexpr uint8_t PROFILE_IMMEDIATE = 0;       // Profile 0 is for immediate operations
constexpr uint8_t PROFILE_TIMER_1 = 1;
constexpr uint8_t PROFILE_TIMER_2 = 2;
constexpr uint8_t PROFILE_TIMER_3 = 3;

// Temperature encoding
constexpr float TEMP_MIN = 15.5f;              // Minimum settable temperature
constexpr float TEMP_MAX = 30.0f;              // Maximum settable temperature
constexpr float TEMP_DEFAULT = 22.0f;          // Default temperature

// Current limits
constexpr uint8_t CURRENT_MAX = 32;            // Max charging current (32A for e-Golf)
constexpr uint8_t CURRENT_DEFAULT = 32;        // Default max current

// SOC limits
constexpr uint8_t SOC_MIN = 0;
constexpr uint8_t SOC_MAX = 100;
constexpr uint8_t SOC_DEFAULT_TARGET = 80;     // Default target SOC

// =============================================================================
// Operation Flags (Byte 0 of profile)
// =============================================================================

namespace OperationFlags {
    constexpr uint8_t CHARGE = 0x01;                       // Enable charging
    constexpr uint8_t CLIMATE = 0x02;                      // Enable climate control
    constexpr uint8_t CLIMATE_WITHOUT_SUPPLY = 0x04;       // Allow climate on battery (no plug required)
    constexpr uint8_t AUTO_DEFROST = 0x08;                 // Enable automatic defrost
    constexpr uint8_t SEAT_HEATER_FRONT_LEFT = 0x10;       // Enable front left seat heater
    constexpr uint8_t SEAT_HEATER_FRONT_RIGHT = 0x20;      // Enable front right seat heater
    constexpr uint8_t SEAT_HEATER_REAR_LEFT = 0x40;        // Enable rear left seat heater
    constexpr uint8_t SEAT_HEATER_REAR_RIGHT = 0x80;       // Enable rear right seat heater
}

// Common operation mode combinations
namespace OperationMode {
    constexpr uint8_t NONE = 0x00;
    constexpr uint8_t CHARGING_ONLY = 0x01;                                          // 0x01
    constexpr uint8_t CLIMATE_ONLY = 0x02;                                           // 0x02 (requires plug)
    constexpr uint8_t CHARGING_AND_CLIMATE = 0x03;                                   // 0x03
    constexpr uint8_t CLIMATE_ALLOW_BATTERY = 0x06;                                  // 0x06 (climate + battery OK)
    constexpr uint8_t CHARGING_AND_CLIMATE_ALLOW_BATTERY = 0x07;                     // 0x07
}

// =============================================================================
// Operation2 Flags (Byte 1 of profile)
// =============================================================================

namespace Operation2Flags {
    constexpr uint8_t WINDOW_HEATER_FRONT = 0x01;          // Front windscreen heater
    constexpr uint8_t WINDOW_HEATER_REAR = 0x02;           // Rear window heater
    constexpr uint8_t PARK_HEATER = 0x04;                  // Auxiliary/park heater (fuel-based)
    constexpr uint8_t PARK_HEATER_AUTOMATIC = 0x08;        // Automatic park heater activation
}

// =============================================================================
// Profile Data Structure (Full format - Record Address 0)
// =============================================================================

/**
 * Full Battery Control Profile
 * 
 * This matches the wire format used when reading profiles from the car.
 * Total size: 20+ bytes (variable due to name field)
 */
struct Profile {
    // Byte 0: Operation flags
    uint8_t operation = 0;
    
    // Byte 1: Operation2 flags
    uint8_t operation2 = 0;
    
    // Byte 2: Maximum charging current (Amps)
    uint8_t maxCurrent = CURRENT_DEFAULT;
    
    // Byte 3: Minimum charge level (%) - car charges to this immediately
    uint8_t minChargeLevel = 0;
    
    // Bytes 4-5: Minimum range (16-bit LE)
    uint16_t minRange = 0;
    
    // Byte 6: Target charge level (%)
    uint8_t targetChargeLevel = SOC_DEFAULT_TARGET;
    
    // Byte 7: Target charge duration
    uint8_t targetChargeDuration = 0;
    
    // Bytes 8-9: Target charge range (16-bit LE)
    uint16_t targetChargeRange = 0;
    
    // Byte 10: Range unit (0=km, 1=miles)
    uint8_t unitRange = 0;
    
    // Byte 11: Range calculation setup (bit 0: enabled)
    uint8_t rangeCalculationSetup = 0;
    
    // Byte 12: Temperature (encoded: actual = (value + 100) / 10)
    uint8_t temperatureRaw = 120;  // Default 22.0C
    
    // Byte 13: Temperature unit (0=Celsius, 1=Fahrenheit)
    uint8_t temperatureUnit = 0;
    
    // Byte 14: Lead time (minutes before departure to start)
    uint8_t leadTime = 30;
    
    // Byte 15: Holding time when plugged (minutes)
    uint8_t holdingTimePlug = 30;
    
    // Byte 16: Holding time on battery (minutes)
    uint8_t holdingTimeBattery = 10;
    
    // Bytes 17-18: Power provider data ID (16-bit LE)
    uint16_t providerDataId = 0;
    
    // Name (variable length, stored separately)
    char name[32] = {0};
    uint8_t nameLength = 0;
    
    // Metadata (not transmitted)
    // IMPORTANT: 'valid' flag is ONLY set when a FULL profile (RecordAddr=0, 20+ bytes) 
    // is received. Compact updates (RecordAddr=6, 4 bytes) do NOT set this flag.
    // This ensures we have complete data before attempting read-modify-write operations.
    bool valid = false;
    unsigned long lastUpdate = 0;
    
    // =========================================================================
    // Helper methods
    // =========================================================================
    
    /**
     * Get temperature in Celsius
     */
    float getTemperature() const {
        return (temperatureRaw + 100) / 10.0f;
    }
    
    /**
     * Set temperature from Celsius
     */
    void setTemperature(float celsius) {
        celsius = constrain(celsius, TEMP_MIN, TEMP_MAX);
        temperatureRaw = static_cast<uint8_t>((celsius * 10) - 100);
    }
    
    /**
     * Check if charging is enabled
     */
    bool isChargingEnabled() const {
        return (operation & OperationFlags::CHARGE) != 0;
    }
    
    /**
     * Check if climate is enabled
     */
    bool isClimateEnabled() const {
        return (operation & OperationFlags::CLIMATE) != 0;
    }
    
    /**
     * Check if climate can run on battery
     */
    bool isClimateAllowedOnBattery() const {
        return (operation & OperationFlags::CLIMATE_WITHOUT_SUPPLY) != 0;
    }
    
    /**
     * Set operation mode
     */
    void setOperationMode(uint8_t mode) {
        operation = mode;
    }
    
    /**
     * Enable charging
     */
    void enableCharging(bool enable) {
        if (enable) {
            operation |= OperationFlags::CHARGE;
        } else {
            operation &= ~OperationFlags::CHARGE;
        }
    }
    
    /**
     * Enable climate
     */
    void enableClimate(bool enable, bool allowBattery = true) {
        if (enable) {
            operation |= OperationFlags::CLIMATE;
            if (allowBattery) {
                operation |= OperationFlags::CLIMATE_WITHOUT_SUPPLY;
            }
        } else {
            operation &= ~(OperationFlags::CLIMATE | OperationFlags::CLIMATE_WITHOUT_SUPPLY);
        }
    }
    
    /**
     * Set target SOC
     */
    void setTargetSoc(uint8_t soc) {
        targetChargeLevel = constrain(soc, SOC_MIN, SOC_MAX);
    }
    
    /**
     * Set max charging current
     */
    void setMaxCurrent(uint8_t amps) {
        maxCurrent = constrain(amps, 0, CURRENT_MAX);
    }
    
    /**
     * Set profile name
     */
    void setName(const char* newName) {
        strncpy(name, newName, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        nameLength = strlen(name);
    }
    
    /**
     * Clear profile to defaults
     */
    void clear() {
        operation = 0;
        operation2 = 0;
        maxCurrent = CURRENT_DEFAULT;
        minChargeLevel = 0;
        minRange = 0;
        targetChargeLevel = SOC_DEFAULT_TARGET;
        targetChargeDuration = 0;
        targetChargeRange = 0;
        unitRange = 0;
        rangeCalculationSetup = 0;
        temperatureRaw = 120;  // 22.0C
        temperatureUnit = 0;
        leadTime = 30;
        holdingTimePlug = 30;
        holdingTimeBattery = 10;
        providerDataId = 0;
        memset(name, 0, sizeof(name));
        nameLength = 0;
        valid = false;
        lastUpdate = 0;
    }
};

// =============================================================================
// Compact Profile Update (Record Address 6)
// =============================================================================

/**
 * Compact profile update structure
 * 
 * Used for quick partial updates to profiles (4 bytes).
 * This is what the OCU sends when changing operation mode.
 */
struct CompactProfileUpdate {
    uint8_t operation;           // Operation flags
    uint8_t operation2;          // Operation2 flags
    uint8_t maxCurrent;          // Max charging current
    uint8_t targetChargeLevel;   // Target SOC
    
    /**
     * Create from full profile
     */
    static CompactProfileUpdate fromProfile(const Profile& p) {
        return {
            p.operation,
            p.operation2,
            p.maxCurrent,
            p.targetChargeLevel
        };
    }
    
    /**
     * Apply to full profile
     */
    void applyTo(Profile& p) const {
        p.operation = operation;
        p.operation2 = operation2;
        p.maxCurrent = maxCurrent;
        p.targetChargeLevel = targetChargeLevel;
    }
};

// =============================================================================
// BAP Array Header for Profile Operations
// =============================================================================

/**
 * BAP Array header byte structure
 */
namespace ArrayHeader {
    // Byte 0: [ASG-ID:4][Transaction-ID:4]
    // Byte 1: [LargeIdx:1][PosTransmit:1][Backward:1][Shift:1][RecordAddr:4]
    // Byte 2: startIndex (or bytes 2-3 if LargeIdx=1)
    // Byte 3: elementCount (or bytes 4-5 if LargeIdx=1)
    
    constexpr uint8_t LARGE_IDX = 0x80;        // Use 16-bit indexes
    constexpr uint8_t POS_TRANSMIT = 0x40;     // Include position with each element
    constexpr uint8_t BACKWARD = 0x20;         // Array direction backward
    constexpr uint8_t SHIFT = 0x10;            // Shift array position
    
    // Record address values (lower 4 bits)
    constexpr uint8_t RECORD_ADDR_FULL = 0x00;     // Full profile format
    constexpr uint8_t RECORD_ADDR_COMPACT = 0x06;  // Compact 4-byte format
}

// =============================================================================
// Timer Configuration (for profiles 1-3)
// =============================================================================

/**
 * Timer schedule for departure-based charging/climate
 */
struct TimerSchedule {
    bool enabled = false;
    uint8_t hour = 7;            // Departure hour (0-23)
    uint8_t minute = 0;          // Departure minute (0-59)
    uint8_t weekdayMask = 0x1F;  // Bitmask: Mon=0x01, Tue=0x02, Wed=0x04, Thu=0x08, Fri=0x10, Sat=0x20, Sun=0x40
    
    bool isActiveOn(uint8_t weekday) const {
        // weekday: 0=Sunday, 1=Monday, etc.
        if (weekday == 0) weekday = 7;  // Convert to Mon=1 format
        return (weekdayMask & (1 << (weekday - 1))) != 0;
    }
    
    void setActiveOn(uint8_t weekday, bool active) {
        if (weekday == 0) weekday = 7;
        if (active) {
            weekdayMask |= (1 << (weekday - 1));
        } else {
            weekdayMask &= ~(1 << (weekday - 1));
        }
    }
    
    void setWeekdays() {
        weekdayMask = 0x1F;  // Mon-Fri
    }
    
    void setEveryday() {
        weekdayMask = 0x7F;  // All days
    }
    
    void setWeekend() {
        weekdayMask = 0x60;  // Sat-Sun
    }
};

} // namespace ChargingProfile
