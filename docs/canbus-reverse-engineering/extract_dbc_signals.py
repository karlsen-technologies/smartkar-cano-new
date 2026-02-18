#!/usr/bin/env python3
"""
DBC Signal Extractor for e-Golf CAN Bus Project

Extracts EV-relevant signals from large DBC files and outputs
a compact summary suitable for documentation.

Filters for: BMS, battery, charger, DCDC, climate, vehicle state signals
"""

import re
import sys
import argparse
from pathlib import Path
from dataclasses import dataclass
from typing import Optional

# Keywords to filter for EV-relevant signals
RELEVANT_KEYWORDS = [
    # Battery/BMS
    'bms', 'batterie', 'battery', 'soc', 'spannung', 'strom', 'ladung',
    'entladung', 'kapazit', 'energie', 'zelle', 'cell', 'hv_', '_hv',
    
    # Charging
    'lade', 'charg', 'dcls', 'ac_', 'dc_', 'plug', 'stecker',
    
    # DC-DC Converter
    'dcdc', 'dc_dc', 'nv_', '_nv', '12v',
    
    # Climate/HVAC
    'klima', 'climate', 'hvac', 'kompr', 'compressor', 'heiz', 'heat',
    'kuhl', 'cool', 'temp', 'luft', 'geblas', 'fan',
    
    # Vehicle state
    'geschwind', 'speed', 'kmh', 'klemme', 'ignition', 'zas_',
    'kilometer', 'odometer', 'reichweite', 'range',
    
    # Electric motor
    'emotor', 'e_motor', 'antrieb', 'drive', 'moment', 'torque', 'drehzahl', 'rpm',
]

# Message name patterns to include entirely
RELEVANT_MESSAGE_PATTERNS = [
    r'^BMS_',
    r'^DCDC_',
    r'^DC_DC',
    r'^Klima',
    r'^EKL_',
    r'^Ladegeraet',
    r'^Charger',
    r'^ESP_\d+',      # Speed signals
    r'^Klemmen',      # Ignition state
    r'^Diagnose',     # Odometer, time
    r'^HV_',
    r'^EMot',
    r'^Antrieb',
]


@dataclass
class Signal:
    name: str
    start_bit: int
    length: int
    byte_order: str  # '0' = big-endian (Motorola), '1' = little-endian (Intel)
    signed: bool
    scale: float
    offset: float
    min_val: float
    max_val: float
    unit: str
    comment: Optional[str] = None


@dataclass
class Message:
    can_id: int
    name: str
    dlc: int
    transmitter: str
    signals: list
    comment: Optional[str] = None


def parse_dbc_file(filepath: Path) -> tuple[dict, dict]:
    """Parse a DBC file and return messages and signal comments."""
    messages = {}
    signal_comments = {}
    message_comments = {}
    current_message = None
    
    with open(filepath, 'r', encoding='latin-1') as f:
        content = f.read()
    
    # Parse messages: BO_ <CAN-ID> <MessageName>: <MessageLength> <SendingNode>
    msg_pattern = re.compile(
        r'BO_ (\d+) (\w+): (\d+) (\w+)'
    )
    
    # Parse signals: SG_ <SignalName> : <StartBit>|<Length>@<ByteOrder><Sign> (<Scale>,<Offset>) [<Min>|<Max>] "<Unit>" <ReceivingNodes>
    sig_pattern = re.compile(
        r'SG_ (\w+) : (\d+)\|(\d+)@([01])([+-]) \(([^,]+),([^)]+)\) \[([^|]+)\|([^\]]+)\] "([^"]*)"'
    )
    
    # Parse comments
    cm_msg_pattern = re.compile(r'CM_ BO_ (\d+) "([^"]*)";')
    cm_sig_pattern = re.compile(r'CM_ SG_ (\d+) (\w+) "([^"]*)";')
    
    lines = content.split('\n')
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Check for message definition
        msg_match = msg_pattern.match(line)
        if msg_match:
            can_id = int(msg_match.group(1))
            msg_name = msg_match.group(2)
            dlc = int(msg_match.group(3))
            transmitter = msg_match.group(4)
            
            current_message = Message(
                can_id=can_id,
                name=msg_name,
                dlc=dlc,
                transmitter=transmitter,
                signals=[]
            )
            messages[can_id] = current_message
            i += 1
            continue
        
        # Check for signal definition
        sig_match = sig_pattern.search(line)
        if sig_match and current_message:
            signal = Signal(
                name=sig_match.group(1),
                start_bit=int(sig_match.group(2)),
                length=int(sig_match.group(3)),
                byte_order=sig_match.group(4),
                signed=sig_match.group(5) == '-',
                scale=float(sig_match.group(6)),
                offset=float(sig_match.group(7)),
                min_val=float(sig_match.group(8)),
                max_val=float(sig_match.group(9)),
                unit=sig_match.group(10)
            )
            current_message.signals.append(signal)
            i += 1
            continue
        
        # Check for message comment
        cm_msg_match = cm_msg_pattern.search(line)
        if cm_msg_match:
            msg_id = int(cm_msg_match.group(1))
            comment = cm_msg_match.group(2)
            message_comments[msg_id] = comment
            i += 1
            continue
        
        # Check for signal comment
        cm_sig_match = cm_sig_pattern.search(line)
        if cm_sig_match:
            msg_id = int(cm_sig_match.group(1))
            sig_name = cm_sig_match.group(2)
            comment = cm_sig_match.group(3)
            signal_comments[(msg_id, sig_name)] = comment
            i += 1
            continue
        
        # Empty line resets current message context
        if not line:
            current_message = None
        
        i += 1
    
    # Apply comments to messages and signals
    for msg_id, msg in messages.items():
        if msg_id in message_comments:
            msg.comment = message_comments[msg_id]
        for sig in msg.signals:
            if (msg_id, sig.name) in signal_comments:
                sig.comment = signal_comments[(msg_id, sig.name)]
    
    return messages


