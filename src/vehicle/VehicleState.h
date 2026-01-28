#pragma once

#include <Arduino.h>

/**
 * VehicleState - Centralized state for all decoded vehicle data
 * 
 * Unified state with automatic data source prioritization:
 * - BAP (extended CAN) takes precedence over standard CAN for duplicates
 * - Thread-safe access via mutex in VehicleManager
 * - All values include timestamps for staleness detection
 * - Source tracking for debugging and telemetry
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
 * Body domain state - doors, locks, trunk, windows
 */
struct BodyState {
    // Central lock state (from ZV_02 0x583)
    LockState centralLock = LockState::UNKNOWN;
    unsigned long centralLockUpdate = 0;
    
    // Individual doors
    DoorState driverDoor;       // Front left (FT)
    DoorState passengerDoor;    // Front right (BT)
    DoorState rearLeftDoor;     // Rear left (HFS)
    DoorState rearRightDoor;    // Rear right (HBFS)
    
    // Trunk/hatch
    bool trunkOpen = false;
    unsigned long trunkUpdate = 0;
    
    // Raw bytes from 0x583 for debugging
    uint8_t zv02_byte2 = 0;
    uint8_t zv02_byte7 = 0;
    
    bool isLocked() const {
        return centralLock == LockState::LOCKED;
    }
    
    bool isUnlocked() const {
        return centralLock == LockState::UNLOCKED;
    }
    
