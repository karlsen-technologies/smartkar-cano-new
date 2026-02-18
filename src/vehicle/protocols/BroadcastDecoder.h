#pragma once

#include <Arduino.h>

/**
 * BroadcastDecoder - Utility functions for extracting signals from CAN frames
 * 
 * VW CAN signals use DBC notation:
 *   start_bit|length@byte_order
 *   @1 = Intel/Little-endian (LSB first) - most common in VW
 *   @0 = Motorola/Big-endian (MSB first)
 * 
 * Formula: physical_value = raw_value * scale + offset
 */
namespace BroadcastDecoder {

/**
 * Extract a signal from CAN data (Intel/Little-endian byte order).
 * This is the most common format in VW CAN messages.
 * 
 * @param data Pointer to CAN frame data (8 bytes)
 * @param startBit Starting bit position (0-63)
 * @param length Number of bits (1-32)
 * @return Raw unsigned value
 * 
 * Example: BMS_SOC_HiRes at bits 47|11@LE
 *   extractSignalLE(data, 47, 11) returns raw SOC value
 *   Then multiply by 0.05 to get percentage
 */
inline uint32_t extractSignalLE(const uint8_t* data, uint8_t startBit, uint8_t length) {
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < length; i++) {
        uint8_t bitPos = startBit + i;
        uint8_t byteIdx = bitPos / 8;
        uint8_t bitIdx = bitPos % 8;
        
        if (byteIdx < 8) {
            if (data[byteIdx] & (1 << bitIdx)) {
                result |= (1UL << i);
            }
        }
    }
    
    return result;
}

/**
 * Extract a signal from CAN data (Motorola/Big-endian byte order).
 * Less common in VW, but some signals use this format.
 * 
 * @param data Pointer to CAN frame data (8 bytes)
 * @param startBit Starting bit position (MSB position in Motorola notation)
 * @param length Number of bits (1-32)
 * @return Raw unsigned value
 */
inline uint32_t extractSignalBE(const uint8_t* data, uint8_t startBit, uint8_t length) {
    // Motorola byte order: start bit is MSB, bits counted down across bytes
    uint32_t result = 0;
    
    uint8_t byteIdx = startBit / 8;
    uint8_t bitIdx = startBit % 8;
    
    for (uint8_t i = 0; i < length; i++) {
        if (byteIdx < 8) {
            if (data[byteIdx] & (1 << bitIdx)) {
                result |= (1UL << (length - 1 - i));
            }
        }
        
        // Move to next bit (going right/down in Motorola order)
        if (bitIdx == 0) {
            bitIdx = 7;
            byteIdx++;
        } else {
            bitIdx--;
        }
    }
    
    return result;
}

/**
 * Extract a single bit from CAN data.
 * 
 * @param data Pointer to CAN frame data
 * @param bitPos Bit position (0-63)
 * @return true if bit is set
 */
inline bool extractBit(const uint8_t* data, uint8_t bitPos) {
    uint8_t byteIdx = bitPos / 8;
    uint8_t bitIdx = bitPos % 8;
    
    if (byteIdx >= 8) return false;
    return (data[byteIdx] & (1 << bitIdx)) != 0;
}

/**
 * Extract a full byte from CAN data.
 * 
 * @param data Pointer to CAN frame data
 * @param byteIdx Byte index (0-7)
 * @return Byte value
 */
inline uint8_t extractByte(const uint8_t* data, uint8_t byteIdx) {
    if (byteIdx >= 8) return 0;
    return data[byteIdx];
}

/**
 * Extract a 16-bit word (little-endian) from CAN data.
 * 
 * @param data Pointer to CAN frame data
 * @param byteIdx Starting byte index (0-6)
 * @return 16-bit value
 */
inline uint16_t extractWord(const uint8_t* data, uint8_t byteIdx) {
    if (byteIdx >= 7) return 0;
    return data[byteIdx] | (data[byteIdx + 1] << 8);
}

/**
 * Apply scale and offset to convert raw value to physical value.
 * 
 * Formula: physical = raw * scale + offset
 */
inline float applyScaleOffset(uint32_t raw, float scale, float offset) {
    return (float)raw * scale + offset;
}

/**
 * Apply scale and offset for signed values.
 * Treats the raw value as signed based on bit length.
 */
