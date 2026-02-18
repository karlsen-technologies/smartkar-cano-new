#!/usr/bin/env python3
"""
VW e-Golf Comfort CAN Bus Protocol Analyzer - Extended Version

This tool parses GVRET CSV trace files and decodes BAP messages for analysis.
Includes enhanced decoding for charge states and response messages.
"""

import csv
import sys
from dataclasses import dataclass, field
from enum import IntEnum
from typing import Optional, List, Dict, Tuple
from pathlib import Path


# =============================================================================
# Constants and Enums
# =============================================================================

class BAPOpCode(IntEnum):
    RESET = 0x00
    GET = 0x01
    SETGET = 0x02
    HEARTBEAT_STATUS = 0x03
    STATUS = 0x04
    STATUS_ACK = 0x05
    ACK = 0x06
    ERROR = 0x07


class ChargeMode(IntEnum):
    OFF = 0x0
    AC = 0x1
    DC = 0x2
    CONDITIONING = 0x3
    AC_AND_CONDITIONING = 0x4
    DC_AND_CONDITIONING = 0x5
    INIT = 0xF


class ChargeState(IntEnum):
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
    UNPLUGGED = 0x0
    PLUGGED = 0x1
    INIT = 0xF


class SupplyState(IntEnum):
    INACTIVE = 0x0
    ACTIVE = 0x1
    CHARGE_STATION_CONNECTED = 0x2
    INIT = 0xF


# Known CAN IDs
CAN_IDS = {
    0x17330301: ("Wake", "Wake-up message"),
    0x000005A7: ("Heartbeat", "Keep-alive / OCU active"),
    0x1B000067: ("BAPInit", "BAP channel initialization"),
    0x17F00067: ("BAPInitResp", "BAP init response"),
    0x17332501: ("BatteryControl_ASG", "Battery Control commands (ASG->FSG)"),
    0x17332510: ("BatteryControl_FSG", "Battery Control responses (FSG->ASG)"),
    0x17330D00: ("DoorLock_ASG", "Door Locking commands"),
    0x17330D01: ("DoorLock_FSG1", "Door Locking responses"),
    0x17330D02: ("DoorLock_FSG2", "Door Locking responses"),
    0x17330D10: ("DoorLock_FSG3", "Door Locking status"),
    0x17333700: ("ENI_ASG", "Extended Network Interface commands"),
    0x17333710: ("ENI_FSG", "Extended Network Interface responses"),
}

# BAP Devices
BAP_DEVICES = {
    0x25: "BatteryControl",
    0x0D: "DoorLocking", 
    0x37: "ENI",
    0x01: "SystemInfo",
    0x07: "Unknown_07",
    0x08: "Unknown_08",
    0x09: "Unknown_09",
    0x0C: "Unknown_0C",
    0x0E: "Unknown_0E",
    0x0F: "Unknown_0F",
    0x11: "Unknown_11",
    0x12: "Unknown_12",
    0x13: "Unknown_13",
    0x23: "Unknown_23",
    0x31: "Unknown_31",
    0x32: "Unknown_32",
    0x33: "MediaSystem",
    0x46: "Unknown_46",
}

# Battery Control Functions
BATTERY_CONTROL_FUNCTIONS = {
    0x01: "GetAllProperties",
    0x02: "BAP-Config",
    0x03: "FunctionList",
    0x04: "HeartbeatConfig",
    0x0E: "FSG-Setup",
    0x0F: "FSG-OperationState",
    0x10: "PlugState",
    0x11: "ChargeState",
    0x12: "ClimateState",
    0x18: "ClimateOperationModeInstallation",
    0x19: "ProfilesArray",
    0x1A: "PowerProvidersArray",
}

# BAP CAN IDs for filtering
BAP_CAN_IDS = {
    0x17332501, 0x17332510,  # BatteryControl
    0x17330D00, 0x17330D01, 0x17330D02, 0x17330D10,  # DoorLocking
    0x17333700, 0x17333710, 0x17333711,  # ENI
    0x17330F01, 0x17330F10, 0x17330F00,  # Unknown
    0x17331201, 0x17331210, 0x17331200,  # Unknown
    0x17332310,  # Unknown
}


