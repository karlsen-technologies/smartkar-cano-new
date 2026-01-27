"""
VW e-Golf Comfort CAN Bus Protocol - Reference Implementation

This module provides all the constants, encoding/decoding functions, and command
generators needed to communicate with a VW e-Golf (MQB platform) via the comfort CAN bus.

Usage:
    from egolf_canbus import EGolfCANBus, Commands, BAPMessage
    
    # Create command sequences
    wake_sequence = Commands.wake_up_sequence()
    start_climate = Commands.start_climate()
    stop_climate = Commands.stop_climate()
    
    # Decode incoming messages
    msg = BAPMessage.from_can_data(can_id, data_bytes)
    if msg.function_id == Functions.CHARGE_STATE:
        state = DataDecoders.decode_charge_state(msg.payload)
        print(f"Charge level: {state['charge_level_percent']}%")
"""

from dataclasses import dataclass
from enum import IntEnum
from typing import List, Tuple, Optional, Dict, Any


# =============================================================================
# CAN Bus Constants
# =============================================================================

class CANBus:
    """Physical CAN bus parameters."""
    SPEED_KBPS = 500
    # Wire colors
    CAN_HIGH = "Orange with green stripes"
    CAN_LOW = "Orange with brown stripes"


class CANIds:
    """Known CAN IDs on the comfort bus."""
    # Wake/Heartbeat
    WAKE = 0x17330301           # Extended - Wake-up message
    HEARTBEAT = 0x000005A7      # Standard - Keep-alive signal
    BAP_INIT = 0x1B000067       # Extended - BAP channel initialization
    
    # Battery Control Channel (Device 0x25)
    BATTERY_CONTROL_ASG = 0x17332501  # Extended - Commands (ASG -> FSG)
    BATTERY_CONTROL_FSG = 0x17332510  # Extended - Responses (FSG -> ASG)
    
    # Door Locking Channel (Device 0x0D)
    DOOR_LOCK_ASG = 0x17330D00        # Extended - Commands
    DOOR_LOCK_FSG = 0x17330D10        # Extended - Responses
    
    # Extended Network Interface (Device 0x37)
    ENI_ASG = 0x17333700              # Extended - Commands
    ENI_FSG = 0x17333710              # Extended - Responses
    
    # Standard CAN IDs (11-bit) - Broadcast messages
    TM_01 = 0x5A7               # Standard - Horn/Flash/Lock commands
    ZV_02 = 0x583               # Standard - Lock/door status
    TSG_FT_01 = 0x3D0           # Standard - Driver door/window
    TSG_BT_01 = 0x3D1           # Standard - Passenger door/window
    KLIMA_SENSOR_02 = 0x5E1     # Standard - Outside temperature
    KLEMMEN_STATUS_01 = 0x3C0   # Standard - Ignition state


# =============================================================================
# BAP Protocol Constants
# =============================================================================

class BAPOpCode(IntEnum):
    """BAP operation codes."""
    RESET = 0x00
    GET = 0x01
    SETGET = 0x02
    HEARTBEAT_STATUS = 0x03
    STATUS = 0x04
    STATUS_ACK = 0x05
    ACK = 0x06
    ERROR = 0x07


class Devices:
    """BAP logical device IDs."""
    BATTERY_CONTROL = 0x25
    DOOR_LOCKING = 0x0D
    ENI = 0x37


class Functions:
    """Battery Control function IDs."""
    GET_ALL_PROPERTIES = 0x01
    BAP_CONFIG = 0x02
    FUNCTION_LIST = 0x03
    HEARTBEAT_CONFIG = 0x04
    FSG_SETUP = 0x0E
    FSG_OPERATION_STATE = 0x0F
    PLUG_STATE = 0x10
    CHARGE_STATE = 0x11
    CLIMATE_STATE = 0x12
    CLIMATE_OPERATION_MODE = 0x18
    PROFILES_ARRAY = 0x19
    POWER_PROVIDERS_ARRAY = 0x1A


