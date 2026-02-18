"""
Analyze START frames that begin at non-zero index.
What do their continuation sequences look like?
"""

import csv
from pathlib import Path

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
                group = (field_6bit >> 4) & 0x03
                index = field_6bit & 0x0F
                
                if not is_cont:
                    # START frame
                    key = (can_id, group)  # Track by CAN ID and group
                    pending[key] = {
                        'can_id': can_id,
                        'start_byte0': byte0,
                        'start_6bit': field_6bit,
                        'start_group': group,
                        'start_index': index,
                        'total_len': data[1] if len(data) > 1 else 0,
                        'byte0_sequence': [byte0],
                        'payload_len': len(data) - 4,
                    }
                else:
                    # CONTINUATION frame
                    key = (can_id, group)
                    if key in pending:
                        msg = pending[key]
                        msg['byte0_sequence'].append(byte0)
                        msg['payload_len'] += len(data) - 1
                        
                        if msg['payload_len'] >= msg['total_len'] - 2:
                            # Only save if start index was non-zero
                            if msg['start_index'] != 0:
                                results.append(msg)
                            del pending[key]
    except Exception as e:
        pass
    
    return results


def main():
    base_dir = Path(r"C:\Users\thomas\Projects\egolf-canbus\workfiles")
    csv_files = list(base_dir.rglob("*.csv"))
    
    all_results = []
    for f in csv_files:
        all_results.extend(analyze_trace(str(f)))
    
    print(f"Found {len(all_results)} messages with non-zero start index")
    print("=" * 70)
    
    # Group by start_index
    by_start_idx = {}
    for msg in all_results:
        idx = msg['start_index']
        if idx not in by_start_idx:
            by_start_idx[idx] = []
        by_start_idx[idx].append(msg)
    
    for start_idx in sorted(by_start_idx.keys()):
        msgs = by_start_idx[start_idx]
        print(f"\n--- START INDEX = {start_idx} ({len(msgs)} messages) ---")
        
        # Show a few examples
        for msg in msgs[:5]:
            seq = msg['byte0_sequence']
            print(f"\nCAN {msg['can_id']:08X}, Group {msg['start_group']}:")
            print(f"  START byte0: 0x{msg['start_byte0']:02X} (6-bit={msg['start_6bit']}, grp={msg['start_group']}, idx={msg['start_index']})")
            
            # Show sequence
            seq_hex = ' '.join(f"{b:02X}" for b in seq)
            print(f"  Sequence: {seq_hex}")
            
            # Show indices
            indices = [b & 0x0F for b in seq]
            print(f"  Indices:  {indices}")
            
            # Check if first CONT matches START index
            if len(seq) > 1:
                first_cont_idx = seq[1] & 0x0F
                if first_cont_idx == msg['start_index']:
                    print(f"  -> First CONT index MATCHES start ({first_cont_idx})")
                else:
                    print(f"  -> First CONT index DIFFERS: start={msg['start_index']}, first_cont={first_cont_idx}")


if __name__ == "__main__":
    main()
