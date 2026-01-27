# Extracted Signals from vw_mqb_2010.dbc


## Battery/BMS


### 0x65C (1628) - BMS_Hybrid_01
DLC: 8, Transmitter: BMS_MQB
Signals (12):
  BMS_HYB_ASV_hinten_Status:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_HYB_ASV_vorne_Status:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_HYB_KD_Fehler:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_HYB_BattFanSpd:
    Bits: 16|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  BMS_HYB_VentilationReq:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_HYB_Spuelbetrieb_Status:
    Bits: 21|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_HYB_Kuehlung_Anf:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BMS_HYB_Temp_vor_Verd:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_HYB_Temp_nach_Verd:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_Temperatur:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_Temperatur_Ansaugluft:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_IstSpannung_HV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 100.0
    Unit: Unit_Volt
    Range: [100.0 .. 350.0]

## Climate/HVAC


### 0x3B5 (949) - Klima_11
DLC: 8, Transmitter: Gateway_MQB
Signals (17):
  KL_Drehz_Anh:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Vorwarn_Komp_ein:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_AC_Schalter:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Komp_Moment_alt:
    Bits: 3|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Zonen:
    Bits: 4|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_Vorwarn_Zuheizer_ein:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Zustand:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Comp_rev_rq:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 8600.0]
  KL_Charisma_FahrPr:
    Bits: 16|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  KL_Charisma_Status:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_Comp_enable:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Last_Kompr:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_NewtoMeter
    Range: [0.0 .. 63.5]
  KL_Spannungs_Anf:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_Thermomanagement:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_StartStopp_Info:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_Anf_KL:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 101.6]
  KL_el_Zuheizer_Stufe:
    Bits: 48|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]

### 0x659 (1625) - Klimakomp_01
DLC: 8, Transmitter: Gateway_MQB
Signals (10):
  EKL_KD_Fehler:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EKL_Comp_SCI_com_stat:
    Bits: 16|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  EKL_Comp_output_stat:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  EKL_Comp_main_stat:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EKL_Comp_ovld_stat:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  EKL_Comp_Inv_stat:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  EKL_Comp_photo_temp_stat:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  EKL_Comp_photo_temp:
    Bits: 32|8 (Intel (LE), unsigned)
    Unit: Unit_DegreCelsi
    Range: [0.0 .. 254.0]
  EKL_Comp_current:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.4]
  EKL_Comp_rev_stat:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 8600.0]

### 0x66E (1646) - Klima_03
DLC: 8, Transmitter: XXX
Signals (0):

## Vehicle State


### 0x0B2 (178) - ESP_19
DLC: 8, Transmitter: Gateway_MQB
Signals (4):
  ESP_HL_Radgeschw_02:
    Bits: 0|16 (Intel (LE), unsigned)
    Formula: raw * 0.0075 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.0 .. 491.49]
  ESP_HR_Radgeschw_02:
    Bits: 16|16 (Intel (LE), unsigned)
    Formula: raw * 0.0075 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.0 .. 491.49]
  ESP_VL_Radgeschw_02:
    Bits: 32|16 (Intel (LE), unsigned)
    Formula: raw * 0.0075 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.0 .. 491.49]
  ESP_VR_Radgeschw_02:
    Bits: 48|16 (Intel (LE), unsigned)
    Formula: raw * 0.0075 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.0 .. 491.49]

### 0x0FD (253) - ESP_21
DLC: 8, Transmitter: Gateway_MQB
Signals (18):
  CHECKSUM:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  COUNTER:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BR_Eingriffsmoment:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -509.0
    Unit: Unit_NewtoMeter
    Range: [-509.0 .. 509.0]
  ESP_v_Signal:
    Bits: 32|16 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.0 .. 655.32]
  ASR_Tastung_passiv:
    Bits: 48|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Tastung_passiv:
    Bits: 49|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Systemstatus:
    Bits: 50|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ASR_Schalteingriff:
    Bits: 51|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_Haltebestaetigung:
    Bits: 53|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_v_Signal:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ABS_Bremsung:
    Bits: 56|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ASR_Anf:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MSR_Anf:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EBV_Eingriff:
    Bits: 59|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EDS_Eingriff:
    Bits: 60|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Eingriff:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_ASP:
    Bits: 62|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Anhaltevorgang_ACC_aktiv:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x101 (257) - ESP_02
