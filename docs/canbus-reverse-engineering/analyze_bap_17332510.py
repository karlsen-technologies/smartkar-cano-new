"""
Analyze BAP messages from CAN ID 0x17332510 (BatteryControl FSG responses)
"""
import csv
from collections import Counter

# BAP decoding functions
def decode_bap_header(byte0, byte1):
    """Decode BAP header bytes to components."""
    opcode = (byte0 >> 4) & 0x07
    device_id = ((byte0 & 0x0F) << 2) | ((byte1 >> 6) & 0x03)
    function_id = byte1 & 0x3F
    return opcode, device_id, function_id

OPCODES = {0: 'Reset', 1: 'Get', 2: 'SetGet', 3: 'HeartbeatStatus', 4: 'Status', 5: 'StatusAck', 6: 'Ack', 7: 'Error'}
FUNCTIONS_25 = {
    0x01: 'GetAllProperties', 0x02: 'BAP-Config', 0x03: 'FunctionList', 0x04: 'HeartbeatConfig',
    0x0E: 'FSG-Setup', 0x0F: 'FSG-OperationState', 0x10: 'PlugState', 0x11: 'ChargeState',
    0x12: 'ClimateState', 0x13: 'Func13', 0x14: 'Func14', 0x15: 'Func15', 0x16: 'Func16',
    0x18: 'ClimateOperationModeInstallation', 0x19: 'ProfilesArray', 0x1A: 'PowerProvidersArray'
}

# Extract frames from trace
frames = []
with open('workfiles/traces/charging/charge_start_externally.csv', 'r') as f:
    reader = csv.reader(f)
    next(reader)  # skip header
    for row in reader:
        if len(row) >= 14 and row[1] == '17332510':
            timestamp = int(row[0])
            dlc = int(row[5])
            data = [int(row[i], 16) for i in range(6, 6+dlc)]
            frames.append({'ts': timestamp, 'dlc': dlc, 'data': data})

print(f"Total frames from 0x17332510: {len(frames)}")
print("=" * 100)

# Categorize frames
short_msgs = []
long_starts = []
long_conts = []

for f in frames:
    data = f['data']
    is_long = bool(data[0] & 0x80)
    is_cont = bool(data[0] & 0x40)
    
    if not is_long:
        short_msgs.append(f)
    elif not is_cont:
        long_starts.append(f)
    else:
        long_conts.append(f)

print(f"Short messages: {len(short_msgs)}")
print(f"Long start frames: {len(long_starts)}")
print(f"Long continuation frames: {len(long_conts)}")
print()

# Parse and reassemble messages
pending = {}  # base_idx -> message_state
completed_messages = []
orphan_conts = []

for f in frames:
    data = f['data']
    is_long = bool(data[0] & 0x80)
    is_cont = bool(data[0] & 0x40)
    
    if not is_long:
        # Short message - complete in one frame
        opcode, device_id, function_id = decode_bap_header(data[0], data[1])
        completed_messages.append({
            'type': 'short',
            'ts': f['ts'],
            'opcode': opcode,
            'device_id': device_id,
            'function_id': function_id,
            'payload': data[2:],
            'raw': data
        })
    
    elif not is_cont:
        # Long START frame
        base_idx = (data[0] >> 2) & 0x0F
        total_len = data[1]
        opcode, device_id, function_id = decode_bap_header(data[2], data[3])
        
        pending[base_idx] = {
            'ts': f['ts'],
            'base_idx': base_idx,
            'total_len': total_len,
            'opcode': opcode,
            'device_id': device_id,
            'function_id': function_id,
            'payload': list(data[4:]),
            'expected_idx': base_idx,
            'expected_seq': 0,
            'raw_frames': [data]
        }
    
    else:
        # Long CONTINUATION frame
        cont_idx = (data[0] >> 2) & 0x0F
        cont_seq = data[0] & 0x03
        
        # Find matching pending message
        matched = False
        for base_idx, msg in list(pending.items()):
            if cont_idx == msg['expected_idx'] and cont_seq == msg['expected_seq']:
                # Match!
                msg['payload'].extend(data[1:])
                msg['raw_frames'].append(data)
                
                # Update expected next
                msg['expected_seq'] += 1
                if msg['expected_seq'] >= 4:
                    msg['expected_seq'] = 0
                    msg['expected_idx'] = (msg['expected_idx'] + 1) & 0x0F
                
                # Check if complete
                if len(msg['payload']) >= msg['total_len']:
                    msg['payload'] = msg['payload'][:msg['total_len']]  # trim excess
                    completed_messages.append({
                        'type': 'long',
                        'ts': msg['ts'],
                        'opcode': msg['opcode'],
                        'device_id': msg['device_id'],
                        'function_id': msg['function_id'],
                        'payload': msg['payload'],
                        'raw_frames': msg['raw_frames'],
                        'base_idx': msg['base_idx']
                    })
                    del pending[base_idx]
                matched = True
                break
        
        if not matched:
            orphan_conts.append({'ts': f['ts'], 'data': data, 'cont_idx': cont_idx, 'cont_seq': cont_seq})

