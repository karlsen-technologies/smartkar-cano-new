# Horn, Flash & Lock Commands - VW e-Golf

This document covers the simple CAN messages for horn, flash, and door lock commands.
These are **NOT** BAP protocol - they are standard CAN broadcast messages.

## TM_01 Message (Telematics/Remote Control)

**CAN ID:** `0x5A7` (1447 decimal)  
**DLC:** 8 bytes  
**Transmitter:** Gateway (originally from OCU/Telematics)  
**Cycle time:** 1000ms (normal), 100ms (fast)

This message is sent by the OCU to trigger remote functions like horn, flash, and lock.

### Signal Layout

| Signal | Start Bit | Length | Values |
|--------|-----------|--------|--------|
| TM_Spiegel_Anklappen | 47 | 1 | 0=inactive, 1=fold mirrors |
| TM_Nur_Hupen | 48 | 1 | 0=inactive, 1=horn only |
| TM_Door_Lock | 49 | 1 | 0=inactive, 1=lock doors |
| TM_Door_Unlock | 50 | 1 | 0=inactive, 1=unlock doors |
| TM_Warnblinken | 51 | 1 | 0=inactive, 1=hazard lights (blink) |
| TM_Panik_Alarm | 52 | 1 | 0=inactive, 1=panic alarm (horn + blink) |
| TM_ZV_Signatur | 53 | 11 | Signature/authentication (1-2047) |

### Byte Layout (Little-Endian)

```
Byte 0: bits 0-7   (unused or other signals)
Byte 1: bits 8-15  (unused or other signals)
Byte 2: bits 16-23 (unused or other signals)
Byte 3: bits 24-31 (unused or other signals)
Byte 4: bits 32-39 (unused or other signals)
Byte 5: bits 40-47 - bit 47 = TM_Spiegel_Anklappen
Byte 6: bits 48-55 - bit 48 = TM_Nur_Hupen
                     bit 49 = TM_Door_Lock  
                     bit 50 = TM_Door_Unlock
                     bit 51 = TM_Warnblinken
                     bit 52 = TM_Panik_Alarm
                     bits 53-55 = TM_ZV_Signatur (lower 3 bits)
Byte 7: bits 56-63 - TM_ZV_Signatur (upper 8 bits)
```

### Example Payloads

All values assume other bytes are 0x00 unless specified.

| Action | Byte 6 | Byte 7 | Full Payload (hex) |
|--------|--------|--------|-------------------|
| Horn only | 0x01 | 0x00 | `00 00 00 00 00 00 01 00` |
| Flash only (hazards) | 0x08 | 0x00 | `00 00 00 00 00 00 08 00` |
| Panic (horn + flash) | 0x10 | 0x00 | `00 00 00 00 00 00 10 00` |
| Lock doors | 0x02 | 0x00 | `00 00 00 00 00 00 02 00` |
| Unlock doors | 0x04 | 0x00 | `00 00 00 00 00 00 04 00` |
| Fold mirrors | 0x00 (byte 5=0x80) | - | `00 00 00 00 00 80 00 00` |

**Note:** The TM_ZV_Signatur field may be required for door lock/unlock to work. 
This could be a rolling code or authentication value. Testing needed.

### Usage Notes

1. **Timing:** The message should be sent repeatedly at ~100ms intervals while the action is active
2. **Duration:** For horn/flash, keep sending for desired duration (e.g., 2-3 seconds)
3. **Wake-up:** The bus may need to be woken up first before sending this message
4. **Authentication:** Door lock/unlock may require the signature field to be valid

---

## Alternative: BCM Direct Control

Some functions may also be controllable via BCM (Body Control Module) messages.

### BCM_Anf_Aussenwarnung (0x2A0 / 672)

External warning request to BCM for acoustic warning via horn.
This is a request signal, the BCM decides whether to activate the horn.

---

## MQB Platform Notes

The e-Golf uses the MQB platform. The TM_01 message (0x5A7) appears to be from the 
MLBevo platform DBCs, but similar functionality should exist on MQB.

The MQB DBC shows:
- `RGS_VL_PC_Aktuator_Warnblinken` at 0x3D8 (984) - Pre-crash warning blink
- `BCM1_Warnblink_Taster` - Hazard button status

**Testing Required:** The exact CAN ID and byte layout need to be verified on the 
e-Golf by:
1. Capturing traces when using the Car-Net app to trigger horn/flash
2. Testing sending TM_01 messages and observing car behavior

---

## Implementation Example (Python)

```python
import can

def send_horn_flash(bus, action='horn', duration=2.0):
    """
    Send horn/flash command via TM_01 message.
    
    Args:
        bus: python-can bus instance
        action: 'horn', 'flash', 'panic', 'lock', 'unlock'
        duration: how long to send the command (seconds)
    """
    # Build payload based on action
    payload = [0x00] * 8
    
    if action == 'horn':
        payload[6] = 0x01  # TM_Nur_Hupen
    elif action == 'flash':
        payload[6] = 0x08  # TM_Warnblinken
    elif action == 'panic':
        payload[6] = 0x10  # TM_Panik_Alarm
    elif action == 'lock':
        payload[6] = 0x02  # TM_Door_Lock
    elif action == 'unlock':
        payload[6] = 0x04  # TM_Door_Unlock
    
    msg = can.Message(
        arbitration_id=0x5A7,
        data=payload,
        is_extended_id=False
    )
    
    # Send repeatedly at 100ms intervals
    import time
    start = time.time()
    while time.time() - start < duration:
        bus.send(msg)
        time.sleep(0.1)
    
    # Send with all zeros to deactivate
    msg.data = [0x00] * 8
    bus.send(msg)
```

---

## TODO

- [ ] Verify TM_01 (0x5A7) works on e-Golf MQB platform
- [ ] Determine if TM_ZV_Signatur is required for lock/unlock
- [ ] Check if wake-up sequence is needed before sending
- [ ] Capture Car-Net app traces to confirm exact message format
- [ ] Test fold mirrors command
