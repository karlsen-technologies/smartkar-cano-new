# OCU Feature Requirements - VW e-Golf

This document defines the features needed to replicate OCU (Online Connectivity Unit) 
functionality for remote car control and monitoring via an app.

## Feature Overview

| Category | Feature | Read | Write | Priority |
|----------|---------|------|-------|----------|
| **Charging** | Battery SOC % | Yes | - | High |
| | Available kWh | Yes | - | High |
| | Charging status (active/inactive) | Yes | - | High |
| | Charge start/stop | - | Yes | High |
| | Plug connected status | Yes | - | High |
| | Charge current/power | Yes | - | Medium |
| | Time to full charge | Yes | - | Medium |
| | Battery temperature | Yes | - | Medium |
| | Battery health/capacity | Yes | - | Low |
| **Climate** | Climate on/off status | Yes | - | High |
| | Start/stop climate | - | Yes | High |
| | Target temperature | Yes | Yes | Medium |
| | Cabin temperature | Yes | - | Medium |
| | Power consumption | Yes | - | Low |
| | Defrost/windscreen mode | - | Yes | Low |
| **Security** | Lock/unlock doors | - | Yes | High |
| | Lock status | Yes | - | High |
| | Window status | Yes | - | Medium |
| | Horn honk | - | Yes | Medium |
| | Lights flash | - | Yes | Medium |
| **Trip Data** | Odometer | Yes | - | High |
| | Estimated range | Yes | - | High |
| | Trip distance | Yes | - | Medium |
| | Trip time | Yes | - | Medium |
| | Average speed | Yes | - | Medium |
| | Average consumption (kWh/100km) | Yes | - | Medium |
| **Vehicle Status** | Ignition state | Yes | - | High |
| | Vehicle speed | Yes | - | Medium |
| | 12V battery voltage | Yes | - | Medium |
| | Faults present (yes/no) | Yes | - | Medium |
| | Next service date | Yes | - | Low |
| **Location** | GPS position | Yes | - | Medium |

---

## 1. Charging Status & Control

### Data Needed (Read)

| Data Point | Source | CAN ID | Signal/Function | Notes |
|------------|--------|--------|-----------------|-------|
| Battery SOC % | BAP | 0x17332510 | BatteryControl.ChargeState (0x11) | `soc` field (0-100) - **PRIMARY** |
| Available kWh | BAP | 0x17332510 | BatteryControl.ChargeState (0x11) | `available_kwh` field |
| | Broadcast | 0x5CA | BMS_Energieinhalt | 50Wh scale, bits 12-22 ✓ |
| Max capacity kWh | Broadcast | 0x5CA | BMS_MaxEnergieinhalt | 50Wh scale, bits 32-42 ✓ |
| Charging active | BAP | 0x17332510 | BatteryControl.ChargeState (0x11) | `charging` field (bool) |
| | Broadcast | 0x5CA | BMS_Ladevorgang_aktiv | Bit 23 ✓ |
| Plug connected | BAP | 0x17332510 | BatteryControl.PlugState (0x10) | `connected` field |
| Battery temp | Broadcast | 0x59E | BMS_Temperatur | 0.5°C scale, -40 offset ✓ |

**⚠️ NOT on Comfort Bus** (these are on Powertrain CAN, not accessible):
- 0x191 - BMS_HV (detailed SOC, voltage, current)
- 0x509 - BMS_Energie (high-res energy)
- 0x5A2 - BMS_Kapazitaet
- 0x564 - Charger temp
- 0x578 - DC charging status

### Commands (Write)

| Action | Protocol | CAN ID | Function | Payload |
|--------|----------|--------|----------|---------|
| Start charging | BAP | 0x17332501 | BatteryControl.StartStopCharge (0x14) | `{start: true}` |
| Stop charging | BAP | 0x17332501 | BatteryControl.StartStopCharge (0x14) | `{start: false}` |

**Sequence:** Wake bus -> Send command -> Wait for ACK

---

## 2. Climate Status & Control

### Data Needed (Read)