# =============================================================================
# Data Enums
# =============================================================================

class ChargeMode(IntEnum):
    """Charging mode values from ChargeState."""
    OFF = 0x0
    AC = 0x1
    DC = 0x2
    CONDITIONING = 0x3
    AC_AND_CONDITIONING = 0x4
    DC_AND_CONDITIONING = 0x5
    INIT = 0xF


class ChargeState(IntEnum):
    """Charging state values."""
    INIT = 0x0
    IDLE = 0x1
    RUNNING = 0x2
    CONSERVATION_CHARGING = 0x3
    ABORTED_TEMP_LOW = 0x4
    ABORTED_DEVICE_ERROR = 0x5
    ABORTED_NO_POWER = 0x6
    ABORTED_NOT_IN_PARK = 0x7
    COMPLETED = 0x8
    NO_ERROR = 0x9


class PlugState(IntEnum):
    """Plug connection state."""
    UNPLUGGED = 0x0
    PLUGGED = 0x1
    INIT = 0xF


class SupplyState(IntEnum):
    """Power supply state."""
    INACTIVE = 0x0
    ACTIVE = 0x1
    CHARGE_STATION_CONNECTED = 0x2
    INIT = 0xF


class ProfileOperation:
    """Battery Control Profile operation flags (byte value)."""
    CHARGE = 0x01                           # Enable charging
    CLIMATE = 0x02                          # Enable climate control
    CLIMATE_WITHOUT_EXTERNAL = 0x04         # Allow climate on battery
    AUTO_DEFROST = 0x08                     # Auto defrost
    SEAT_HEATER_FRONT_LEFT = 0x10
    SEAT_HEATER_FRONT_RIGHT = 0x20
    SEAT_HEATER_REAR_LEFT = 0x40
    SEAT_HEATER_REAR_RIGHT = 0x80
    
    # Common combinations
    CHARGE_ONLY = 0x01
    CLIMATE_ONLY = 0x02
    CHARGE_AND_CLIMATE = 0x03
    CHARGE_CLIMATE_BATTERY = 0x05  # Charge + climate allowed on battery
    CLIMATE_WITH_BATTERY = 0x06    # Climate + climate allowed on battery


# =============================================================================
# BAP Message Encoding/Decoding
# =============================================================================

@dataclass
class CANMessage:
    """Represents a CAN message to send."""
    can_id: int
    extended: bool
    data: List[int]
    
    def __repr__(self):
        ext = "X" if self.extended else "S"
        data_hex = ' '.join(f'{b:02X}' for b in self.data)
        return f"CAN[{self.can_id:08X}{ext}] {data_hex}"


@dataclass
class BAPMessage:
    """Represents a decoded BAP message."""
    can_id: int
    frame_type: str  # 'Short', 'LongStart', 'LongContinuation'
    message_index: int
    opcode: Optional[int]
    device_id: Optional[int]
    function_id: Optional[int]
    payload: List[int]
    total_length: Optional[int]
    
    @classmethod
    def from_can_data(cls, can_id: int, data: List[int]) -> 'BAPMessage':
        """Parse CAN frame data as a BAP message."""
        if len(data) < 2:
            return cls(can_id, 'Invalid', 0, None, None, None, [], None)
        
        first_byte = data[0]
        is_long = bool(first_byte & 0x80)
        is_continuation = bool(first_byte & 0x40)
        
        if not is_long:
            # Short message
            opcode = (data[0] >> 4) & 0x07
            device_id = ((data[0] & 0x0F) << 2) | ((data[1] >> 6) & 0x03)
            function_id = data[1] & 0x3F
            payload = data[2:]
            return cls(can_id, 'Short', 0, opcode, device_id, function_id, payload, len(payload))
        
        elif not is_continuation:
            # Long start message
            message_index = (first_byte >> 2) & 0x0F
            total_length = data[1]
            opcode = (data[2] >> 4) & 0x07
            device_id = ((data[2] & 0x0F) << 2) | ((data[3] >> 6) & 0x03)
            function_id = data[3] & 0x3F
            payload = data[4:]
            return cls(can_id, 'LongStart', message_index, opcode, device_id, function_id, payload, total_length)
        
        else:
            # Long continuation message
            message_index = (first_byte >> 2) & 0x0F
            payload = data[1:]
            return cls(can_id, 'LongContinuation', message_index, None, None, None, payload, None)


