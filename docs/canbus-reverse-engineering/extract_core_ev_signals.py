#!/usr/bin/env python3
"""
Extract only the most important EV signals for e-Golf documentation.

Focuses on standard CAN IDs (11-bit, < 0x800) that are likely routed
to the comfort CAN bus via the gateway.
"""

import re
import sys
from pathlib import Path
from dataclasses import dataclass


@dataclass
class Signal:
    name: str
    start_bit: int
    length: int
    byte_order: str
    signed: bool
    scale: float
    offset: float
    unit: str
    comment: str = ""


@dataclass
class Message:
    can_id: int
    name: str
    dlc: int
    signals: list


def parse_dbc(filepath: Path) -> dict:
    """Minimal DBC parser - just get messages and signals."""
    messages = {}
    signal_comments = {}
    
    with open(filepath, 'r', encoding='latin-1') as f:
        content = f.read()
    
    # Parse messages
    msg_pattern = re.compile(r'BO_ (\d+) (\w+): (\d+) (\w+)')
    sig_pattern = re.compile(
        r'SG_ (\w+) : (\d+)\|(\d+)@([01])([+-]) \(([^,]+),([^)]+)\) \[([^|]+)\|([^\]]+)\] "([^"]*)"'
    )
    cm_sig_pattern = re.compile(r'CM_ SG_ (\d+) (\w+) "([^"]*)";')
    
    # Get signal comments
    for match in cm_sig_pattern.finditer(content):
        msg_id = int(match.group(1))
        sig_name = match.group(2)
        comment = match.group(3)
        signal_comments[(msg_id, sig_name)] = comment
    
    current_msg = None
    for line in content.split('\n'):
        line = line.strip()
        
        msg_match = msg_pattern.match(line)
        if msg_match:
            can_id = int(msg_match.group(1))
            current_msg = Message(
                can_id=can_id,
                name=msg_match.group(2),
                dlc=int(msg_match.group(3)),
                signals=[]
            )
            messages[can_id] = current_msg
            continue
        
        sig_match = sig_pattern.search(line)
        if sig_match and current_msg:
            sig = Signal(
                name=sig_match.group(1),
                start_bit=int(sig_match.group(2)),
                length=int(sig_match.group(3)),
                byte_order=sig_match.group(4),
                signed=sig_match.group(5) == '-',
                scale=float(sig_match.group(6)),
                offset=float(sig_match.group(7)),
                unit=sig_match.group(10),
                comment=signal_comments.get((current_msg.can_id, sig_match.group(1)), "")
            )
            current_msg.signals.append(sig)
        
        if not line:
            current_msg = None
    
    return messages


# Core message IDs we want (standard 11-bit only)
CORE_MESSAGES = {
    # BMS - Battery Management System
    0x191: "BMS_01 - Core battery voltage/current/SOC",
    0x1A1: "BMS_02 - Charge/discharge current limits", 
    0x39D: "BMS_03 - Voltage limits",
    0x2AF: "BMS_05 - Additional battery data",
    0x59E: "BMS_06 - Temperature",
    0x5A2: "BMS_04 - Mode and capacity",
    0x5CA: "BMS_07 - Energy content",
    0x509: "BMS_10 - Usable SOC/energy",
    0x578: "BMS_DC_01 - DC fast charging",
    
    # DC-DC Converter
    0x2AE: "DCDC_01 - HV/LV voltages and currents",
    0x3F4: "DCDC_02 - Additional DCDC data",
    0x5CD: "DCDC_03 - DCDC mode and temperature",
    
    # Climate
    0x3B5: "Klima_11 - Climate system status",
    0x659: "Klimakomp_01 - Electric AC compressor",
    0x668: "Klima_12 - Climate temperatures",
    0x66E: "Klima_03 - Climate control",
    0x671: "Klima_06 - Climate settings",
    0x5A1: "Klima_13 - Additional climate",
    
    # Vehicle State
    0x3C0: "Klemmen_Status_01 - Ignition/terminal status",
    0x0FD: "ESP_21 - Vehicle speed",
    0x6B2: "Diagnose_01 - Odometer, time",
    
    # Charger
    0x564: "LAD_01 - Charger status",
    0x67E: "LAD_02 - Charger data",
    
    # Range
    0x5F5: "Reichweite_01 - Range estimate",
    0x5F7: "Reichweite_02 - Range details",
    
    # Hybrid/EV specific
    0x3B1: "DC_Hybrid_01 - Hybrid DC-DC",
    0x65C: "BMS_Hybrid_01 - Hybrid battery",
}