DLC: 8, Transmitter: Gateway_MQB
Signals (16):
  ESP_02_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_02_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_QBit_Gierrate:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Laengsbeschl:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Querb:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Stillstandsflag:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Querbeschleunigung:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 0.01 + -1.27
    Unit: Unit_ForceOfGravi
    Range: [-1.27 .. 1.27]
  ESP_Laengsbeschl:
    Bits: 24|10 (Intel (LE), unsigned)
    Formula: raw * 0.03125 + -16.0
    Unit: Unit_MeterPerSeconSquar
    Range: [-16.0 .. 15.90625]
  ESP_Verteil_Wankmom:
    Bits: 34|5 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -1.0
    Range: [-1.0 .. 1.0]
  ESP_QBit_Anf_Vert_Wank:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Gierrate:
    Bits: 40|14 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 0.0
    Unit: Unit_DegreOfArcPerSecon
    Range: [0.0 .. 163.82]
  ESP_VZ_Gierrate:
    Bits: 54|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Notbremsanzeige:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_SpannungsAnf:
    Bits: 56|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_PLA_Abbruch:
    Bits: 57|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  ESP_Status_ESP_PLA:
    Bits: 60|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x106 (262) - ESP_05
DLC: 8, Transmitter: Gateway_MQB
Signals (35):
  CHECKSUM:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  COUNTER:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_QBit_Bremsdruck:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Fahrer_bremst:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Schwelle_Unterdruck:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_Bremsdruck:
    Bits: 16|10 (Intel (LE), unsigned)
    Formula: raw * 0.3 + -30.0
    Unit: Unit_Bar
    Range: [-30.0 .. 276.6]
  ESP_Fahrer_bremst:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Verz_TSK_aktiv:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Lenkeingriff_ADS:
    Bits: 28|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Konsistenz_TSK:
    Bits: 29|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Bremsruck_AWV2:
    Bits: 30|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Konsistenz_AWV2:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ECD_Fehler:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ECD_nicht_verfuegbar:
    Bits: 33|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Status_Bremsentemp:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Autohold_Standby:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_HDC_Standby:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_HBA_aktiv:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Prefill_ausgeloest:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Rueckwaertsfahrt_erkannt:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Status_Anfahrhilfe:
    Bits: 40|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_HDC_aktiv:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_StartStopp_Info:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_Eingr_HL:
    Bits: 44|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Eingr_HR:
    Bits: 45|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Eingr_VL:
    Bits: 46|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Eingr_VR:
    Bits: 47|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_BKV_Unterdruck:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_MilliBar
    Range: [0.0 .. 1012.0]
  ESP_Autohold_aktiv:
    Bits: 56|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_FStatus_Anfahrhilfe:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Verz_EPB_aktiv:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ECD_Bremslicht:
    Bits: 59|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Verzoeg_EPB_verf:
    Bits: 60|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Status_Bremsdruck:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Anforderung_EPB:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x116 (278) - ESP_10
DLC: 8, Transmitter: Gateway_MQB
Signals (14):
  ESP_10_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_10_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_QBit_Wegimpuls_VL:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Wegimpuls_VR:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Wegimpuls_HL:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Wegimpuls_HR:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Wegimpuls_VL:
    Bits: 16|10 (Intel (LE), unsigned)
    Range: [0.0 .. 1000.0]
  ESP_Wegimpuls_VR:
    Bits: 26|10 (Intel (LE), unsigned)
    Range: [0.0 .. 1000.0]
  ESP_Wegimpuls_HL:
    Bits: 36|10 (Intel (LE), unsigned)
    Range: [0.0 .. 1000.0]
  ESP_Wegimpuls_HR:
    Bits: 46|10 (Intel (LE), unsigned)
    Range: [0.0 .. 1000.0]
  ESP_VL_Fahrtrichtung:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_VR_Fahrtrichtung:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_HL_Fahrtrichtung:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_HR_Fahrtrichtung:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x11E (286) - VehicleSpeed
DLC: 8, Transmitter: XXX
Signals (3):
  VehicleSpeed_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  VehicleSpeed_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  Speed:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 0.125 + 0.0
    Range: [0.0 .. 1.0]

### 0x1A2 (418) - ESP_15
DLC: 8, Transmitter: XXX
Signals (2):
  ESP_15_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_15_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x1AB (427) - ESP_33
DLC: 8, Transmitter: XXX
Signals (2):
  ESP_33_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_33_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x392 (914) - ESP_07
