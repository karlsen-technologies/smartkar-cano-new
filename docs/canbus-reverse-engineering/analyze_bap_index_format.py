"""
Analyze BAP long message byte 0 format across multiple traces.

Goal: Determine the correct interpretation of bits 5-0 in BAP long message headers.

Hypothesis A: OR of two 6-bit numbers
  - Group: 6-bit value using upper bits (0x00, 0x10, 0x20, 0x30...)
  - Index: 6-bit value using lower bits (0-15 typically)
  - Combined: group | index

Hypothesis B: Split 2+4 bits  
  - Group: 2-bit field (bits 5-4) → values 0-3
  - Index: 4-bit field (bits 3-0) → values 0-15
  - Combined: (group << 4) | index

Key distinguishing patterns:
- If index ever exceeds 15 within a message → supports Hypothesis A
- If we see "group" values like 0x30 (48) that aren't 0,16,32,48 → helps distinguish
"""

import csv
import os
from pathlib import Path
from collections import defaultdict
from typing import Dict, List, Tuple, Set

# BAP channel CAN IDs (extended) that use long messages
BAP_CAN_IDS = {
    0x17332510,  # Battery Control FSG
    0x17332501,  # Battery Control ASG
    0x17330D10,  # Door Locking FSG
    0x17330D00,  # Door Locking ASG
    0x17333710,  # ENI FSG
    0x17333700,  # ENI ASG
    0x17330110,  # Climate FSG
    0x17330100,  # Climate ASG
    # Add more as needed
}

def parse_csv_row(row: dict) -> Tuple[int, bool, List[int]]:
    """Parse a CSV row into (can_id, is_extended, data_bytes)."""
    can_id = int(row['ID'], 16)
    extended = row.get('Extended', 'false').lower() == 'true'
    
    data = []
    for i in range(1, 9):
        col = f'D{i}'
        if col in row and row[col]:
            try:
                data.append(int(row[col], 16))
            except ValueError:
                break
    
    return can_id, extended, data


def analyze_trace(filepath: str) -> Dict:
    """Analyze a single trace file for BAP long message patterns."""
    results = {
        'start_frames': [],      # (can_id, byte0, full_data)
        'cont_frames': [],       # (can_id, byte0, full_data)
        'messages': [],          # Assembled messages with frame sequences
    }
    
    # For assembling messages
    pending = {}  # (can_id, base_index) -> {frames: [...], byte0_values: [...]}
    
    try:
        with open(filepath, 'r', encoding='utf-8-sig') as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    can_id, extended, data = parse_csv_row(row)
                except (KeyError, ValueError):
                    continue
                
                if not extended or len(data) < 2:
                    continue
                
                byte0 = data[0]
                is_long = bool(byte0 & 0x80)
                is_cont = bool(byte0 & 0x40)
                
                if not is_long:
                    continue
                
                # Extract the 6-bit field (bits 5-0)
                field_6bit = byte0 & 0x3F
                
                # Hypothesis B interpretation
                group_2bit = (byte0 >> 4) & 0x03
                index_4bit = byte0 & 0x0F
                
                if not is_cont:
                    # START frame
                    results['start_frames'].append({
                        'can_id': can_id,
                        'byte0': byte0,
                        'field_6bit': field_6bit,
                        'group_2bit': group_2bit,
                        'index_4bit': index_4bit,
                        'total_len': data[1] if len(data) > 1 else 0,
                        'data': data,
                    })
                    
                    # Start tracking this message
                    # Use field_6bit as base for now
                    key = (can_id, field_6bit)
                    pending[key] = {
                        'start_byte0': byte0,
                        'byte0_sequence': [byte0],
                        'frames': [data],
                        'total_len': data[1] if len(data) > 1 else 0,
                        'payload_so_far': len(data) - 4 if len(data) > 4 else 0,
                    }
                else:
                    # CONTINUATION frame
                    results['cont_frames'].append({
                        'can_id': can_id,
                        'byte0': byte0,
                        'field_6bit': field_6bit,
                        'group_2bit': group_2bit,
                        'index_4bit': index_4bit,
                        'data': data,
                    })
                    
                    # Try to match to a pending message
                    # Try matching by group_2bit (Hypothesis B)
                    matched = False
                    for key, msg in list(pending.items()):
                        msg_can_id, msg_base = key
                        if msg_can_id != can_id:
                            continue
                        
                        start_group = (msg['start_byte0'] >> 4) & 0x03
                        if group_2bit == start_group:
                            msg['byte0_sequence'].append(byte0)
                            msg['frames'].append(data)
                            msg['payload_so_far'] += len(data) - 1
                            
                            # Check if complete
                            if msg['payload_so_far'] >= msg['total_len'] - 2:
                                results['messages'].append({
                                    'can_id': can_id,
                                    'start_byte0': msg['start_byte0'],
                                    'byte0_sequence': msg['byte0_sequence'],
                                    'num_frames': len(msg['frames']),
                                })
                                del pending[key]
                            matched = True
                            break
    
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
    
    return results


def analyze_all_traces(base_dir: str):
    """Analyze all CSV traces in directory tree."""
    all_start_frames = []
    all_cont_frames = []
    all_messages = []
    
    # Find all CSV files
    base_path = Path(base_dir)
    csv_files = list(base_path.rglob("*.csv"))
    
    print(f"Found {len(csv_files)} CSV files")
    
    for csv_file in csv_files:
        results = analyze_trace(str(csv_file))
        all_start_frames.extend(results['start_frames'])
        all_cont_frames.extend(results['cont_frames'])
        all_messages.extend(results['messages'])
    
    return all_start_frames, all_cont_frames, all_messages


