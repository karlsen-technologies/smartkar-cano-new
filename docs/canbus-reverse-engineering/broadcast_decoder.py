"""
VW e-Golf Broadcast Signal Decoder

This module decodes standard CAN broadcast signals (non-BAP) from the comfort CAN bus.
These are periodic status messages that provide real-time vehicle state.

Usage:
    from broadcast_decoder import BroadcastDecoder
    
    decoder = BroadcastDecoder()
    
    # Process incoming CAN message
    result = decoder.decode(can_id, data_bytes)
    if result:
        print(f"Message type: {result['type']}")
        print(f"Data: {result['data']}")
    
    # Or decode specific signals
    lock_state = decoder.decode_lock_status(data)
    temp = decoder.decode_outside_temp(data)
"""

from dataclasses import dataclass
from typing import List, Dict, Any, Optional
from enum import IntEnum


# =============================================================================
# CAN IDs for Broadcast Messages
# =============================================================================

class BroadcastCANIds:
    """Standard CAN IDs (11-bit) for broadcast messages on comfort bus.
    
    All IDs verified present in ignthenprofilethenoff.csv trace.
    """
    
    # Security/Doors
    ZV_02 = 0x583                # Lock status + all doors
    TSG_FT_01 = 0x3D0            # Driver door/window module
    TSG_BT_01 = 0x3D1            # Passenger door/window module
    
    # Climate/Temperature
    KLIMA_SENSOR_02 = 0x5E1      # Outside temperature
    CLIMATE_SETTINGS = 0x594     # Target temp, departure timer
    THERMO_VERBRAUCH_01 = 0x3E9  # Thermal consumption
    SEAT_WINDOW_HEAT = 0x588     # Seat/window heating status
    KLIMA_01 = 0x668             # Blower voltage, seat heating
    KLIMA_02 = 0x66E             # Climate status, cabin temp
    
    # BMS (limited data available on comfort bus)
    BMS_06 = 0x59E               # Battery temperature
    BMS_07 = 0x5CA               # Energy content, charging active
    
    # Vehicle Status
    KLEMMEN_STATUS_01 = 0x3C0    # Ignition state, key status
    DIAGNOSE_01 = 0x6B2          # Odometer, date/time
    
    # GPS/Navigation (standard CAN)
    NAV_DATA_01 = 0x484          # Heading, DOP values
    NAV_DATA_02 = 0x485          # Altitude, UTC, satellites
    NAV_POS_01 = 0x486           # Latitude, Longitude, Fix
    
    # Commands (TM_01)
    TM_01 = 0x5A7                # Horn/Flash/Lock commands


class ExtendedCANIds:
    """Extended CAN IDs (29-bit) for broadcast messages."""
    
    # GPS/Navigation (extended CAN)
    NAV_POS_02_MAP_MATCHED = 0x1A5554A8  # Map-matched position


# =============================================================================
# Enums for decoded values
# =============================================================================

class LockState(IntEnum):
    """Vehicle lock state."""
    UNKNOWN = 0
    LOCKED = 1
    UNLOCKED = 2


class DoorState(IntEnum):
    """Door open/closed state."""
    CLOSED = 0
    OPEN = 1


class IgnitionState(IntEnum):
    """Ignition/terminal states."""
    OFF = 0
    ACCESSORY = 1
    ON = 2
    START = 3


class GPSFixType(IntEnum):
    """GPS fix type."""
    NO_FIX = 0
    FIX_2D = 1
    FIX_3D = 2
    DGPS = 3


# =============================================================================
# Data Classes for Decoded Messages
# =============================================================================

@dataclass
class LockStatus:
    """Decoded lock and door status from ZV_02 (0x583)."""
    lock_state: LockState
    driver_door_open: bool
    passenger_door_open: bool
    rear_left_door_open: bool
    rear_right_door_open: bool
    trunk_open: bool
    raw_bytes: List[int]
    
    def all_doors_closed(self) -> bool:
        return not any([
            self.driver_door_open,
            self.passenger_door_open,
            self.rear_left_door_open,
            self.rear_right_door_open,
            self.trunk_open
        ])


@dataclass
class DoorModuleStatus:
    """Decoded door module status from TSG_FT/BT_01."""
    door_open: bool
    door_locked: bool
    window_position_pct: float  # 0 = closed, 100 = fully open
    raw_bytes: List[int]


@dataclass
class TemperatureData:
    """Decoded temperature values."""
    outside_temp_c: float
    raw_value: int


@dataclass
class ClimateSettings:
    """Decoded climate control settings from 0x594."""
    target_temp_c: float
    departure_timer_active: bool
    raw_bytes: List[int]


