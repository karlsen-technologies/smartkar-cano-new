"""
Analyze wrap behavior in BAP long messages with >16 continuations.

We need to see what happens when the lower 4 bits reach 1111 (15).
Does it:
  - Wrap within the same "group" (CF -> C0)?
  - Increment to next group (CF -> D0)?
  - Something else?
"""

import csv
from pathlib import Path
from collections import defaultdict

def parse_csv_row(row: dict):
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


def analyze_trace_for_wraps(filepath: str):
    """Find messages where index wraps (passes through xF)."""
    results = []
    pending = {}
    
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
                
                field_6bit = byte0 & 0x3F
                
                if not is_cont:
                    # START frame
                    key = (can_id, field_6bit)
                    pending[key] = {
                        'can_id': can_id,
                        'start_byte0': byte0,
                        'start_6bit': field_6bit,
                        'total_len': data[1] if len(data) > 1 else 0,
                        'byte0_sequence': [byte0],
                        'payload_len': len(data) - 4,
                    }
                else:
                    # CONTINUATION frame - match by upper 2 bits of 6-bit field
                    cont_group = (field_6bit >> 4) & 0x03
                    
                    for key, msg in list(pending.items()):
                        msg_can_id, msg_start = key
                        if msg_can_id != can_id:
                            continue
                        
                        start_group = (msg['start_6bit'] >> 4) & 0x03
                        if cont_group == start_group:
                            msg['byte0_sequence'].append(byte0)
                            msg['payload_len'] += len(data) - 1
                            
                            if msg['payload_len'] >= msg['total_len'] - 2:
                                # Complete
                                if len(msg['byte0_sequence']) > 16:
                                    results.append(msg)
                                del pending[key]
                            break
    except Exception as e:
        pass
    
    return results


def main():
    base_dir = Path(r"C:\Users\thomas\Projects\egolf-canbus\workfiles")
    csv_files = list(base_dir.rglob("*.csv"))
    
    all_results = []
    for f in csv_files:
        all_results.extend(analyze_trace_for_wraps(str(f)))
    
    print(f"Found {len(all_results)} messages with >16 frames (wrap candidates)")
    print("=" * 70)
    
    # Analyze wrap patterns
    wrap_patterns = defaultdict(list)
    
    for msg in all_results:
        seq = msg['byte0_sequence']
        
        # Find where the wrap happens (index goes from F to something)
        for i in range(1, len(seq)):
            prev_idx = seq[i-1] & 0x0F
            curr_idx = seq[i] & 0x0F
            prev_grp = (seq[i-1] >> 4) & 0x03
            curr_grp = (seq[i] >> 4) & 0x03
            
            # Check for wrap (prev=15, curr could be 0 or something else)
            if prev_idx == 0x0F:
                wrap_info = {
                    'can_id': msg['can_id'],
                    'frame_num': i,
                    'prev_byte0': seq[i-1],
                    'curr_byte0': seq[i],
                    'prev_6bit': seq[i-1] & 0x3F,
                    'curr_6bit': seq[i] & 0x3F,
                    'sequence_around_wrap': seq[max(0,i-3):min(len(seq),i+3)],
                }
                pattern = f"{seq[i-1]:02X} -> {seq[i]:02X}"
                wrap_patterns[pattern].append(wrap_info)
    
    print("\nWRAP PATTERNS OBSERVED:")
    print("-" * 70)
    
    for pattern, instances in sorted(wrap_patterns.items()):
        print(f"\n{pattern}: {len(instances)} occurrences")
        
        # Analyze the pattern
        parts = pattern.split(" -> ")
        before = int(parts[0], 16)
        after = int(parts[1], 16)
        
        before_6bit = before & 0x3F
        after_6bit = after & 0x3F
        before_grp = (before_6bit >> 4) & 0x03
        after_grp = (after_6bit >> 4) & 0x03
        before_idx = before_6bit & 0x0F
        after_idx = after_6bit & 0x0F
        
        print(f"  Before: 6-bit={before_6bit:2d} (group={before_grp}, idx={before_idx})")
        print(f"  After:  6-bit={after_6bit:2d} (group={after_grp}, idx={after_idx})")
        
        if after_idx == 0 and after_grp == before_grp:
            print(f"  --> INDEX WRAPS TO 0, GROUP STAYS SAME (supports Theory 1)")
        elif after_6bit == (before_6bit + 1) & 0x3F:
            print(f"  --> 6-BIT FIELD INCREMENTS (supports simple counter)")
        else:
            print(f"  --> UNEXPECTED PATTERN")
        
        # Show one example
        ex = instances[0]
        seq_hex = ' '.join(f"{b:02X}" for b in ex['sequence_around_wrap'])
        print(f"  Example sequence: {seq_hex}")
    
    # Also show some full sequences for manual inspection
    print("\n" + "=" * 70)
    print("FULL SEQUENCES OF LONG MESSAGES (for manual inspection):")
    print("-" * 70)
    
    shown = set()
    for msg in all_results[:20]:
        seq = msg['byte0_sequence']
        if len(seq) > 16:
            key = (msg['can_id'], tuple(seq))
            if key not in shown:
                shown.add(key)
                print(f"\nCAN {msg['can_id']:08X}: {len(seq)} frames")
                
                # Print in rows of 8
                for row_start in range(0, len(seq), 8):
                    row = seq[row_start:row_start+8]
                    hex_row = ' '.join(f"{b:02X}" for b in row)
                    
                    # Also show the 6-bit values
                    six_bit = ' '.join(f"{b & 0x3F:2d}" for b in row)
                    print(f"  {hex_row}  |  6-bit: {six_bit}")


if __name__ == "__main__":
    main()