class BAPEncoder:
    """Encode BAP messages for transmission."""
    
    @staticmethod
    def encode_header(opcode: int, device_id: int, function_id: int) -> Tuple[int, int]:
        """Encode BAP header bytes."""
        byte0 = ((opcode & 0x07) << 4) | ((device_id >> 2) & 0x0F)
        byte1 = ((device_id & 0x03) << 6) | (function_id & 0x3F)
        return byte0, byte1
    
    @staticmethod
    def short_message(opcode: int, device_id: int, function_id: int, 
                      payload: Optional[List[int]] = None) -> List[int]:
        """Encode a short BAP message (max 6 byte payload)."""
        if payload is None:
            payload = []
        if len(payload) > 6:
            raise ValueError("Payload too long for short message")
        
        b0, b1 = BAPEncoder.encode_header(opcode, device_id, function_id)
        result = [b0, b1] + payload
        
        # Pad to standard 8 bytes (or actual length needed)
        while len(result) < 8:
            result.append(0x00)
        
        return result
    
    @staticmethod
    def long_message(opcode: int, device_id: int, function_id: int,
                     payload: List[int], message_index: int = 0) -> List[List[int]]:
        """Encode a long BAP message, returns list of CAN frame data."""
        frames = []
        b0, b1 = BAPEncoder.encode_header(opcode, device_id, function_id)
        
        # Total length includes the 2-byte header in the payload count
        total_length = len(payload) + 2
        
        # Start frame
        start_frame = [
            0x80 | ((message_index & 0x0F) << 2),
            total_length,
            b0, b1
        ]
        start_frame.extend(payload[:4])
        frames.append(start_frame)
        
        # Continuation frames
        remaining = payload[4:]
        while remaining:
            cont_frame = [0xC0 | ((message_index & 0x0F) << 2)]
            cont_frame.extend(remaining[:7])
            frames.append(cont_frame)
            remaining = remaining[7:]
        
        return frames


# =============================================================================
# Data Decoders
# =============================================================================