# =============================================================================
# Data Classes
# =============================================================================

@dataclass
class CANFrame:
    timestamp: int
    can_id: int
    extended: bool
    direction: str
    bus: int
    length: int
    data: List[int]
    
    @classmethod
    def from_gvret_row(cls, row: Dict) -> 'CANFrame':
        """Parse a GVRET CSV row into a CANFrame."""
        data = []
        for i in range(1, 9):
            val = row.get(f'D{i}', '').strip()
            if val:
                data.append(int(val, 16))
        
        return cls(
            timestamp=int(row['Time Stamp']) if row['Time Stamp'] else 0,
            can_id=int(row['ID'], 16),
            extended=row['Extended'].lower() == 'true',
            direction=row['Dir'],
            bus=int(row['Bus']) if row['Bus'] else 0,
            length=int(row['LEN']) if row['LEN'] else len(data),
            data=data
        )
    
    @property
    def id_name(self) -> str:
        """Get human-readable name for the CAN ID."""
        if self.can_id in CAN_IDS:
            return CAN_IDS[self.can_id][0]
        return f"0x{self.can_id:08X}"
    
    @property
    def data_hex(self) -> str:
        """Get data as hex string."""
        return ' '.join(f'{b:02X}' for b in self.data)


@dataclass 
class BAPMessage:
    frame: CANFrame
    frame_type: str  # 'Short', 'LongStart', 'LongContinuation'
    message_index: int
    opcode: Optional[int]
    device_id: Optional[int]
    function_id: Optional[int]
    payload: List[int]
    total_length: Optional[int]
    
    @classmethod
    def from_can_frame(cls, frame: CANFrame) -> 'BAPMessage':
        """Parse a CAN frame as a BAP message."""
        if len(frame.data) < 2:
            return cls(frame, 'Invalid', 0, None, None, None, [], None)
        
        first_byte = frame.data[0]
        is_long = bool(first_byte & 0x80)
        is_continuation = bool(first_byte & 0x40)
        
        if not is_long:
            # Short message
            opcode = (frame.data[0] >> 4) & 0x07
            device_id = ((frame.data[0] & 0x0F) << 2) | ((frame.data[1] >> 6) & 0x03)
            function_id = frame.data[1] & 0x3F
            payload = frame.data[2:frame.length] if frame.length > 2 else []
            
            return cls(frame, 'Short', 0, opcode, device_id, function_id, payload, len(payload))
        
        elif not is_continuation:
            # Long start message
            message_index = (first_byte >> 2) & 0x0F
            total_length = frame.data[1]
            opcode = (frame.data[2] >> 4) & 0x07
            device_id = ((frame.data[2] & 0x0F) << 2) | ((frame.data[3] >> 6) & 0x03)
            function_id = frame.data[3] & 0x3F
            payload = frame.data[4:frame.length] if frame.length > 4 else []
            
            return cls(frame, 'LongStart', message_index, opcode, device_id, function_id, payload, total_length)
        
        else:
            # Long continuation message
            message_index = (first_byte >> 2) & 0x0F
            payload = frame.data[1:frame.length] if frame.length > 1 else []
            
            return cls(frame, 'LongContinuation', message_index, None, None, None, payload, None)
    
    @property
    def opcode_name(self) -> str:
        if self.opcode is None:
            return "N/A"
        try:
            return BAPOpCode(self.opcode).name
        except ValueError:
            return f"Unknown({self.opcode})"
    
    @property
    def device_name(self) -> str:
        if self.device_id is None:
            return "N/A"
        return BAP_DEVICES.get(self.device_id, f"Dev_0x{self.device_id:02X}")
    
    @property
    def function_name(self) -> str:
        if self.function_id is None:
            return "N/A"
        if self.device_id == 0x25:
            return BATTERY_CONTROL_FUNCTIONS.get(self.function_id, f"Func_0x{self.function_id:02X}")
        return f"0x{self.function_id:02X}"
    
    @property
    def payload_hex(self) -> str:
        return ' '.join(f'{b:02X}' for b in self.payload)