| Data Point | Source | CAN ID | Signal/Function | Notes |
|------------|--------|--------|-----------------|-------|
| Climate active | BAP | 0x17332510 | BatteryControl.ClimateState (0x12) | `active` field |
| | Broadcast | 0x66E | KL_STH_aktiv / KL_STL_aktiv | Heating/vent active ✓ |
| Target temp | BAP | 0x17332510 | BatteryControl.ClimateState (0x12) | `target_temp` field |
| Cabin temp | Broadcast | 0x66E | KL_Innen_Temp | 0.5°C scale, -50 offset ✓ |
| Outside temp | Broadcast | 0x5E1 | BCM1_Aussen_Temp_ungef | 0.5°C scale, -50 offset ✓ |
| Blower voltage | Broadcast | 0x668 | KL_Geblspng_Soll | 0.05V scale + 1.45 ✓ |
| Blower current | Broadcast | 0x66E | KL_I_Geblaese | 0.25A scale ✓ |
| Seat heating L | Broadcast | 0x668 | KL_SIH_Soll_li | Steps 0-6 ✓ |
| Seat heating R | Broadcast | 0x668 | KL_SIH_Soll_re | Steps 0-6 ✓ |

**⚠️ NOT on Comfort Bus:**
- 0x659 - Compressor current/speed (on HVAC bus)

### Commands (Write)

| Action | Protocol | CAN ID | Function | Payload |
|--------|----------|--------|----------|---------|
| Start climate | BAP | 0x17332501 | BatteryControl.StartStopClimate (0x15) | `{start: true, temp: 21.0}` |
| Stop climate | BAP | 0x17332501 | BatteryControl.StartStopClimate (0x15) | `{start: false}` |
| Set temperature | BAP | 0x17332501 | BatteryControl.ClimateSettings? | TBD - need trace |

**Note:** Climate commands require the car to be locked and plugged in (or have sufficient battery).

---

## 3. Security (Lock/Unlock/Horn/Flash)

### Data Needed (Read)

| Data Point | Source | CAN ID | Signal/Function | Notes |
|------------|--------|--------|-----------------|-------|
| Lock status | Broadcast | 0x583 | ZV_02 | See byte analysis below ✓ |
| Driver door open | Broadcast | 0x3D0 | FT_Tuer_geoeffnet (bit 0) | TSG_FT_01 ✓ |
| Passenger door open | Broadcast | 0x3D1 | BT_Tuer_geoeffnet (bit 0) | TSG_BT_01 ✓ |
| All doors status | Broadcast | 0x583 | ZV_FT/BT/HFS/HBFS_offen | ZV_02 message |
| Trunk open | Broadcast | 0x583 | ZV_HD_offen | ZV_02 byte 3 |
| Window position (driver) | Broadcast | 0x3D0 | FT_FH_Oeffnung | 0-100%, 0.5% scale |
| Window position (pass) | Broadcast | 0x3D1 | BT_FH_Oeffnung | 0-100%, 0.5% scale |

#### 0x583 (ZV_02) Lock Status Message - Confirmed in Traces

| Byte | Locked | Unlocked | Interpretation |
|------|--------|----------|----------------|
| 2 | 0x0A | 0x80 | Lock state flags |
| 7 | 0x80 | 0x40 | Additional lock state |

**Note:** Exact bit mapping differs from DBC. Observed patterns:
- Locked: byte 2 = 0x0A or 0x08, byte 7 = 0x80
- Unlocked: byte 2 = 0x80, byte 7 = 0x40

#### 0x3D0 (TSG_FT_01) Driver Door Module

| Signal | Bit Position | Description |
|--------|--------------|-------------|
| FT_Tuer_geoeffnet | 0 | Door open (1=open) |
| FT_verriegelt | 1 | Door locked |
| FT_FH_Oeffnung | 24-31 | Window position (0-100%) |

#### 0x3D1 (TSG_BT_01) Passenger Door Module

| Signal | Bit Position | Description |
|--------|--------------|-------------|
| BT_Tuer_geoeffnet | 0 | Door open (1=open) |
| BT_verriegelt | 1 | Door locked |
| BT_FH_Oeffnung | 24-31 | Window position (0-100%) |

### Commands (Write)

| Action | Protocol | CAN ID | Payload | Notes |
|--------|----------|--------|---------|-------|
| Lock doors | Direct | 0x5A7 | `00 00 00 00 00 00 02 00` | TM_01 byte 6 bit 1 |
| Unlock doors | Direct | 0x5A7 | `00 00 00 00 00 00 04 00` | TM_01 byte 6 bit 2 |
| Horn honk | Direct | 0x5A7 | `00 00 00 00 00 00 01 00` | TM_01 byte 6 bit 0 |
| Flash lights | Direct | 0x5A7 | `00 00 00 00 00 00 08 00` | TM_01 byte 6 bit 3 - **CONFIRMED** |
| Panic (horn+flash) | Direct | 0x5A7 | `00 00 00 00 00 00 10 00` | TM_01 byte 6 bit 4 |

**Status:** Horn/flash confirmed working. Lock/unlock uses same mechanism (TM_01).

---