print(f"Completed messages: {len(completed_messages)}")
print(f"Orphan continuations: {len(orphan_conts)}")
print(f"Pending incomplete: {len(pending)}")
print()

# Display all completed messages
print("=" * 100)
print("COMPLETED BAP MESSAGES FROM 0x17332510 (BatteryControl FSG)")
print("=" * 100)

for i, msg in enumerate(completed_messages):
    opcode_name = OPCODES.get(msg['opcode'], f"Unknown({msg['opcode']})")
    func_name = FUNCTIONS_25.get(msg['function_id'], f"Func{msg['function_id']:02X}")
    
    print(f"\n[{i+1}] ts={msg['ts']:>12} | {msg['type']:5} | Device=0x{msg['device_id']:02X} | {opcode_name:15} | {func_name}")
    
    payload_hex = ' '.join(f'{b:02X}' for b in msg['payload'])
    print(f"    Payload ({len(msg['payload'])} bytes): {payload_hex}")
    
    # Decode specific functions
    if msg['function_id'] == 0x11 and len(msg['payload']) >= 9:  # ChargeState
        p = msg['payload']
        charge_mode = (p[0] >> 4) & 0x0F
        charge_state = p[0] & 0x0F
        soc = p[1]
        remaining_time = p[2]
        current_range = p[3]
        unit_range = p[4]
        current = p[5]
        battery_climate = (p[6] >> 4) & 0x0F
        start_reason = (p[8] >> 4) if len(p) > 8 else 0
        target_soc = p[8] & 0x0F if len(p) > 8 else 0
        
        mode_names = {0: 'OFF', 1: 'AC', 2: 'DC', 3: 'COND', 4: 'AC+COND', 5: 'DC+COND', 15: 'INIT'}
        state_names = {0: 'INIT', 1: 'IDLE', 2: 'RUNNING', 3: 'CONSERV', 8: 'COMPLETED'}
        reason_names = {0: 'INIT', 1: 'Timer1', 2: 'Timer2', 3: 'Timer3', 4: 'Immediately', 5: 'PushButton'}
        
        print(f"    -> ChargeMode={mode_names.get(charge_mode, charge_mode)}, ChargeState={state_names.get(charge_state, charge_state)}")
        print(f"    -> SOC={soc}%, RemainingTime={remaining_time}min, Range={current_range}, Current={current}")
        print(f"    -> StartReason={reason_names.get(start_reason, start_reason)}, TargetSOC={'Max' if target_soc==1 else 'Min' if target_soc==0 else target_soc}")
    
    elif msg['function_id'] == 0x12 and len(msg['payload']) >= 5:  # ClimateState
        p = msg['payload']
        climate_mode = p[0]
        current_temp = p[1]
        temp_unit = p[2]
        climate_time = p[3] | (p[4] << 8) if len(p) > 4 else p[3]
        climate_state = (p[5] >> 4) & 0x0F if len(p) > 5 else 0
        
        actual_temp = (current_temp + 100) / 10.0 if current_temp != 0xFF else 'N/A'
        print(f"    -> ClimateMode=0x{climate_mode:02X}, Temp={actual_temp}C, State={climate_state}")
    
    elif msg['function_id'] == 0x10 and len(msg['payload']) >= 2:  # PlugState
        p = msg['payload']
        lock_setup = (p[0] >> 4) & 0x0F
        lock_state = p[0] & 0x0F
        supply_state = (p[1] >> 4) & 0x0F
        plug_state = p[1] & 0x0F
        
        print(f"    -> LockSetup={lock_setup}, LockState={lock_state}, SupplyState={supply_state}, PlugState={plug_state}")
    
    if msg['type'] == 'long':
        print(f"    Base index: {msg.get('base_idx', 'N/A')}, Frames: {len(msg.get('raw_frames', []))}")

# Summary by function
print("\n" + "=" * 100)
print("SUMMARY BY FUNCTION")
print("=" * 100)
func_counts = Counter((msg['function_id'], msg['opcode']) for msg in completed_messages)
for (func_id, opcode), count in sorted(func_counts.items()):
    func_name = FUNCTIONS_25.get(func_id, f"Func{func_id:02X}")
    opcode_name = OPCODES.get(opcode, f"Op{opcode}")
    print(f"  {func_name:30} + {opcode_name:15}: {count} messages")

# Show orphan continuations if any
if orphan_conts:
    print("\n" + "=" * 100)
    print("ORPHAN CONTINUATIONS (no matching start)")
    print("=" * 100)
    for oc in orphan_conts:
        print(f"  ts={oc['ts']}, idx={oc['cont_idx']}, seq={oc['cont_seq']}, data={' '.join(f'{b:02X}' for b in oc['data'])}")

# Show pending incomplete if any
if pending:
    print("\n" + "=" * 100)
    print("PENDING INCOMPLETE MESSAGES")
    print("=" * 100)
    for base_idx, msg in pending.items():
        func_name = FUNCTIONS_25.get(msg['function_id'], f"Func{msg['function_id']:02X}")
        print(f"  base_idx={base_idx}, func={func_name}, expected_len={msg['total_len']}, got={len(msg['payload'])}")