class DataDecoders:
    """Decode BAP message payloads into structured data."""
    
    @staticmethod
    def decode_plug_state(payload: List[int]) -> Dict[str, Any]:
        """Decode PlugState (function 0x10) payload."""
        if len(payload) < 2:
            return {"error": "Payload too short"}
        
        return {
            "lock_setup": (payload[0] >> 4) & 0x0F,
            "lock_state": payload[0] & 0x0F,
            "supply_state": (payload[1] >> 4) & 0x0F,
            "supply_state_name": SupplyState((payload[1] >> 4) & 0x0F).name,
            "plug_state": payload[1] & 0x0F,
            "plug_state_name": PlugState(payload[1] & 0x0F).name,
        }
    
    @staticmethod
    def decode_charge_state(payload: List[int]) -> Dict[str, Any]:
        """Decode ChargeState (function 0x11) payload."""
        if len(payload) < 2:
            return {"error": "Payload too short"}
        
        result = {
            "charge_mode": (payload[0] >> 4) & 0x0F,
            "charge_mode_name": ChargeMode((payload[0] >> 4) & 0x0F).name,
            "charge_state": payload[0] & 0x0F,
            "charge_state_name": ChargeState(payload[0] & 0x0F).name,
            "charge_level_percent": payload[1],
        }
        
        if len(payload) >= 3:
            result["remaining_time_min"] = payload[2]
        if len(payload) >= 4:
            result["current_range"] = payload[3]
        if len(payload) >= 5:
            result["unit_range"] = payload[4]
        if len(payload) >= 6:
            result["current_amps"] = payload[5]
        if len(payload) >= 7:
            result["battery_climate_state"] = (payload[6] >> 4) & 0x0F
        if len(payload) >= 9:
            result["start_reason"] = (payload[8] >> 4) & 0x0F
            result["target_soc"] = payload[8] & 0x0F
        
        return result
    
    @staticmethod
    def decode_climate_state(payload: List[int]) -> Dict[str, Any]:
        """Decode ClimateState (function 0x12) payload."""
        if len(payload) < 1:
            return {"error": "Payload too short"}
        
        mode = payload[0]
        result: Dict[str, Any] = {
            "climating_active": bool(mode & 0x01),
            "auto_defrost": bool(mode & 0x02),
            "heating": bool(mode & 0x04),
            "cooling": bool(mode & 0x08),
            "ventilation": bool(mode & 0x10),
            "fuel_based_heating": bool(mode & 0x20),
        }
        
        if len(payload) >= 2:
            result["current_temperature_raw"] = payload[1]
            # Temperature formula: (raw + 100) / 10
            result["current_temperature_c"] = (payload[1] + 100) / 10
        if len(payload) >= 3:
            result["temperature_unit"] = payload[2]
        if len(payload) >= 5:
            result["climating_time_min"] = payload[3] | (payload[4] << 8)
        if len(payload) >= 6:
            result["climate_state"] = (payload[5] >> 4) & 0x0F
        
        return result
    
    @staticmethod
    def decode_operation_mode(payload: List[int]) -> Dict[str, Any]:
        """Decode ClimateOperationModeInstallation (function 0x18) payload."""
        if len(payload) < 2:
            return {"error": "Payload too short"}
        
        mode = payload[1]
        return {
            "immediately": bool(mode & 0x01),
            "timer1": bool(mode & 0x02),
            "timer2": bool(mode & 0x04),
            "timer3": bool(mode & 0x08),
            "timer4": bool(mode & 0x10),
        }
    
    @staticmethod
    def decode_profile_operation(byte: int) -> Dict[str, bool]:
        """Decode ProfileOperation flags byte."""
        return {
            "charge": bool(byte & 0x01),
            "climate": bool(byte & 0x02),
            "climate_without_external_supply": bool(byte & 0x04),
            "auto_defrost": bool(byte & 0x08),
            "seat_heater_front_left": bool(byte & 0x10),
            "seat_heater_front_right": bool(byte & 0x20),
            "seat_heater_rear_left": bool(byte & 0x40),
            "seat_heater_rear_right": bool(byte & 0x80),
        }


# =============================================================================
# Command Generators
# =============================================================================