@dataclass
class SeatHeatingStatus:
    """Decoded seat/window heating from 0x588."""
    driver_seat_heat_level: int      # 0-3
    passenger_seat_heat_level: int   # 0-3
    front_window_heat: bool
    rear_window_heat: bool
    raw_bytes: List[int]


@dataclass
class IgnitionStatus:
    """Decoded ignition state from Klemmen_Status_01 (0x3C0)."""
    key_inserted: bool
    ignition_on: bool  # Terminal 15
    raw_bytes: List[int]


@dataclass
class OdometerData:
    """Decoded odometer and time from Diagnose_01 (0x6B2)."""
    odometer_km: int
    year: int
    month: int
    day: int
    hour: int
    minute: int
    raw_bytes: List[int]


@dataclass
class BMSStatus:
    """Decoded BMS data from BMS_07 (0x5CA).
    
    Note: This is limited BMS data available on comfort bus.
    Full BMS data (voltage, current, detailed SOC) is on powertrain CAN.
    """
    energy_content_wh: int        # Current energy in Wh
    max_energy_content_wh: int    # Max capacity in Wh
    charging_active: bool         # Charging in progress
    battery_diagnosis: int        # 0-7 diagnosis code
    raw_bytes: List[int]
    
    @property
    def soc_percent(self) -> float:
        """Estimated SOC from energy content."""
        if self.max_energy_content_wh > 0:
            return (self.energy_content_wh / self.max_energy_content_wh) * 100
        return 0.0


@dataclass
class BMSTemperature:
    """Decoded BMS temperature from BMS_06 (0x59E)."""
    battery_temp_c: float         # Main battery temperature
    coolant_inlet_temp_c: float   # Coolant inlet (Vorlauf)
    coolant_outlet_temp_c: float  # Coolant outlet (Rücklauf)
    raw_bytes: List[int]


@dataclass
class ClimateStatus:
    """Decoded climate status from Klima_02 (0x66E)."""
    standby_heating_active: bool  # STH active
    standby_vent_active: bool     # STL active
    blower_current_a: float       # Blower current in amps
    cabin_temp_c: Optional[float] # Cabin temperature if available
    raw_bytes: List[int]


@dataclass
class GPSPosition:
    """Decoded GPS position from NavPos_01 (0x486)."""
    latitude: float           # Degrees (negative = South)
    longitude: float          # Degrees (negative = West)
    satellites: int           # Number of satellites
    fix_type: GPSFixType      # Fix quality
    raw_bytes: List[int]
    
    @property
    def has_fix(self) -> bool:
        return self.fix_type != GPSFixType.NO_FIX
    
    @property
    def coordinates(self) -> tuple:
        """Return (lat, lon) tuple."""
        return (self.latitude, self.longitude)


@dataclass
class GPSData:
    """Decoded GPS metadata from NavData_02 (0x485)."""
    satellites_in_use: int
    satellites_in_view: int
    altitude_m: float         # Meters above sea level
    horizontal_accuracy_m: float
    accuracy_available: bool
    utc_timestamp: int        # Unix timestamp
    raw_bytes: List[int]


@dataclass
class GPSHeading:
    """Decoded GPS heading/DOP from NavData_01 (0x484)."""
    heading_deg: float        # 0-359.9 degrees
    hdop: float               # Horizontal dilution of precision
    vdop: float               # Vertical dilution of precision
    pdop: float               # Position dilution of precision
    initialized: bool
    raw_bytes: List[int]


# =============================================================================
# Main Decoder Class
# =============================================================================