inline float applyScaleOffsetSigned(uint32_t raw, uint8_t bits, float scale, float offset) {
    // Check if sign bit is set
    uint32_t signBit = 1UL << (bits - 1);
    int32_t signedRaw;
    
    if (raw & signBit) {
        // Negative: sign-extend
        signedRaw = (int32_t)(raw | (~0UL << bits));
    } else {
        signedRaw = (int32_t)raw;
    }
    
    return (float)signedRaw * scale + offset;
}

// ============================================================================
// VW-specific signal extraction helpers
// These match the signal definitions from the DBC files
// ============================================================================

/**
 * BMS_01 (0x191) - Core battery data
 */
struct BMS01Data {
    float current;      // BMS_IstStrom_02: A (negative = discharge)
    float voltage;      // BMS_IstSpannung: V
    float socHiRes;     // BMS_SOC_HiRes: %
};

inline BMS01Data decodeBMS01(const uint8_t* data) {
    BMS01Data result;
    
    // BMS_IstStrom_02: bits 12|12@LE, scale=1.0, offset=-2047 A
    uint32_t rawCurrent = extractSignalLE(data, 12, 12);
    result.current = applyScaleOffset(rawCurrent, 1.0f, -2047.0f);
    
    // BMS_IstSpannung: bits 24|12@LE, scale=0.25, offset=0 V
    uint32_t rawVoltage = extractSignalLE(data, 24, 12);
    result.voltage = applyScaleOffset(rawVoltage, 0.25f, 0.0f);
    
    // BMS_SOC_HiRes: bits 47|11@LE, scale=0.05, offset=0 %
    uint32_t rawSoc = extractSignalLE(data, 47, 11);
    result.socHiRes = applyScaleOffset(rawSoc, 0.05f, 0.0f);
    
    return result;
}

/**
 * BMS_10 (0x509) - Usable SOC and energy
 */
struct BMS10Data {
    float energyWh;         // BMS_Energieinhalt_HiRes: Wh
    float maxEnergyWh;      // BMS_MaxEnergieinhalt_HiRes: Wh
    float usableSoc;        // BMS_NutzbarerSOC: %
};

inline BMS10Data decodeBMS10(const uint8_t* data) {
    BMS10Data result;
    
    // BMS_Energieinhalt_HiRes: bits 0|15@LE, scale=4.0, offset=0 Wh
    uint32_t rawEnergy = extractSignalLE(data, 0, 15);
    result.energyWh = applyScaleOffset(rawEnergy, 4.0f, 0.0f);
    
    // BMS_MaxEnergieinhalt_HiRes: bits 15|15@LE, scale=4.0, offset=0 Wh
    uint32_t rawMaxEnergy = extractSignalLE(data, 15, 15);
    result.maxEnergyWh = applyScaleOffset(rawMaxEnergy, 4.0f, 0.0f);
    
    // BMS_NutzbarerSOC: bits 30|8@LE, scale=0.5, offset=0 %
    uint32_t rawSoc = extractSignalLE(data, 30, 8);
    result.usableSoc = applyScaleOffset(rawSoc, 0.5f, 0.0f);
    
    return result;
}

/**
 * BMS_07 (0x5CA) - Charging status
 */
struct BMS07Data {
    bool chargingActive;    // BMS_Ladevorgang_aktiv
    bool balancingActive;   // BMS_Balancing_Aktiv
    float energyWh;         // BMS_Energieinhalt (low-res)
    float maxEnergyWh;      // BMS_MaxEnergieinhalt (low-res)
};

inline BMS07Data decodeBMS07(const uint8_t* data) {
    BMS07Data result;
    
    // BMS_Ladevorgang_aktiv: bit 23
    result.chargingActive = extractBit(data, 23);
    
    // BMS_Balancing_Aktiv: bits 30|2@LE (0=off, 1-3=active)
    uint32_t rawBalancing = extractSignalLE(data, 30, 2);
    result.balancingActive = (rawBalancing > 0);
    
    // BMS_Energieinhalt: bits 12|11@LE, scale=50, offset=0 Wh
    uint32_t rawEnergy = extractSignalLE(data, 12, 11);
    result.energyWh = applyScaleOffset(rawEnergy, 50.0f, 0.0f);
    
    // BMS_MaxEnergieinhalt: bits 32|11@LE, scale=50, offset=0 Wh
    uint32_t rawMaxEnergy = extractSignalLE(data, 32, 11);
    result.maxEnergyWh = applyScaleOffset(rawMaxEnergy, 50.0f, 0.0f);
    
    return result;
}

/**
 * BMS_06 (0x59E) - Battery temperature
 */