class Commands:
    """Generate command sequences for common operations."""
    
    @staticmethod
    def wake_up_sequence() -> List[CANMessage]:
        """Generate the wake-up sequence to wake car from sleep."""
        return [
            CANMessage(CANIds.WAKE, True, [0x40, 0x00, 0x01, 0x1F]),
            CANMessage(CANIds.HEARTBEAT, False, [0x00] * 8),
            CANMessage(CANIds.BAP_INIT, True, [0x67, 0x10, 0x41, 0x84, 0x14, 0x00, 0x00, 0x00]),
        ]
    
    @staticmethod
    def heartbeat() -> CANMessage:
        """Generate keep-alive heartbeat message."""
        return CANMessage(CANIds.HEARTBEAT, False, [0x00] * 8)
    
    @staticmethod
    def configure_profile(profile_operation: int = ProfileOperation.CLIMATE_WITH_BATTERY) -> List[CANMessage]:
        """
        Configure profile 0 with specified operation mode.
        
        Args:
            profile_operation: ProfileOperation flag combination
        
        Common values:
            0x01 = Charge only
            0x02 = Climate only  
            0x03 = Charge + Climate
            0x05 = Charge + Climate on battery allowed
            0x06 = Climate + Climate on battery allowed
        """
        # Long BAP message to update profile 0 via record address 6
        # Payload structure for profile update:
        # [array_control, profile_index, operation, operation2, maxCurrent, targetChargeLevel]
        payload = [
            0x22,              # Array control flags (record addr 6, position transmitted)
            profile_operation, # Operation flags
            0x00,              # Operation2 flags
            0x01,              # Profile index 0 (immediately profile)
            profile_operation, # Repeat operation (structure requirement)
            0x00,              # maxCurrent (0 = default)
            0x20,              # targetChargeLevel (0x20 = 32... or default)
            0x00,              # Additional padding
        ]
        
        frames = BAPEncoder.long_message(
            BAPOpCode.SETGET, 
            Devices.BATTERY_CONTROL, 
            Functions.PROFILES_ARRAY,
            payload
        )
        
        return [CANMessage(CANIds.BATTERY_CONTROL_ASG, True, f) for f in frames]
    
    @staticmethod
    def start_immediately() -> CANMessage:
        """Start climate/charging immediately (profile 0)."""
        data = BAPEncoder.short_message(
            BAPOpCode.SETGET,
            Devices.BATTERY_CONTROL,
            Functions.CLIMATE_OPERATION_MODE,
            [0x00, 0x01]  # immediately = true
        )
        return CANMessage(CANIds.BATTERY_CONTROL_ASG, True, data)
    
    @staticmethod
    def stop_immediately() -> CANMessage:
        """Stop climate/charging."""
        data = BAPEncoder.short_message(
            BAPOpCode.SETGET,
            Devices.BATTERY_CONTROL,
            Functions.CLIMATE_OPERATION_MODE,
            [0x00, 0x00]  # all timers = false
        )
        return CANMessage(CANIds.BATTERY_CONTROL_ASG, True, data)
    
    @staticmethod
    def start_climate() -> List[CANMessage]:
        """
        Full sequence to start climate control.
        Configures profile 0 for climate with battery, then starts.
        """
        return (
            Commands.configure_profile(ProfileOperation.CLIMATE_WITH_BATTERY) +
            [Commands.start_immediately()]
        )
    
    @staticmethod
    def stop_climate() -> CANMessage:
        """Stop climate control."""
        return Commands.stop_immediately()
    
    @staticmethod
    def start_charging() -> List[CANMessage]:
        """
        Full sequence to start charging.
        Configures profile 0 for charging, then starts.
        """
        return (
            Commands.configure_profile(ProfileOperation.CHARGE_ONLY) +
            [Commands.start_immediately()]
        )
    
    @staticmethod
    def stop_charging() -> CANMessage:
        """Stop charging."""
        return Commands.stop_immediately()
    
    @staticmethod
    def start_charge_and_climate() -> List[CANMessage]:
        """
        Full sequence to start both charging and climate.
        """
        return (
            Commands.configure_profile(ProfileOperation.CHARGE_AND_CLIMATE) +
            [Commands.start_immediately()]
        )
    
    @staticmethod
    def get_bap_config() -> CANMessage:
        """Request BAP configuration from Battery Control."""
        data = BAPEncoder.short_message(
            BAPOpCode.GET,
            Devices.BATTERY_CONTROL,
            Functions.BAP_CONFIG
        )
        return CANMessage(CANIds.BATTERY_CONTROL_ASG, True, data)
    
    @staticmethod
    def get_charge_state() -> CANMessage:
        """Request current charge state."""
        data = BAPEncoder.short_message(
            BAPOpCode.GET,
            Devices.BATTERY_CONTROL,
            Functions.CHARGE_STATE
        )
        return CANMessage(CANIds.BATTERY_CONTROL_ASG, True, data)
    
    @staticmethod
    def get_climate_state() -> CANMessage:
        """Request current climate state."""
        data = BAPEncoder.short_message(
            BAPOpCode.GET,
            Devices.BATTERY_CONTROL,
            Functions.CLIMATE_STATE
        )
        return CANMessage(CANIds.BATTERY_CONTROL_ASG, True, data)
    
    @staticmethod
    def get_plug_state() -> CANMessage:
        """Request current plug state."""
        data = BAPEncoder.short_message(
            BAPOpCode.GET,
            Devices.BATTERY_CONTROL,
            Functions.PLUG_STATE
        )
        return CANMessage(CANIds.BATTERY_CONTROL_ASG, True, data)
    
    # -------------------------------------------------------------------------
    # TM_01 Commands (Standard CAN - Horn/Flash/Lock)
    # -------------------------------------------------------------------------
    
    @staticmethod
    def horn() -> CANMessage:
        """Sound the horn briefly. CONFIRMED WORKING."""
        return CANMessage(CANIds.TM_01, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00])
    
    @staticmethod
    def flash() -> CANMessage:
        """Flash the hazard lights. CONFIRMED WORKING."""
        return CANMessage(CANIds.TM_01, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00])
    
    @staticmethod
    def lock_doors() -> CANMessage:
        """Lock all doors via TM_01."""
        return CANMessage(CANIds.TM_01, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00])
    
    @staticmethod
    def unlock_doors() -> CANMessage:
        """Unlock all doors via TM_01."""
        return CANMessage(CANIds.TM_01, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00])
    
    @staticmethod
    def panic() -> CANMessage:
        """Activate panic mode (horn + flash)."""
        return CANMessage(CANIds.TM_01, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00])
    
    @staticmethod
    def horn_and_flash() -> CANMessage:
        """Sound horn and flash lights together."""
        return CANMessage(CANIds.TM_01, False, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00])