DLC: 8, Transmitter: Gateway_MQB
Signals (18):
  ESP_07_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_07_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_ACC_LDE:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Quattro_Antrieb:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Codierung_ADS:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_RTA_HL:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 0.048828125 + -6.20117
    Unit: Unit_PerCent
    Range: [-6.20117 .. 6.152345625]
  ESP_RTA_HR:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.048828125 + -6.20117
    Unit: Unit_PerCent
    Range: [-6.20117 .. 6.152345625]
  ESP_RTA_VR:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.048828125 + -6.20117
    Unit: Unit_PerCent
    Range: [-6.20117 .. 6.152345625]
  OBD_Fehler_Radsensor_HL:
    Bits: 40|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  OBD_Fehler_Radsensor_HR:
    Bits: 44|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  OBD_Fehler_Radsensor_VL:
    Bits: 48|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  OBD_Fehler_Radsensor_VR:
    Bits: 52|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_Qualifizierung_Antriebsart:
    Bits: 56|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Offroad_Modus:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_MKB_ausloesbar:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_MKB_Status:
    Bits: 59|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_CM_Variante:
    Bits: 60|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_OBD_Status:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x3C0 (960) - Klemmen_Status_01
DLC: 4, Transmitter: Gateway_MQB
Signals (6):
  CHECKSUM:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  COUNTER:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ZAS_Kl_S:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ZAS_Kl_15:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Indicates ignition on
  ZAS_Kl_X:
    Bits: 18|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ZAS_Kl_50:
    Bits: 19|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x65D (1629) - ESP_20
DLC: 8, Transmitter: Gateway_MQB
Signals (8):
  CHECKSUM:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  COUNTER:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BR_Systemart:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_Zaehnezahl:
    Bits: 16|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_Charisma_FahrPr:
    Bits: 24|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_Charisma_Status:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BR_QBit_Reifenumfang:
    Bits: 51|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BR_Reifenumfang:
    Bits: 52|12 (Intel (LE), unsigned)
    Unit: Unit_MilliMeter
    Range: [0.0 .. 4095.0]

### 0x6B2 (1714) - Diagnose_01
DLC: 8, Transmitter: Gateway_MQB
Signals (10):
  DGN_Verlernzaehler:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
  KBI_Kilometerstand:
    Bits: 8|20 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 1048573.0]
  UH_Jahr:
    Bits: 28|7 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 2000.0
    Unit: Unit_Year
    Range: [2000.0 .. 2127.0]
  UH_Monat:
    Bits: 35|4 (Intel (LE), unsigned)
    Unit: Unit_Month
    Range: [1.0 .. 12.0]
  UH_Tag:
    Bits: 39|5 (Intel (LE), unsigned)
    Unit: Unit_Day
    Range: [1.0 .. 31.0]
  UH_Stunde:
    Bits: 44|5 (Intel (LE), unsigned)
    Unit: Unit_Hours
    Range: [0.0 .. 23.0]
  UH_Minute:
    Bits: 49|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 59.0]
  UH_Sekunde:
    Bits: 55|6 (Intel (LE), unsigned)
    Unit: Unit_Secon
    Range: [0.0 .. 59.0]
  Kombi_02_alt:
    Bits: 62|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  Uhrzeit_01_alt:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

## Electric Motor


### 0x0A7 (167) - Motor_11
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  MO_QBit_Motormomente:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x0A8 (168) - Motor_12
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (3):
  MO_Momentenintegral_02:
    Bits: 40|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  MO_QBit_Drehzahl_01:
    Bits: 47|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Drehzahl_01:
    Bits: 48|16 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 16383.0]

### 0x107 (263) - Motor_04
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (2):
  MO_Schaltempf_verfbar:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Ladedruck:
    Bits: 39|9 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 0.0
    Unit: Unit_Bar
    Range: [0.0 .. 5.1]

### 0x121 (289) - Motor_20
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  MO_Moment_im_Leerlauf:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x32B (811) - Motor_Hybrid_02
DLC: 8, Transmitter: Motor_Hybrid_MQB
Signals (2):
  MO_HYB_Drehzahl_VM:
    Bits: 16|16 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 16256.0]
  MO_HYB_LowSpeedModus:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x32C (812) - Motor_17
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (6):
  MO_Prio_MAX_Wunschdrehzahl:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Prio_MIN_Wunschdrehzahl:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Luftpfad_aktiv:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Drehzahlbeeinflussung:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 0.39 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 99.45]
  MO_MIN_Wunschdrehzahl:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 6350.0]
  MO_MAX_Wunschdrehzahl:
    Bits: 32|9 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12750.0]