def is_relevant_message(msg: Message) -> bool:
    """Check if a message is relevant based on name patterns."""
    for pattern in RELEVANT_MESSAGE_PATTERNS:
        if re.match(pattern, msg.name, re.IGNORECASE):
            return True
    return False


def is_relevant_signal(sig: Signal) -> bool:
    """Check if a signal is relevant based on keywords."""
    name_lower = sig.name.lower()
    for keyword in RELEVANT_KEYWORDS:
        if keyword in name_lower:
            return True
    return False


def filter_relevant(messages: dict, include_all_signals: bool = False) -> dict:
    """Filter messages to only include EV-relevant ones."""
    filtered = {}
    
    for can_id, msg in messages.items():
        # Check if message name matches relevant patterns
        if is_relevant_message(msg):
            filtered[can_id] = msg
            continue
        
        # Check if any signal is relevant
        if not include_all_signals:
            relevant_signals = [s for s in msg.signals if is_relevant_signal(s)]
            if relevant_signals:
                # Create a copy with only relevant signals
                filtered_msg = Message(
                    can_id=msg.can_id,
                    name=msg.name,
                    dlc=msg.dlc,
                    transmitter=msg.transmitter,
                    signals=relevant_signals,
                    comment=msg.comment
                )
                filtered[can_id] = filtered_msg
    
    return filtered


def format_signal(sig: Signal, indent: str = "  ") -> str:
    """Format a signal for output."""
    byte_order = "Intel (LE)" if sig.byte_order == '1' else "Motorola (BE)"
    signed = "signed" if sig.signed else "unsigned"
    
    lines = [f"{indent}{sig.name}:"]
    lines.append(f"{indent}  Bits: {sig.start_bit}|{sig.length} ({byte_order}, {signed})")
    
    if sig.scale != 1 or sig.offset != 0:
        lines.append(f"{indent}  Formula: raw * {sig.scale} + {sig.offset}")
    
    if sig.unit:
        lines.append(f"{indent}  Unit: {sig.unit}")
    
    if sig.min_val != 0 or sig.max_val != 0:
        lines.append(f"{indent}  Range: [{sig.min_val} .. {sig.max_val}]")
    
    if sig.comment:
        # Truncate long comments
        comment = sig.comment[:100] + "..." if len(sig.comment) > 100 else sig.comment
        lines.append(f"{indent}  Comment: {comment}")
    
    return '\n'.join(lines)


def format_message(msg: Message) -> str:
    """Format a message for output."""
    lines = [f"\n### 0x{msg.can_id:03X} ({msg.can_id}) - {msg.name}"]
    lines.append(f"DLC: {msg.dlc}, Transmitter: {msg.transmitter}")
    
    if msg.comment:
        lines.append(f"Description: {msg.comment}")
    
    lines.append(f"Signals ({len(msg.signals)}):")
    for sig in sorted(msg.signals, key=lambda s: s.start_bit):
        lines.append(format_signal(sig))
    
    return '\n'.join(lines)


