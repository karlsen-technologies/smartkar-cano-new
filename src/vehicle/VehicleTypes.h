#pragma once

#include <Arduino.h>

/**
 * VehicleTypes - Common type definitions used across vehicle domains
 * 
 * This file contains enums and small structs that are shared between
 * multiple domain managers. Each domain manager has its own State struct.
 */

/**
 * Data source enumeration - tracks where data originated
 */
enum class DataSource : uint8_t {
    NONE = 0,       // No data received yet
    CAN_STD = 1,    // Standard 11-bit CAN
    BAP = 2,        // BAP extended CAN (29-bit)
    COMPUTED = 3    // Derived/calculated value
};

/**
 * Lock state enumeration
 */
enum class LockState : uint8_t {
    UNKNOWN = 0,
    LOCKED = 1,
    UNLOCKED = 2
};

/**
 * Door state (individual door)
 */
struct DoorState {
    bool open = false;              // Door physically open
    bool locked = false;            // Door lock engaged
    uint8_t windowPosition = 0;     // Window position 0-200 (0.5% scale, 0=closed, 200=fully open)
    unsigned long lastUpdate = 0;   // Timestamp of last update
    
    bool isStale(unsigned long timeout = 10000) const {
        return (millis() - lastUpdate) > timeout;
    }
    
    float windowPercent() const {
        return windowPosition * 0.5f;  // Convert to 0-100%
    }
};

/**
 * Ignition/terminal state
 */
enum class IgnitionState : uint8_t {
    OFF = 0,            // All off
    ACCESSORY = 1,      // Key inserted (Kl. S)
    ON = 2,             // Ignition on (Kl. 15)
    START = 3           // Start requested (Kl. 50)
};

/**
 * BAP Plug State (from BAP function 0x10)
 * Separate state since plug connection is independent of battery/charging
 */
struct PlugState {
    uint8_t lockSetup = 0;
    uint8_t lockState = 0;         // 0=unlocked, 1=locked, 2=error
    uint8_t supplyState = 0x0F;    // 0=inactive, 1=active, 2=station connected, F=init
    uint8_t plugState = 0x0F;      // 0=unplugged, 1=plugged, F=init
    unsigned long lastUpdate = 0;
    
    bool isPlugged() const { return plugState == 0x01; }
    bool hasSupply() const { return supplyState == 0x01 || supplyState == 0x02; }
    bool isValid() const { return plugState != 0x0F; }
    
    const char* plugStateStr() const {
        switch (plugState) {
            case 0x00: return "unplugged";
            case 0x01: return "plugged";
            default: return "unknown";
        }
    }
};

/**
 * BAP Charge Mode enumeration (from BAP function 0x11)
 */
enum class BapChargeMode : uint8_t {
    OFF = 0x0,
    AC = 0x1,
    DC = 0x2,
    CONDITIONING = 0x3,
    AC_AND_CONDITIONING = 0x4,
    DC_AND_CONDITIONING = 0x5,
    INIT = 0xF
};

/**
 * BAP Charge Status enumeration (from BAP function 0x11)
 */
enum class BapChargeStatus : uint8_t {
    INIT = 0x0,
    IDLE = 0x1,
    RUNNING = 0x2,
    CONSERVATION = 0x3,
    ABORTED_TEMP_LOW = 0x4,
    ABORTED_DEVICE_ERROR = 0x5,
    ABORTED_NO_POWER = 0x6,
    ABORTED_NOT_IN_PARK = 0x7,
    COMPLETED = 0x8,
    NO_ERROR = 0x9
};

/**
 * Range estimation tendency
 */
enum class RangeTendency : uint8_t {
    STABLE = 0,
    INCREASING = 1,
    DECREASING = 2,
    UNKNOWN = 3
};

/**
 * Lightweight structs for BAP callbacks
 * These are minimal data structures passed in callbacks from BatteryControlChannel
 */

/**
 * Battery state for BAP callbacks (from function 0x11)
 */
struct BatteryState {
    float soc = 0.0f;
    DataSource socSource = DataSource::NONE;
    unsigned long socUpdate = 0;
    uint8_t chargingMode = 0;
    uint8_t chargingStatus = 0;
    uint8_t chargingAmps = 0;
    uint8_t targetSoc = 0;
    uint8_t remainingTimeMin = 0;
    bool charging = false;
    DataSource chargingSource = DataSource::NONE;
    unsigned long chargingUpdate = 0;
    unsigned long chargingDetailsUpdate = 0;
};

/**
 * Climate state for BAP callbacks (from function 0x12)
 */
struct ClimateState {
    bool climateActive = false;
    bool heating = false;
    bool cooling = false;
    bool ventilation = false;
    bool autoDefrost = false;
    float insideTemp = 0.0f;
    uint16_t climateTimeMin = 0;
    DataSource climateActiveSource = DataSource::NONE;
    DataSource insideTempSource = DataSource::NONE;
    unsigned long climateActiveUpdate = 0;
    unsigned long insideTempUpdate = 0;
};
