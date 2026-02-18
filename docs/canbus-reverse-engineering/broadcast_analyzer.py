#!/usr/bin/env python3
"""
VW e-Golf Broadcast Channel Analyzer

Analyzes standard (non-BAP) CAN IDs to understand their data content
by comparing values across different vehicle states.
"""

import csv
import sys
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional


@dataclass
class CANMessage:
    timestamp: int
    can_id: int
    data: List[int]
    
    @classmethod
    def from_row(cls, row: Dict) -> Optional['CANMessage']:
        try:
            data = []
            for i in range(1, 9):
                val = row.get(f'D{i}', '').strip()
                if val:
                    data.append(int(val, 16))
            
            can_id = int(row['ID'], 16)
            extended = row['Extended'].lower() == 'true'
            
            # Skip extended IDs (BAP traffic)
            if extended or can_id > 0x7FF:
                return None
                
            return cls(
                timestamp=int(row['Time Stamp']) if row['Time Stamp'] else 0,
                can_id=can_id,
                data=data
            )
        except (ValueError, KeyError):
            return None


def load_standard_messages(filepath: str) -> List[CANMessage]:
    """Load only standard CAN ID messages from a trace."""
    messages = []
    with open(filepath, 'r', newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            msg = CANMessage.from_row(row)
            if msg:
                messages.append(msg)
    return messages


def analyze_single_trace(filepath: str) -> Dict[int, List[List[int]]]:
    """Analyze a single trace and return unique data patterns per CAN ID."""
    messages = load_standard_messages(filepath)
    
    # Group by CAN ID
    by_id: Dict[int, Set[tuple]] = defaultdict(set)
    for msg in messages:
        by_id[msg.can_id].add(tuple(msg.data))
    
    # Convert to list
    result = {}
    for can_id, patterns in by_id.items():
        result[can_id] = [list(p) for p in patterns]
    
    return result


def find_changing_bytes(patterns: List[List[int]]) -> List[int]:
    """Find which byte positions change across patterns."""
    if not patterns or len(patterns) < 2:
        return []
    
    max_len = max(len(p) for p in patterns)
    changing = []
    
    for i in range(max_len):
        values = set()
        for p in patterns:
            if i < len(p):
                values.add(p[i])
            else:
                values.add(None)
        
        if len(values) > 1:
            changing.append(i)
    
    return changing


# Known/suspected CAN ID meanings based on common VW patterns
KNOWN_IDS = {
    0x0FD: "Steering angle / wheel speed",
    0x101: "Speed / RPM related", 
    0x107: "Unknown periodic",
    0x12B: "Unknown status",
    0x131: "Gear/transmission status",
    0x184: "Battery/HV status",
    0x187: "Unknown status",
    0x1FA: "Unknown",
    0x2AF: "Unknown periodic",
    0x30B: "Unknown",
    0x30D: "Unknown",
    0x31B: "Unknown",
    0x31E: "Unknown",
    0x32A: "Unknown",
    0x32E: "Unknown",
    0x366: "Unknown",
    0x386: "Unknown",
    0x391: "Unknown",
    0x395: "Unknown",
    0x3B1: "Unknown",
    0x3B5: "Unknown",
    0x3BE: "Unknown",
    0x3C0: "Unknown counter",
    0x3C7: "Unknown",
    0x3CE: "Unknown",
    0x3CF: "Unknown",
    0x3D0: "Unknown",
    0x3D1: "Unknown",
    0x3D4: "Unknown",
    0x3D5: "Unknown counter/timer",
    0x3D6: "Unknown",
    0x3DA: "Unknown",
    0x3DC: "Charge plug / power status",
    0x3E5: "Unknown",
    0x3E9: "Unknown",
    0x3EB: "Unknown",
    0x462: "Unknown",
    0x481: "Unknown",
    0x483: "Unknown",
    0x484: "Unknown",
    0x485: "Unknown",
    0x486: "Unknown",
    0x48B: "Unknown",
    0x52A: "Unknown",
    0x530: "Unknown",
    0x551: "Unknown",
    0x556: "Unknown",
    0x569: "Unknown",
    0x583: "Door/lock status",
    0x584: "Unknown",
    0x585: "Door/window status",
    0x588: "Unknown",
    0x592: "Unknown",
    0x593: "Unknown",
    0x59E: "Unknown",
    0x5A0: "Unknown",
    0x5A7: "OCU heartbeat",
    0x5AC: "Unknown",
    0x5B5: "Unknown",
    0x5CA: "Unknown",
    0x5E1: "Temperature/climate related",
    0x5E7: "Unknown",
    0x5E9: "Unknown",
    0x5EA: "Unknown",
    0x5EB: "Unknown",
    0x5EC: "Unknown",
    0x5F0: "Range/SOC related",
    0x5F2: "Unknown",
    0x5F4: "Range/SOC related",
    0x640: "Unknown",
    0x641: "Unknown",
    0x643: "Unknown",
    0x647: "Unknown",
    0x64E: "Unknown",
    0x650: "Unknown",
    0x656: "Unknown",
    0x658: "Unknown",
    0x65A: "Unknown",
    0x65E: "Unknown",
    0x663: "Unknown",
    0x668: "Unknown",
    0x66E: "Unknown",
    0x670: "Unknown",
    0x671: "Unknown",
    0x6A6: "Unknown",
    0x6AE: "Unknown",
    0x6B0: "Unknown",
    0x6B2: "Unknown",
    0x6B4: "Unknown (VW identifier)",
    0x6B5: "Unknown",
    0x6B6: "Unknown",
    0x6B7: "Unknown",
    0x6B8: "Unknown",
    0x6B9: "Unknown",
}


def compare_traces(trace1: str, trace2: str, label1: str, label2: str):
    """Compare two traces to find differences."""
    print(f"\n{'='*80}")
    print(f"Comparing: {label1} vs {label2}")
    print('='*80)
    
    data1 = analyze_single_trace(trace1)
    data2 = analyze_single_trace(trace2)
    
    all_ids = sorted(set(data1.keys()) | set(data2.keys()))
    
    print(f"\nCAN IDs in {label1}: {len(data1)}")
    print(f"CAN IDs in {label2}: {len(data2)}")
    
    print(f"\n{'CAN ID':<10} {'Description':<30} {'Diff?':<8} Notes")
    print("-" * 80)
    
    for can_id in all_ids:
        desc = KNOWN_IDS.get(can_id, "Unknown")
        
        patterns1 = data1.get(can_id, [])
        patterns2 = data2.get(can_id, [])
        
        if not patterns1:
            print(f"0x{can_id:03X}     {desc:<30} {'NEW':<8} Only in {label2}")
        elif not patterns2:
            print(f"0x{can_id:03X}     {desc:<30} {'GONE':<8} Only in {label1}")
        else:
            # Check if data differs
            set1 = set(tuple(p) for p in patterns1)
            set2 = set(tuple(p) for p in patterns2)
            
            if set1 != set2:
                # Find specific differences
                only1 = set1 - set2
                only2 = set2 - set1
                
                if len(only1) <= 3 and len(only2) <= 3:
                    print(f"0x{can_id:03X}     {desc:<30} {'DIFF':<8}")
                    for p in sorted(only1)[:2]:
                        print(f"           {label1}: {' '.join(f'{b:02X}' for b in p)}")
                    for p in sorted(only2)[:2]:
                        print(f"           {label2}: {' '.join(f'{b:02X}' for b in p)}")


def analyze_door_lock_traces():
    """Analyze lock/unlock traces to find door lock CAN messages."""
    print("\n" + "="*80)
    print("Door Lock/Unlock Analysis")
    print("="*80)
    
    lock_trace = Path("workfiles/traces/lock unlock pre.csv")
    
    if not lock_trace.exists():
        print(f"Lock trace not found: {lock_trace}")
        return
    
    messages = load_standard_messages(str(lock_trace))
    
    # Look for 0x583 which is suspected door status
    door_patterns: Dict[int, List[Tuple[int, List[int]]]] = defaultdict(list)
    
    for msg in messages:
        if msg.can_id == 0x583:
            door_patterns[msg.can_id].append((msg.timestamp, msg.data))
    
    print(f"\nCAN ID 0x583 (suspected door status):")
    print("-" * 60)
    
    seen = set()
    for ts, data in door_patterns[0x583]:
        key = tuple(data)
        if key not in seen:
            seen.add(key)
            print(f"  {' '.join(f'{b:02X}' for b in data)}")
            # Decode individual bytes
            print(f"    Byte 0: 0x{data[0]:02X} = {data[0]:08b}")
            print(f"    Byte 1: 0x{data[1]:02X} = {data[1]:08b}")
            print(f"    Byte 2: 0x{data[2]:02X} = {data[2]:08b}")
            print(f"    Byte 3: 0x{data[3]:02X} = {data[3]:08b}")
            print(f"    Byte 4: 0x{data[4]:02X} = {data[4]:08b}")
            print(f"    Byte 5: 0x{data[5]:02X} = {data[5]:08b}")
            print(f"    Byte 6: 0x{data[6]:02X} = {data[6]:08b}")
            print(f"    Byte 7: 0x{data[7]:02X} = {data[7]:08b}")
            print()


def analyze_charging_traces():
    """Analyze charging-related CAN IDs."""
    print("\n" + "="*80)
    print("Charging Related CAN IDs Analysis")  
    print("="*80)
    
    # Look at charge start trace
    charge_trace = Path("workfiles/traces/charging/ext_charge_start_btn.csv")
    
    if not charge_trace.exists():
        print(f"Charge trace not found: {charge_trace}")
        return
        
    messages = load_standard_messages(str(charge_trace))
    
    # Focus on suspected charging-related IDs
    charge_ids = [0x3DC, 0x5F0, 0x5F4, 0x184, 0x187]
    
    for can_id in charge_ids:
        patterns = set()
        for msg in messages:
            if msg.can_id == can_id:
                patterns.add(tuple(msg.data))
        
        if patterns:
            print(f"\nCAN ID 0x{can_id:03X} ({KNOWN_IDS.get(can_id, 'Unknown')}):")
            print("-" * 60)
            for p in sorted(patterns)[:10]:
                print(f"  {' '.join(f'{b:02X}' for b in p)}")


def analyze_id_in_trace(trace_path: str, can_id: int):
    """Deep analyze a specific CAN ID in a trace."""
    print(f"\n{'='*80}")
    print(f"Deep Analysis of CAN ID 0x{can_id:03X} in {Path(trace_path).name}")
    print('='*80)
    
    messages = load_standard_messages(trace_path)
    
    target_msgs = [m for m in messages if m.can_id == can_id]
    
    if not target_msgs:
        print(f"No messages found for CAN ID 0x{can_id:03X}")
        return
    
    print(f"\nTotal messages: {len(target_msgs)}")
    
    # Get unique patterns
    patterns = set()
    for msg in target_msgs:
        patterns.add(tuple(msg.data))
    
    print(f"Unique patterns: {len(patterns)}")
    
    # Find changing bytes
    pattern_list = [list(p) for p in patterns]
    changing = find_changing_bytes(pattern_list)
    
    print(f"Changing byte positions: {changing}")
    
    print(f"\nAll unique patterns:")
    for i, p in enumerate(sorted(patterns)):
        data_str = ' '.join(f'{b:02X}' for b in p)
        print(f"  [{i+1:3d}] {data_str}")
        
        if len(patterns) <= 10:
            # Show bit breakdown for small pattern sets
            for byte_idx in changing[:4]:  # First 4 changing bytes
                if byte_idx < len(p):
                    print(f"        Byte {byte_idx}: 0x{p[byte_idx]:02X} = {p[byte_idx]:08b}")


def print_summary():
    """Print a summary of common broadcast CAN IDs."""
    print("\n" + "="*80)
    print("VW e-Golf Standard CAN ID Summary")
    print("="*80)
    print("\nThese are non-BAP broadcast messages on the comfort CAN bus:")
    print()
    
    categories = {
        "Battery/Charging": [0x184, 0x187, 0x3DC, 0x5F0, 0x5F4],
        "Door/Lock Status": [0x583, 0x585, 0x593],
        "Climate": [0x5E1],
        "Periodic/Heartbeat": [0x107, 0x131, 0x5A7],
        "Vehicle State": [0x0FD, 0x101, 0x366, 0x391],
    }
    
    for category, ids in categories.items():
        print(f"\n{category}:")
        for can_id in ids:
            desc = KNOWN_IDS.get(can_id, "Unknown")
            print(f"  0x{can_id:03X} - {desc}")


def main():
    if len(sys.argv) < 2:
        print("VW e-Golf Broadcast Channel Analyzer")
        print("\nUsage:")
        print(f"  {sys.argv[0]} summary              - Print summary of known IDs")
        print(f"  {sys.argv[0]} <trace.csv>          - Analyze a single trace")
        print(f"  {sys.argv[0]} <trace.csv> 0x5F0    - Deep analyze specific CAN ID")
        print(f"  {sys.argv[0]} doors                - Analyze door lock traces")
        print(f"  {sys.argv[0]} charging             - Analyze charging traces")
        print(f"  {sys.argv[0]} compare <t1> <t2>    - Compare two traces")
        return
    
    cmd = sys.argv[1]
    
    if cmd == 'summary':
        print_summary()
    
    elif cmd == 'doors':
        analyze_door_lock_traces()
    
    elif cmd == 'charging':
        analyze_charging_traces()
    
    elif cmd == 'compare' and len(sys.argv) >= 4:
        t1, t2 = sys.argv[2], sys.argv[3]
        l1 = Path(t1).stem
        l2 = Path(t2).stem
        compare_traces(t1, t2, l1, l2)
    
    elif Path(cmd).exists():
        if len(sys.argv) >= 3:
            # Deep analyze specific ID
            can_id = int(sys.argv[2], 16)
            analyze_id_in_trace(cmd, can_id)
        else:
            # General analysis
            data = analyze_single_trace(cmd)
            print(f"\n{'='*80}")
            print(f"Trace Analysis: {Path(cmd).name}")
            print('='*80)
            print(f"\nUnique standard CAN IDs: {len(data)}")
            print(f"\n{'CAN ID':<10} {'Description':<35} {'Patterns':<10}")
            print("-" * 60)
            
            for can_id in sorted(data.keys()):
                patterns = data[can_id]
                desc = KNOWN_IDS.get(can_id, "Unknown")
                print(f"0x{can_id:03X}     {desc:<35} {len(patterns):>5}")
    
    else:
        print(f"File not found: {cmd}")


if __name__ == '__main__':
    main()