    bool anyDoorOpen() const {
        return driverDoor.open || passengerDoor.open || 
               rearLeftDoor.open || rearRightDoor.open || trunkOpen;
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
 * Drive domain state - ignition, speed, odometer
 */
struct DriveState {
    // Ignition from 0x3C0
    IgnitionState ignition = IgnitionState::OFF;
    bool keyInserted = false;       // ZAS_Kl_S
    bool ignitionOn = false;        // ZAS_Kl_15
    bool startRequested = false;    // ZAS_Kl_50
    unsigned long ignitionUpdate = 0;
    
    // Speed from 0x0FD
    float speedKmh = 0.0f;          // Current speed in km/h
    unsigned long speedUpdate = 0;
    
    // Odometer from 0x6B2
    uint32_t odometerKm = 0;        // Total km
    unsigned long odometerUpdate = 0;
    
    // Vehicle time from 0x6B2
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    unsigned long timeUpdate = 0;
    
    bool isOn() const {
        return ignitionOn;
    }
    
    bool isMoving() const {
        return speedKmh > 1.0f;
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
 * Battery domain state - SOC, voltage, current, charging
 * Consolidates data from CAN (BMS) and BAP (Battery Control)
 */
struct BatteryState {
    // === Primary SOC (BAP priority, CAN fallback) ===
    float soc = 0.0f;                   // State of charge 0-100% (unified field)
    DataSource socSource = DataSource::NONE;
    unsigned long socUpdate = 0;
    
    // === Charging Status (unified) ===
    bool charging = false;              // Is vehicle currently charging?
    DataSource chargingSource = DataSource::NONE;
    unsigned long chargingUpdate = 0;
    
    // Detailed charging info (from BAP when available) - populated later after enums defined
    uint8_t chargingMode = 0x0F;        // BapChargeMode (init=0x0F)
    uint8_t chargingStatus = 0x00;      // BapChargeStatus (init=0x00)
    uint8_t chargingAmps = 0;           // Current draw (A)
    uint8_t targetSoc = 0;              // Target SOC (%)
    uint8_t remainingTimeMin = 0;       // Minutes to target
    unsigned long chargingDetailsUpdate = 0;
    
    // === Battery Core (CAN only - not on comfort CAN, placeholders) ===
    float voltage = 0.0f;               // HV battery voltage (V) - NOT AVAILABLE
    float current = 0.0f;               // Battery current (A) - NOT AVAILABLE
    unsigned long voltageUpdate = 0;
    
    // === Energy (CAN BMS_07 0x5CA) ===
    float energyWh = 0.0f;              // Current energy content (Wh)
    float maxEnergyWh = 0.0f;           // Max energy content (Wh)
    unsigned long energyUpdate = 0;
    
    // === Temperature (CAN BMS_06 0x59E) ===
    float temperature = 0.0f;           // Battery temp (°C)
    unsigned long tempUpdate = 0;
    
    // === Power Meter (CAN Motor_Hybrid_06 0x483) ===
    float powerKw = 0.0f;               // Charging/climate power in kW
    unsigned long powerUpdate = 0;
    
    // === Cell Balancing (CAN BMS_07 0x5CA) ===
    bool balancingActive = false;       // Cell balancing in progress
    unsigned long balancingUpdate = 0;
    
    // === 12V System (NOT AVAILABLE - powertrain CAN only) ===
    float dcdc12v = 0.0f;               // 12V system voltage
    float dcdcCurrent = 0.0f;           // 12V system current
    unsigned long dcdcUpdate = 0;
    
    // === Helper methods ===
    bool isCharging() const {
        return charging;
    }
    
    bool hasSoc() const {
        return socSource != DataSource::NONE && soc > 0.0f;
    }
    
    float power() const {
        return voltage * current;  // Negative = discharge (NOT AVAILABLE)
    }
};

/**
 * Climate domain state
 * Consolidates data from CAN (Klima) and BAP (Climate Control)
 */
struct ClimateState {
    // === Interior Temperature (unified - CAN passive, BAP active) ===
    float insideTemp = 0.0f;            // Inside temperature (°C)
    DataSource insideTempSource = DataSource::NONE;
    unsigned long insideTempUpdate = 0;
    
    // === Exterior Temperature (CAN only) ===
    float outsideTemp = 0.0f;           // Outside temperature (°C)
    unsigned long outsideTempUpdate = 0;
    
    // === Standby Modes (CAN only 0x66E) ===
    bool standbyHeatingActive = false;  // Standby heating (not controlled via BAP)
    bool standbyVentActive = false;     // Standby ventilation
    unsigned long standbyUpdate = 0;
    
    // === Active Climate Control (BAP) ===
    bool climateActive = false;         // Active climate control running
    bool heating = false;               // Heating mode
    bool cooling = false;               // Cooling mode
    bool ventilation = false;           // Ventilation mode
    bool autoDefrost = false;           // Auto defrost enabled
    uint16_t climateTimeMin = 0;        // Remaining time (minutes)
    unsigned long climateActiveUpdate = 0;
    
    // Helper methods
    bool hasActiveClimate() const {
        return climateActive;
    }
    
    bool hasStandbyMode() const {
        return standbyHeatingActive || standbyVentActive;
    }
};

/**
 * GPS state (from CAN, if available - backup to modem GPS)
 */
struct CanGpsState {
    // From 0x486 (NavPos_01) - Position
    double latitude = 0.0;
    double longitude = 0.0;
    uint8_t satellites = 0;
    uint8_t fixType = 0;        // 0=none, 1=2D, 2=3D, 3=DGPS
    unsigned long positionUpdate = 0;
    
    // From 0x485 (NavData_02) - Altitude & Satellites
    float altitude = 0.0f;      // meters
    uint32_t utcTime = 0;       // Unix timestamp
    uint8_t satsInUse = 0;
    uint8_t satsInView = 0;
    uint8_t accuracy = 0;       // Horizontal accuracy in meters
    unsigned long altitudeUpdate = 0;
    
    // From 0x484 (NavData_01) - Heading & DOP
    float heading = 0.0f;       // degrees (0-359.9)
    float hdop = 0.0f;
    float vdop = 0.0f;
    float pdop = 0.0f;
    bool gpsInit = false;
    unsigned long headingUpdate = 0;
    
    bool hasFix() const {
        return fixType >= 2;  // 2D or better
    }
    
    bool isValid() const {
        return hasFix() && (millis() - positionUpdate) < 30000;
    }
    
    const char* fixTypeStr() const {
        switch (fixType) {
            case 0: return "None";
            case 1: return "2D";
            case 2: return "3D";
            case 3: return "DGPS";
            default: return "Unknown";
        }
    }
};

/**
 * Range estimation state (from instrument cluster)
 */
enum class RangeTendency : uint8_t {
    STABLE = 0,
    INCREASING = 1,
    DECREASING = 2,
    UNKNOWN = 3
};

struct RangeState {
    // From 0x5F5 (Reichweite_01) - Range data
    uint16_t totalRangeKm = 0;          // RW_Gesamt_Reichweite (0-2044 km, 2045-2047=invalid)
    uint16_t electricRangeKm = 0;       // RW_Primaer_Reichweite
    uint16_t maxDisplayRangeKm = 0;     // RW_Gesamt_Reichweite_Max_Anzeige
    float consumption = 0.0f;           // RW_Prim_Reichweitenverbrauch (kWh/100km or km/kWh)
    uint8_t consumptionUnit = 0;        // 0=kWh/100km, 1=km/kWh
    unsigned long reichweite01Update = 0;
    
    // From 0x5F7 (Reichweite_02) - Range display
    uint16_t displayRangeKm = 0;        // RW_Gesamt_Reichweite_Anzeige
    uint16_t displayElectricRangeKm = 0; // RW_Primaer_Reichweite_Anzeige
    RangeTendency tendency = RangeTendency::UNKNOWN;  // Range trend
    bool reserveWarning = false;        // Low range warning active
    bool displayInMiles = false;        // Display unit: false=km, true=miles
    unsigned long reichweite02Update = 0;
    
    // Invalid value marker (2045-2047 = 0x7FD-0x7FF)
    static constexpr uint16_t INVALID_RANGE = 2045;
    
    bool isValid() const {
        return totalRangeKm < INVALID_RANGE && electricRangeKm < INVALID_RANGE;
    }
    
    const char* tendencyStr() const {
        switch (tendency) {
            case RangeTendency::STABLE: return "stable";
            case RangeTendency::INCREASING: return "increasing";
            case RangeTendency::DECREASING: return "decreasing";
            default: return "unknown";
        }
    }
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
 * Combined vehicle state - Unified view of all vehicle data
 * 
 * Data source priority (for fields with multiple sources):
 * 1. BAP (extended CAN 29-bit) - most detailed and reliable
 * 2. Standard CAN (11-bit) - broadcast data, less detailed
 * 3. Computed - derived values
 */
struct VehicleState {
    // === Core Vehicle Domains ===
    BodyState body;         // Doors, locks, windows
    DriveState drive;       // Ignition, speed, odometer
    BatteryState battery;   // SOC, charging, energy (consolidated CAN+BAP)
    ClimateState climate;   // Temperature, HVAC (consolidated CAN+BAP)
    CanGpsState gps;        // GPS position from CAN
    RangeState range;       // Range estimation
    
    // === Plug State (BAP only) ===
    PlugState plug;         // Charging plug connection state
    
    // === Global Metadata ===
    unsigned long lastCanActivity = 0;
    uint32_t canFrameCount = 0;
    
    void markCanActivity() {
        lastCanActivity = millis();
        canFrameCount++;
    }
    
    bool isAwake() const {
        // Vehicle is awake if we've received CAN data recently
        return (millis() - lastCanActivity) < 5000;
    }
};