class BroadcastDecoder:
    """
    Decode broadcast CAN messages from VW e-Golf comfort bus.
    
    This handles standard 11-bit CAN IDs that broadcast periodic status.
    """
    
    def decode(self, can_id: int, data: List[int]) -> Optional[Dict[str, Any]]:
        """
        Decode a CAN message and return structured data.
        
        Returns None if the CAN ID is not recognized.
        Returns dict with 'type' and 'data' keys on success.
        """
        handlers = {
            BroadcastCANIds.ZV_02: ('LockStatus', self.decode_lock_status),
            BroadcastCANIds.TSG_FT_01: ('DriverDoor', self.decode_door_module),
            BroadcastCANIds.TSG_BT_01: ('PassengerDoor', self.decode_door_module),
            BroadcastCANIds.KLIMA_SENSOR_02: ('OutsideTemp', self.decode_outside_temp),
            BroadcastCANIds.CLIMATE_SETTINGS: ('ClimateSettings', self.decode_climate_settings),
            BroadcastCANIds.SEAT_WINDOW_HEAT: ('SeatHeating', self.decode_seat_heating),
            BroadcastCANIds.KLEMMEN_STATUS_01: ('Ignition', self.decode_ignition),
            BroadcastCANIds.DIAGNOSE_01: ('Odometer', self.decode_odometer),
            # BMS signals (limited data on comfort bus)
            BroadcastCANIds.BMS_06: ('BMSTemperature', self.decode_bms_temperature),
            BroadcastCANIds.BMS_07: ('BMSStatus', self.decode_bms_status),
            # Climate signals
            BroadcastCANIds.KLIMA_02: ('ClimateStatus', self.decode_climate_status),
            # GPS signals
            BroadcastCANIds.NAV_DATA_01: ('GPSHeading', self.decode_gps_heading),
            BroadcastCANIds.NAV_DATA_02: ('GPSData', self.decode_gps_data),
            BroadcastCANIds.NAV_POS_01: ('GPSPosition', self.decode_gps_position),
            # Extended CAN GPS
            ExtendedCANIds.NAV_POS_02_MAP_MATCHED: ('GPSMapMatched', self.decode_gps_map_matched),
        }
        
        if can_id not in handlers:
            return None
        
        msg_type, handler = handlers[can_id]
        try:
            decoded = handler(data)
            return {
                'can_id': can_id,
                'can_id_hex': f'0x{can_id:03X}',
                'type': msg_type,
                'data': decoded
            }
        except Exception as e:
            return {
                'can_id': can_id,
                'can_id_hex': f'0x{can_id:03X}',
                'type': msg_type,
                'error': str(e),
                'raw': data
            }
    
    # -------------------------------------------------------------------------
    # Individual Signal Decoders
    # -------------------------------------------------------------------------
    
    def decode_lock_status(self, data: List[int]) -> LockStatus:
        """
        Decode ZV_02 (0x583) lock and door status.
        
        Confirmed patterns from trace analysis:
        - Locked: byte 2 = 0x0A or 0x08, byte 7 = 0x80
        - Unlocked: byte 2 = 0x80, byte 7 = 0x40
        """
        if len(data) < 8:
            raise ValueError(f"ZV_02 requires 8 bytes, got {len(data)}")
        
        # Determine lock state from observed patterns
        byte2 = data[2]
        byte7 = data[7]
        
        if byte2 in (0x0A, 0x08) or byte7 == 0x80:
            lock_state = LockState.LOCKED
        elif byte2 == 0x80 or byte7 == 0x40:
            lock_state = LockState.UNLOCKED
        else:
            lock_state = LockState.UNKNOWN
        
        # Door states from DBC (may need adjustment based on actual e-Golf)
        # ZV_FT_offen, ZV_BT_offen, ZV_HFS_offen, ZV_HBFS_offen, ZV_HD_offen
        # These are typically in byte 1 and byte 3
        driver_open = bool(data[1] & 0x01)          # ZV_FT_offen (front left/driver)
        passenger_open = bool(data[1] & 0x02)       # ZV_BT_offen (front right/passenger)
        rear_left_open = bool(data[1] & 0x04)       # ZV_HFS_offen (rear left)
        rear_right_open = bool(data[1] & 0x08)      # ZV_HBFS_offen (rear right)
        trunk_open = bool(data[3] & 0x01)           # ZV_HD_offen (trunk/hatch)
        
        return LockStatus(
            lock_state=lock_state,
            driver_door_open=driver_open,
            passenger_door_open=passenger_open,
            rear_left_door_open=rear_left_open,
            rear_right_door_open=rear_right_open,
            trunk_open=trunk_open,
            raw_bytes=data
        )
    
    def decode_door_module(self, data: List[int]) -> DoorModuleStatus:
        """
        Decode TSG_FT_01 (0x3D0) or TSG_BT_01 (0x3D1) door module.
        
        Signals:
        - Bit 0: Door open (1=open)
        - Bit 1: Door locked
        - Byte 3: Window position (0-200 scale, 0.5% per step)
        """
        if len(data) < 4:
            raise ValueError(f"TSG message requires at least 4 bytes, got {len(data)}")
        
        door_open = bool(data[0] & 0x01)
        door_locked = bool(data[0] & 0x02)
        
        # Window position is 0-200 (representing 0-100% at 0.5% resolution)
        window_raw = data[3] if len(data) > 3 else 0
        window_pct = window_raw * 0.5
        
        return DoorModuleStatus(
            door_open=door_open,
            door_locked=door_locked,
            window_position_pct=window_pct,
            raw_bytes=data
        )
    
    def decode_outside_temp(self, data: List[int]) -> TemperatureData:
        """
        Decode Klima_Sensor_02 (0x5E1) outside temperature.
        
        Signal: BCM1_Aussen_Temp_ungef
        Location: Byte 0, 8 bits
        Formula: temp_celsius = raw_value * 0.5 - 50
        """
        if len(data) < 1:
            raise ValueError("Temperature message requires at least 1 byte")
        
        raw = data[0]
        temp_c = raw * 0.5 - 50
        
        return TemperatureData(
            outside_temp_c=temp_c,
            raw_value=raw
        )
    
    def decode_climate_settings(self, data: List[int]) -> ClimateSettings:
        """
        Decode climate settings (0x594).
        
        Target temp formula: temp_celsius = raw_value * 0.5 + 15.5
        Departure timer active: Byte 2 bit 6
        """
        if len(data) < 3:
            raise ValueError("Climate settings requires at least 3 bytes")
        
        # Temperature is typically in byte 0 or 1 (need to verify)
        temp_raw = data[0]
        target_temp = temp_raw * 0.5 + 15.5
        
        # Departure timer active flag
        departure_active = bool(data[2] & 0x40)
        
        return ClimateSettings(
            target_temp_c=target_temp,
            departure_timer_active=departure_active,
            raw_bytes=data
        )
    
    def decode_seat_heating(self, data: List[int]) -> SeatHeatingStatus:
        """
        Decode seat/window heating (0x588).
        
        Byte 1 bits 7-8: Passenger seat heat (0-3)
        Byte 1 bits 5-6: Driver seat heat (0-3)
        Byte 1 bit 1: Front window heat
        Byte 1 bit 2: Rear window heat
        """
        if len(data) < 2:
            raise ValueError("Seat heating requires at least 2 bytes")
        
        byte1 = data[1]
        
        passenger_heat = (byte1 >> 6) & 0x03
        driver_heat = (byte1 >> 4) & 0x03
        front_window = bool(byte1 & 0x02)
        rear_window = bool(byte1 & 0x04)
        
        return SeatHeatingStatus(
            driver_seat_heat_level=driver_heat,
            passenger_seat_heat_level=passenger_heat,
            front_window_heat=front_window,
            rear_window_heat=rear_window,
            raw_bytes=data
        )
    
    def decode_ignition(self, data: List[int]) -> IgnitionStatus:
        """
        Decode Klemmen_Status_01 (0x3C0) ignition state.
        
        ZAS_Kl_S (key inserted): Bit 16
        ZAS_Kl_15 (ignition on): Bit 17
        """
        if len(data) < 3:
            raise ValueError("Ignition status requires at least 3 bytes")
        
        # Bits 16-17 are in byte 2
        key_inserted = bool(data[2] & 0x01)
        ignition_on = bool(data[2] & 0x02)
        
        return IgnitionStatus(
            key_inserted=key_inserted,
            ignition_on=ignition_on,
            raw_bytes=data
        )
    
    def decode_odometer(self, data: List[int]) -> OdometerData:
        """
        Decode Diagnose_01 (0x6B2) odometer and time.
        
        KBI_Kilometerstand: 20-bit value in bits 8-27
        Time fields in remaining bytes
        """
        if len(data) < 8:
            raise ValueError("Odometer message requires 8 bytes")
        
        # Odometer: 20 bits starting at bit 8 (byte 1, bits 0-7 + byte 2 bits 0-7 + byte 3 bits 0-3)
        odometer = data[1] | (data[2] << 8) | ((data[3] & 0x0F) << 16)
        
        # Time fields (positions may vary, this is from DBC)
        # UH_Jahr, UH_Monat, UH_Tag, UH_Stunde, UH_Minute
        year = 2000 + (data[4] if len(data) > 4 else 0)
        month = data[5] if len(data) > 5 else 0
        day = data[6] if len(data) > 6 else 0
        hour = (data[7] >> 3) if len(data) > 7 else 0
        minute = (data[7] & 0x07) * 8  # Approximate, need to verify
        
        return OdometerData(
            odometer_km=odometer,
            year=year,
            month=month,
            day=day,
            hour=hour,
            minute=minute,
            raw_bytes=data
        )
    
    # -------------------------------------------------------------------------
    # BMS Signal Decoders (limited data on comfort bus)
    # -------------------------------------------------------------------------
    
    def decode_bms_status(self, data: List[int]) -> BMSStatus:
        """
        Decode BMS_07 (0x5CA) battery status.
        
        Signals:
        - BMS_07_CRC: bits 0-7 (CRC)
        - BMS_07_BZ: bits 8-11 (counter)
        - BMS_Energieinhalt: bits 12-22 (11 bits), scale 50 Wh
        - BMS_Ladevorgang_aktiv: bit 23
        - BMS_Batteriediagnose: bits 24-26 (3 bits)
        - BMS_MaxEnergieinhalt: bits 32-42 (11 bits), scale 50 Wh
        """
        if len(data) < 8:
            raise ValueError("BMS_07 requires 8 bytes")
        
        # Convert to 64-bit value
        val = 0
        for i, b in enumerate(data):
            val |= b << (i * 8)
        
        energy_raw = (val >> 12) & 0x7FF       # 11 bits
        charging_active = bool((val >> 23) & 1)
        diagnosis = (val >> 24) & 0x7          # 3 bits
        max_energy_raw = (val >> 32) & 0x7FF   # 11 bits
        
        # Apply scaling (50 Wh per step)
        energy_wh = energy_raw * 50
        max_energy_wh = max_energy_raw * 50
        
        return BMSStatus(
            energy_content_wh=energy_wh,
            max_energy_content_wh=max_energy_wh,
            charging_active=charging_active,
            battery_diagnosis=diagnosis,
            raw_bytes=data
        )
    
    def decode_bms_temperature(self, data: List[int]) -> BMSTemperature:
        """
        Decode BMS_06 (0x59E) battery temperatures.
        
        Signals:
        - BMS_Temperatur: bits 16-23 (8 bits), scale 0.5, offset -40
        - BMS_IstVorlaufTemperatur: bits 24-31 (8 bits), scale 0.5, offset -40
        - BMS_IstRuecklaufTemperatur_02: bits 56-63 (8 bits), scale 0.5, offset -40
        """
        if len(data) < 8:
            raise ValueError("BMS_06 requires 8 bytes")
        
        battery_temp_raw = data[2]
        inlet_temp_raw = data[3]
        outlet_temp_raw = data[7]
        
        # Apply scaling: 0.5°C scale, -40 offset
        battery_temp = battery_temp_raw * 0.5 - 40
        inlet_temp = inlet_temp_raw * 0.5 - 40
        outlet_temp = outlet_temp_raw * 0.5 - 40
        
        return BMSTemperature(
            battery_temp_c=battery_temp,
            coolant_inlet_temp_c=inlet_temp,
            coolant_outlet_temp_c=outlet_temp,
            raw_bytes=data
        )
    
    # -------------------------------------------------------------------------
    # Climate Signal Decoders
    # -------------------------------------------------------------------------
    
    def decode_climate_status(self, data: List[int]) -> ClimateStatus:
        """
        Decode Klima_02/Klima_03 (0x66E) climate status.
        
        Signals:
        - KL_STL_aktiv: bit 0 (standby ventilation active)
        - KL_STH_aktiv: bit 1 (standby heating active)
        - KL_I_Geblaese: bits 40-47 (8 bits), scale 0.25 A
        """
        if len(data) < 6:
            raise ValueError("Klima_02 requires at least 6 bytes")
        
        standby_vent = bool(data[0] & 0x01)
        standby_heat = bool(data[0] & 0x02)
        
        # Blower current at byte 5 (bits 40-47)
        blower_current = data[5] * 0.25 if len(data) > 5 else 0.0
        
        # Cabin temp may be elsewhere - leaving as None for now
        cabin_temp = None
        
        return ClimateStatus(
            standby_heating_active=standby_heat,
            standby_vent_active=standby_vent,
            blower_current_a=blower_current,
            cabin_temp_c=cabin_temp,
            raw_bytes=data
        )
    
    # -------------------------------------------------------------------------
    # GPS Signal Decoders
    # -------------------------------------------------------------------------
    
    def decode_gps_position(self, data: List[int]) -> GPSPosition:
        """
        Decode NavPos_01 (0x486) GPS position.
        
        Signals (little-endian bit order):
        - NP_LatDegree: bits 0-26 (27 bits), scale 0.000001
        - NP_LongDegree: bits 27-54 (28 bits), scale 0.000001
        - NP_LatDirection: bit 55 (0=North, 1=South)
        - NP_LongDirection: bit 56 (0=East, 1=West)
        - NP_Sat: bits 57-61 (5 bits)
        - NP_Fix: bits 62-63 (2 bits)
        """
        if len(data) < 8:
            raise ValueError("NavPos_01 requires 8 bytes")
        
        # Convert to 64-bit value (little endian)
        val = 0
        for i, b in enumerate(data):
            val |= b << (i * 8)
        
        # Extract signals
        lat_raw = val & 0x7FFFFFF           # bits 0-26 (27 bits)
        long_raw = (val >> 27) & 0xFFFFFFF  # bits 27-54 (28 bits)
        lat_dir = (val >> 55) & 1           # bit 55
        long_dir = (val >> 56) & 1          # bit 56
        sat_count = (val >> 57) & 0x1F      # bits 57-61
        fix = (val >> 62) & 0x3             # bits 62-63
        
        # Convert to degrees
        latitude = lat_raw * 0.000001
        longitude = long_raw * 0.000001
        
        # Apply direction signs
        if lat_dir:
            latitude = -latitude    # South
        if long_dir:
            longitude = -longitude  # West
        
        return GPSPosition(
            latitude=latitude,
            longitude=longitude,
            satellites=sat_count,
            fix_type=GPSFixType(fix),
            raw_bytes=data
        )
    
    def decode_gps_data(self, data: List[int]) -> GPSData:
        """
        Decode NavData_02 (0x485) GPS metadata.
        
        Signals:
        - ND_SatInUse: bits 0-4 (5 bits)
        - POS_Guete_horizontal_verfuegbar: bit 5
        - ND_SatInView: bits 8-12 (5 bits)
        - POS_Guete_horizontal: bits 13-19 (7 bits), scale 2
        - NP_Altitude: bits 20-31 (12 bits), scale 2, offset -500
        - ND_UTC: bits 32-63 (32 bits)
        """
        if len(data) < 8:
            raise ValueError("NavData_02 requires 8 bytes")
        
        # Convert to 64-bit value
        val = 0
        for i, b in enumerate(data):
            val |= b << (i * 8)
        
        sat_in_use = val & 0x1F
        accuracy_avail = bool((val >> 5) & 1)
        sat_in_view = (val >> 8) & 0x1F
        accuracy_raw = (val >> 13) & 0x7F
        altitude_raw = (val >> 20) & 0xFFF
        utc = (val >> 32) & 0xFFFFFFFF
        
        # Apply scaling
        altitude = altitude_raw * 2 - 500
        accuracy = accuracy_raw * 2
        
        return GPSData(
            satellites_in_use=sat_in_use,
            satellites_in_view=sat_in_view,
            altitude_m=altitude,
            horizontal_accuracy_m=accuracy,
            accuracy_available=accuracy_avail,
            utc_timestamp=utc,
            raw_bytes=data
        )
    
    def decode_gps_heading(self, data: List[int]) -> GPSHeading:
        """
        Decode NavData_01 (0x484) heading and DOP values.
        
        Signals:
        - ND_VDOP: bits 0-9 (10 bits), scale 0.025
        - ND_TDOP: bits 10-19 (10 bits), scale 0.025
        - ND_HDOP: bits 20-29 (10 bits), scale 0.025
        - ND_GDOP: bits 30-39 (10 bits), scale 0.025
        - ND_PDOP: bits 40-49 (10 bits), scale 0.025
        - ND_Heading: bits 50-61 (12 bits), scale 0.1
        - ND_Init: bit 62
        """
        if len(data) < 8:
            raise ValueError("NavData_01 requires 8 bytes")
        
        # Convert to 64-bit value
        val = 0
        for i, b in enumerate(data):
            val |= b << (i * 8)
        
        vdop_raw = val & 0x3FF
        # tdop_raw = (val >> 10) & 0x3FF  # Not commonly used
        hdop_raw = (val >> 20) & 0x3FF
        # gdop_raw = (val >> 30) & 0x3FF  # Not commonly used
        pdop_raw = (val >> 40) & 0x3FF
        heading_raw = (val >> 50) & 0xFFF
        init = bool((val >> 62) & 1)
        
        # Apply scaling
        vdop = vdop_raw * 0.025
        hdop = hdop_raw * 0.025
        pdop = pdop_raw * 0.025
        heading = heading_raw * 0.1
        
        return GPSHeading(
            heading_deg=heading,
            hdop=hdop,
            vdop=vdop,
            pdop=pdop,
            initialized=init,
            raw_bytes=data
        )
    
    def decode_gps_map_matched(self, data: List[int]) -> Dict[str, Any]:
        """
        Decode NavPos_02_Map_Matched (0x1A5554A8) map-matched position.
        
        This is a multiplexed message with MUX in bits 0-1.
        MUX=0: Position data (lat/long)
        MUX=1: Heading and timing data
        """
        if len(data) < 8:
            raise ValueError("NavPos_02 requires 8 bytes")
        
        # Convert to 64-bit value
        val = 0
        for i, b in enumerate(data):
            val |= b << (i * 8)
        
        mux = val & 0x3
        odd_even = (val >> 2) & 1
        
        result: Dict[str, Any] = {
            'mux': mux,
            'odd_even': odd_even,
            'raw_bytes': data
        }
        
        if mux == 0:
            # Position data
            pos_status = (val >> 3) & 0xF
            lat_dir = (val >> 7) & 1
            lat_raw = (val >> 8) & 0x7FFFFFF
            long_dir = (val >> 35) & 1
            long_raw = (val >> 36) & 0xFFFFFFF
            
            latitude = lat_raw * 0.000001
            longitude = long_raw * 0.000001
            
            if lat_dir:
                latitude = -latitude
            if long_dir:
                longitude = -longitude
            
            result['type'] = 'position'
            result['latitude'] = latitude
            result['longitude'] = longitude
            result['position_status'] = pos_status
            
        elif mux == 1:
            # Heading/timing data
            heading_raw = (val >> 4) & 0xFFF
            last_sync = (val >> 16) & 0x3FFF
            heading_trust = (val >> 30) & 0x3
            utc_ref = (val >> 32) & 0xFFFFFFFF
            
            result['type'] = 'heading'
            result['heading_deg'] = heading_raw * 0.1
            result['last_gps_sync_sec'] = last_sync
            result['heading_trust'] = heading_trust
            result['utc_reference'] = utc_ref
        
        return result


