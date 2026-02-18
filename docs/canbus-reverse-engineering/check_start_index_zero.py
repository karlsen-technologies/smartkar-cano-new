"""
Simple check: Do all BAP START frames have index = 0?

If 6-bit field is only ever 0, 16, 32, or 48 for START frames,
then the format is confirmed as 2-bit group + 4-bit index,
and groups > 3 are not possible.
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


def main():
    base_dir = Path(r"C:\Users\thomas\Projects\egolf-canbus\workfiles")
    csv_files = list(base_dir.rglob("*.csv"))
    
    # Track START frame 6-bit values
    start_values = defaultdict(lambda: defaultdict(int))  # can_id -> 6-bit value -> count
    
    for filepath in csv_files:
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
                    
                    if is_long and not is_cont:
                        # START frame
                        field_6bit = byte0 & 0x3F
                        start_values[can_id][field_6bit] += 1
        except:
            pass
    
    # Analyze results
    print("BAP START FRAME 6-BIT VALUES BY CAN ID")
    print("=" * 70)
    
    # Expected values if index always starts at 0
    expected = {0, 16, 32, 48}
    
    # Known BAP CAN IDs (0x1733xxxx pattern)
    bap_ids = []
    non_bap_ids = []
    
    for can_id in sorted(start_values.keys()):
        is_bap = (can_id >> 16) == 0x1733
        values = start_values[can_id]
        
        if is_bap:
            bap_ids.append(can_id)
        else:
            non_bap_ids.append(can_id)
    
    print("\n--- KNOWN BAP CHANNEL IDs (0x1733xxxx) ---\n")
    
    all_bap_conform = True
    for can_id in bap_ids:
        values = start_values[can_id]
        unexpected = {v for v in values.keys() if v not in expected}
        
        values_str = ', '.join(f"{v}({values[v]}x)" for v in sorted(values.keys()))
        
        if unexpected:
            all_bap_conform = False
            print(f"CAN {can_id:08X}: {values_str}  <-- UNEXPECTED: {unexpected}")
        else:
            print(f"CAN {can_id:08X}: {values_str}")
    
    print("\n--- NON-BAP CAN IDs ---\n")
    for can_id in non_bap_ids:
        values = start_values[can_id]
        unexpected = {v for v in values.keys() if v not in expected}
        
        values_str = ', '.join(f"{v}({values[v]}x)" for v in sorted(values.keys()))
        
        if unexpected:
            print(f"CAN {can_id:08X}: {values_str}  <-- has non-zero index")
        else:
            print(f"CAN {can_id:08X}: {values_str}")
    
    print("\n" + "=" * 70)
    if all_bap_conform:
        print("RESULT: All BAP channel START frames have index = 0")
        print("        Only groups 0, 1, 2, 3 are used (6-bit = 0, 16, 32, 48)")
        print("        FORMAT CONFIRMED: [Long:1][Cont:1][Group:2][Index:4]")
    else:
        print("RESULT: Some BAP START frames have non-zero index!")


if __name__ == "__main__":
    main()