# =============================================================================
# Data Decoders
# =============================================================================

def decode_charge_state(payload: List[int]) -> dict:
    """Decode ChargeState (function 0x11) payload."""
    if len(payload) < 2:
        return {"error": "Payload too short", "raw": payload}
    
    charge_mode = (payload[0] >> 4) & 0x0F
    charge_state = payload[0] & 0x0F
    
    result: dict = {
        "charge_mode": ChargeMode(charge_mode).name if charge_mode in [e.value for e in ChargeMode] else f"Unknown({charge_mode})",
        "charge_state": ChargeState(charge_state).name if charge_state in [e.value for e in ChargeState] else f"Unknown({charge_state})",
    }
    
    if len(payload) >= 2:
        result["charge_level_percent"] = payload[1]
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


def decode_climate_state(payload: List[int]) -> dict:
    """Decode ClimateState (function 0x12) payload."""
    if len(payload) < 1:
        return {"error": "Payload too short"}
    
    climate_mode = payload[0]
    
    result: dict = {
        "climating_active": bool(climate_mode & 0x01),
        "auto_defrost": bool(climate_mode & 0x02),
        "heating": bool(climate_mode & 0x04),
        "cooling": bool(climate_mode & 0x08),
        "ventilation": bool(climate_mode & 0x10),
        "fuel_based_heating": bool(climate_mode & 0x20),
    }
    
    if len(payload) >= 2:
        result["current_temperature_raw"] = payload[1]
    if len(payload) >= 3:
        result["temperature_unit"] = payload[2]
    if len(payload) >= 5:
        result["climating_time_min"] = payload[3] | (payload[4] << 8)
    if len(payload) >= 6:
        result["climate_state"] = (payload[5] >> 4) & 0x0F
    
    return result


def decode_plug_state(payload: List[int]) -> Dict:
    """Decode PlugState (function 0x10) payload."""
    if len(payload) < 2:
        return {"error": "Payload too short"}
    
    lock_setup = (payload[0] >> 4) & 0x0F
    lock_state = payload[0] & 0x0F
    supply_state = (payload[1] >> 4) & 0x0F
    plug_state = payload[1] & 0x0F
    
    return {
        "lock_setup": lock_setup,
        "lock_state": lock_state,
        "supply_state": SupplyState(supply_state).name if supply_state in [e.value for e in SupplyState] else f"Unknown({supply_state})",
        "plug_state": PlugState(plug_state).name if plug_state in [e.value for e in PlugState] else f"Unknown({plug_state})",
    }


def decode_operation_mode_installation(payload: List[int]) -> Dict:
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


def decode_profile_operation(byte: int) -> Dict:
    """Decode BatteryControlProfileOperation byte."""
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


def decode_bap_message_payload(msg: BAPMessage) -> Optional[Dict]:
    """Decode a BAP message payload if we know the function."""
    if msg.device_id != 0x25:
        return None
    
    if msg.function_id == 0x10:
        return decode_plug_state(msg.payload)
    elif msg.function_id == 0x11:
        return decode_charge_state(msg.payload)
    elif msg.function_id == 0x12:
        return decode_climate_state(msg.payload)
    elif msg.function_id == 0x18:
        return decode_operation_mode_installation(msg.payload)
    
    return None


# =============================================================================
# Trace Analysis Functions
# =============================================================================