## 4. Trip Data & Odometer

### Data Needed (Read)

| Data Point | Source | CAN ID | Signal/Function | Notes |
|------------|--------|--------|-----------------|-------|
| Odometer km | Broadcast | 0x6B2 | KBI_Kilometerstand | 20-bit, bits 8-27 ✓ |
| **Estimated range** | Broadcast | 0x5F5 | RW_Gesamt_Reichweite | 11 bits, 1 km scale ✓ |
| **Electric range** | Broadcast | 0x5F5 | RW_Primaer_Reichweite | 11 bits, 1 km scale ✓ |
| **Range for display** | Broadcast | 0x5F7 | RW_Gesamt_Reichweite_Anzeige | 11 bits, 1 km scale ✓ |
| Consumption | Broadcast | 0x5F5 | RW_Prim_Reichweitenverbrauch | 11 bits, 0.1 scale ✓ |
| Range tendency | Broadcast | 0x5F7 | RW_Tendenz | 0=stable, 1=up, 2=down ✓ |
| Reserve warning | Broadcast | 0x5F7 | RW_Reservewarnung_aktiv | 1=active ✓ |
| Trip distance | Broadcast | TBD | - | May be in Kombi messages |
| Trip time | Broadcast | TBD | - | May be in Kombi messages |
| Average speed | Calculated | - | distance / time | |
| Current speed | Broadcast | 0x0FD | ESP_v_Signal | 0.01 km/h scale |

**Note:** Trip data may reset on ignition cycle. Need to investigate persistence.

### Range Messages (0x5F5, 0x5F7)

These messages provide the car's calculated range estimate directly from the BMS/gateway.

#### 0x5F5 Reichweite_01 (Range Data)

| Signal | Bits | Scale | Unit | Description |
|--------|------|-------|------|-------------|
| RW_Gesamt_Reichweite_Max_Anzeige | 0-10 | 1 | km | Maximum display range |
| RW_Reservewarnung_2_aktiv | 16-17 | 1 | - | Reserve warning level 2 |
| RW_RWDS_Lastprofil | 18-28 | 1 | A | Load profile for charging calc |
| RW_Gesamt_Reichweite | 29-39 | 1 | km | **Total estimated range** |
| RW_Prim_Reichweitenverbrauch | 40-50 | 0.1 | - | Primary consumption value |
| RW_Prim_Reichweitenv_Einheit | 51-52 | 1 | - | Unit: 0=kWh/100km, 1=km/kWh |
| RW_Primaer_Reichweite | 53-63 | 1 | km | **Primary (electric) range** |

#### 0x5F7 Reichweite_02 (Range Display)

| Signal | Bits | Scale | Unit | Description |
|--------|------|-------|------|-------------|
| RW_Tendenz | 0-2 | 1 | - | Trend: 0=stable, 1=increasing, 2=decreasing |
| RW_Texte | 3-4 | 1 | - | Text message index |
| RW_Reservewarnung_aktiv | 5 | 1 | - | Reserve warning active |
| RW_Reichweite_Einheit_Anzeige | 6 | 1 | - | Display unit: 0=km, 1=miles |
| RW_Gesamt_Reichweite_Anzeige | 7-17 | 1 | km | **Total range for display** |
| RW_Primaer_Reichweite_Anzeige | 18-28 | 1 | km | Electric range for display |
| RW_Sekundaer_Reichweite_Anzeige | 29-39 | 1 | km | Secondary range (N/A for BEV) |
| RW_Sekundaer_Reichweite | 40-50 | 1 | km | Secondary range (N/A for BEV) |
| RW_Sek_Reichweitenv_Einheit | 51-52 | 1 | - | Secondary unit |
| RW_Sek_Reichweitenverbrauch | 53-63 | 0.1 | - | Secondary consumption |

**Invalid/Init values:** 2045-2047 (0x7FD-0x7FF) indicate no data available.

**Note:** For a pure BEV like e-Golf, "Secondary" values are unused (set to invalid). Primary = electric range.

---

## 5. Vehicle Status

### Data Needed (Read)

| Data Point | Source | CAN ID | Signal/Function | Notes |
|------------|--------|--------|-----------------|-------|
| Ignition on | Broadcast | 0x3C0 | ZAS_Kl_15 | Bit 17 ✓ |
| Key inserted | Broadcast | 0x3C0 | ZAS_Kl_S | Bit 16 ✓ |
| Date/time | Broadcast | 0x6B2 | UH_Jahr/Monat/Tag/... | Vehicle clock ✓ |
| Service due | TBD | TBD | - | May be in diagnostic/UDS |