# =============================================================================
# High-Level Protocol Handler
# =============================================================================

class EGolfCANBus:
    """
    High-level interface for e-Golf CAN bus communication.
    
    This class manages BAP message state and provides convenient methods
    for sending commands and processing responses.
    
    Usage:
        egolf = EGolfCANBus()
        
        # Generate commands
        for msg in egolf.wake_up():
            send_can_message(msg.can_id, msg.extended, msg.data)
        
        # Process incoming messages
        response = egolf.process_incoming(can_id, data)
        if response:
            print(f"Received: {response}")
    """
    
    def __init__(self):
        # Track long message assembly
        self._long_messages: Dict[Tuple[int, int], BAPMessage] = {}
    
    def wake_up(self) -> List[CANMessage]:
        """Get wake-up sequence."""
        return Commands.wake_up_sequence()
    
    def heartbeat(self) -> CANMessage:
        """Get heartbeat message - send every 200-500ms while active."""
        return Commands.heartbeat()
    
    def start_climate(self) -> List[CANMessage]:
        """Get command sequence to start climate control."""
        return Commands.start_climate()
    
    def stop_climate(self) -> CANMessage:
        """Get command to stop climate control."""
        return Commands.stop_climate()
    
    def start_charging(self) -> List[CANMessage]:
        """Get command sequence to start charging."""
        return Commands.start_charging()
    
    def stop_charging(self) -> CANMessage:
        """Get command to stop charging."""
        return Commands.stop_charging()
    
    def horn(self) -> CANMessage:
        """Sound the horn briefly."""
        return Commands.horn()
    
    def flash(self) -> CANMessage:
        """Flash the hazard lights."""
        return Commands.flash()
    
    def lock_doors(self) -> CANMessage:
        """Lock all doors."""
        return Commands.lock_doors()
    
    def unlock_doors(self) -> CANMessage:
        """Unlock all doors."""
        return Commands.unlock_doors()
    
    def process_incoming(self, can_id: int, data: List[int]) -> Optional[Dict[str, Any]]:
        """
        Process an incoming CAN message and return decoded data if available.
        
        Returns None if the message is not from a known BAP channel or
        if it's a partial long message still being assembled.
        """
        # Only process Battery Control responses for now
        if can_id != CANIds.BATTERY_CONTROL_FSG:
            return None
        
        msg = BAPMessage.from_can_data(can_id, data)
        
        if msg.frame_type == 'LongStart':
            # Store for assembly
            key = (can_id, msg.message_index)
            self._long_messages[key] = msg
            return None
        
        elif msg.frame_type == 'LongContinuation':
            # Append to existing message
            key = (can_id, msg.message_index)
            if key in self._long_messages:
                self._long_messages[key].payload.extend(msg.payload)
                
                start_msg = self._long_messages[key]
                # Check if complete (total_length includes 2-byte header)
                if start_msg.total_length is not None and len(start_msg.payload) >= (start_msg.total_length - 2):
                    # Message complete, decode and return
                    del self._long_messages[key]
                    return self._decode_message(start_msg)
            return None
        
        elif msg.frame_type == 'Short':
            return self._decode_message(msg)
        
        return None
    
    def _decode_message(self, msg: BAPMessage) -> Optional[Dict[str, Any]]:
        """Decode a complete BAP message."""
        if msg.device_id != Devices.BATTERY_CONTROL:
            return {"raw": msg}
        
        result = {
            "opcode": msg.opcode,
            "opcode_name": BAPOpCode(msg.opcode).name if msg.opcode is not None else None,
            "device_id": msg.device_id,
            "function_id": msg.function_id,
        }
        
        if msg.function_id == Functions.PLUG_STATE:
            result["type"] = "PlugState"
            result["data"] = DataDecoders.decode_plug_state(msg.payload)
        elif msg.function_id == Functions.CHARGE_STATE:
            result["type"] = "ChargeState"
            result["data"] = DataDecoders.decode_charge_state(msg.payload)
        elif msg.function_id == Functions.CLIMATE_STATE:
            result["type"] = "ClimateState"
            result["data"] = DataDecoders.decode_climate_state(msg.payload)
        elif msg.function_id == Functions.CLIMATE_OPERATION_MODE:
            result["type"] = "OperationMode"
            result["data"] = DataDecoders.decode_operation_mode(msg.payload)
        else:
            result["type"] = "Unknown"
            result["payload"] = msg.payload
        
        return result