def load_gvret_trace(filepath: str) -> List[CANFrame]:
    """Load a GVRET CSV trace file."""
    frames = []
    with open(filepath, 'r', newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            try:
                frame = CANFrame.from_gvret_row(row)
                frames.append(frame)
            except (ValueError, KeyError) as e:
                pass  # Skip invalid rows silently
    return frames


def analyze_battery_control_traffic(frames: List[CANFrame]) -> None:
    """Analyze specifically the BatteryControl BAP traffic."""
    print("\n" + "="*80)
    print("Battery Control Traffic Analysis")
    print("="*80)
    
    bc_frames = [f for f in frames if f.can_id in [0x17332501, 0x17332510]]
    
    # Track long messages
    long_messages: Dict[Tuple[int, int], BAPMessage] = {}  # (can_id, index) -> start msg
    
    for frame in bc_frames:
        msg = BAPMessage.from_can_frame(frame)
        
        direction = "CMD" if frame.can_id == 0x17332501 else "RSP"
        
        if msg.frame_type == 'Short':
            decoded = decode_bap_message_payload(msg)
            print(f"\n[{direction}] SHORT {msg.opcode_name} -> {msg.device_name}/{msg.function_name}")
            print(f"    Raw: {msg.frame.data_hex}")
            if decoded:
                for k, v in decoded.items():
                    print(f"    {k}: {v}")
        
        elif msg.frame_type == 'LongStart':
            key = (frame.can_id, msg.message_index)
            long_messages[key] = msg
            print(f"\n[{direction}] LONG START {msg.opcode_name} -> {msg.device_name}/{msg.function_name}")
            print(f"    Raw: {msg.frame.data_hex}")
            print(f"    Index: {msg.message_index}, Total: {msg.total_length} bytes")
        
        elif msg.frame_type == 'LongContinuation':
            key = (frame.can_id, msg.message_index)
            if key in long_messages:
                start_msg = long_messages[key]
                start_msg.payload.extend(msg.payload)
                print(f"[{direction}] LONG CONT (idx={msg.message_index})")
                print(f"    Raw: {msg.frame.data_hex}")
                print(f"    Combined payload: {start_msg.payload_hex}")
                
                # Try to decode
                decoded = decode_bap_message_payload(start_msg)
                if decoded:
                    for k, v in decoded.items():
                        print(f"    {k}: {v}")


def find_charge_state_messages(frames: List[CANFrame]) -> None:
    """Find and decode all charge state messages in a trace."""
    print("\n" + "="*80)
    print("Charge State Messages")
    print("="*80)
    
    for frame in frames:
        if frame.can_id == 0x17332510:  # FSG response
            msg = BAPMessage.from_can_frame(frame)
            
            # Handle long messages (need to track state)
            if msg.frame_type == 'Short' and msg.device_id == 0x25 and msg.function_id == 0x11:
                print(f"\nChargeState (Short):")
                print(f"  Raw: {msg.frame.data_hex}")
                decoded = decode_charge_state(msg.payload)
                for k, v in decoded.items():
                    print(f"  {k}: {v}")


def analyze_trace_summary(frames: List[CANFrame]) -> None:
    """Print a summary analysis of a trace."""
    print(f"\n{'='*80}")
    print(f"Trace Summary: {len(frames)} frames")
    print('='*80)
    
    # Count CAN IDs
    id_counts: Dict[int, int] = {}
    for frame in frames:
        id_counts[frame.can_id] = id_counts.get(frame.can_id, 0) + 1
    
    print("\n--- CAN ID Summary ---")
    
    # Extended IDs (BAP traffic)
    extended = [(cid, cnt) for cid, cnt in id_counts.items() if cid > 0xFFFF]
    standard = [(cid, cnt) for cid, cnt in id_counts.items() if cid <= 0xFFFF]
    
    print("\nExtended CAN IDs (likely BAP):")
    for can_id, count in sorted(extended, key=lambda x: -x[1])[:20]:
        id_info = CAN_IDS.get(can_id, ("", ""))
        name = id_info[0] if id_info[0] else f"0x{can_id:08X}"
        print(f"  {name:25s}: {count:5d} frames")
    
    print("\nStandard CAN IDs (broadcast data):")
    for can_id, count in sorted(standard, key=lambda x: -x[1])[:20]:
        id_info = CAN_IDS.get(can_id, ("", ""))
        name = id_info[0] if id_info[0] else f"0x{can_id:03X}"
        print(f"  {name:25s}: {count:5d} frames")


# =============================================================================
# Message Encoding (for reference implementation)
# =============================================================================

def encode_bap_header(opcode: int, device_id: int, function_id: int) -> Tuple[int, int]:
    """Encode BAP header bytes from components."""
    byte0 = ((opcode & 0x07) << 4) | ((device_id >> 2) & 0x0F)
    byte1 = ((device_id & 0x03) << 6) | (function_id & 0x3F)
    return byte0, byte1


def encode_short_bap_message(opcode: int, device_id: int, function_id: int, payload: List[int]) -> List[int]:
    """Encode a short BAP message."""
    if len(payload) > 6:
        raise ValueError("Payload too long for short message (max 6 bytes)")
    
    byte0, byte1 = encode_bap_header(opcode, device_id, function_id)
    result = [byte0, byte1] + payload
    
    # Pad to 8 bytes
    while len(result) < 8:
        result.append(0x00)
    
    return result


def generate_start_climate_command() -> List[int]:
    """Generate the command to start climate immediately."""
    return encode_short_bap_message(BAPOpCode.SETGET, 0x25, 0x18, [0x00, 0x01])


def generate_stop_climate_command() -> List[int]:
    """Generate the command to stop climate."""
    return encode_short_bap_message(BAPOpCode.SETGET, 0x25, 0x18, [0x00, 0x00])


def generate_start_charge_command() -> List[int]:
    """Generate the command to start charging immediately.
    
    Note: This is the same as climate - the profile operation mode determines
    whether charging or climate (or both) will be activated.
    """
    return encode_short_bap_message(BAPOpCode.SETGET, 0x25, 0x18, [0x00, 0x01])


# =============================================================================
# Main
# =============================================================================

def main():
    if len(sys.argv) < 2:
        print("VW e-Golf Comfort CAN Protocol Analyzer")
        print("\nUsage:")
        print(f"  {sys.argv[0]} <trace.csv>           - Analyze a GVRET trace file")
        print(f"  {sys.argv[0]} <trace.csv> --battery - Focus on battery control traffic")
        print(f"  {sys.argv[0]} <trace.csv> --charge  - Find charge state messages")
        print(f"  {sys.argv[0]} --commands            - Show command generation examples")
        return
    
    if sys.argv[1] == '--commands':
        print("\n" + "="*80)
        print("Command Generation Examples")
        print("="*80)
        
        print("\n--- Start Climate ---")
        cmd = generate_start_climate_command()
        print(f"  CAN ID: 0x17332501 (extended)")
        print(f"  Data: {' '.join(f'{b:02X}' for b in cmd)}")
        
        print("\n--- Stop Climate ---")
        cmd = generate_stop_climate_command()
        print(f"  CAN ID: 0x17332501 (extended)")
        print(f"  Data: {' '.join(f'{b:02X}' for b in cmd)}")
        
        print("\n--- Start Charging ---")
        print("  Note: Same command as start climate - the profile mode determines action")
        print("  First configure profile 0 with charge flag set:")
        print("    Operation byte 0x01 = charge only")
        print("    Operation byte 0x03 = charge + climate")
        cmd = generate_start_charge_command()
        print(f"  CAN ID: 0x17332501 (extended)")
        print(f"  Data: {' '.join(f'{b:02X}' for b in cmd)}")
        return
    
    filepath = sys.argv[1]
    if not Path(filepath).exists():
        print(f"Error: File not found: {filepath}")
        return
    
    print(f"Loading trace: {filepath}")
    frames = load_gvret_trace(filepath)
    print(f"Loaded {len(frames)} frames")
    
    if '--battery' in sys.argv:
        analyze_battery_control_traffic(frames)
    elif '--charge' in sys.argv:
        find_charge_state_messages(frames)
    else:
        analyze_trace_summary(frames)


if __name__ == '__main__':
    main()