def main():
    dbc_files = [
        Path("workfiles/MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc"),
        Path("workfiles/vw_mqb_2010.dbc"),
    ]
    
    all_messages = {}
    
    for dbc_file in dbc_files:
        if dbc_file.exists():
            print(f"Parsing {dbc_file.name}...", file=sys.stderr)
            messages = parse_dbc(dbc_file)
            
            for can_id, desc in CORE_MESSAGES.items():
                if can_id in messages and can_id not in all_messages:
                    all_messages[can_id] = (messages[can_id], dbc_file.name)
    
    # Output markdown
    print("# Core EV Signals for e-Golf\n")
    print("Extracted from VW DBC files. These signals use standard 11-bit CAN IDs")
    print("and are likely available on the comfort CAN bus via gateway routing.\n")
    print("**Note:** Signal positions use DBC notation: `start_bit|length@byte_order`")
    print("- `@1` = Intel/Little-endian (LSB first)")
    print("- `@0` = Motorola/Big-endian (MSB first)")
    print("- Formula: `physical_value = raw_value * scale + offset`\n")
    
    # Group by category
    categories = {
        "Battery Management (BMS)": [0x191, 0x1A1, 0x39D, 0x2AF, 0x59E, 0x5A2, 0x5CA, 0x509, 0x578],
        "DC-DC Converter": [0x2AE, 0x3F4, 0x5CD, 0x3B1],
        "Climate/HVAC": [0x3B5, 0x659, 0x668, 0x66E, 0x671, 0x5A1],
        "Vehicle State": [0x3C0, 0x0FD, 0x6B2],
        "Charger": [0x564, 0x67E],
        "Range": [0x5F5, 0x5F7],
        "Hybrid Battery": [0x65C],
    }
    
    for category, ids in categories.items():
        msgs_in_cat = [(cid, all_messages[cid]) for cid in ids if cid in all_messages]
        if not msgs_in_cat:
            continue
            
        print(f"\n## {category}\n")
        
        for can_id, (msg, source) in msgs_in_cat:
            print(f"### 0x{can_id:03X} ({can_id}) - {msg.name}")
            print(f"- DLC: {msg.dlc} bytes")
            print(f"- Source: {source}")
            if can_id in CORE_MESSAGES:
                print(f"- Purpose: {CORE_MESSAGES[can_id].split(' - ')[1]}")
            print()
            
            if msg.signals:
                print("| Signal | Bits | Scale | Offset | Unit | Description |")
                print("|--------|------|-------|--------|------|-------------|")
                
                for sig in sorted(msg.signals, key=lambda s: s.start_bit):
                    bo = "LE" if sig.byte_order == '1' else "BE"
                    sign = "s" if sig.signed else "u"
                    bits = f"{sig.start_bit}|{sig.length}@{bo},{sign}"
                    comment = sig.comment[:40] + "..." if len(sig.comment) > 40 else sig.comment
                    print(f"| {sig.name} | {bits} | {sig.scale} | {sig.offset} | {sig.unit} | {comment} |")
                print()
    
    # Summary of missing
    missing = [cid for cid in CORE_MESSAGES if cid not in all_messages]
    if missing:
        print("\n## Messages Not Found in DBC Files\n")
        for cid in missing:
            print(f"- 0x{cid:03X}: {CORE_MESSAGES[cid]}")


if __name__ == '__main__':
    main()