# =============================================================================
# Example Usage
# =============================================================================

if __name__ == "__main__":
    print("VW e-Golf CAN Bus Protocol Reference Implementation")
    print("=" * 60)
    
    # Show wake-up sequence
    print("\n--- Wake-up Sequence ---")
    for msg in Commands.wake_up_sequence():
        print(f"  {msg}")
    
    # Show climate control commands
    print("\n--- Start Climate ---")
    for msg in Commands.start_climate():
        print(f"  {msg}")
    
    print("\n--- Stop Climate ---")
    print(f"  {Commands.stop_climate()}")
    
    # Show charging commands
    print("\n--- Start Charging ---")
    for msg in Commands.start_charging():
        print(f"  {msg}")
    
    print("\n--- Stop Charging ---")
    print(f"  {Commands.stop_charging()}")
    
    # Show data request commands
    print("\n--- Data Requests ---")
    print(f"  Get Charge State: {Commands.get_charge_state()}")
    print(f"  Get Climate State: {Commands.get_climate_state()}")
    print(f"  Get Plug State: {Commands.get_plug_state()}")
    
    # Show TM_01 commands
    print("\n--- TM_01 Commands (Horn/Flash/Lock) ---")
    print(f"  Horn:        {Commands.horn()}")
    print(f"  Flash:       {Commands.flash()}")
    print(f"  Lock:        {Commands.lock_doors()}")
    print(f"  Unlock:      {Commands.unlock_doors()}")
    print(f"  Panic:       {Commands.panic()}")
    print(f"  Horn+Flash:  {Commands.horn_and_flash()}")
    
    # Example decoding
    print("\n--- Example Decoding ---")
    # Simulated charge state response
    test_data = [0x11, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF]
    decoded = DataDecoders.decode_charge_state(test_data)
    print(f"  Charge State Payload: {' '.join(f'{b:02X}' for b in test_data)}")
    for k, v in decoded.items():
        print(f"    {k}: {v}")
