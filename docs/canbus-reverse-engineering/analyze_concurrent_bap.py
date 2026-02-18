"""
Analyze concurrent BAP message assembly from CAN ID 0x17332510
Focus on interleaved frames where multiple messages are in-flight simultaneously
"""
import csv

# Extract frames from trace with timestamps for ordering
frames = []
with open('workfiles/traces/charging/charge_start_externally.csv', 'r') as f:
    reader = csv.reader(f)
    next(reader)  # skip header
    line_num = 1
    for row in reader:
        line_num += 1
        if len(row) >= 14 and row[1] == '17332510':
            timestamp = int(row[0])
            dlc = int(row[5])
            data = [int(row[i], 16) for i in range(6, 6+dlc)]
            frames.append({'ts': timestamp, 'dlc': dlc, 'data': data, 'line': line_num})

print(f"Total frames: {len(frames)}")
print()

# Track pending messages and detect concurrency
pending = {}  # base_idx -> message info
concurrent_events = []
max_concurrent = 0

print("=" * 120)
print("FRAME-BY-FRAME ANALYSIS WITH PENDING MESSAGE TRACKING")
print("=" * 120)

for i, f in enumerate(frames):
    data = f['data']
    is_long = bool(data[0] & 0x80)
    is_cont = bool(data[0] & 0x40)
    
    data_hex = ' '.join(f'{b:02X}' for b in data)
    
    if not is_long:
        # Short message - standalone
        frame_type = "SHORT"
        idx_info = "-"
    elif not is_cont:
        # Long START frame
        base_idx = (data[0] >> 2) & 0x0F
        total_len = data[1]
        frame_type = "START"
        idx_info = f"idx={base_idx}"
        
        # Check if we already have something pending at this index
        if base_idx in pending:
            print(f"  *** WARNING: Overwriting pending message at index {base_idx}!")
        
        pending[base_idx] = {
            'start_frame': i,
            'total_len': total_len,
            'received': 4,  # first 4 payload bytes in start frame
            'expected_idx': base_idx,
            'expected_seq': 0,
            'ts': f['ts']
        }
    else:
        # Long CONTINUATION frame
        cont_idx = (data[0] >> 2) & 0x0F
        cont_seq = data[0] & 0x03
        frame_type = "CONT"
        idx_info = f"idx={cont_idx} seq={cont_seq}"
        
        # Find and update matching pending message
        matched_base = None
        for base_idx, msg in pending.items():
            if cont_idx == msg['expected_idx'] and cont_seq == msg['expected_seq']:
                matched_base = base_idx
                msg['received'] += 7
                msg['expected_seq'] += 1
                if msg['expected_seq'] >= 4:
                    msg['expected_seq'] = 0
                    msg['expected_idx'] = (msg['expected_idx'] + 1) & 0x0F
                
                # Check if complete
                if msg['received'] >= msg['total_len']:
                    del pending[base_idx]
                break
        
        if matched_base is None:
            idx_info += " ORPHAN!"
    
    # Track concurrency
    num_pending = len(pending)
    if num_pending > max_concurrent:
        max_concurrent = num_pending
    
    if num_pending > 1:
        concurrent_events.append({
            'frame': i,
            'ts': f['ts'],
            'pending_indices': list(pending.keys()),
            'num_pending': num_pending
        })
    
    # Print frame info with pending state
    pending_str = ', '.join(f"{k}" for k in sorted(pending.keys())) if pending else "none"
    print(f"[{i:3}] ts={f['ts']:>12} | {frame_type:5} | {idx_info:15} | Pending: [{pending_str:10}] | {data_hex}")

print()
print("=" * 120)
print("CONCURRENCY SUMMARY")
print("=" * 120)
print(f"Maximum concurrent pending messages: {max_concurrent}")
print()

if concurrent_events:
    print("Frames where multiple messages were being assembled simultaneously:")
    print()
    
    # Group consecutive concurrent frames
    groups = []
    current_group = None
    
    for evt in concurrent_events:
        if current_group is None:
            current_group = {'start': evt['frame'], 'end': evt['frame'], 'max': evt['num_pending'], 'indices': set(evt['pending_indices'])}
        elif evt['frame'] == current_group['end'] + 1:
            current_group['end'] = evt['frame']
            current_group['max'] = max(current_group['max'], evt['num_pending'])
            current_group['indices'].update(evt['pending_indices'])
        else:
            groups.append(current_group)
            current_group = {'start': evt['frame'], 'end': evt['frame'], 'max': evt['num_pending'], 'indices': set(evt['pending_indices'])}
    
    if current_group:
        groups.append(current_group)
    
    for g in groups:
        print(f"  Frames {g['start']}-{g['end']}: {g['max']} concurrent messages, indices: {sorted(g['indices'])}")
else:
    print("No concurrent message assembly detected - all messages were sequential.")

print()
print("=" * 120)
print("DETAILED INTERLEAVING EXAMPLES")
print("=" * 120)

# Re-run to find specific interleaving patterns
pending = {}
interleave_examples = []

for i, f in enumerate(frames):
    data = f['data']
    is_long = bool(data[0] & 0x80)
    is_cont = bool(data[0] & 0x40)
    
    if not is_long:
        continue
    elif not is_cont:
        base_idx = (data[0] >> 2) & 0x0F
        total_len = data[1]
        
        # If we have other pending messages, this is interleaving
        if pending:
            interleave_examples.append({
                'type': 'new_start_while_pending',
                'frame': i,
                'new_idx': base_idx,
                'existing_pending': list(pending.keys())
            })
        
        pending[base_idx] = {
            'total_len': total_len,
            'received': 4,
            'expected_idx': base_idx,
            'expected_seq': 0,
        }
    else:
        cont_idx = (data[0] >> 2) & 0x0F
        cont_seq = data[0] & 0x03
        
        for base_idx, msg in list(pending.items()):
            if cont_idx == msg['expected_idx'] and cont_seq == msg['expected_seq']:
                msg['received'] += 7
                msg['expected_seq'] += 1
                if msg['expected_seq'] >= 4:
                    msg['expected_seq'] = 0
                    msg['expected_idx'] = (msg['expected_idx'] + 1) & 0x0F
                
                if msg['received'] >= msg['total_len']:
                    del pending[base_idx]
                break

if interleave_examples:
    print("Cases where a new message started while another was still pending:")
    print()
    for ex in interleave_examples:
        print(f"  Frame {ex['frame']}: Started new message at index {ex['new_idx']}")
        print(f"           While indices {ex['existing_pending']} were still pending")
        print()
else:
    print("No interleaving detected in this trace.")