inline float decodeBMS06Temperature(const uint8_t* data) {
    // BMS_Temperatur: bits 16|8@LE, scale=0.5, offset=-40 C
    uint32_t raw = extractSignalLE(data, 16, 8);
    return applyScaleOffset(raw, 0.5f, -40.0f);
}

/**
 * DCDC_01 (0x2AE) - DC-DC converter
 */
struct DCDC01Data {
    float hvVoltage;        // DC_IstSpannung_HV: V
    float lvVoltage;        // DC_IstSpannung_NV: V  
    float lvCurrent;        // DC_IstStrom_NV: A
};

inline DCDC01Data decodeDCDC01(const uint8_t* data) {
    DCDC01Data result;
    
    // DC_IstSpannung_HV: bits 12|12@LE, scale=0.25, offset=0 V
    uint32_t rawHV = extractSignalLE(data, 12, 12);
    result.hvVoltage = applyScaleOffset(rawHV, 0.25f, 0.0f);
    
    // DC_IstSpannung_NV: bits 56|8@LE, scale=0.1, offset=0 V
    uint32_t rawLV = extractSignalLE(data, 56, 8);
    result.lvVoltage = applyScaleOffset(rawLV, 0.1f, 0.0f);
    
    // DC_IstStrom_NV: bits 34|10@LE, scale=1.0, offset=-511 A
    uint32_t rawCurrent = extractSignalLE(data, 34, 10);
    result.lvCurrent = applyScaleOffset(rawCurrent, 1.0f, -511.0f);
    
    return result;
}

/**
 * Klemmen_Status_01 (0x3C0) - Ignition status
 */
struct IgnitionData {
    bool keyInserted;       // ZAS_Kl_S (bit 16)
    bool ignitionOn;        // ZAS_Kl_15 (bit 17)
    bool startRequested;    // ZAS_Kl_50 (bit 19)
};

inline IgnitionData decodeIgnition(const uint8_t* data) {
    IgnitionData result;
    
    result.keyInserted = extractBit(data, 16);      // ZAS_Kl_S
    result.ignitionOn = extractBit(data, 17);       // ZAS_Kl_15
    result.startRequested = extractBit(data, 19);   // ZAS_Kl_50
    
    return result;
}

/**
 * ESP_21 (0x0FD) - Vehicle speed
 */
inline float decodeSpeed(const uint8_t* data) {
    // ESP_v_Signal: bits 32|16@LE, scale=0.01, offset=0 km/h
    uint32_t raw = extractSignalLE(data, 32, 16);
    return applyScaleOffset(raw, 0.01f, 0.0f);
}

/**
 * Diagnose_01 (0x6B2) - Odometer and time
 */
struct DiagnoseData {
    uint32_t odometerKm;    // KBI_Kilometerstand (20 bits)
    uint16_t year;          // UH_Jahr
    uint8_t month;          // UH_Monat
    uint8_t day;            // UH_Tag
    uint8_t hour;           // UH_Stunde
    uint8_t minute;         // UH_Minute
    uint8_t second;         // UH_Sekunde
};

inline DiagnoseData decodeDiagnose(const uint8_t* data) {
    DiagnoseData result;
    
    // KBI_Kilometerstand: bits 8|20@LE
    result.odometerKm = extractSignalLE(data, 8, 20);
    
    // UH_Jahr: bits 28|7@LE, offset=2000
    result.year = extractSignalLE(data, 28, 7) + 2000;
    
    // UH_Monat: bits 35|4@LE
    result.month = extractSignalLE(data, 35, 4);
    
    // UH_Tag: bits 39|5@LE
    result.day = extractSignalLE(data, 39, 5);
    
    // UH_Stunde: bits 44|5@LE
    result.hour = extractSignalLE(data, 44, 5);
    
    // UH_Minute: bits 49|6@LE
    result.minute = extractSignalLE(data, 49, 6);
    
    // UH_Sekunde: bits 55|6@LE
    result.second = extractSignalLE(data, 55, 6);
    
    return result;
}

/**
 * TSG_FT_01 (0x3D0) - Driver door module
 */
struct DoorModuleData {
    bool doorOpen;          // Tuer_geoeffnet (bit 0)
    bool doorLocked;        // verriegelt (bit 1)
    uint8_t windowPos;      // FH_Oeffnung (byte 3, 0-200 = 0-100%)
};