### 0x3BE (958) - Motor_14
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  MO_Klima_Eingr:
    Bits: 33|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x3C7 (967) - Motor_26
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  MO_HYB_Status_HV_Ladung:
    Bits: 8|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]

### 0x640 (1600) - Motor_07
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (11):
  MO_QBit_Ansaugluft_Temp:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_QBit_Oel_Temp:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_QBit_Kuehlmittel_Temp:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_HYB_Fehler_HV_Netz:
    Bits: 4|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_aktives_Getriebeheizen:
    Bits: 5|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Ansaugluft_Temp:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
  MO_Oel_Temp:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -60.0
    Unit: Unit_DegreCelsi
    Range: [-60.0 .. 192.0]
  MO_Kuehlmittel_Temp:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
  MO_Heizungspumpenansteuerung:
    Bits: 53|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  MO_SpannungsAnf:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Nachlaufzeit_Heizungspumpe:
    Bits: 58|6 (Intel (LE), unsigned)
    Formula: raw * 15.0 + 0.0
    Unit: Unit_Secon
    Range: [0.0 .. 945.0]

### 0x641 (1601) - Motor_Code_01
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  MO_Faktor_Momente_02:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x647 (1607) - Motor_09
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (2):
  MO_ITM_Kuehlmittel_Temp:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-45.75 .. 143.25]
  SCR_Reichweite:
    Bits: 16|15 (Intel (LE), unsigned)
    Range: [0.0 .. 32766.0]

### 0x65F (1631) - Motor_16
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (3):
  MO_SpannungsAnf_02:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  MO_Heizstrom_EKAT:
    Bits: 17|7 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 126.0]
  MO_Heizstrom_SCR:
    Bits: 24|6 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 62.0]

### 0x670 (1648) - Motor_18
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (2):
  MO_Drehzahl_Warnung:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_obere_Drehzahlgrenze:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [50.0 .. 12750.0]

### 0x97F0007C (2549088380) - KN_EMotor_01
DLC: 8, Transmitter: LEH_MQB
Signals (2):
  EMotor_KompSchutz:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EMotor_Nachlauftyp:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x9B00007C (2600468604) - NMH_EMotor_01
DLC: 8, Transmitter: LEH_MQB
Signals (9):
  NM_EMotor_01_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  NM_EMotor_01_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_EMotor_01_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_EMotor_01_Wakeup:
    Bits: 24|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  NM_EMotor_01_NM_aktiv_KL15:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_EMotor_01_NM_aktiv_Diagnose:
    Bits: 33|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_EMotor_01_NM_aktiv_Tmin:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_EMotor_01_NL_Daten_EEPROM:
    Bits: 48|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_EMotor_01_UDS_CC:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

## Other


### 0x040 (64) - Airbag_01
DLC: 8, Transmitter: Airbag_MQB
Signals (1):
  AB_Versorgungsspannung:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x09F (159) - LH_EPS_03
DLC: 8, Transmitter: XXX
Signals (3):
  EPS_Lenkmoment:
    Bits: 40|10 (Intel (LE), unsigned)
    Unit: Unit_centiNewtoMeter
    Range: [0.0 .. 8.0]
    Comment: Steering input by driver, torque
  EPS_Lenkmoment_QBit:
    Bits: 54|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EPS_VZ_Lenkmoment:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steering input by driver, direction

### 0x0AE (174) - Getriebe_12
DLC: 8, Transmitter: Getriebe_DQ_Hybrid_MQB
Signals (2):
  GE_Drehzahlmesser_Daempfung:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  GE_Aufnahmemoment:
    Bits: 48|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -509.0
    Unit: Unit_NewtoMeter
    Range: [-509.0 .. 509.0]

### 0x126 (294) - HCA_01
DLC: 8, Transmitter: Frontsensorik
Signals (1):
  EA_ACC_Wunschgeschwindigkeit:
    Bits: 41|10 (Intel (LE), unsigned)
    Formula: raw * 0.32 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.0 .. 327.04]
    Comment: Emergency Alert Anforderung neue Wunschgeschwindigkeit

### 0x2A7 (679) - ACC_13
DLC: 8, Transmitter: XXX
Signals (1):
  ACC_Tempolimitassistent:
    Bits: 48|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x30B (779) - Kombi_01
DLC: 8, Transmitter: Gateway_MQB
Signals (2):
  KBI_NV_in_Anzeige:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KBI_Anzeigefehler_NV:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x324 (804) - ACC_04
DLC: 8, Transmitter: XXX
Signals (1):
  ACC_Tempolimit:
    Bits: 51|5 (Intel (LE), unsigned)
    Range: [0.0 .. 31.0]