def print_analysis(start_frames, cont_frames, messages):
    """Print analysis of the collected data."""
    
    print("\n" + "="*70)
    print("BAP LONG MESSAGE INDEX FORMAT ANALYSIS")
    print("="*70)
    
    # Unique byte0 values for START frames
    start_byte0_values = sorted(set(f['byte0'] for f in start_frames))
    print(f"\n## Unique START frame byte0 values ({len(start_byte0_values)} total):")
    print("Byte0  Binary     6-bit  Grp(2b) Idx(4b)")
    for b in start_byte0_values:
        field = b & 0x3F
        grp = (b >> 4) & 0x03
        idx = b & 0x0F
        print(f"  {b:02X}   {b:08b}   {field:2d}     {grp}       {idx}")
    
    # Unique byte0 values for CONT frames
    cont_byte0_values = sorted(set(f['byte0'] for f in cont_frames))
    print(f"\n## Unique CONTINUATION frame byte0 values ({len(cont_byte0_values)} total):")
    print("Byte0  Binary     6-bit  Grp(2b) Idx(4b)")
    for b in cont_byte0_values:
        field = b & 0x3F
        grp = (b >> 4) & 0x03
        idx = b & 0x0F
        print(f"  {b:02X}   {b:08b}   {field:2d}     {grp}       {idx}")
    
    # Look for messages with many continuations (>4) to see index progression
    print(f"\n## Messages with >4 continuation frames ({len([m for m in messages if m['num_frames'] > 5])} found):")
    for msg in messages:
        if msg['num_frames'] > 5:
            byte0_hex = ' '.join(f'{b:02X}' for b in msg['byte0_sequence'])
            print(f"\nCAN {msg['can_id']:08X}: {msg['num_frames']} frames")
            print(f"  Byte0 sequence: {byte0_hex}")
            
            # Analyze the progression
            print("  Frame  Byte0  Grp  Idx  6-bit")
            for i, b in enumerate(msg['byte0_sequence']):
                frame_type = "START" if i == 0 else f"CONT{i}"
                grp = (b >> 4) & 0x03
                idx = b & 0x0F
                field = b & 0x3F
                print(f"  {frame_type:6} {b:02X}    {grp}    {idx:2d}   {field:2d}")
    
    # Check for evidence distinguishing hypotheses
    print("\n" + "="*70)
    print("HYPOTHESIS TESTING")
    print("="*70)
    
    # Look for index values > 15 in continuation frames (would prove Hypothesis A)
    high_index_conts = [f for f in cont_frames if (f['byte0'] & 0x0F) > 15]
    print(f"\n1. Continuation frames with index (bits 3-0) > 15: {len(high_index_conts)}")
    if high_index_conts:
        print("   -> This would be impossible under Hypothesis B (4-bit index)")
        for f in high_index_conts[:5]:
            print(f"      {f['byte0']:02X} at CAN {f['can_id']:08X}")
    else:
        print("   -> No evidence against Hypothesis B")
    
    # Look for "group" values (bits 5-4) > 3 in START frames
    # Under Hypothesis B, only values 0-3 are valid
    high_group_starts = [f for f in start_frames if ((f['byte0'] >> 4) & 0x03) > 3]
    print(f"\n2. START frames with group (bits 5-4) > 3: {len(high_group_starts)}")
    # This is actually impossible since we only have 2 bits, but let's check the 6-bit value
    
    # What 6-bit values do we see in START frames?
    start_6bit_values = sorted(set(f['field_6bit'] for f in start_frames))
    print(f"\n3. Unique 6-bit field values in START frames: {start_6bit_values}")
    
    # Are they all multiples of 16? (0, 16, 32, 48)
    multiples_of_16 = [v for v in start_6bit_values if v % 16 == 0]
    non_multiples = [v for v in start_6bit_values if v % 16 != 0]
    print(f"   Multiples of 16: {multiples_of_16}")
    print(f"   Non-multiples of 16: {non_multiples}")
    
    if non_multiples:
        print("   -> START frames have index != 0, suggests they can start mid-sequence")
    else:
        print("   -> All START frames have index=0, consistent with Hypothesis B")
    
    # Analyze continuation progressions within messages
    print("\n4. Continuation index progressions within messages:")
    index_increments = defaultdict(int)
    for msg in messages:
        if len(msg['byte0_sequence']) < 2:
            continue
        for i in range(1, len(msg['byte0_sequence'])):
            prev_idx = msg['byte0_sequence'][i-1] & 0x0F
            curr_idx = msg['byte0_sequence'][i] & 0x0F
            if i == 1:
                # First cont after start
                diff = curr_idx - prev_idx
            else:
                diff = (curr_idx - prev_idx) & 0x0F  # Handle wrap
            index_increments[diff] += 1
    
    print(f"   Index differences between consecutive frames: {dict(index_increments)}")


if __name__ == "__main__":
    base_dir = r"C:\Users\thomas\Projects\egolf-canbus\workfiles"
    start_frames, cont_frames, messages = analyze_all_traces(base_dir)
    
    print(f"\nCollected {len(start_frames)} START frames")
    print(f"Collected {len(cont_frames)} CONTINUATION frames")
    print(f"Assembled {len(messages)} complete messages")
    
    print_analysis(start_frames, cont_frames, messages)