inline DoorModuleData decodeDriverDoor(const uint8_t* data) {
    DoorModuleData result;
    
    result.doorOpen = extractBit(data, 0);
    result.doorLocked = extractBit(data, 1);
    result.windowPos = extractByte(data, 3);  // Window position in byte 3
    
    return result;
}

/**
 * TSG_BT_01 (0x3D1) - Passenger door module
 */
inline DoorModuleData decodePassengerDoor(const uint8_t* data) {
    DoorModuleData result;
    
    result.doorOpen = extractBit(data, 0);
    result.doorLocked = extractBit(data, 1);
    result.windowPos = extractByte(data, 3);
    
    return result;
}

/**
 * ZV_02 (0x583) - Central locking status
 * 
 * Note: The exact mapping differs from DBC. Based on observed traces:
 * - Locked: byte 2 = 0x0A or 0x08, byte 7 = 0x80
 * - Unlocked: byte 2 = 0x80, byte 7 = 0x40
 */
struct LockStatusData {
    uint8_t byte2;          // Lock state byte
    uint8_t byte7;          // Additional lock state
    bool isLocked;          // Interpreted lock state
};

inline LockStatusData decodeLockStatus(const uint8_t* data) {
    LockStatusData result;
    
    result.byte2 = extractByte(data, 2);
    result.byte7 = extractByte(data, 7);
    
    // Interpret based on observed patterns
    // Locked: byte2 low nibble is 0x0A/0x08, byte7 = 0x80
    // Unlocked: byte2 = 0x80, byte7 = 0x40
    if ((result.byte2 & 0x0F) == 0x0A || (result.byte2 & 0x0F) == 0x08) {
        result.isLocked = true;
    } else if (result.byte2 == 0x80 && result.byte7 == 0x40) {
        result.isLocked = false;
    } else {
        // Unknown state - keep previous (default to locked for safety)
        result.isLocked = (result.byte7 == 0x80);
    }
    
    return result;
}

/**
 * Klima_03 (0x66E) - Climate status
 */
struct KlimaData {
    float insideTemp;           // KL_Innen_Temp: C
    bool standbyHeatingActive;  // KL_STH_aktiv
    bool standbyVentActive;     // KL_STL_aktiv
};

inline KlimaData decodeKlima03(const uint8_t* data) {
    KlimaData result;
    
    // KL_Innen_Temp: bits 32|8@LE, scale=0.5, offset=-50 C
    uint32_t rawTemp = extractSignalLE(data, 32, 8);
    result.insideTemp = applyScaleOffset(rawTemp, 0.5f, -50.0f);
    
    // KL_STL_aktiv: bit 0
    result.standbyVentActive = extractBit(data, 0);
    
    // KL_STH_aktiv: bit 1  
    result.standbyHeatingActive = extractBit(data, 1);
    
    return result;
}

// ============================================================================
// GPS signal extraction (from infotainment via gateway)
// ============================================================================

/**
 * NavPos_01 (0x486) - GPS Position
 */
struct NavPosData {
    double latitude;        // NP_LatDegree: degrees
    double longitude;       // NP_LongDegree: degrees
    bool latSouth;          // NP_LatDirection: 0=North, 1=South
    bool longWest;          // NP_LongDirection: 0=East, 1=West
    uint8_t satellites;     // NP_Sat: count
    uint8_t fixType;        // NP_Fix: 0=none, 1=2D, 2=3D, 3=DGPS
};

inline NavPosData decodeNavPos01(const uint8_t* data) {
    NavPosData result;
    
    // NP_LatDegree: bits 0-26 (27 bits), scale 0.000001°
    uint32_t rawLat = extractSignalLE(data, 0, 27);
    result.latitude = rawLat * 0.000001;
    
    // NP_LongDegree: bits 27-54 (28 bits), scale 0.000001°
    uint32_t rawLong = extractSignalLE(data, 27, 28);
    result.longitude = rawLong * 0.000001;
    
    // NP_LatDirection: bit 55 (0=North, 1=South)
    result.latSouth = extractBit(data, 55);
    
    // NP_LongDirection: bit 56 (0=East, 1=West)
    result.longWest = extractBit(data, 56);
    
    // NP_Sat: bits 57-61 (5 bits)
    result.satellites = extractSignalLE(data, 57, 5);
    
    // NP_Fix: bits 62-63 (2 bits)
    result.fixType = extractSignalLE(data, 62, 2);
    
    // Apply direction signs
    if (result.latSouth) result.latitude = -result.latitude;
    if (result.longWest) result.longitude = -result.longitude;
    
    return result;
}