### 0x365 (869) - BEM_05
DLC: 8, Transmitter: Gateway_MQB
Signals (2):
  BEM_HYB_DC_uSollLV:
    Bits: 50|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 10.6
    Unit: Unit_Volt
    Range: [10.6 .. 16.0]
  BEM_HYB_DC_uMinLV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 25.3]

### 0x391 (913) - OBD_01
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  OBD_Eng_Cool_Temp:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 215.0]

### 0x397 (919) - LDW_02
DLC: 8, Transmitter: XXX
Signals (1):
  LDW_Frontscheibenheizung_aktiv:
    Bits: 60|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x3B1 (945) - DC_Hybrid_01
DLC: 8, Transmitter: LEH_MQB
Signals (9):
  DC_HYB_iAktLV:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  DC_HYB_iAktReserveLV:
    Bits: 22|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  DC_HYB_uAktLV:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 25.3]
  DC_HYB_LangsRegelung:
    Bits: 40|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  DC_HYB_Abregelung_Temperatur:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  DC_HYB_Fehler_RedLeistung:
    Bits: 42|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  DC_HYB_Fehler_intern:
    Bits: 43|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  DC_HYB_Fehler_Spannung:
    Bits: 44|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  DC_HYB_Auslastungsgrad:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]

### 0x3C8 (968) - Getriebe_14
DLC: 8, Transmitter: Getriebe_DQ_Hybrid_MQB
Signals (3):
  GE_Verlustmoment:
    Bits: 32|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
  GE_Heizwunsch:
    Bits: 52|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  GE_Sumpftemperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -58.0
    Unit: Unit_DegreCelsi
    Range: [-58.0 .. 196.0]

### 0x3DB (987) - Gateway_72
DLC: 8, Transmitter: Gateway_MQB
Signals (4):
  Klima_Sensor_02_alt:
    Bits: 5|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  Klima_01_alt:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BCM1_OBD_FStatus_ATemp:
    Bits: 52|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BCM1_Aussen_Temp_ungef:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -50.0
    Unit: Unit_DegreCelsi
    Range: [-50.0 .. 76.0]

### 0x3DD (989) - Gateway_74
DLC: 8, Transmitter: Gateway_MQB
Signals (4):
  Klima_02_alt:
    Bits: 5|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ELV_Anf_Klemme_50:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Geblaesespannung_Soll:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 0.5
    Unit: Unit_Volt
    Range: [2.0 .. 13.0]
  KL_Umluftklappe_Status:
    Bits: 48|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x494 (1172) - STS_01
DLC: 8, Transmitter: Gateway_MQB
Signals (1):
  STS_Laderelais:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x521 (1313) - STH_01
DLC: 8, Transmitter: Gateway_MQB
Signals (3):
  STH_Zusatzheizung:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  STH_Heizleistung:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  STH_Wassertemp:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 142.25]

### 0x643 (1603) - Einheiten_01
DLC: 8, Transmitter: Gateway_MQB
Signals (1):
  KBI_Einheit_Temp:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x6B7 (1719) - Kombi_02
DLC: 8, Transmitter: Gateway_MQB
Signals (3):
  KBI_Kilometerstand:
    Bits: 0|20 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 1048573.0]
  KBI_QBit_Aussen_Temp_gef:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KBI_Aussen_Temp_gef:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -50.0
    Unit: Unit_DegreCelsi
    Range: [-50.0 .. 75.0]

### 0x6B8 (1720) - Kombi_03
DLC: 8, Transmitter: XXX
Signals (2):
  KBI_Reifenumfang:
    Bits: 0|12 (Intel (LE), unsigned)
    Unit: Unit_MilliMeter
    Range: [0.0 .. 4095.0]
  KBI_Reifenumfang_Sekundaer:
    Bits: 48|12 (Intel (LE), unsigned)
    Unit: Unit_MilliMeter
    Range: [0.0 .. 4095.0]

### 0x97F0007B (2549088379) - KN_Hybrid_01
DLC: 8, Transmitter: BMS_MQB
Signals (1):
  BMS_HYB_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x9B000010 (2600468496) - NMH_Gateway
DLC: 8, Transmitter: Gateway_MQB
Signals (1):
  NM_Gateway_Energie_LIN_Aktivi000:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x9B000076 (2600468598) - NMH_MO_01
DLC: 8, Transmitter: Motor_Diesel_MQB
Signals (1):
  NM_MO_01_NM_aktiv_HV_Abschaltung:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]