# =============================================================================
# TM_01 Command Generator
# =============================================================================

class TM01Commands:
    """
    Generate TM_01 (0x5A7) commands for horn, flash, and lock control.
    
    These are standard CAN messages (11-bit ID) sent directly on the comfort bus.
    Confirmed working: horn, flash
    Same mechanism: lock, unlock
    """
    
    CAN_ID = 0x5A7
    
    # Byte 6 bit definitions
    BIT_HORN = 0x01         # Bit 0 - Honk horn
    BIT_LOCK = 0x02         # Bit 1 - Lock doors
    BIT_UNLOCK = 0x04       # Bit 2 - Unlock doors
    BIT_FLASH = 0x08        # Bit 3 - Flash lights (hazards)
    BIT_PANIC = 0x10        # Bit 4 - Panic mode (horn + flash)
    
    @classmethod
    def _make_command(cls, byte6_value: int) -> Dict[str, Any]:
        """Create a TM_01 command message."""
        data = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, byte6_value, 0x00]
        return {
            'can_id': cls.CAN_ID,
            'can_id_hex': f'0x{cls.CAN_ID:03X}',
            'extended': False,
            'data': data,
            'data_hex': ' '.join(f'{b:02X}' for b in data)
        }
    
    @classmethod
    def horn(cls) -> Dict[str, Any]:
        """Sound the horn briefly."""
        return cls._make_command(cls.BIT_HORN)
    
    @classmethod
    def flash(cls) -> Dict[str, Any]:
        """Flash the hazard lights."""
        return cls._make_command(cls.BIT_FLASH)
    
    @classmethod
    def lock(cls) -> Dict[str, Any]:
        """Lock all doors."""
        return cls._make_command(cls.BIT_LOCK)
    
    @classmethod
    def unlock(cls) -> Dict[str, Any]:
        """Unlock all doors."""
        return cls._make_command(cls.BIT_UNLOCK)
    
    @classmethod
    def panic(cls) -> Dict[str, Any]:
        """Activate panic mode (horn + flash)."""
        return cls._make_command(cls.BIT_PANIC)
    
    @classmethod
    def horn_and_flash(cls) -> Dict[str, Any]:
        """Sound horn and flash lights together."""
        return cls._make_command(cls.BIT_HORN | cls.BIT_FLASH)