/**
 * NavData_02 (0x485) - Altitude, UTC, Satellites
 */
struct NavData02Data {
    uint8_t satsInUse;      // ND_SatInUse: satellites in fix
    uint8_t satsInView;     // ND_SatInView: satellites visible
    float altitude;         // NP_Altitude: meters
    uint32_t utcTime;       // ND_UTC: Unix timestamp
    bool accuracyOK;        // Accuracy_OK flag
    uint8_t accuracy;       // Horizontal accuracy in meters
};

inline NavData02Data decodeNavData02(const uint8_t* data) {
    NavData02Data result;
    
    // ND_SatInUse: bits 0-4 (5 bits)
    result.satsInUse = extractSignalLE(data, 0, 5);
    
    // Accuracy_OK: bit 5
    result.accuracyOK = extractBit(data, 5);
    
    // ND_SatInView: bits 8-12 (5 bits)
    result.satsInView = extractSignalLE(data, 8, 5);
    
    // Accuracy: bits 13-19 (7 bits), scale 2m
    result.accuracy = extractSignalLE(data, 13, 7) * 2;
    
    // NP_Altitude: bits 20-31 (12 bits), scale 2, offset -500m
    uint32_t rawAlt = extractSignalLE(data, 20, 12);
    result.altitude = applyScaleOffset(rawAlt, 2.0f, -500.0f);
    
    // ND_UTC: bits 32-63 (32 bits) - Unix timestamp
    result.utcTime = extractSignalLE(data, 32, 32);
    
    return result;
}

/**
 * NavData_01 (0x484) - Heading and DOP values
 */
struct NavData01Data {
    float vdop;             // ND_VDOP: Vertical DOP
    float tdop;             // ND_TDOP: Time DOP
    float hdop;             // ND_HDOP: Horizontal DOP
    float gdop;             // ND_GDOP: Geometric DOP
    float pdop;             // ND_PDOP: Position DOP
    float heading;          // ND_Heading: degrees (0-359.9)
    bool gpsInit;           // ND_Init: GPS initialized
};

inline NavData01Data decodeNavData01(const uint8_t* data) {
    NavData01Data result;
    
    // ND_VDOP: bits 0-9 (10 bits), scale 0.025
    result.vdop = extractSignalLE(data, 0, 10) * 0.025f;
    
    // ND_TDOP: bits 10-19 (10 bits), scale 0.025
    result.tdop = extractSignalLE(data, 10, 10) * 0.025f;
    
    // ND_HDOP: bits 20-29 (10 bits), scale 0.025
    result.hdop = extractSignalLE(data, 20, 10) * 0.025f;
    
    // ND_GDOP: bits 30-39 (10 bits), scale 0.025
    result.gdop = extractSignalLE(data, 30, 10) * 0.025f;
    
    // ND_PDOP: bits 40-49 (10 bits), scale 0.025
    result.pdop = extractSignalLE(data, 40, 10) * 0.025f;
    
    // ND_Heading: bits 50-61 (12 bits), scale 0.1°
    result.heading = extractSignalLE(data, 50, 12) * 0.1f;
    
    // ND_Init: bit 62
    result.gpsInit = extractBit(data, 62);
    
    return result;
}

// ============================================================================
// Range estimation signals (Reichweite)
// ============================================================================

/**
 * Reichweite_01 (0x5F5) - Range data from instrument cluster
 */
struct Reichweite01Data {
    uint16_t maxDisplayRange;   // RW_Gesamt_Reichweite_Max_Anzeige (km)
    uint16_t totalRange;        // RW_Gesamt_Reichweite (km)
    uint16_t electricRange;     // RW_Primaer_Reichweite (km)
    float consumption;          // RW_Prim_Reichweitenverbrauch
    uint8_t consumptionUnit;    // 0=kWh/100km, 1=km/kWh
    uint8_t reserveWarning2;    // RW_Reservewarnung_2_aktiv
};

