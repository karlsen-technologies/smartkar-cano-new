"""
Check messages that START at ambiguous index values like 8, 9, 10, etc.
to see what their continuation patterns look like.

If START at 0x88 (6-bit = 8) has continuations C8, C9, CA...
  → Then it's just a counter, group 0, index starts at 8
  
If START at 0x88 has continuations C0, C1, C2... 
  → Then group and index are separate, this is "group 4, index 0"
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


def analyze_trace(filepath: str):
    """Find messages with START at non-standard indices."""
    results = []
    pending = {}  # (can_id, start_6bit) -> message info
    
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
                group_2bit = (byte0 >> 4) & 0x03
                index_4bit = byte0 & 0x0F
                
                if not is_cont:
                    # START frame
                    # We're interested in ones where index_4bit != 0
                    key = (can_id, field_6bit)
                    pending[key] = {
                        'can_id': can_id,
                        'start_byte0': byte0,
                        'start_6bit': field_6bit,
                        'start_group': group_2bit,
                        'start_index': index_4bit,
                        'total_len': data[1] if len(data) > 1 else 0,
                        'cont_byte0s': [],
                        'frames': [data],
                        'payload_len': len(data) - 4,
                    }
                else:
                    # CONTINUATION frame - try to match
                    # Match by group (bits 5-4)
                    for key, msg in list(pending.items()):
                        msg_can_id, msg_start = key
                        if msg_can_id != can_id:
                            continue
                        
                        start_group = msg['start_group']
                        if group_2bit == start_group:
                            msg['cont_byte0s'].append(byte0)
                            msg['frames'].append(data)
                            msg['payload_len'] += len(data) - 1
                            
                            if msg['payload_len'] >= msg['total_len'] - 2:
                                # Complete - save if interesting
                                if msg['start_index'] != 0:
                                    results.append(msg)
                                del pending[key]
                            break
    except Exception as e:
        print(f"Error: {e}")
    
    return results


def main():
    base_dir = Path(r"C:\Users\thomas\Projects\egolf-canbus\workfiles")
    csv_files = list(base_dir.rglob("*.csv"))
    
    all_results = []
    for f in csv_files:
        all_results.extend(analyze_trace(str(f)))
    
    print(f"Found {len(all_results)} messages with non-zero start index\n")
    
    # Group by start_6bit value
    by_start = defaultdict(list)
    for msg in all_results:
        by_start[msg['start_6bit']].append(msg)
    
    for start_val in sorted(by_start.keys()):
        msgs = by_start[start_val]
        print(f"\n{'='*60}")
        print(f"START 6-bit value: {start_val} (0x{start_val:02X})")
        print(f"  As split 2+4: group={start_val >> 4}, index={start_val & 0x0F}")
        print(f"  Found {len(msgs)} messages")
        
        # Show a few examples
        for msg in msgs[:3]:
            print(f"\n  CAN {msg['can_id']:08X}:")
            print(f"    START: 0x{msg['start_byte0']:02X}")
            cont_hex = ' '.join(f"{b:02X}" for b in msg['cont_byte0s'][:10])
            if len(msg['cont_byte0s']) > 10:
                cont_hex += " ..."
            print(f"    CONTs: {cont_hex}")
            
            # Analyze continuation pattern
            if msg['cont_byte0s']:
                cont_indices = [b & 0x0F for b in msg['cont_byte0s']]
                cont_groups = [(b >> 4) & 0x03 for b in msg['cont_byte0s']]
                print(f"    CONT indices (4-bit): {cont_indices[:10]}")
                print(f"    CONT groups (2-bit):  {cont_groups[:10]}")
                
                # Does first CONT match START index or start at 0?
                first_cont_idx = cont_indices[0]
                if first_cont_idx == msg['start_index']:
                    print(f"    -> First CONT index MATCHES start index ({first_cont_idx})")
                elif first_cont_idx == 0:
                    print(f"    -> First CONT index is 0 (independent of start)")
                else:
                    print(f"    -> First CONT index is {first_cont_idx} (start was {msg['start_index']})")


if __name__ == "__main__":
    main()