def output_markdown(messages: dict, source_file: str) -> str:
    """Generate markdown documentation."""
    lines = [f"# Extracted Signals from {source_file}\n"]
    
    # Group by category based on message name
    categories = {
        'Battery/BMS': [],
        'Charging': [],
        'DC-DC Converter': [],
        'Climate/HVAC': [],
        'Vehicle State': [],
        'Electric Motor': [],
        'Other': [],
    }
    
    for can_id, msg in sorted(messages.items()):
        name_lower = msg.name.lower()
        if 'bms' in name_lower or 'batter' in name_lower:
            categories['Battery/BMS'].append(msg)
        elif 'lade' in name_lower or 'charg' in name_lower or 'dcls' in name_lower:
            categories['Charging'].append(msg)
        elif 'dcdc' in name_lower or 'dc_dc' in name_lower:
            categories['DC-DC Converter'].append(msg)
        elif 'klima' in name_lower or 'ekl' in name_lower:
            categories['Climate/HVAC'].append(msg)
        elif any(x in name_lower for x in ['esp', 'klemm', 'diagnose', 'geschw']):
            categories['Vehicle State'].append(msg)
        elif any(x in name_lower for x in ['emot', 'antrieb', 'motor']):
            categories['Electric Motor'].append(msg)
        else:
            categories['Other'].append(msg)
    
    for category, msgs in categories.items():
        if msgs:
            lines.append(f"\n## {category}\n")
            for msg in sorted(msgs, key=lambda m: m.can_id):
                lines.append(format_message(msg))
    
    return '\n'.join(lines)


def output_compact(messages: dict, source_file: str) -> str:
    """Generate compact table format."""
    lines = [f"# Signals from {source_file}\n"]
    lines.append("| CAN ID | Message | Signal | Bits | Scale | Offset | Unit |")
    lines.append("|--------|---------|--------|------|-------|--------|------|")
    
    for can_id, msg in sorted(messages.items()):
        for sig in sorted(msg.signals, key=lambda s: s.start_bit):
            byte_order = "LE" if sig.byte_order == '1' else "BE"
            lines.append(
                f"| 0x{can_id:03X} | {msg.name} | {sig.name} | "
                f"{sig.start_bit}\\|{sig.length}@{byte_order} | "
                f"{sig.scale} | {sig.offset} | {sig.unit} |"
            )
    
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(description='Extract EV-relevant signals from DBC files')
    parser.add_argument('dbc_file', help='Path to DBC file')
    parser.add_argument('--format', choices=['markdown', 'compact', 'json'], 
                        default='markdown', help='Output format')
    parser.add_argument('--all-signals', action='store_true',
                        help='Include all signals from relevant messages')
    parser.add_argument('--output', '-o', help='Output file (default: stdout)')
    parser.add_argument('--stats', action='store_true', help='Show statistics only')
    
    args = parser.parse_args()
    
    filepath = Path(args.dbc_file)
    if not filepath.exists():
        print(f"Error: File not found: {filepath}", file=sys.stderr)
        sys.exit(1)
    
    print(f"Parsing {filepath.name}...", file=sys.stderr)
    messages = parse_dbc_file(filepath)
    print(f"Found {len(messages)} messages", file=sys.stderr)
    
    filtered = filter_relevant(messages, args.all_signals)
    total_signals = sum(len(m.signals) for m in filtered.values())
    print(f"Filtered to {len(filtered)} relevant messages with {total_signals} signals", file=sys.stderr)
    
    if args.stats:
        print("\nRelevant messages by CAN ID:")
        for can_id, msg in sorted(filtered.items()):
            print(f"  0x{can_id:03X} {msg.name}: {len(msg.signals)} signals")
        return
    
    if args.format == 'markdown':
        output = output_markdown(filtered, filepath.name)
    elif args.format == 'compact':
        output = output_compact(filtered, filepath.name)
    else:
        import json
        data = {
            hex(can_id): {
                'name': msg.name,
                'dlc': msg.dlc,
                'signals': [
                    {
                        'name': s.name,
                        'start_bit': s.start_bit,
                        'length': s.length,
                        'scale': s.scale,
                        'offset': s.offset,
                        'unit': s.unit
                    }
                    for s in msg.signals
                ]
            }
            for can_id, msg in sorted(filtered.items())
        }
        output = json.dumps(data, indent=2)
    
    if args.output:
        with open(args.output, 'w', encoding='utf-8') as f:
            f.write(output)
        print(f"Written to {args.output}", file=sys.stderr)
    else:
        print(output)


if __name__ == '__main__':
    main()