inline Reichweite01Data decodeReichweite01(const uint8_t* data) {
    Reichweite01Data result;
    
    // RW_Gesamt_Reichweite_Max_Anzeige: bits 0-10 (11 bits), scale 1 km
    result.maxDisplayRange = extractSignalLE(data, 0, 11);
    
    // RW_Reservewarnung_2_aktiv: bits 16-17 (2 bits)
    result.reserveWarning2 = extractSignalLE(data, 16, 2);
    
    // RW_Gesamt_Reichweite: bits 29-39 (11 bits), scale 1 km
    result.totalRange = extractSignalLE(data, 29, 11);
    
    // RW_Prim_Reichweitenverbrauch: bits 40-50 (11 bits), scale 0.1
    result.consumption = extractSignalLE(data, 40, 11) * 0.1f;
    
    // RW_Prim_Reichweitenv_Einheit: bits 51-52 (2 bits)
    result.consumptionUnit = extractSignalLE(data, 51, 2);
    
    // RW_Primaer_Reichweite: bits 53-63 (11 bits), scale 1 km
    result.electricRange = extractSignalLE(data, 53, 11);
    
    return result;
}

/**
 * Reichweite_02 (0x5F7) - Range display data
 */
struct Reichweite02Data {
    uint8_t tendency;           // RW_Tendenz: 0=stable, 1=increasing, 2=decreasing
    uint8_t textIndex;          // RW_Texte
    bool reserveWarning;        // RW_Reservewarnung_aktiv
    bool displayInMiles;        // RW_Reichweite_Einheit_Anzeige: 0=km, 1=miles
    uint16_t displayTotalRange; // RW_Gesamt_Reichweite_Anzeige (km)
    uint16_t displayElectricRange; // RW_Primaer_Reichweite_Anzeige (km)
    uint16_t displaySecondaryRange; // RW_Sekundaer_Reichweite_Anzeige (N/A for BEV)
};

inline Reichweite02Data decodeReichweite02(const uint8_t* data) {
    Reichweite02Data result;
    
    // RW_Tendenz: bits 0-2 (3 bits)
    result.tendency = extractSignalLE(data, 0, 3);
    
    // RW_Texte: bits 3-4 (2 bits)
    result.textIndex = extractSignalLE(data, 3, 2);
    
    // RW_Reservewarnung_aktiv: bit 5
    result.reserveWarning = extractBit(data, 5);
    
    // RW_Reichweite_Einheit_Anzeige: bit 6
    result.displayInMiles = extractBit(data, 6);
    
    // RW_Gesamt_Reichweite_Anzeige: bits 7-17 (11 bits), scale 1 km
    result.displayTotalRange = extractSignalLE(data, 7, 11);
    
    // RW_Primaer_Reichweite_Anzeige: bits 18-28 (11 bits), scale 1 km
    result.displayElectricRange = extractSignalLE(data, 18, 11);
    
    // RW_Sekundaer_Reichweite_Anzeige: bits 29-39 (11 bits), scale 1 km
    result.displaySecondaryRange = extractSignalLE(data, 29, 11);
    
    return result;
}

// ============================================================================
// Motor/Hybrid signals - Power meter
// ============================================================================

/**
 * Motor_Hybrid_06 (0x483) - Power meter limits
 * 
 * Contains the powermeter signal that shows charging power or climate power.
 * This message is forwarded from powertrain CAN to comfort CAN via gateway.
 * 
 * MO_Powermeter_Charge_Grenze at bits 18|10@LE gives power in ~10W units.
 * - During AC charging: actual charging power from grid (741 ≈ 7.4kW)
 * - During climate: HVAC power consumption (up to ~10kW for cold start)
 */
struct MotorHybrid06Data {
    uint16_t powermeterGrenze;      // Mo_Powermeter_Grenze: general limit
    uint16_t chargeGrenze;          // MO_Powermeter_Charge_Grenze: charge/climate power
    uint16_t strategicLimit;        // MO_Powermeter_Grenze_strategisch
    float powerKw;                  // Converted to kW (~10W per unit)
};

inline MotorHybrid06Data decodeMotorHybrid06(const uint8_t* data) {
    MotorHybrid06Data result;
    
    // Mo_Powermeter_Grenze: bits 0|12@LE, scale=1
    result.powermeterGrenze = extractSignalLE(data, 0, 12);
    
    // MO_Powermeter_Charge_Grenze: bits 18|10@LE, scale=1
    // This is the key signal for charging/climate power
    result.chargeGrenze = extractSignalLE(data, 18, 10);
    
    // MO_Powermeter_Grenze_strategisch: bits 28|12@LE, scale=1
    result.strategicLimit = extractSignalLE(data, 28, 12);
    
    // Convert to kW: approximately 10W per unit
    // 741 units ≈ 7.4kW (matches e-Golf's 7.2kW AC charging max)
    result.powerKw = result.chargeGrenze * 0.01f;
    
    return result;
}

} // namespace BroadcastDecoder