# =============================================================================
# Example Usage
# =============================================================================

if __name__ == "__main__":
    print("VW e-Golf Broadcast Signal Decoder")
    print("=" * 60)
    
    decoder = BroadcastDecoder()
    
    # Example: Decode outside temperature
    print("\n--- Outside Temperature (0x5E1) ---")
    temp_data = [0x7B, 0x00, 0x00, 0x00]  # 0x7B = 123 -> 11.5°C
    result = decoder.decode(BroadcastCANIds.KLIMA_SENSOR_02, temp_data)
    if result:
        print(f"  Type: {result['type']}")
        print(f"  Temperature: {result['data'].outside_temp_c}°C")
    
    # Example: Decode lock status (locked)
    print("\n--- Lock Status (0x583) - Locked ---")
    lock_data = [0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x80]
    result = decoder.decode(BroadcastCANIds.ZV_02, lock_data)
    if result:
        print(f"  Type: {result['type']}")
        print(f"  Lock State: {result['data'].lock_state.name}")
        print(f"  All Doors Closed: {result['data'].all_doors_closed()}")
    
    # Example: Decode lock status (unlocked)
    print("\n--- Lock Status (0x583) - Unlocked ---")
    unlock_data = [0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x40]
    result = decoder.decode(BroadcastCANIds.ZV_02, unlock_data)
    if result:
        print(f"  Lock State: {result['data'].lock_state.name}")
    
    # Example: TM_01 commands
    print("\n--- TM_01 Commands (0x5A7) ---")
    print(f"  Horn:   {TM01Commands.horn()['data_hex']}")
    print(f"  Flash:  {TM01Commands.flash()['data_hex']}")
    print(f"  Lock:   {TM01Commands.lock()['data_hex']}")
    print(f"  Unlock: {TM01Commands.unlock()['data_hex']}")
    print(f"  Panic:  {TM01Commands.panic()['data_hex']}")
    
    # List all supported CAN IDs
    print("\n--- Supported Broadcast CAN IDs ---")
    print(f"  0x{BroadcastCANIds.ZV_02:03X} - Lock/door status (ZV_02)")
    print(f"  0x{BroadcastCANIds.TSG_FT_01:03X} - Driver door/window (TSG_FT_01)")
    print(f"  0x{BroadcastCANIds.TSG_BT_01:03X} - Passenger door/window (TSG_BT_01)")
    print(f"  0x{BroadcastCANIds.KLIMA_SENSOR_02:03X} - Outside temperature")
    print(f"  0x{BroadcastCANIds.CLIMATE_SETTINGS:03X} - Climate target temp")
    print(f"  0x{BroadcastCANIds.SEAT_WINDOW_HEAT:03X} - Seat/window heating")
    print(f"  0x{BroadcastCANIds.KLEMMEN_STATUS_01:03X} - Ignition state")
    print(f"  0x{BroadcastCANIds.DIAGNOSE_01:03X} - Odometer/time")
    print(f"  0x{BroadcastCANIds.BMS_06:03X} - BMS temperature")
    print(f"  0x{BroadcastCANIds.BMS_07:03X} - BMS status/energy")
    print(f"  0x{BroadcastCANIds.KLIMA_02:03X} - Climate status")
    print(f"  0x{BroadcastCANIds.NAV_DATA_01:03X} - GPS heading/DOP (NavData_01)")
    print(f"  0x{BroadcastCANIds.NAV_DATA_02:03X} - GPS altitude/UTC (NavData_02)")
    print(f"  0x{BroadcastCANIds.NAV_POS_01:03X} - GPS position (NavPos_01)")
    print(f"  0x{ExtendedCANIds.NAV_POS_02_MAP_MATCHED:08X} - Map-matched position (extended)")
    
    # Example: Decode BMS status
    print("\n--- BMS Status (0x5CA) ---")
    # Real data from trace: 7C,E4,12,80,3A,A2,FE,01
    bms_data = [0x7C, 0xE4, 0x12, 0x80, 0x3A, 0xA2, 0xFE, 0x01]
    result = decoder.decode(BroadcastCANIds.BMS_07, bms_data)
    if result:
        bms = result['data']
        print(f"  Energy: {bms.energy_content_wh} Wh")
        print(f"  Max Energy: {bms.max_energy_content_wh} Wh")
        print(f"  Estimated SOC: {bms.soc_percent:.1f}%")
        print(f"  Charging Active: {bms.charging_active}")
    
    # Example: Decode BMS temperature
    print("\n--- BMS Temperature (0x59E) ---")
    # Real data from trace: 03,00,69,FF,E0,00,81,FF
    bms_temp = [0x03, 0x00, 0x69, 0xFF, 0xE0, 0x00, 0x81, 0xFF]
    result = decoder.decode(BroadcastCANIds.BMS_06, bms_temp)
    if result:
        temp = result['data']
        print(f"  Battery Temp: {temp.battery_temp_c}C")
        print(f"  Coolant Inlet: {temp.coolant_inlet_temp_c}C")
        print(f"  Coolant Outlet: {temp.coolant_outlet_temp_c}C")
    
    # Example: Decode GPS position
    print("\n--- GPS Position (0x486) ---")
    # Real data from trace: 04,80,27,FC,A2,09,09,4A
    gps_data = [0x04, 0x80, 0x27, 0xFC, 0xA2, 0x09, 0x09, 0x4A]
    result = decoder.decode(BroadcastCANIds.NAV_POS_01, gps_data)
    if result:
        pos = result['data']
        print(f"  Latitude:  {pos.latitude:.6f}")
        print(f"  Longitude: {pos.longitude:.6f}")
        print(f"  Satellites: {pos.satellites}")
        print(f"  Fix Type: {pos.fix_type.name}")
    
    # Example: Decode GPS metadata
    print("\n--- GPS Data (0x485) ---")
    # Real data from trace: 05,0D,50,10,B0,42,5F,66
    gps_meta = [0x05, 0x0D, 0x50, 0x10, 0xB0, 0x42, 0x5F, 0x66]
    result = decoder.decode(BroadcastCANIds.NAV_DATA_02, gps_meta)
    if result:
        data = result['data']
        print(f"  Satellites in use: {data.satellites_in_use}")
        print(f"  Satellites in view: {data.satellites_in_view}")
        print(f"  Altitude: {data.altitude_m}m")
        print(f"  UTC Timestamp: {data.utc_timestamp}")