**⚠️ NOT on Comfort Bus:**
- 0x2AE - DCDC converter (12V/HV voltage)
- 0x5A2 - BMS_Fehlerstatus
- 0x5CD - DC_Fehlerstatus
- 0x191 - BMS_IstSpannung (HV voltage)

---

## 6. Location (GPS)

GPS data is available on the comfort CAN bus via gateway routing from infotainment.
**Status:** CONFIRMED - All signals decoded and verified.

### Data Needed (Read)

| Data Point | Source | CAN ID | Signal/Function | Notes |
|------------|--------|--------|-----------------|-------|
| Latitude | Broadcast | 0x486 | NP_LatDegree | 27 bits, 0.000001° scale |
| Longitude | Broadcast | 0x486 | NP_LongDegree | 28 bits, 0.000001° scale |
| Lat direction | Broadcast | 0x486 | NP_LatDirection | 0=North, 1=South |
| Long direction | Broadcast | 0x486 | NP_LongDirection | 0=East, 1=West |
| Satellites | Broadcast | 0x486 | NP_Sat | 5 bits, count |
| Fix type | Broadcast | 0x486 | NP_Fix | 2 bits (0=none, 1=2D, 2=3D) |
| Altitude | Broadcast | 0x485 | NP_Altitude | 12 bits, scale 2, offset -500m |
| Heading | Broadcast | 0x484 | ND_Heading | 12 bits, 0.1° scale |
| UTC timestamp | Broadcast | 0x485 | ND_UTC | 32 bits, Unix timestamp |
| Sats in use | Broadcast | 0x485 | ND_SatInUse | 5 bits |
| Sats in view | Broadcast | 0x485 | ND_SatInView | 5 bits |
| HDOP | Broadcast | 0x484 | ND_HDOP | 10 bits, scale 0.025 |
| Map-matched pos | Broadcast | 0x1A5554A8 | NavPos_02_Map_Matched | Extended CAN, multiplexed |

### GPS CAN Messages

#### 0x484 NavData_01 - Heading & Precision
```
ND_VDOP:    bits 0-9   (10 bits) - Vertical DOP, scale 0.025
ND_TDOP:    bits 10-19 (10 bits) - Time DOP, scale 0.025  
ND_HDOP:    bits 20-29 (10 bits) - Horizontal DOP, scale 0.025
ND_GDOP:    bits 30-39 (10 bits) - Geometric DOP, scale 0.025
ND_PDOP:    bits 40-49 (10 bits) - Position DOP, scale 0.025
ND_Heading: bits 50-61 (12 bits) - Heading 0-359.9°, scale 0.1
ND_Init:    bit 62              - GPS initialized flag
```

#### 0x485 NavData_02 - Altitude & Satellites
```
ND_SatInUse:  bits 0-4   (5 bits) - Satellites used in fix
Accuracy_OK:  bit 5               - Horizontal accuracy available
ND_SatInView: bits 8-12  (5 bits) - Satellites visible
Accuracy:     bits 13-19 (7 bits) - Horizontal accuracy, scale 2m
NP_Altitude:  bits 20-31 (12 bits)- Altitude, scale 2, offset -500m
ND_UTC:       bits 32-63 (32 bits)- Unix timestamp
```

#### 0x486 NavPos_01 - Position (Primary GPS message)
```
NP_LatDegree:     bits 0-26  (27 bits) - Latitude, scale 0.000001°
NP_LongDegree:    bits 27-54 (28 bits) - Longitude, scale 0.000001°  
NP_LatDirection:  bit 55               - 0=North, 1=South
NP_LongDirection: bit 56               - 0=East, 1=West
NP_Sat:           bits 57-61 (5 bits)  - Satellite count
NP_Fix:           bits 62-63 (2 bits)  - Fix type (0=none, 1=2D, 2=3D, 3=DGPS)
```

#### 0x1A5554A8 NavPos_02_Map_Matched - Map-Matched Position (Extended CAN)
```
Multiplexed message (MUX in bits 0-1):

MUX=0 (Position):
  POS_LatDirection:  bit 7
  POS_LatDegree:     bits 8-34  (27 bits) - scale 0.000001°
  POS_LongDirection: bit 35
  POS_LongDegree:    bits 36-63 (28 bits) - scale 0.000001°

MUX=1 (Heading/Timing):
  POS_Heading:       bits 4-15  (12 bits) - scale 0.1°
  POS_Last_GPS_Sync: bits 16-29 (14 bits) - seconds since sync
  POS_UTC_REF:       bits 32-63 (32 bits) - UTC reference
```

### Example Decoding (from trace)
```python
# NavPos_01: 04,80,27,FC,A2,09,09,4A
# Decoded: 69.697540°N, 18.953311°E, 5 sats, 2D fix

# NavData_02: 05,0D,50,10,B0,42,5F,66  
# Decoded: 5 sats in use, 13 in view, 22m altitude, UTC 1717519024
```

---

## 7. Additional Broadcast Signals (Confirmed in Traces)

### Climate/Temperature Related

| CAN ID | Name | Contents | Notes |
|--------|------|----------|-------|
| 0x5E1 | Klima_Sensor_02 | Outside temperature | Byte 0, 0.5°C scale, -50 offset |
| 0x594 | Climate Settings | Target temp, departure timer | Temp = value * 0.5 + 15.5°C |
| 0x3E9 | Thermo_Verbrauch_01 | Driver temperature | Bytes 1 & 2 |
| 0x588 | Seat/Window Heat | Seat heat, window defrost | See bit mapping below |

#### 0x5E1 Outside Temperature Encoding
```
Signal: BCM1_Aussen_Temp_ungef (approximate outside temp)
Location: Byte 0, 8 bits
Formula: temp_celsius = raw_value * 0.5 - 50
Example: 0x7B (123) = 123 * 0.5 - 50 = 11.5°C
Range: -50°C (0x00) to +77.5°C (0xFF)
```

#### 0x594 Climate Temperature Encoding
```
Temperature range: 15.5°C (0x00) to 30.0°C (0x1D)
Formula: temp_celsius = raw_value * 0.5 + 15.5
Byte 2 bit 6: Departure timer active
```

#### 0x588 Heating Controls
```
Byte 1 bits 7-8: Passenger seat heat
Byte 1 bits 5-6: Driver seat heat  
Byte 1 bit 1: Front window heat
Byte 1 bit 2: Rear window heat
```

---

## Data Source Priority

For each data point, prefer sources in this order:

1. **BAP protocol** - Structured, semantic data, designed for this purpose
2. **Broadcast signals** - Real-time values, good for live monitoring
3. **Calculated** - Derived from other values
4. **UDS/Diagnostic** - Last resort, may require authentication

---

## Signal Confirmation Status

| Signal | CAN ID | Status | Source |
|--------|--------|--------|--------|
| Lock/door status | 0x583 | **Confirmed in traces** | ZV_02 |
| Driver door/window | 0x3D0 | **Confirmed in traces** | TSG_FT_01 |
| Pass door/window | 0x3D1 | **Confirmed in traces** | TSG_BT_01 |
| Climate temp | 0x594 | **Confirmed in traces** | User analysis |
| Horn/flash command | 0x5A7 | **Confirmed working** | TM_01 |
| Seat/window heat | 0x588 | **Confirmed in traces** | User analysis |
| Odometer/time | 0x6B2 | **Confirmed in traces** | Diagnose_01 |
| Ignition state | 0x3C0 | **Confirmed in traces** | Klemmen_Status_01 |
| Outside temp | 0x5E1 | **Confirmed in traces** | Klima_Sensor_02 |
| Range data | 0x5F5 | **Confirmed in traces** | Reichweite_01 |
| Range display | 0x5F7 | **Confirmed in traces** | Reichweite_02 |
| GPS position | 0x486 | **Confirmed in traces** | NavPos_01 |
| GPS altitude/UTC | 0x485 | **Confirmed in traces** | NavData_02 |
| GPS heading/DOP | 0x484 | **Confirmed in traces** | NavData_01 |
| GPS map-matched | 0x1A5554A8 | **Confirmed in traces** | NavPos_02_Map_Matched |

---

## Unknown / Needs Investigation

| Feature | What's Missing | How to Find |
|---------|----------------|-------------|
| Trip computer data | BAP channel or broadcast | Capture during drive |
| Service interval | Where stored | Likely UDS diagnostic |

---

## Next Steps

1. ~~Analyze door locking traces~~ - **DONE** - 0x583, 0x3D0, 0x3D1 confirmed
2. ~~Document horn/flash~~ - **DONE** - TM_01 (0x5A7) confirmed
3. ~~Confirm GPS CAN IDs~~ - **DONE** - 0x484, 0x485, 0x486, 0x1A5554A8
4. ~~Map outside temperature~~ - **DONE** - 0x5E1 (Klima_Sensor_02)
5. ~~Create implementation module~~ - **DONE** - `broadcast_decoder.py`
6. **Test lock/unlock commands** - Verify TM_01 lock/unlock works
7. **Capture trip data** - Find trip computer signals during drive
