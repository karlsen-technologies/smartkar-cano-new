# Extracted Signals from MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc


## Battery/BMS


### 0x18D (397) - BMS_MV_01
DLC: 8, Transmitter: Vector__XXX
Signals (5):
  BMS_MV_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_MV_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_MV_IstStrom:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -2047.0
    Unit: Unit_Amper
    Range: [-2047.0 .. 2046.0]
  BMS_MV_IstSpannung:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: actual voltage of the battery / Momentanwert: Batteriespannung
  BMS_MV_SOC_HiRes:
    Bits: 47|11 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: State of Charge / aktueller Ladezustand der Batterie (höhere Auflösung)

### 0x191 (401) - BMS_01
DLC: 8, Transmitter: BMC_MLBevo
Signals (8):
  BMS_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BMS_IstStrom_02:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -2047.0
    Unit: Unit_Amper
    Range: [-2047.0 .. 2046.0]
  BMS_IstSpannung:
    Bits: 24|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: actual voltage of the battery / Momentanwert: Batteriespannung
  BMS_Spannung_ZwKr:
    Bits: 36|11 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1022.5]
  BMS_SOC_HiRes:
    Bits: 47|11 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: State of Charge / aktueller Ladezustand der Batterie (höhere Auflösung)
  BMS_IstStrom_02_OffsetVZ:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Vorzeichen des Offsets, bzw. der Nachkommastelle
  BMS_IstStrom_02_Offset:
    Bits: 60|4 (Intel (LE), unsigned)
    Formula: raw * 0.0625 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 0.9375]
    Comment: Nachkommastelle des BMS_IstStrom_02

### 0x1A1 (417) - BMS_02
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_MaxDyn_LadeStrom_02_Offset:
    Bits: 8|4 (Intel (LE), unsigned)
    Formula: raw * 0.0625 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 0.9375]
    Comment: Nachkommastelle BMS_MaxDyn_LadeStrom_02
  BMS_MaxDyn_EntladeStrom_02:
    Bits: 12|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MaxDyn_LadeStrom_02:
    Bits: 23|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_Min_EntladeSpannung:
    Bits: 34|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: min. voltage / minimal erlaubte Batteriespannung (zwingend einzuhaltender Grenzwert)
  BMS_MinDyn_EntladeSpannung:
    Bits: 44|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1022.0]
    Comment: min. voltage during discharge with current MaxDyn_EntladeStrom resp. iDisChMax / rechnerischer Wert:...
  BMS_MinDyn_LadeSpannung:
    Bits: 54|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1022.0]

### 0x1A3 (419) - BMS_MV_02
DLC: 8, Transmitter: Vector__XXX
Signals (7):
  BMS_MV_02_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_MV_02_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_MV_MaxDyn_EntladeStrom:
    Bits: 12|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MV_MaxDyn_LadeStrom:
    Bits: 23|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MV_Min_EntladeSpannung:
    Bits: 34|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: min. voltage / minimal erlaubte Batteriespannung (zwingend einzuhaltender Grenzwert)
  BMS_MV_MinDynEntladeSpannung:
    Bits: 44|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: min. voltage during discharge with current MaxDyn_EntladeStrom resp. iDisChMax / rechnerischer Wert:...
  BMS_MV_MinDyn_LadeSpannung:
    Bits: 54|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: min. voltage during charge with current MaxDyn_LadeStrom resp. iChMax / rechnerischer Wert: minimale...

### 0x1F3 (499) - BMS_NV_01
DLC: 8, Transmitter: BMS_NV
Signals (9):
  BMS_NV_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Berechnung siehe Lastenheft 'End-to-End Kommunikationsabsicherung'
  BMS_NV_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: -
  BMS_NV_FMAus_Info:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BMS_NV_TE_Status:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Garantierte Unterstützung des Zusatzspeichers. Nur bei garantierter Bordnetztstützung des Zusatzspei...
  BMS_NV_IstStrom:
    Bits: 16|18 (Intel (LE), unsigned)
    Formula: raw * 0.01 + -1300.0
    Unit: Unit_Amper
    Range: [-1300.0 .. 1300.0]
  BMS_NV_IstSpannung:
    Bits: 34|14 (Intel (LE), unsigned)
    Formula: raw * 0.001 + 4.0
    Unit: Unit_Volt
    Range: [4.0 .. 20.381]
    Comment: actual voltage of the battery / Momentanwert: Batteriespannung
  BMS_NV_StSt_Nutzbare_Ladung:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_AmperSecon
    Range: [0.0 .. 12650.0]
    Comment: Energie aus dem Zusatzspeicher, welche für die StartStopp-Funktion in den Stopp-Phasen genutzt wird....
  BMS_NV_BatterieAb:
    Bits: 56|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_NV_IstModus:
    Bits: 57|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x1F8 (504) - BMS_NV_02
DLC: 8, Transmitter: BMS_NV
Signals (6):
  BMS_NV_Fehlerstatus:
    Bits: 15|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktueller Fehlerstatus der Batterie
  BMS_NV_Qe_LI_Inhalt:
    Bits: 20|12 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 0.0
    Unit: Unit_AmperHour
    Range: [0.0 .. 40.0]
    Comment: Bei größer 0 ist ein RSG Start mit den vorgegebenen Strom und Spannungsgrenzen noch möglich
  BMS_NV_IMaxStart:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 1.0
    Unit: Unit_Amper
    Range: [1.0 .. 1013.0]
    Comment: Aktuell gemessenes Stromdelta bei aktuellem Spannungseinbruch
  BMS_NV_Ri_Batterie:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.2 + 0.0
    Unit: Unit_MilliOhm
    Range: [0.0 .. 50.6]
    Comment: Innenwiderstand in Entladerichtung
  BMS_NV_SOC:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: State of Charge / aktueller Ladezustand der Batterie
  BMS_NV_UMinStart:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 2.0
    Unit: Unit_Volt
    Range: [2.0 .. 14.65]
    Comment: Aktuell gemessener Spannungseinbruch (Minimalwert der Spannung) des letzten Motorstarts 

### 0x2AF (687) - BMS_05
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_As_Entladezaehler:
    Bits: 21|10 (Intel (LE), unsigned)
    Unit: Unit_AmperSecon
    Range: [0.0 .. 1023.0]
    Comment: Signal zur Berechnung des Lastprofils durch die RWB
  BMS_As_Entladezaehler_Ueberlauf:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Wird beim ersten Überlauf von BMS_As_Entladezahler auf 1 gesetzt. Reset über KL15-Wechsel
  BMS_Rekuperation:
    Bits: 32|15 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_WattSecond
    Range: [0.0 .. 327670.0]
    Comment: Zähler: Anzeigesignal: Energiezufuhr in die HV-Batterie (Reku). Darf nicht durch Laden der Batterie ...
  BMS_Rekuperation_Ueberlauf:
    Bits: 47|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Rekuperationleistungssignal BMS_Rekuperation mindest 1x übergelaufen
  BMS_Verbrauch:
    Bits: 48|15 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_WattSecond
    Range: [0.0 .. 327670.0]
    Comment: Zähler: Energieentnahme aus der HV-Batterie
  BMS_Verbrauch_Ueberlauf:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Verbrauchssignal mindest 1x übergelaufen

### 0x367 (871) - BMS_MV_03
DLC: 8, Transmitter: Vector__XXX
Signals (8):
  BMS_MV_03_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_MV_03_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_MV_Max_LadeSpannung:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: max. voltage / maximal erlaubte Batteriespannung (zwingend einzuhaltender Grenzwert)
  BMS_MV_MaxPred_EntladeStrom:
    Bits: 22|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MV_MaxPred_LadeStrom:
    Bits: 33|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MV_MinPred_EntladeSpannung:
    Bits: 44|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: static el. limit, min. battery voltage (e.g. 10s) during current MaxPred_EntladeStrom resp iDisChPre...
  BMS_MV_MinPred_LadeSpannung:
    Bits: 54|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: static el. limit, min. battery voltage (e.g. 10s) during current MaxPred_LadeStrom resp. iChPredMax ...
  BMS_MV_Leistungsreduzierung:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisierung der Performanceeinschränkung der 48V Batterie aufgrund hoher Temperatur

### 0x36A (874) - BMS_NV_03
DLC: 8, Transmitter: BMS_NV
Signals (7):
  BMS_NV_Max_LadeStrom:
    Bits: 12|10 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 850.0]
    Comment: maximal zulässiger Ladestrom
  BMS_NV_Max_EntladeStrom:
    Bits: 22|10 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 850.0]
    Comment: maximal zulässiger Entladestrom
  BMS_NV_Max_LadeSpannung:
    Bits: 32|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 11.0
    Unit: Unit_Volt
    Range: [11.0 .. 17.1]
    Comment: max. voltage / maximal erlaubte Batteriespannung (zwingend einzuhaltender Grenzwert)
  BMS_NV_Diagnose_Anf:
    Bits: 38|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Wunsch zur Diagnose des Trennelements und Anforderung von Prüfpulsen für BZE
  BMS_NV_Min_EntladeSpannung:
    Bits: 41|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 8.0
    Unit: Unit_Volt
    Range: [8.0 .. 14.1]
    Comment: min. voltage / minimal erlaubte Batteriespannung (zwingend einzuhaltender Grenzwert)
  BMS_NV_Kapazitaet_akt:
    Bits: 47|9 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_AmperHour
    Range: [0.0 .. 40.0]
    Comment: Gesamtenergiekapazitaet der NV-Batterie unter Berücksichtigung der Alterung
  BMS_NV_Temperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -55.0
    Unit: Unit_DegreCelsi
    Range: [-55.0 .. 127.0]
    Comment: battery temperature / Momentanwert: Temperatur der Batterie

### 0x39D (925) - BMS_03
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_Leerlaufspannung:
    Bits: 0|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1022.0]
    Comment: Leerlaufspannung der Gesamtbatterie, Open Circuit Voltage of the battery system
  BMS_Max_LadeSpannung:
    Bits: 12|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: max. voltage / maximal erlaubte Batteriespannung (zwingend einzuhaltender Grenzwert)
  BMS_MaxPred_EntladeStrom_02:
    Bits: 22|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MaxPred_LadeStrom_02:
    Bits: 33|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_MinPred_EntladeSpannung:
    Bits: 44|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1022.0]
    Comment: static el. limit, min. battery voltage (~10s) during current MaxPred_EntladeStrom resp iDisChPredMax...
  BMS_MinPred_LadeSpannung:
    Bits: 54|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1022.0]
    Comment: static el. limit, min. battery voltage (~10s) during current MaxPred_LadeStrom resp. iChPredMax / re...

### 0x3FD (1021) - BMS_MV_10
DLC: 8, Transmitter: Vector__XXX
Signals (2):
  BMS_MV_Pred_Entladeperformance:
    Bits: 50|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  BMS_MV_Pred_Ladeperformance:
    Bits: 57|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]

### 0x457 (1111) - BMS_14
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_14_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_14_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_Warnung_Stuetzung_NV:
    Bits: 26|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  BMS_RIso_GND_HVM:
    Bits: 29|12 (Intel (LE), unsigned)
    Formula: raw * 5.0 + -20465.0
    Unit: Unit_KiloOhm
    Range: [-20465.0 .. 0.0]
  BMS_RIso_GND_HVP:
    Bits: 41|12 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_KiloOhm
    Range: [0.0 .. 20465.0]
  BMS_Spannung_ZwKr2:
    Bits: 53|11 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1022.5]

### 0x509 (1289) - BMS_10
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_Energieinhalt_HiRes:
    Bits: 0|15 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 131060.0]
  BMS_MaxEnergieinhalt_HiRes:
    Bits: 15|15 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 131060.0]
  BMS_NutzbarerSOC:
    Bits: 30|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Aktueller relativer Ladezustand der HV-Batterie
  BMS_NutzbarerEnergieinhalt:
    Bits: 38|12 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 102325.0]
    Comment: Restenergieinhalt der Traktionsbatterie, bezogen auf die Betriebsstrategiegrenzen
  BMS_Pred_Entladeperformance:
    Bits: 50|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  BMS_Pred_Ladeperformance:
    Bits: 57|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]

### 0x578 (1400) - BMS_DC_01
DLC: 8, Transmitter: BMC_MLBevo
Signals (5):
  BMS_DC_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_DC_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BMS_Status_DCLS:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status der Spannungsüberwachung an der DC-Ladeschnittstelle
  BMS_DCLS_Spannung:
    Bits: 14|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
  BMS_DCLS_MaxLadeStrom:
    Bits: 24|9 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 400.0]
    Comment: maximaler zulässiger DC-Ladestrom

### 0x59E (1438) - BMS_06
DLC: 8, Transmitter: BMC_MLBevo
Signals (12):
  BMS_Temperierung_Anf:
    Bits: 0|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Temperierungsbedarf der HV-Batterie
  BMS_Status_Ventil:
    Bits: 3|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Status des 3-2-Wege-Ventils (betätigt/unbetätigt/Fehler)
  BMS_Temp_Epsilon:
    Bits: 8|4 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -15.0
    Unit: Unit_DegreCelsi
    Range: [-15.0 .. 0.0]
    Comment: Das Signal beschreibt die Differenz zwischen Batterie Soll-Temperatur und Batterie Ist-Temperatur.
  BMS_Soll_PWM_Pumpe:
    Bits: 12|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  BMS_Temperatur:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
    Comment: battery temperature / Momentanwert: Temperatur der Traktionsbatterie
  BMS_IstVorlaufTemperatur:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
    Comment: Aktuelle Vorlauftemperatur der Batterie
  BMS_Fehler_NTKreis:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_Fehlerstatus_EWP_02:
    Bits: 37|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Fehlerstatus der elektrischen Wasserpumpe der HV-Batterie
  BMS_SollVolumenstrom:
    Bits: 40|6 (Intel (LE), unsigned)
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 61.0]
    Comment: Soll Volumenstrom für Kühlkreislauf Batterie
  BMS_Batterie_Voko_Anf:
    Bits: 46|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung des BMC die Bauteilvorkonditionierung der HV-Batterie durchzuführen.
  BMS_SollVorlauftemperatur:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
    Comment: Soll_Vorlauftemperatur_Kühlmedium in °C
  BMS_IstRuecklaufTemperatur_02:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
    Comment: Aktuelle Ruecklauftemperatur der Batterie

### 0x5A2 (1442) - BMS_04
DLC: 8, Transmitter: BMC_MLBevo
Signals (9):
  BMS_04_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_04_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BMS_Status_ServiceDisconnect:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_Status_Spgfreiheit:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BMS_OBD_Lampe_Anf:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung MIL (MalfunctionIndicationLamp) von BMS (Motorsteuergerät reicht diese Info an Kombi wei...
  BMS_IstModus:
    Bits: 17|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  BMS_Fehlerstatus:
    Bits: 20|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktueller Fehlerstatus der Batterie
  BMS_Kapazitaet_02:
    Bits: 23|11 (Intel (LE), unsigned)
    Formula: raw * 0.2 + 0.0
    Unit: Unit_AmperHour
    Range: [0.0 .. 409.2]
  BMS_Soll_SOC_HiRes:
    Bits: 53|11 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Der Sollwert für den HV-Batterie-SOC bei einer SOC-steuernden Betriebsstrategie

### 0x5A3 (1443) - BMS_MV_04
DLC: 8, Transmitter: Vector__XXX
Signals (5):
  BMS_MV_04_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_MV_04_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_MV_OBD_Lampe_Anf:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung MIL (MalfunctionIndicationLamp) von BMS (Motorsteuergerät reicht diese Info an Kombi wei...
  BMS_MV_IstModus:
    Bits: 17|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  BMS_MV_FehlerStatus:
    Bits: 20|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktueller Fehlerstatus der Batterie

### 0x5CA (1482) - BMS_07
DLC: 8, Transmitter: BMC_MLBevo
Signals (13):
  BMS_07_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_07_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BMS_Energieinhalt:
    Bits: 12|11 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 102250.0]
  BMS_Ladevorgang_aktiv:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_Batteriediagnose:
    Bits: 24|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Anzeige von Textwarnungen
  BMS_Freig_max_Performanz:
    Bits: 27|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Freigabe der Komponente, dass eine maximale Leistung zur Verfügung gestellt werden kann.
  BMS_Balancing_Aktiv:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Signal zeigt an, ob mindestens eine Zelle der HV Batterie balanciert wird
  BMS_MaxEnergieinhalt:
    Bits: 32|11 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 102250.0]
  BMS_Ausgleichsladung_Anf:
    Bits: 43|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BMS_Gesamtst_Spgfreiheit:
    Bits: 44|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BMS_RIso_Ext:
    Bits: 46|12 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_KiloOhm
    Range: [0.0 .. 20465.0]
    Comment: Minimum des Isolationswiderstands zwischen Batteriegehäuse und HV+ bzw. Batteriegehäuse und HV- bei ...
  BMS_KundenWarnung:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Kunden Warnung aufgrund eines Batteriefehlers
  BMS_Fehler_KuehlkreislaufLeckage:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Bei einer Leckage im Kühlkreislauf wird dieser Fehler gesetzt.

### 0x92DD54A8 (2463978664) - BMS_NV_04
DLC: 8, Transmitter: BMS_NV
Signals (1):
  BMS_NV_Polspannung:
    Bits: 55|9 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 10.5
    Unit: Unit_Volt
    Range: [10.5 .. 15.59]
    Comment: Spannung gemessen unmittelbar vor den Polen des Speichers

### 0x92DD54E0 (2463978720) - BMS_NV_05
DLC: 8, Transmitter: BMS_NV
Signals (6):
  BMS_NV_05_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_NV_05_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_NV_SollPred_EntladeSpannung:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 4.0
    Unit: Unit_Volt
    Range: [4.0 .. 16.65]
    Comment: min. voltage during discharge with current BMS_NV_IstPred_EntladeStrom // fester Wert: für eine Daue...
  BMS_NV_SollDyn_EntladeSpannung:
    Bits: 20|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 4.0
    Unit: Unit_Volt
    Range: [4.0 .. 16.65]
    Comment: min. voltage during discharge with current BMS_NV_IstDyn_EntladeStrom // fester Wert: für eine Dauer...
  BMS_NV_IstDyn_EntladeStrom:
    Bits: 42|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]
  BMS_NV_IstPred_EntladeStrom:
    Bits: 53|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2045.0]

### 0x92DD550B (2463978763) - BMS_30
DLC: 8, Transmitter: BMC_MLBevo
Signals (3):
  BMS_30_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  BMS_30_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  BMS_Ueberspannung_Zelle:
    Bits: 61|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Anforderung Notabschaltung an HVK aufgrund Überladung

### 0x96A954A6 (2527679654) - BMS_11
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_IsoZustand:
    Bits: 17|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Ergebnis des aktuellen/letzten Isolations-Meßzyklus: Messung läuft; Messung Batterie-intern oder Ges...
  BMS_IsoFehler:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Ergebnis der Isolationsmessung des BMS
  BMS_IstTemperatur_hoechste:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.0]
    Comment: aktuell höchste Temperatur in der Batterie
  BMS_IstTemperatur_niedrigste:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.0]
    Comment: aktuell niedrigsteTemperatur in der Batterie
  BMS_IstZellspannung_hoechste:
    Bits: 40|12 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 1000.0
    Unit: Unit_MilliVolt
    Range: [1000.0 .. 5091.0]
    Comment: höchster Wert aller Zellspannungen im BMS
  BMS_IstZellspannung_niedrigste:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 1000.0
    Unit: Unit_MilliVolt
    Range: [1000.0 .. 5091.0]
    Comment: niedrigster Wert aller Zellspannungen im BMS

### 0x96A955EB (2527679979) - BMS_09
DLC: 8, Transmitter: BMC_MLBevo
Signals (6):
  BMS_HV_Auszeit_Status:
    Bits: 21|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status HV-Auszeit.
  BMS_HV_Auszeit:
    Bits: 23|9 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_Minut
    Range: [0.0 .. 2036.0]
    Comment: Zeit seit letzter Hochvoltaktivität.
  BMS_Kapazitaet:
    Bits: 32|11 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_AmperHour
    Range: [0.0 .. 204.6]
    Comment: Gesamtenergiekapazitaet der HV-Batterie unter Berücksichtigung der Alterung
  BMS_SOC_Kaltstart:
    Bits: 43|11 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Abhängig von der notwendigen Kaltstartleistung berechneter SOC Wert.
  BMS_max_Grenz_SOC:
    Bits: 54|5 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 70.0
    Unit: Unit_PerCent
    Range: [70.0 .. 100.0]
    Comment: oberer Grenz SOC - Betriebsstrategie
  BMS_min_Grenz_SOC:
    Bits: 59|5 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 30.0]
    Comment: unterer Grenz SOC - Betriebsstrategie

### 0x97332500 (2536711424) - BAP_BatteryControl_ASG_01
DLC: 8, Transmitter: Gateway
Signals (1):
  BAP_BatteryControl_ASG_01_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]

### 0x97332501 (2536711425) - BAP_BatteryControl_ASG_02
DLC: 8, Transmitter: Gateway
Signals (1):
  BAP_BatteryControl_ASG_02_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]

### 0x97332502 (2536711426) - BAP_BatteryControl_ASG_03
DLC: 8, Transmitter: Gateway
Signals (1):
  BAP_BatteryControl_ASG_03_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]

### 0x97332503 (2536711427) - BAP_BatteryControl_ASG_04
DLC: 8, Transmitter: Gateway
Signals (1):
  BAP_BatteryControl_ASG_04_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]

### 0x97332504 (2536711428) - BAP_BatteryControl_ASG_05
DLC: 8, Transmitter: AWC
Signals (1):
  BAP_BatteryControl_ASG_05_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 255.0]
    Comment: BatteryControl Anzeigesteuergeraet

### 0x97332505 (2536711429) - BAP_BatteryControl_ASG_06
DLC: 8, Transmitter: Gateway
Signals (1):
  BAP_BatteryControl_ASG_06_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: BatteryControl Anzeigesteuergerät Head-Up Display

### 0x97332510 (2536711440) - BAP_BatteryControl_FSG_01
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  BAP_BatteryControl_FSG_01_Header:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]

### 0x97F0009C (2549088412) - KN_BMS_MV
DLC: 8, Transmitter: Vector__XXX
Signals (9):
  BMS_MV_KompSchutz:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund Komponentenschutz aktiv
  BMS_MV_Abschaltstufe:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktiver Abschaltstufe
  BMS_MV_Transport_Mode:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem Transport Mode und/oder Transport Schutz
  BMS_MV_Nachlauftyp:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  BMS_MV_SNI:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Das Signal beinhaltet den Source Node Identifier (Knotenadresse, die auch in der Abbildungsvorschrif...
  NM_BMS_MV_Wakeup:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: enthält die Weckursache; stehen mehrere Weckursachen parallel an, dann muss der kleinste Wert übertr...
  BMS_MV_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob das SG nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv war
  BMS_MV_Subsystemaktiv:
    Bits: 62|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob ein Subsystem des SGs nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv...
  BMS_MV_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei gesetztem Bit ist mindestens ein Kundendienstfehler eingetragen

### 0x97FC009C (2549874844) - ISOx_BMS_48V_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISO_BMS_48V_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request Batterie-Management-System 48V

### 0x97FC029C (2549875356) - OBDC_BMS_48V_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_BMS_48V_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE009C (2550005916) - ISOx_BMS_48V_Resp
DLC: 8, Transmitter: BMS_MV
Signals (1):
  ISO_BMS_48V_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Response Batterie-Management-System 48V

### 0x97FE029C (2550006428) - OBDC_BMS_48V_Resp
DLC: 8, Transmitter: BMS_MV
Signals (1):
  OBDC_BMS_48V_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9A555454 (2589283412) - BMS_MV_06
DLC: 8, Transmitter: Vector__XXX
Signals (9):
  BMS_MV_Temperierung_Anf:
    Bits: 0|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Temperierungsbedarf der HV-Batterie
  BMS_MV_Speichertechnologie:
    Bits: 3|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Speichertechnologie des MV-Speichers. Zuordnung zur Technologie in MV FLAH.
  BMS_MV_Temp_Epsilon:
    Bits: 8|4 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -15.0
    Unit: Unit_DegreCelsi
    Range: [-15.0 .. 0.0]
    Comment: Das Signal beschreibt die Differenz zwischen Batterie Soll-Temperatur und Batterie Ist-Temperatur.
  BMS_MV_Soll_PWM_Pumpe:
    Bits: 12|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Fehlende Beschreibung (Import)
  BMS_MV_Temperatur:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_MV_IstVorlaufTemperatur:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_MV_SollVolumenstrom:
    Bits: 40|6 (Intel (LE), unsigned)
    Range: [0.0 .. 61.0]
    Comment: Mindestanforderung des Soll Volumenstrom für Kühlkreislauf Batterie
  BMS_MV_SollVorlauftemperatur:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]
  BMS_MV_IstRuecklaufTemperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 86.5]

### 0x9A555516 (2589283606) - BMS_MV_09
DLC: 8, Transmitter: Vector__XXX
Signals (2):
  BMS_MV_SOC_Kaltstart:
    Bits: 43|11 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Abhängig von der notwendigen Kaltstartleistung berechneter SOC Wert.
  BMS_MV_max_Grenz_SOC:
    Bits: 54|5 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 70.0
    Unit: Unit_PerCent
    Range: [70.0 .. 100.0]
    Comment: oberer Grenz SOC - Betriebsstrategie

### 0x9A555518 (2589283608) - BMS_MV_07
DLC: 8, Transmitter: Vector__XXX
Signals (2):
  BMS_MV_Energieinhalt:
    Bits: 12|11 (Intel (LE), unsigned)
    Unit: Unit_WattHour
    Range: [0.0 .. 2045.0]
  BMS_MV_MaxEnergieinhalt:
    Bits: 32|11 (Intel (LE), unsigned)
    Unit: Unit_WattHour
    Range: [0.0 .. 2045.0]

### 0x9A555535 (2589283637) - BMS_12
DLC: 8, Transmitter: BMC_MLBevo
Signals (8):
  BMS_Pack_SN_01:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 1, ASCII Codierung
  BMS_Pack_SN_02:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 2, ASCII Codierung
  BMS_Pack_SN_03:
    Bits: 16|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 3, ASCII Codierung
  BMS_Pack_SN_04:
    Bits: 24|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 4, ASCII Codierung
  BMS_Pack_SN_05:
    Bits: 32|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 5, ASCII Codierung
  BMS_Pack_SN_06:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 6, ASCII Codierung
  BMS_Pack_SN_07:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 7, ASCII Codierung
  BMS_Pack_SN_08:
    Bits: 56|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 8, ASCII Codierung

### 0x9A555536 (2589283638) - BMS_13
DLC: 8, Transmitter: BMC_MLBevo
Signals (8):
  BMS_Pack_SN_09:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 9, ASCII Codierung
  BMS_Pack_SN_10:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 10, ASCII Codierung
  BMS_Pack_SN_11:
    Bits: 16|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 11, ASCII Codierung
  BMS_Pack_SN_12:
    Bits: 24|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 12, ASCII Codierung
  BMS_Pack_SN_13:
    Bits: 32|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 13, ASCII Codierung
  BMS_Pack_SN_14:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 14, ASCII Codierung
  BMS_Pack_SN_15:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 15, ASCII Codierung
  BMS_Pack_SN_16:
    Bits: 56|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 16, ASCII Codierung

### 0x9A555539 (2589283641) - BMS_16
DLC: 8, Transmitter: BMC_MLBevo
Signals (8):
  BMS_IstTemperaturMin_Sensor_ID:
    Bits: 6|5 (Intel (LE), unsigned)
    Range: [1.0 .. 30.0]
    Comment: RTM-Signal: Nummer (ID) des Sensors (innerhalb des Moduls) mit dem aktuell niedrigsten Temperaturmeß...
  BMS_IstTemperaturMax_Sensor_ID:
    Bits: 11|5 (Intel (LE), unsigned)
    Range: [1.0 .. 30.0]
    Comment: RTM-Signal: Nummer (ID) des Sensors (innerhalb des Moduls) mit dem aktuell höchsten Temperaturmeßwer...
  BMS_IstTemperaturMin_Modul_ID:
    Bits: 16|8 (Intel (LE), unsigned)
    Range: [1.0 .. 254.0]
    Comment: RTM-Signal: Nummer (ID) des Moduls mit dem aktuell niedrigsten Temperaturmeßwert
  BMS_IstTemperaturMax_Modul_ID:
    Bits: 24|8 (Intel (LE), unsigned)
    Range: [1.0 .. 254.0]
    Comment: RTM-Signal: Nummer (ID) des Moduls mit dem aktuell höchsten Temperaturmeßwert
  BMS_IstZellSpannungMin_Modul_ID:
    Bits: 32|8 (Intel (LE), unsigned)
    Range: [1.0 .. 254.0]
    Comment: RTM-Signal: Nummer (ID) des Moduls mit der aktuell niedrigsten Zellspannung
  BMS_IstZellSpannungMax_Modul_ID:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [1.0 .. 254.0]
    Comment: RTM-Signal: Nummer (ID) des Moduls mit der aktuell höchsten Zellspannung
  BMS_IstZellSpannungMin_Zell_ID:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [1.0 .. 254.0]
    Comment: RTM-Signal: Nummer (ID) der Zelle (innerhalb des Moduls) mit der aktuell niedrigsten Zellspannung
  BMS_IstZellSpannungMax_Zell_ID:
    Bits: 56|8 (Intel (LE), unsigned)
    Range: [1.0 .. 254.0]
    Comment: RTM-Signal: Nummer (ID) der Zelle (innerhalb des Moduls) mit der aktuell höchsten Zellspannung

### 0x9A555543 (2589283651) - BMS_17
DLC: 8, Transmitter: BMC_MLBevo
Signals (14):
  BMS_RtmWarnUeberstrom:
    Bits: 22|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Real Time Monitor Over-current of overall current of traction battery
  BMS_RtmWarnPackKommunikation:
    Bits: 25|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Real Time Monitor Traction battery CAN communication fault
  BMS_RtmWarnPackUeberladen:
    Bits: 28|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Energy-storage over-charging warning
  BMS_RtmWarnSocHoch:
    Bits: 31|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Excessively-high SOC warning
  BMS_RtmWarnSocNiedrig:
    Bits: 34|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Low SOC warning
  BMS_RtmWarnSocSprung:
    Bits: 37|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal SOC jump warning
  BMS_RtmWarnTZelleDiff:
    Bits: 40|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Temperature difference warning
  BMS_RtmWarnTZelleMax:
    Bits: 43|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Max_temperature_cell_Warning
  BMS_RtmWarnTZelleMin:
    Bits: 46|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Min_temperature_cell_Warning
  BMS_RtmWarnUPackMax:
    Bits: 49|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Max_Voltage_of_battery_Warning
  BMS_RtmWarnUPackMin:
    Bits: 52|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Min_Voltage_of_battery_Warning
  BMS_RtmWarnUZelleMax:
    Bits: 55|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Max_voltage_cell_Warning
  BMS_RtmWarnUZelleMin:
    Bits: 58|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Min_voltage_cell_Warning
  BMS_RtmWarnZelleZustand:
    Bits: 61|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal Cell poor-consistency warning

### 0x9A555544 (2589283652) - BMS_18
DLC: 8, Transmitter: BMC_MLBevo
Signals (2):
  BMS_Energie_Entladung:
    Bits: 32|16 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 0.0
    Unit: Unit_KiloWattHour
    Range: [0.0 .. 655.33]
    Comment: Energie, welche seit dem letzten Fahrzyklus aus der HV-Batterie entnommen wurde.
  BMS_Energie_Ladung:
    Bits: 48|16 (Intel (LE), unsigned)
    Formula: raw * 0.01 + 0.0
    Unit: Unit_KiloWattHour
    Range: [0.0 .. 655.33]
    Comment: Energie, welche seit dem letzten Fahrzyklus in die HV-Batterie geladen wurde.

### 0x9A555552 (2589283666) - BMS_27
DLC: 8, Transmitter: BMC_MLBevo
Signals (5):
  BMS_LadegrenzeAntw_SOC:
    Bits: 26|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: SOC-Grenze bis zu welcher mit der Leistung aus BMS_LadegrenzeAntw_Leistung geladen werden kann.
  BMS_EnergieAntw_Zaehler:
    Bits: 33|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Fortlaufender Zähler für die Antwort Energie.
  BMS_EnergieAntw_Energie:
    Bits: 37|11 (Intel (LE), unsigned)
    Formula: raw * 250.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 511250.0]
    Comment: Für Laden der HV-Batterie notwendige Energie (um angefragten SOC-Hub zu erreichen).
  BMS_LadegrenzeAntw_Leistung:
    Bits: 48|12 (Intel (LE), unsigned)
    Formula: raw * 200.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 818600.0]
    Comment: Ladeleistung mit welcher bis zu SOC aus BMS_LadegrenzeAntw_SOC geladen werden kann.
  BMS_LadegrenzeAntw_Zaehler:
    Bits: 60|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Fortlaufender Zähler für die Antwort Leistung.

### 0x9A555625 (2589283877) - BMS_15
DLC: 8, Transmitter: BMC_MLBevo
Signals (8):
  BMS_Pack_SN_17:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 17, ASCII Codierung
  BMS_Pack_SN_18:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 18, ASCII Codierung
  BMS_Pack_SN_19:
    Bits: 16|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 19, ASCII Codierung
  BMS_Pack_SN_20:
    Bits: 24|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 20, ASCII Codierung
  BMS_Pack_SN_21:
    Bits: 32|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 21, ASCII Codierung
  BMS_Pack_SN_22:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 22, ASCII Codierung
  BMS_Pack_SN_23:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 23, ASCII Codierung
  BMS_Pack_SN_24:
    Bits: 56|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 24, ASCII Codierung

### 0x9A555626 (2589283878) - BMS_19
DLC: 8, Transmitter: BMC_MLBevo
Signals (2):
  BMS_Pack_SN_25:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 25, ASCII Codierung
  BMS_Pack_SN_26:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Seriennummer der HV Batterie, Byte 26, ASCII Codierung

### 0x9B00009C (2600468636) - NMH_BMS_48V
DLC: 8, Transmitter: BMS_MV
Signals (11):
  NM_BMS_48V_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  NM_BMS_48V_CBV_AWB:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist zu setzen, wenn das SG aktiv den Bus geweckt hat
  NM_BMS_48V_CBV_CRI:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist dauerhaft gesetzt, wenn das NM-Layout dem neuen NM-Layout für Teilnetzbetrieb entspri...
  NM_BMS_48V_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_BMS_48V_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Car Wakeup (Gateway wertet aus)
  NM_BMS_48V_UDS_CC:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschraenkung aufgrund aktivem UDS-Communication Control; siehe VW80124
  NM_BMS_48V_NM_aktiv_KL15:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuergeraet hat KL15-EIN erkannt
  NM_BMS_48V_NM_aktiv_Diagnose:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diagnose befindet sich nach VW80124 in einer NonDefaultDiagnostic-Session, z.B. ExtendedDiagnostic-S...
  NM_BMS_48V_NM_aktiv_Tmin:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Mindestaktivzeit ist noch nicht abgelaufen (bei jedem Weckereignis wird die Mindestaktivzeit neu ges...
  NM_BMS_48V_NM_aktiv_Hochvolt:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Hochvolt aktiv
  NM_BMS_48V_CAB:
    Bits: 40|24 (Intel (LE), unsigned)
    Range: [0.0 .. 16777215.0]
    Comment: CAB = Clusteraktivbits

### 0x9BFCE300 (2617041664) - DEV_BMS_MV_Resp_00
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  DEV_BMS_MV_Resp_00_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFCE3FF (2617041919) - DEV_BMS_MV_Resp_FF
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  DEV_BMS_MV_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFCE701 (2617042689) - XCP_BMS_MV_DTO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  XCP_BMS_MV_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x9BFFDA00 (2617235968) - DEV_BMS_Resp_00
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  DEV_BMS_Resp_00_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFFDAFF (2617236223) - DEV_BMS_Resp_FF
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  DEV_BMS_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFFDE00 (2617236992) - CCP_BMS_CRO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  CCP_BMS_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x9BFFDE01 (2617236993) - CCP_BMS_DTO_01
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  CCP_BMS_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

## Charging


### 0x744 (1860) - ISO_Ladegeraet_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISO_Ladegeraet_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request Ladegerät

### 0x7AE (1966) - ISO_Ladegeraet_Resp
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  ISO_Ladegeraet_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Response Ladegerät

### 0x97F00044 (2549088324) - KN_Ladegeraet
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (7):
  KN_Ladegeraet_ECUKnockOutTimer:
    Bits: 32|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 62.0]
    Comment: Ausgabe des ECUKnockOut-Timer in der Knotenbotschaft
  KN_Ladegeraet_BusKnockOut:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des BusKnockout-Status in der Knotenbotschaft
  KN_Ladegeraet_BusKnockOutTimer:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
    Comment: Ausgabe des BusKnockOut-Timer in der Knotenbotschaft
  NM_Ladegeraet_Wakeup:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: enthält die Weckursache; stehen mehrere Weckursachen parallel an, dann muss der kleinste Wert übertr...
  KN_Ladegeraet_ECUKnockOut:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des ECUKnockout-Status in der Knotenbotschaft
  NMH_Ladegeraet_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob Steuergerät lokal aktiv war
  NMH_Ladegeraet_Subsystemaktiv:
    Bits: 62|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob ein Subsystem des SGs nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv...

### 0x97F000C6 (2549088454) - KN_Ladegeraet_2
DLC: 8, Transmitter: Ladegeraet_2
Signals (12):
  Ladegeraet_2_KompSchutz:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund Komponentenschutz aktiv
  Ladegeraet_2_Abschaltstufe:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktiver Abschaltstufe
  Ladegeraet_2_Transport_Mode:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem Transport Mode und/oder Transport Schutz
  Ladegeraet_2_Nachlauftyp:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  Ladegeraet_2_SNI:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  KN_Ladegeraet_2_ECUKnockOutTimer:
    Bits: 32|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 62.0]
    Comment: Ausgabe des ECUKnockOut-Timer in der Knotenbotschaft
  KN_Ladegeraet_2_BusKnockOut:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des BusKnockout-Status in der Knotenbotschaft
  KN_Ladegeraet_2_BusKnockOutTimer:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
    Comment: Ausgabe des BusKnockOut-Timer in der Knotenbotschaft
  NM_Ladegeraet_2_Wakeup:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: enthält die Weckursache; stehen mehrere Weckursachen parallel an, dann muss der kleinste Wert übertr...
  KN_Ladegeraet_2_ECUKnockOut:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des ECUKnockout-Status in der Knotenbotschaft
  NM_Ladegeraet_2_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob das SG nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv war.
  Ladegeraet2_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei gesetztem Bit ist mindestens ein Kundendienstfehler eingetragen

### 0x97FC00C6 (2549874886) - ISOx_Ladegeraet_2_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISO_Ladegeraet_2_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request Ladegerät_2

### 0x97FC0144 (2549875012) - KS_Ladegeraet_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  KS_Ladegeraet_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Komponentenschutz (Funktion 01h). Request mit NA des Slaves.

### 0x97FC01C6 (2549875142) - KS_Ladegeraet_2_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  KS_Ladegeraet_2_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC0244 (2549875268) - OBDC_Ladegeraet_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_Ladegeraet_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC02C6 (2549875398) - OBDC_Ladegeraet_2_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_Ladegeraet_2_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FC0844 (2549876804) - PnC_Ladegeraet_Req
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  PnC_Ladegeraet_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Plug and Charge request an Ladegeraet

### 0x97FE00C6 (2550005958) - ISOx_Ladegeraet_2_Resp
DLC: 8, Transmitter: Ladegeraet_2
Signals (1):
  ISO_Ladegeraet_2_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Response Ladegerät_2

### 0x97FE0144 (2550006084) - KS_Ladegeraet_Resp
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  KS_Ladegeraet_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Komponentenschutz (Funktion 01h). Response mit eigener NA.

### 0x97FE01C6 (2550006214) - KS_Ladegeraet_2_Resp
DLC: 8, Transmitter: Ladegeraet_2
Signals (1):
  KS_Ladegeraet_2_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE0244 (2550006340) - OBDC_Ladegeraet_Resp
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  OBDC_Ladegeraet_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE02C6 (2550006470) - OBDC_Ladegeraet_2_Resp
DLC: 8, Transmitter: Ladegeraet_2
Signals (1):
  OBDC_Ladegeraet_2_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE0844 (2550007876) - PnC_Ladegeraet_Resp
DLC: 8, Transmitter: Gateway
Signals (1):
  PnC_Ladegeraet_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Plug and Charge response an Ladegeraet

### 0x9B000044 (2600468548) - NMH_Ladegeraet
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (16):
  NM_Ladegeraet_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  NM_Ladegeraet_CBV_AWB:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist zu setzen, wenn das SG aktiv den Bus geweckt hat
  NM_Ladegeraet_CBV_CRI:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist dauerhaft gesetzt, wenn das NM-Layout dem neuen NM-Layout für Teilnetzbetrieb entspri...
  NM_Ladegeraet_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_Ladegeraet_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_Ladegeraet_UDS_CC:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem UDS-Communication Control; siehe VW80124
  NM_Ladegeraet_NM_aktiv_KL15:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuergerät hat KL15-EIN erkannt
  NM_Ladegeraet_NM_aktiv_Diagnose:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diagnose befindet sich nach VW80124 in einer NonDefaultDiagnostic-Session, z.B. ExtendedDiagnostic-S...
  NM_Ladegeraet_NM_aktiv_Tmin:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Mindestaktivzeit ist noch nicht abgelaufen (bei jedem Weckereignis wird die Mindestaktivzeit neu ges...
  NM_Ladegeraet_NM_aktiv_Bere_CVN:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_Ladegeraet_NM_aktiv_Vorkondit:
    Bits: 28|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Wachhalten aufgrund Vorkonditionierung
  NM_Ladegeraet_NM_aktiv_CVN_Berec:
    Bits: 29|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Berechnung CVN ist aktiv
  NM_Ladegeraet_NM_aktiv_NotlaufSt:
    Bits: 30|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Lokaler Nachlauf bei Kl15EIN durch aktivierten Notlauf bei gestecktem oder defekten Steckerkontakt
  NM_Ladegeraet_NM_aktiv_Ladezeit:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bus gefordert bei Ladezeitberechnung
  NM_Ladegeraet_NM_aktiv_Laden:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bus gefordert bei aktivem Ladevorgang
  NM_Ladegeraet_CAB:
    Bits: 40|24 (Intel (LE), unsigned)
    Range: [0.0 .. 16777215.0]
    Comment: CAB = Clusteraktivbits

### 0x9B0000C6 (2600468678) - NMH_Ladegeraet_2
DLC: 8, Transmitter: Ladegeraet_2
Signals (11):
  NM_Ladegeraet_2_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  NM_Ladegeraet_2_CBV_AWB:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist zu setzen, wenn das SG aktiv den Bus geweckt hat
  NM_Ladegeraet_2_CBV_CRI:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist dauerhaft gesetzt, wenn das NM-Layout dem neuen NM-Layout für Teilnetzbetrieb entspri...
  NM_Ladegeraet_2_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_Ladegeraet_2_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_Ladegeraet_2_UDS_CC:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem UDS-Communication Control; siehe VW80124
  NM_Ladegeraet_2_NM_aktiv_KL15:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuergerät hat KL15-EIN erkannt
  NM_Ladegeraet_2_NM_aktiv_Diag:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diagnose befindet sich nach VW80124 in einer NonDefaultDiagnostic-Session, z.B. ExtendedDiagnostic-S...
  NM_Ladegeraet_2_NM_aktiv_Tmin:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Mindestaktivzeit ist noch nicht abgelaufen (bei jedem Weckereignis wird die Mindestaktivzeit neu ges...
  NM_Ladegeraet_2_NM_aktiv_Laden:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bus gefordert bei aktivem Ladevorgang
  NM_Ladegeraet_2_CAB:
    Bits: 40|24 (Intel (LE), unsigned)
    Range: [0.0 .. 16777215.0]
    Comment: CAB = Clusteraktivbits

### 0x9BFE2200 (2617123328) - DEV_Ladegeraet_Req_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  DEV_Ladegeraet_Req_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE2201 (2617123329) - DEV_Ladegeraet_Resp_01
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  DEV_Ladegeraet_Resp_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE22FF (2617123583) - DEV_Ladegeraet_Resp_FF
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  DEV_Ladegeraet_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE2600 (2617124352) - CCP_Ladegeraet_CRO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  CCP_Ladegeraet_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE2601 (2617124353) - CCP_Ladegeraet_DTO_01
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  CCP_Ladegeraet_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFF3300 (2617193216) - DEV_Ladegeraet_2_Req_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  DEV_Ladegeraet_2_Req_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFF3301 (2617193217) - DEV_Ladegeraet_2_Resp_01
DLC: 8, Transmitter: Ladegeraet_2
Signals (1):
  DEV_Ladegeraet_2_Resp_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFF33FF (2617193471) - DEV_Ladegeraet_2_Resp_FF
DLC: 8, Transmitter: Ladegeraet_2
Signals (1):
  DEV_Ladegeraet_2_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFF3700 (2617194240) - CCP_Ladegeraet_2_CRO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  CCP_Ladegeraet_2_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFF3701 (2617194241) - CCP_Ladegeraet_2_DTO_01
DLC: 8, Transmitter: Ladegeraet_2
Signals (1):
  CCP_Ladegeraet_2_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

## DC-DC Converter


### 0x2AD (685) - DCDC_MV_01
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (6):
  DCDC_MV_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_MV_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  DC_MV_IstSpannung_MV:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: Momentanwert:  Hochspannung am DC/DC Wandler
  DC_MV_IstStrom_MV:
    Bits: 24|10 (Intel (LE), unsigned)
    Formula: raw * 0.4 + -204.0
    Unit: Unit_Amper
    Range: [-204.0 .. 204.0]
  DC_MV_IstStrom_NV:
    Bits: 34|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  DC_MV_IstSpannung_NV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 25.3]
    Comment: Momentanwert: Bordnetzspannung am DC/DC Wandler

### 0x2AE (686) - DCDC_01
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (6):
  DCDC_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  DC_IstSpannung_HV:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: Momentanwert:  Hochspannung am DC/DC Wandler
  DC_IstStrom_HV_02:
    Bits: 24|10 (Intel (LE), unsigned)
    Formula: raw * 0.4 + -204.0
    Unit: Unit_Amper
    Range: [-204.0 .. 204.0]
  DC_IstStrom_NV:
    Bits: 34|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  DC_IstSpannung_NV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 25.3]
    Comment: Momentanwert: Bordnetzspannung am DC/DC Wandler

### 0x3F4 (1012) - DCDC_02
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (7):
  DCDC_02_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_02_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  DC_HYB_iAktReserveLV:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  DC_Verbrauch_Ueberlauf:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Überlauf-Bit für das Signal DC_Verbrauch
  DC_Verbrauch:
    Bits: 24|10 (Intel (LE), unsigned)
    Unit: Unit_WattSecond
    Range: [0.0 .. 1023.0]
    Comment: Energieverbrauch vom DCDC für Versorgung 12V-Bordnetz inkl. seines eigenen Wirkungsgrads (bspw.: HV_...
  DC_Verlustleistung:
    Bits: 34|6 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 305.0]
    Comment: Aktuelle Verlustleistung des DC/DC-Wandlers.
  DC_HYB_Auslastungsgrad:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]

### 0x3FA (1018) - DCDC_MV_02
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (5):
  DCDC_MV_02_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_MV_02_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  DC_MV_akt_Stromreserve_NV:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  DC_MV_MaxStrom_MV:
    Bits: 50|6 (Intel (LE), unsigned)
    Formula: raw * 2.5 + -152.5
    Unit: Unit_Amper
    Range: [-152.5 .. 0.0]
  DC_MV_Auslastungsgrad:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Momentanwert: Auslastung DC/DC (DFM Signal), entspricht I_aktuell / I_verfügbar * 100%

### 0x5CC (1484) - DCDC_MV_03
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (12):
  DCDC_MV_03_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_MV_03_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  DCDC_MV_Peakstrom_verfuegbar:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Peakstrom verfügbar/nicht verfügbar
  DC_MV_Tiefsetzen_Notbetrieb:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: DCDC-Wandler arbeitet im Notbetrieb; ungeregelter Betrieb mit festem Tastverhältnis
  DC_MV_Fehlerstatus:
    Bits: 16|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktueller Fehlerstatus des DCDC-Wandler:
  DC_MV_12V_Bordnetzstuetzung:
    Bits: 19|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: DC/DC-Wandler befindet sich im Betriebsmodus Bordnetzstützung
  DC_MV_Abregelung_Temperatur:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Abregelung aufgrund Übertemperatur im Leistungsteil (Hochtemperatur-Abregelung)
  DC_MV_IstModus:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktuelle Betriebsart des DC/DC Wandlers;
  DC_MV_IstModus_02:
    Bits: 36|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: aktuelle Betriebsart des DC/DC Wandlers
  DC_MV_Dyn_MinSpannung:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: Spannungsgrenze, um eine Leistungsbeschränkung des eAWS-Systems in einem speicherlosen 48V-Netz durc...
  DC_MV_Dyn_MaxSpannung:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: Spannungsgrenze, um eine Leistungsbeschränkung des eAWS-Systems in einem speicherlosen 48V-Netz durc...
  DC_MV_Temperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des DC/DC Wandlers

### 0x5CD (1485) - DCDC_03
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (10):
  DCDC_03_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_03_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  DC_Fehlerstatus:
    Bits: 16|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktueller Fehlerstatus des DCDC-Wandler:
  DC_Peakstrom_verfuegbar:
    Bits: 19|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Peakstrom verfügbar/nicht verfügbar
  DC_Abregelung_Temperatur:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Abregelung aufgrund Übertemperatur im Leistungsteil (Hochtemperatur-Abregelung)
  DC_IstModus_02:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  DC_HV_EKK_IstModus:
    Bits: 28|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  DC_Status_Spgfreiheit_HV:
    Bits: 46|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: HV DC/DC-Wandler Status HV-Spannungsfreiheit
  DC_IstSpannung_EKK_HV:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 508.0]
    Comment: Ausgangsspannung des Kombiwandlers am 400V-Anschluss
  DC_Temperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des DC/DC Wandlers

### 0x741 (1857) - ISO_DCDC_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISO_DCDC_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request DCDC-Wandler

### 0x7AB (1963) - ISO_DCDC_Resp
DLC: 8, Transmitter: DCDC_IHEV
Signals (1):
  ISO_DCDC_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Response DCDC-Wandler

### 0x92DD54AD (2463978669) - DCDC_HV_02_02
DLC: 8, Transmitter: DCDC_HV_02
Signals (5):
  DCDC_HV_02_02_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_HV_02_02_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  HVLB_Temperatur:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: HV-Lade-Booster Temperatur
  HVLB_IstSpannung_HVLS:
    Bits: 40|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: HV-Lade-Booster IstSpannung Eingangsseite
  HVLB_MaxDynStrom_HVLS:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + -512.0
    Unit: Unit_Amper
    Range: [-512.0 .. 511.25]
    Comment: HV-Lade Booster, maximal zulässiger Strom Eingangsseite

### 0x96A95500 (2527679744) - DCDC_FC_03
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (21):
  DCDC_FC_IstModus:
    Bits: 12|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Ist-Modus des DCDC_FC
  DCDC_FC_Abschaltung_Ges:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: BZW schaltet sofort ab und fordert die Abschaltung des Gesamtsystems.
  DCDC_FC_Abschaltung_Prim:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: BZW schaltet sofort ab und fordert die Abschaltung des primären HV-Netzes.
  DCDC_FC_Anf_Abschaltung_Ges:
    Bits: 18|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: BZW ist noch aktiv und fordert die Abschaltung des Gesamtsystems.
  DCDC_FC_Anf_Abschaltung_Prim:
    Bits: 19|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: BZW ist noch aktiv und fordert die Abschaltung des primären HV-Netzes.
  DCDC_FC_Begrenz_MaxSpannung_Prim:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Überschreitung der vorgegebenen maximalen Eingangsspannung.
  DCDC_FC_Begrenz_MaxSpannung_Sek:
    Bits: 21|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Überschreitung der vorgegebenen maximalen Ausgangsspannung.
  DCDC_FC_Begrenz_MaxStrom_Prim:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Überschreitung des vorgegebenen maximalen Eingangsstroms.
  DCDC_FC_Begrenz_MaxStrom_Sek:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Überschreitung des vorgegebenen maximalen Ausgangsstroms.
  DCDC_FC_Begrenz_MinSpannung_Prim:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Unterschreitung der vorgegebenen minimalen Eingangsspannung.
  DCDC_FC_Begrenz_MinSpannung_Sek:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Unterschreitung der vorgegebenen minimalen Ausgangsspannung.
  DCDC_FC_Begrenz_MinStrom_Prim:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Unterschreitung des vorgegebenen minimalen Eingangsstroms.
  DCDC_FC_Begrenz_Temp:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Begrenzung aufgrund Überschreitung BZW-interner Temperaturgrenzen
  DCDC_FC_Diodenbetrieb:
    Bits: 28|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diodenbetrieb des DCDC_FC, ungereglter Stromfluss über die Diode
  DCDC_FC_Fehlerstatus:
    Bits: 29|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Fehlerstatus des DCDC_FC
  DCDC_FC_Status_HVSwitch:
    Bits: 32|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Status der HV-Schaltelemente im BZW zur Abtrennung des Brennstoffzellenstacks
  DCDC_FC_Status_Interlock:
    Bits: 35|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status Signal Interlock
  DCDC_FC_Status_Isowaechter_Netz2:
    Bits: 37|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Status des Isowächters im DCDC_FC
  DCDC_FC_Status_Kl15c:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status Signal Kl15c
  DCDC_FC_Status_Kl30C:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status Signal Kl30C
  DCDC_FC_Temperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Ist-Temperatur des DCDC_FC

### 0x97F00041 (2549088321) - KN_DCDC
DLC: 8, Transmitter: DCDC_IHEV
Signals (10):
  DCDC_KompSchutz:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund Komponentenschutz aktiv
  DCDC_Abschaltstufe:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktiver Abschaltstufe
  DCDC_Transport_Mode:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem Transport Mode und/oder Transport Schutz
  DCDC_Nachlauftyp:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  DCDC_SNI:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Das Signal beinhaltet den Source Node Identifier (Knotenadresse, die auch in der Abbildungsvorschrif...
  KN_DCDC_BusKnockOutTimer:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
    Comment: Ausgabe des BusKnockOut-Timer in der Knotenbotschaft
  NM_DCDC_Wakeup:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: enthält die Weckursache; stehen mehrere Weckursachen parallel an, dann muss der kleinste Wert übertr...
  NMH_DCDC_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob Steuergerät lokal aktiv war
  NMH_DCDC_Subsystemaktiv:
    Bits: 62|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob ein Subsystem des SGs nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv...
  DCDC_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei gesetztem Bit ist mindestens ein Kundendienstfehler eingetragen

### 0x97F000B7 (2549088439) - KN_DCDC_HV
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (12):
  DCDC_HV_KompSchutz:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschraenkung aufgrund Komponentenschutz aktiv
  DCDC_HV_Abschaltstufe:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschraenkung aufgrund aktiver Abschaltstufe
  DCDC_HV_Transport_Mode:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschraenkung aufgrund aktivem Transport Mode und/oder Transport Schutz
  DCDC_HV_Nachlauftyp:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  DCDC_HV_SNI:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  DCDC_HV_ECUKnockOutTimer:
    Bits: 32|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 62.0]
    Comment: Ausgabe des ECUKnockOut-Timer in der Knotenbotschaft
  DCDC_HV_BusKnockOut:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des BusKnockout-Status in der Knotenbotschaft
  DCDC_HV_BusKnockOutTimer:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
    Comment: Ausgabe des BusKnockOut-Timer in der Knotenbotschaft
  NM_DCDC_HV_Wakeup:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: enthaelt die Weckursache; stehen mehrere Weckursachen parallel an, dann muss der kleinste Wert übert...
  DCDC_HV_ECUKnockOut:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des ECUKnockout-Status in der Knotenbotschaft
  DCDC_HV_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob das SG nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv war.
  DCDC_HV_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei gesetztem Bit ist mindestens ein Kundendienstfehler eingetragen

### 0x97F000C7 (2549088455) - KN_DCDC_HV_02
DLC: 8, Transmitter: DCDC_HV_02
Signals (12):
  DCDC_HV_02_KompSchutz:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund Komponentenschutz aktiv
  DCDC_HV_02_Abschaltstufe:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktiver Abschaltstufe
  DCDC_HV_02_Transport_Mode:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem Transport Mode und/oder Transport Schutz
  DCDC_HV_02_Nachlauftyp:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  DCDC_HV_02_SNI:
    Bits: 8|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  KN_DCDC_HV_02_ECUKnockOutTimer:
    Bits: 32|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 62.0]
    Comment: Ausgabe des ECUKnockOut-Timer in der Knotenbotschaft
  KN_DCDC_HV_02_BusKnockOut:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des BusKnockout-Status in der Knotenbotschaft
  KN_DCDC_HV_02_BusKnockOutTimer:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
    Comment: Ausgabe des BusKnockOut-Timer in der Knotenbotschaft
  NM_DCDC_HV_02_Wakeup:
    Bits: 48|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: enthält die Weckursache; stehen mehrere Weckursachen parallel an, dann muss der kleinste Wert übertr...
  KN_DCDC_HV_02_ECUKnockOut:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des ECUKnockout-Status in der Knotenbotschaft
  NM_DCDC_HV_02_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob das SG nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv war.
  DCDC_HV_02_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei gesetztem Bit ist mindestens ein Kundendienstfehler eingetragen

### 0x97FC00B7 (2549874871) - ISOx_DCDC_HV_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISO_DCDC_HV_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC00C7 (2549874887) - ISOx_DCDC_HV_02_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISO_DCDC_HV_02_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request DCDC_HV_02

### 0x97FC00CF (2549874895) - ISOx_DCDC_HV_03_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  ISOx_DCDC_HV_03_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request

### 0x97FC0241 (2549875265) - OBDC_DCDC_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_DCDC_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FC02B7 (2549875383) - OBDC_DCDC_HV_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_DCDC_HV_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FC02C7 (2549875399) - OBDC_DCDC_HV_02_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_DCDC_HV_02_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FC02CF (2549875407) - OBDC_DCDC_HV_03_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_DCDC_HV_03_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE00B7 (2550005943) - ISOx_DCDC_HV_Resp
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (1):
  ISO_DCDC_HV_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE00C7 (2550005959) - ISOx_DCDC_HV_02_Resp
DLC: 8, Transmitter: DCDC_HV_02
Signals (1):
  ISO_DCDC_HV_02_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Request DCDC_HV_02

### 0x97FE00CF (2550005967) - ISOx_DCDC_HV_03_Resp
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (1):
  ISOx_DCDC_HV_03_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnose Response

### 0x97FE0241 (2550006337) - OBDC_DCDC_Resp
DLC: 8, Transmitter: DCDC_IHEV
Signals (1):
  OBDC_DCDC_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE02B7 (2550006455) - OBDC_DCDC_HV_Resp
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (1):
  OBDC_DCDC_HV_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE02C7 (2550006471) - OBDC_DCDC_HV_02_Resp
DLC: 8, Transmitter: DCDC_HV_02
Signals (1):
  OBDC_DCDC_HV_02_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE02CF (2550006479) - OBDC_DCDC_HV_03_Resp
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (1):
  OBDC_DCDC_HV_03_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x9A555545 (2589283653) - DCDC_HV_02_01
DLC: 8, Transmitter: DCDC_HV_02
Signals (13):
  DCDC_HV_02_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  DCDC_HV_02_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszähler, wird mit jeder Sendebotschaft inkrementiert
  HVLB_IstModus_TN1:
    Bits: 13|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  HVLB_IstSpannung_DCLS:
    Bits: 15|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: HV-Lade-Booster IstSpannung DC-Ladesäule
  HVLB_StatusSpgfreiheit_HV:
    Bits: 27|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: HV-Lade-Booster Status HV-Spannungsfreiheit
  HVLB_IstModus:
    Bits: 29|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: IstModus HV-Lade-Booster
  HVLB_IstStrom_HVHS:
    Bits: 32|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + -512.0
    Unit: Unit_Amper
    Range: [-512.0 .. 511.25]
    Comment: HV-Lade-Booster IstStrom Ausgangsseite
  HVLB_IstSpannung_HVHS:
    Bits: 44|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: HV-Lade-Booster IstSpannung Ausgangsseite
  HVLB_DC_Laden_ohneBoost_verfuegb:
    Bits: 56|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status DC-Laden ohne Booster
  HVLB_DC_Laden_mitBoost_verfuegb:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status DC-Laden mit Booster
  HVLB_AC_Laden_verfuegbar:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status Verfügbarkeit AC-Laden , Freigabe Schützansteuerung
  HVLB_Warnzustand:
    Bits: 59|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Warnzustand HV-Lade-Booster
  HVLB_Fehlerstatus:
    Bits: 61|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Fehlerstatus HV-Booster

### 0x9A55554E (2589283662) - DCDC_HV_02_03
DLC: 8, Transmitter: DCDC_HV_02
Signals (2):
  HVLB_PlanAntw_Verlustleistung:
    Bits: 48|12 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 204650.0]
    Comment: Antwort auf die Ladeplan-Anfrage für die Ladeplanung: Verlustleistung
  HVLB_PlanAntw_Zaehler:
    Bits: 60|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Antwort auf die Ladeplan-Anfrage für die Ladeplanung: Zähler

### 0x9B000041 (2600468545) - NMH_DCDC
DLC: 8, Transmitter: DCDC_IHEV
Signals (11):
  NM_DCDC_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  NM_DCDC_CBV_AWB:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist zu setzen, wenn das SG aktiv den Bus geweckt hat
  NM_DCDC_CBV_CRI:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist dauerhaft gesetzt, wenn das NM-Layout dem neuen NM-Layout für Teilnetzbetrieb entspri...
  NM_DCDC_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_DCDC_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NM_DCDC_UDS_CC:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem UDS-Communication Control; siehe VW80124
  NM_DCDC_NM_aktiv_KL15:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuergerät hat KL15-EIN erkannt
  NM_DCDC_NM_aktiv_Diagnose:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diagnose befindet sich nach VW80124 in einer NonDefaultDiagnostic-Session, z.B. ExtendedDiagnostic-S...
  NM_DCDC_NM_aktiv_Tmin:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Mindestaktivzeit ist noch nicht abgelaufen (bei jedem Weckereignis wird die Mindestaktivzeit neu ges...
  NM_DCDC_NM_aktiv_Anf_MVK:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung MVK (Nachlaufursache, SG nicht im NM-Zustand 'Ready to Sleep')
  NM_DCDC_CAB:
    Bits: 40|24 (Intel (LE), unsigned)
    Range: [0.0 .. 16777215.0]
    Comment: CAB = Clusteraktivbits

### 0x9B0000B7 (2600468663) - NMH_DCDC_HV
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (11):
  NM_DCDC_HV_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  NM_DCDC_HV_CBV_AWB:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist zu setzen, wenn das SG aktiv den Bus geweckt hat
  NM_DCDC_HV_CBV_CRI:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist dauerhaft gesetzt, wenn das NM-Layout dem neuen NM-Layout für Teilnetzbetrieb entspri...
  NM_DCDC_HV_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_DCDC_HV_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Car Wakeup (Gateway wertet aus)
  NM_DCDC_HV_UDS_CC:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem UDS-Communication Control; siehe VW80124
  NM_DCDC_HV_NM_aktiv_KL15:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuergerät hat KL15-EIN erkannt
  NM_DCDC_HV_NM_aktiv_Diagnose:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diagnose befindet sich nach VW80124 in einer NonDefaultDiagnostic-Session, z.B. ExtendedDiagnostic-S...
  NM_DCDC_HV_NM_aktiv_Tmin:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Mindestaktivzeit ist noch nicht abgelaufen (bei jedem Weckereignis wird die Mindestaktivzeit neu ges...
  NM_DCDC_HV_NM_aktiv_Anf_HVK:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung HVK (Nachlaufursache, SG nicht im NM-Zustand 'Ready to Sleep')
  NM_DCDC_HV_CAB:
    Bits: 40|24 (Intel (LE), unsigned)
    Range: [0.0 .. 16777215.0]
    Comment: CAB = Clusteraktivbits

### 0x9B0000C7 (2600468679) - NMH_DCDC_HV_02
DLC: 8, Transmitter: DCDC_HV_02
Signals (11):
  NM_DCDC_HV_02_SNI:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Source Node-Identifier (SNI) identifiziert den Sender der NM-Botschaft (Knotenadresse) eindeutig im ...
  NM_DCDC_HV_02_CBV_AWB:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist zu setzen, wenn das SG aktiv den Bus geweckt hat
  NM_DCDC_HV_02_CBV_CRI:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Signal ist dauerhaft gesetzt, wenn das NM-Layout dem neuen NM-Layout für Teilnetzbetrieb entspri...
  NM_DCDC_HV_02_NM_State:
    Bits: 16|6 (Intel (LE), unsigned)
    Range: [0.0 .. 63.0]
  NM_DCDC_HV_02_Car_Wakeup:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Car Wakeup (Gateway wertet aus)
  NM_DCDC_HV_02_UDS_CC:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktionseinschränkung aufgrund aktivem UDS-Communication Control; siehe VW80124
  NM_DCDC_HV_02_NM_aktiv_KL15:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuergerät hat KL15-EIN erkannt
  NM_DCDC_HV_02_NM_aktiv_Diag:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Diagnose befindet sich nach VW80124 in einer NonDefaultDiagnostic-Session, z.B. ExtendedDiagnostic-S...
  NM_DCDC_HV_02_NM_aktiv_Tmin:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Mindestaktivzeit ist noch nicht abgelaufen (bei jedem Weckereignis wird die Mindestaktivzeit neu ges...
  NM_DCDC_HV_02_NM_aktiv_Anf_HVK:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung HVK (Nachlaufursache, SG nicht im NM-Zustand 'Ready to Sleep')
  NM_DCDC_HV_02_CAB:
    Bits: 40|24 (Intel (LE), unsigned)
    Range: [0.0 .. 16777215.0]
    Comment: CAB = Clusteraktivbits

### 0x9BFDBB00 (2617096960) - DEV_DCDC_HV_Resp_00
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (1):
  DEV_DCDC_HV_Resp_00_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFDBBFF (2617097215) - DEV_DCDC_HV_Resp_FF
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (1):
  DEV_DCDC_HV_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFDBF00 (2617097984) - XCP_DCDC_HV_CRO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  XCP_DCDC_HV_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFDBF01 (2617097985) - XCP_DCDC_HV_DTO_01
DLC: 8, Transmitter: DCDC_800V_PAG
Signals (1):
  XCP_DCDC_HV_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE0A00 (2617117184) - DEV_DCDC_Resp_00
DLC: 8, Transmitter: DCDC_IHEV
Signals (1):
  DEV_DCDC_Resp_00_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE0AFF (2617117439) - DEV_DCDC_Resp_FF
DLC: 8, Transmitter: DCDC_IHEV
Signals (1):
  DEV_DCDC_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE0E00 (2617118208) - XCP_DCDC_CRO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  XCP_DCDC_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE0E01 (2617118209) - XCP_DCDC_DTO_01
DLC: 8, Transmitter: DCDC_IHEV
Signals (1):
  XCP_DCDC_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE3B00 (2617129728) - DEV_DCDC_HV_02_Req_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  DEV_DCDC_HV_02_Req_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE3B01 (2617129729) - DEV_DCDC_HV_02_Resp_01
DLC: 8, Transmitter: DCDC_HV_02
Signals (1):
  DEV_DCDC_HV_02_Resp_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE3BFF (2617129983) - DEV_DCDC_HV_02_Resp_FF
DLC: 8, Transmitter: DCDC_HV_02
Signals (1):
  DEV_DCDC_HV_02_Resp_FF_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE3F00 (2617130752) - CCP_DCDC_HV_02_CRO_01
DLC: 8, Transmitter: Vector__XXX
Signals (1):
  CCP_DCDC_HV_02_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x9BFE3F01 (2617130753) - CCP_DCDC_HV_02_DTO_01
DLC: 8, Transmitter: DCDC_HV_02
Signals (1):
  CCP_DCDC_HV_02_DTO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

## Climate/HVAC


### 0x3B5 (949) - Klima_11
DLC: 8, Transmitter: Gateway
Description: ab MQB 7.4
Signals (20):
  KL_Drehz_Anh:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Vorwarn_Komp_ein:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Vorwarnung Kompressor ein; 140ms vorher; Information für Motor zum Aufbau Momentenreserve
  KL_AC_Schalter:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Komp_Moment_alt:
    Bits: 3|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Vorwarn_Zuheizer_ein:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Zustand:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Kompressorkupplung_linear:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 20.0 + 0.0
    Unit: Unit_MilliAmper
    Range: [0.0 .. 4000.0]
    Comment: Sollwert zur Ansteuerung der Kompressorkupplung
  KL_Charisma_FahrPr:
    Bits: 16|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  KL_Charisma_Status:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_nachtr_Stopp_Anf:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal dient dem Motor als Trigger, dass ein nachträglicher Motorstopp eingeleitet werden kan...
  KL_T_Charge:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signal für Tastenbetätigung Charge-Taste (SOC Increasing)
  KL_Last_Kompr:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_NewtoMeter
    Range: [0.0 .. 63.5]
    Comment: Aktuelles Verlustmoment der Klimaanlage bezogen auf die Kurbelwelle. Wird der Klimakompressor nicht ...
  KL_Spannungs_Anf:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Forderung des Klima-SG's zur Spannungsanhebung.
  KL_Thermomanagement:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_StartStopp_Info:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Stopp-Freigabe und Start-Anforderung an den Start-Stopp-Koordinator
  KL_Freilauf_Info:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Freilauf Informationssignal von Klima zum Freilaufkoordinator. Klima kann über dieses Signal den Fre...
  KL_Anf_KL:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 101.6]
  KL_el_Zuheizer_Stufe:
    Bits: 48|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  KL_Ausstattung_Klima:
    Bits: 51|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Austtattungsart der Klimabedienung
  KL_Variante_Standheizung:
    Bits: 54|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Identifikation der Variante der Standheizungen zur Berechnung der energetischen Zielzeit

### 0x50F (1295) - Klima_TME_01
DLC: 8, Transmitter: Gateway
Signals (9):
  KL_Kuehlenergie_instat_pred:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 6325.0]
    Comment: Erwartete thermische Energie für den instationären Kühlbetrieb bis zum Übergang in den stationären B...
  KL_Heizenergie_instat_pred:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 6325.0]
    Comment: Erwartete thermische Energie für den instationären Heizbetrieb bis zum Übergang in den stationären B...
  KL_PTC_ein:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: PTC Aus/Ein
  KL_Leistungspriorisierung:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Priorisierungsvorgaben
  KL_Verdampfertemperatur:
    Bits: 20|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -20.0
    Unit: Unit_DegreCelsi
    Range: [-20.0 .. 70.0]
    Comment: Istwert der Verdampfertemperatur
  KL_Heizbedarf:
    Bits: 30|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_DegreCelsi
    Range: [0.1 .. 90.0]
    Comment: Bedarf an Heizmenge für Innenraumklimatisierung
  KL_Kuehlbedarf:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_DegreCelsi
    Range: [0.0 .. 25.0]
    Comment: Bedarf an Kühlmenge für Innenraumklimatisierung
  KL_Kuehlleistg_stat_erw:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 12650.0]
    Comment: Erwartete thermische Kühlleistungsaufnahme für den stationären Klimabetrieb bei aktuellen klimatisch...
  KL_Heizleistg_stat_erw:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 12650.0]
    Comment: Erwartete thermische Heizleistungsaufnahme für den stationären Klimabetrieb bei aktuellen klimatisch...

### 0x5A1 (1441) - Klima_13
DLC: 8, Transmitter: Gateway
Signals (14):
  KL_Temp_Soll_hl:
    Bits: 7|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Eingestellte Solltemperatur hinten links für verteilte Komfortfunktionen
  KL_Temp_Soll_hr:
    Bits: 12|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Eingestellte Solltemperatur hinten rechts für verteilte Komfortfunktionen
  KL_Temp_Soll_vl:
    Bits: 17|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Eingestellte Solltemperatur vorne links für verteilte Komfortfunktionen
  KL_Temp_Soll_vr:
    Bits: 22|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Eingestellte Solltemperatur vorne rechts für verteilte Komfortfunktionen
  KL_SIH_Stufe_hl:
    Bits: 27|3 (Intel (LE), unsigned)
    Range: [0.0 .. 6.0]
    Comment: Eingestellte Sitzheizungsstufe hinten links für verteilte Komfortfunktionen
  KL_SIH_Stufe_hr:
    Bits: 30|3 (Intel (LE), unsigned)
    Range: [0.0 .. 6.0]
    Comment: Eingestellte Sitzheizungsstufe hinten rechts für verteilte Komfortfunktionen
  KL_SIL_Stufe_hl:
    Bits: 33|3 (Intel (LE), unsigned)
    Range: [0.0 .. 6.0]
    Comment: Eingestellte Sitzlüftungsstufe hinten links für verteilte Komfortfunktionen
  KL_SIL_Stufe_hr:
    Bits: 36|3 (Intel (LE), unsigned)
    Range: [0.0 .. 6.0]
    Comment: Eingestellte Sitzlüftungsstufe hinten rechts für verteilte Komfortfunktionen
  KL_SIL_LEL_Verteilung_HR:
    Bits: 39|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Lüftungsleistung auf Sitzfläche und Lehnenfläche für Sitz hin...
  KL_SIL_LEL_Verteilung_HL:
    Bits: 42|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Lüftungsleistung auf Sitzfläche und Lehnenfläche für Sitz hin...
  KL_SIH_LEH_Verteilung_HR:
    Bits: 45|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Heizleistung auf Sitz und Lehne für Sitz hinten rechts.
  KL_PTC_linear_Anf:
    Bits: 48|6 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 3100.0]
    Comment: Kundenanforderung des PTC (linear)
  KL_SIH_LEH_Verteilung_HL:
    Bits: 54|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Heizleistung auf Sitz und Lehne für Sitz hinten links
  KL_Frischluftklappe_Status:
    Bits: 57|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Aktuelle Position der Frischluftklappe

### 0x5E1 (1505) - Klima_Sensor_02
DLC: 8, Transmitter: Gateway
Description: ab MQB 7.3
Signals (8):
  BCM1_Aussen_Temp_ungef:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -50.0
    Unit: Unit_DegreCelsi
    Range: [-50.0 .. 76.0]
  BCM_Heizungsabsperrventil_Status:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Das Signal beschreibt den Zustand des Heizungsabsperrventils, welches als Portexpander vom BCM1 elek...
  BCM_Heizungspumpe_Status:
    Bits: 10|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Das Signal beschreibt den Zustand der Heizungspumpe, welche als Portexpander vom BCM1 elektrisch ang...
  BCM_Kompressorkupplung_Status:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Das Signal beschreibt den Zustand der Klimakompressorkupplung, welche als Portexpander vom BCM1 elek...
  BCM1_PTC_stufig_Status:
    Bits: 28|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Status des BCM1-Heizverbrauchers PTC (stufig)
  BCM1_FStatus_Aussentemp_ungef:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehlerstatus für Aussentemperatur (Kombianzeige)
  BCM1_Kompressorstrom_ist:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_MilliAmper
    Range: [0.0 .. 1000.0]
    Comment: IST-Strom vom Klima-Kompressor
  BCM1_OBD_FStatus_ATemp:
    Bits: 44|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x5E9 (1513) - Klima_Sensor_04
DLC: 8, Transmitter: Gateway
Signals (6):
  DS_Kaeltemittel_P:
    Bits: 8|11 (Intel (LE), unsigned)
    Formula: raw * 0.0161 + 0.0
    Unit: Unit_Bar
    Range: [0.0 .. 32.9245]
    Comment: Drucksensor Kältemittel Druck, das Signal wird bei Kl.15 AUS mit dem letzten empfangenen Wert gesend...
  DS_Status:
    Bits: 19|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ION_Status:
    Bits: 21|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ION_Status_LED:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status Funktions LED
  AAU_Geblaese:
    Bits: 24|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Ansteuergrad des Gebläses
  ION_Status_Taster:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status Kundenwunsch (Bedientaster) Ionisator

### 0x668 (1640) - Klima_12
DLC: 8, Transmitter: Gateway
Description: ab MQB IS-4
Signals (18):
  KL_LRH_Taster:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Tasterbetätigung Lenkradheizung
  KL_LRH_Stufe:
    Bits: 1|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Eingestellte Temperaturstufe der Lenkradheizung
  HSH_Taster:
    Bits: 3|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  FSH_Taster:
    Bits: 5|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Zuheizer_Freigabe:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Beschlagsgefahr:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_SIH_Soll_li:
    Bits: 8|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Einstellung Sitzheizung Fahrer (0...6)
  KL_SIH_Soll_re:
    Bits: 11|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Einstellung Sitzheizung Beifahrer (0...6)
  KRH_Soll_li:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des Sollwertes für die Kopfraumheizung links.
  KL_SIL_Soll_li:
    Bits: 16|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Einstellung Sitzlüftung Fahrer (0...6)
  KL_SIL_Soll_re:
    Bits: 19|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Einstellung Sitzlüftung Beifahrer (0...6)
  KRH_Soll_re:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des Sollwertes für die Kopfraumheizung rechts.
  KL_Geblspng_Soll:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 1.45
    Unit: Unit_Volt
    Range: [1.5 .. 14.0]
  KL_Geblspng_Fond_Soll:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 1.45
    Unit: Unit_Volt
    Range: [1.5 .. 14.0]
  KL_I_Geblaese:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 63.5]
    Comment: Stromaufnahme Gebläse
  KL_Kompressorstrom_soll:
    Bits: 48|10 (Intel (LE), unsigned)
    Range: [0.0 .. 1021.0]
    Comment: Sollvorgabe Klima-Kompressorstrom
  KL_Umluftklappe_Status:
    Bits: 58|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Status Umluftklappe
  KL_PTC_Verbauinfo:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Im Klima- bzw. Heizungsbedienteil wird der Verbau eines PTCs und dessen elektr. Leistungsklasse codi...

### 0x66E (1646) - Klima_03
DLC: 8, Transmitter: Gateway
Description: ab MQB 7.3
Signals (29):
  KL_STL_aktiv:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_STH_aktiv:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Solarluefter_aktiv:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Umluft_Taste:
    Bits: 3|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Geblaese_Fond_Status:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  KL_STH_Ansteuerung:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  KL_STH_Betriebsdauer:
    Bits: 10|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 60.0]
    Comment: Betriebsdauer STH
  KL_Magnetventil:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung des Bypassventils.
  KL_WaPu:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Ansteuerung der Standheizungswasserpumpe
  KL_Geblaese_Status:
    Bits: 18|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  KL_Restwaerme_aktiv:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Kompressorkupplung:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signal zur Aktivierung/Deaktivierung der Kompressorkupplung
  KL_BCmE_Livetip_Freigabe:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_HYB_ASV_hinten_schliessen_Anf:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung der Klima an das BMS, das Absperrventil des Kältemittelkreislaufs zur Batteriekühlung zu...
  KL_HYB_ASV_vorne_schliessen_Anf:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung der Klima an das BMS, das Absperrventil des Kältemittelkreislaufs der Klimaanlage zu sch...
  KL_ErwVK_Zusatzrelais:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuerung Zusatzrelais für Erweiterte Voko
  KL_Status_Beduftung:
    Bits: 28|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status der Beduftung
  KL_Status_Ionisator_Front:
    Bits: 29|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status des Ionisator Front
  KL_STH_Timer_Status:
    Bits: 30|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_ErwVK_Anf:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Erweiterte Voko angefordertert / aktiv
  KL_Innen_Temp:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -50.0
    Unit: Unit_DegreCelsi
    Range: [-50.0 .. 76.0]
  KL_I_Geblaese_Fond:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 63.5]
    Comment: Stromaufnahme Gebläse Heck
  KL_Anf_AussenspiegelHzg:
    Bits: 48|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung Außenspiegelheizung seitens Klimafunktion (z.B. bei Erweiterter VoKo)
  KL_Heizkreisventilstellung:
    Bits: 49|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Bei MQB PHEV: Stellung Heizkreisventil Motor
  KL_FSH_Automatikbetrieb:
    Bits: 51|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_SIH_LEH_Verteilung_VL:
    Bits: 52|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Heizleistung auf Sitzfläche und Lehnefläche für Sitz vorne li...
  KL_SIH_LEL_Verteilung_VL:
    Bits: 55|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Lüftungsleistung auf Sitzfläche und Lehnenfläche für Sitz vor...
  KL_SIH_LEH_Verteilung_VR:
    Bits: 58|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Heizleistung auf Sitzfläche und Lehnefläche für Sitz vorne re...
  KL_SIH_LEL_Verteilung_VR:
    Bits: 61|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Kunden eingestellte Aufteilung der Lüftungsleistung auf Sitzfläche und Lehnenfläche für Sitz vor...

### 0x671 (1649) - Klima_06
DLC: 8, Transmitter: Gateway
Signals (18):
  KL_ZZ_Minute:
    Bits: 0|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 59.0]
    Comment: Zielzeittimeranzeige im Display der Funkfernbedienung (Minute); Initwert ist der zuletzt gültige Wer...
  KL_ZZ_Status_Timer:
    Bits: 6|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Timerstatusanzeige im Display der Funkfernbedienung (Timer aktiv/n.aktiv)
  KL_ZZ_Monat:
    Bits: 8|4 (Intel (LE), unsigned)
    Unit: Unit_Month
    Range: [1.0 .. 12.0]
    Comment: Zielzeittimeranzeige im Display der Funkfernbedienung (Datum, Monat); Initwert ist der zuletzt gülti...
  KL_ZZ_Stunde:
    Bits: 16|5 (Intel (LE), unsigned)
    Unit: Unit_Hours
    Range: [0.0 .. 23.0]
    Comment: Zielzeittimeranzeige im Display der Funkfernbedienung (Stunde); Initwert ist der zuletzt gültige Wer...
  KL_ZZ_Betriebsmodus:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Betriebsdauerberechnung anhand Aussentemperaturauswertung mit Faktor 'ZZ_Betriebsmodus'
  KL_ZZ_Tag:
    Bits: 24|5 (Intel (LE), unsigned)
    Unit: Unit_Day
    Range: [1.0 .. 31.0]
    Comment: Zielzeittimeranzeige im Display der Funkfernbedienung (Datum, Tag); Initwert ist der zuletzt gültige...
  KL_Betriebsmodus_FH:
    Bits: 29|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Synchronisation des Betriebsmodus zwischen Front und Fond
  KL_Stopp_Wiederstart_Anz_01:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_02:
    Bits: 33|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_03:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_04:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_05:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_06:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_07:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_08:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_09:
    Bits: 40|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_Std1:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...
  KL_Stopp_Wiederstart_Anz_Std2:
    Bits: 42|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für eine Stoppverbot-/Wiederstartanfrage der Klima-SG...

### 0x6B0 (1712) - Klima_Sensor_01
DLC: 6, Transmitter: Gateway
Signals (5):
  FS_Taupunkt:
    Bits: 0|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -39.6
    Unit: Unit_DegreCelsi
    Range: [-39.5 .. 60.4]
  FS_ResponseError:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  FS_Temp_Scheibe:
    Bits: 16|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -39.6
    Unit: Unit_DegreCelsi
    Range: [-39.5 .. 60.4]
  FS_Temp_Sensor:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-39.5 .. 87.0]
  FS_Luftfeuchte_rel:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -0.5
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]

### 0x6B5 (1717) - Klima_Sensor_03
DLC: 8, Transmitter: Gateway
Signals (21):
  FSA_Taupunkt:
    Bits: 0|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 60.0]
    Comment: 'Init' falls nicht verbaut
  FSA_Err_Defekt_Tsens_TF:
    Bits: 10|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestFailed Flag, Statusbotschaft für diden Feuchtesensor, testFailed (TF)
  FSA_Luftfeuchte_TNCTOC:
    Bits: 11|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signal meldet den Status der Diagnosen nach OBD LIN-LAH, testNotCompletedThisOperationCycle (TNCTOC)
  FSA_Luftfeuchte_TF:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestFailed Flag, Statusbotschaft für den Feuchtesensor, testFailed (TF)
  FSA_Err_Unterspannung_TF:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestFailed Flag, Statusbotschaft für den Feuchtesensor, testFailed (TF)
  FSA_Err_RAMROM_Check_TF:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestFailed Flag, Statusbotschaft für den Feuchtesensor, testFailed (TF)
  FSA_ResponseError:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  FSA_Temp_Luft:
    Bits: 16|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 61.0]
  FSA_Err_RAMROM_Check_TNCTOC:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signal meldet den Status der Diagnosen nach OBD LIN-LAH, testNotCompletedThisOperationCycle (TNCTOC)
  FSA_Lokal_Bus_elektr_Fehler:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bit wird gesetzt, wenn fuer lokaler Datenbus elektr. Fehler eingetragen wird
  FSA_Timeout:
    Bits: 28|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bit wird gesetzt, wenn Time-Out DTC eingetragen wird
  FSA_Lokal_Bus_Unterbrechung:
    Bits: 29|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bit wird gesetzt, wenn fuer lokaler Datenbus DTC wegen Unterbrechung eingetragen wird
  FSA_Err_Ueberspannung_TF:
    Bits: 30|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestFailed Flag, Statusbotschaft für den Feuchtesensor Außen, testFailed (TF)
  FSA_Luftfeuchte_rel:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 126.0]
    Comment: Maximaler Feuchtebereich bis 126% für theoretisch errechnete Feuchtewerte des Sensors. Signalwerte >...
  FSA_HW:
    Bits: 40|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Hardwarestand
  FSA_SW:
    Bits: 44|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Softwarestand
  FSA_Temp_Sensor:
    Bits: 48|11 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 90.0]
  FSA_Sensorheizung_Status:
    Bits: 59|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  FSA_Err_Defekt_HumSense_TF:
    Bits: 60|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestFailed Flag, Statusbotschaft für Feuchtesensor außen, testFailed (TF)
  FSA_Err_Defekt_HumSens_TNCTOC:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: OBD TestNotCompletedThisOperationCycle nach LIN LAH für Feuchteelement, testNotCompletedThisOperatio...
  FSA_ErrMemState:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: OBD Signal statusbotschaft für den Feuchtesensor außen

### 0x92DD54CD (2463978701) - Klima_18
DLC: 8, Transmitter: Gateway
Signals (9):
  KL_Temp_Soll_vr_Prognose:
    Bits: 32|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Solltemperatur vorne rechts (Kundenwunsch) für die Prognose bei einem Fahrprogrammwechsel im BEV
  KL_Leistungsreduktions_Hinweis:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signal zum Auslösen der Kombi-Meldung das bei einem Klima Defizit die Leistung der Klimatisierung re...
  KL_AC_Schalter_Prognose:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Erwarteter Zustand des Klimakompressorwunschs bei einem Fahrprogramm-Wechsel
  KL_Temp_Soll_vl_Prognose:
    Bits: 40|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Solltemperatur vorne links (Kundenwunsch) für die Prognose bei einem Fahrprogrammwechsel im BEV
  KL_Sollklimamodus_FPR_1:
    Bits: 45|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Ziel Modus der Klimatisierung für ein definierbares Fahrprogramm.
  KL_Temp_Soll_hr_Prognose:
    Bits: 48|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Solltemperatur hinten rechts (Kundenwunsch) für die Prognose bei einem Fahrprogrammwechsel im BEV
  KL_Sollklimamodus_FPR_2:
    Bits: 53|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Ziel Modus der Klimatisierung für ein definierbares Fahrprogramm.
  KL_Temp_Soll_hl_Prognose:
    Bits: 56|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 15.5
    Unit: Unit_DegreCelsi
    Range: [15.5 .. 30.0]
    Comment: Solltemperatur (Kundenwunsch) für die Prognose bei einem Fahrprogrammwechsel im BEV
  KL_Sollklimamodus_FPR_3:
    Bits: 61|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Ziel Modus der Klimatisierung für ein definierbares Fahrprogramm.

### 0x9A55548D (2589283469) - Klima_14
DLC: 8, Transmitter: Gateway
Signals (11):
  KL_Verdampferleistung:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 6325.0]
  KL_VK_Anf:
    Bits: 8|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Starten einer Standklimatisierung ausgehend von der Climatronic Richtung Lademanagment.
  KL_Verdampferleistung_Faktor:
    Bits: 11|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_Geblaese_Fond_erw:
    Bits: 12|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Erwarteter Fond-Gebläsestatus im eingeschwungenen Zustand
  KL_Geblaese_erw:
    Bits: 16|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Erwarteter Gebläsestatus im eingeschwungenen Zustand
  KL_Vorkond_Strategie:
    Bits: 20|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Strategie der Vorkonditionierung des Innenraums
  KL_Tempklappe_re:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Sollwert der Temperaturklappe rechts
  KL_Tempklappe_li:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  KL_Tempklappe_Fond:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Sollwert der Temperaturklappe Fond
  KL_Kuehllstg_Delta_Eingriff:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + -6300.0
    Unit: Unit_Watt
    Range: [-6300.0 .. 6350.0]
    Comment: Durch Kundeneingriff erwartete Veränderung der  Kühlleistung bei aktuellen klimatischen Bedingungen
  KL_Heizlstg_Delta_Eingriff:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + -6300.0
    Unit: Unit_Watt
    Range: [-6300.0 .. 6350.0]
    Comment: Durch Kundeneingriff erwartete Veränderung der  Heizleistung bei aktuellen klimatischen Bedingungen

### 0x9A555492 (2589283474) - Klima_TME_02
DLC: 8, Transmitter: Gateway
Signals (4):
  KL_Temp_vor_Verdampfer:
    Bits: 32|7 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -20.0
    Unit: Unit_DegreCelsi
    Range: [-20.0 .. 70.0]
    Comment: Ist Lufttemperatur vor Verdampfer
  KL_Eps_Innen:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -50.0
    Unit: Unit_DegreCelsi
    Range: [-50.0 .. 50.0]
    Comment: mittlerer Wert der Klima Regelabweichung
  KL_Tempklappe_Fond_li:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Sollwert der Temperaturklappe Fond links
  KL_Tempklappe_Fond_re:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Sollwert der Temperaturklappe Fond rechts

### 0x9A555525 (2589283621) - Klima_17
DLC: 8, Transmitter: Gateway
Signals (17):
  KL_NV_Energie_Anf:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 2530.0]
    Comment: Benötigte Energie (12V) für die Erweiterte Vorkonditionierung
  KL_FlHz_Soll_VL:
    Bits: 20|3 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 6.0]
    Comment: Sollwertvorgabe für Flächenheizung vorne links
  KL_FlHz_Soll_HL:
    Bits: 23|3 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 6.0]
    Comment: Sollwertvorgabe für Flächenheizung hinten links
  KL_FlHz_Soll_VR:
    Bits: 26|3 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 6.0]
    Comment: Sollwertvorgabe für Flächenheizung vorne rechts
  KL_FlHz_Soll_HR:
    Bits: 29|3 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 6.0]
    Comment: Sollwertvorgabe für Flächenheizung hinten rechts
  KL_Umluft_Connect_status:
    Bits: 32|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Umluft Status zum Backend Server
  KL_ECO_PRO_Aktiv:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktion ECO-PRO durch Kundeneingriff aktiviert
  KL_QBit_Sonnenintensitaet:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KL_ZR_Anforderung:
    Bits: 37|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung an MIB die Displays zu aktivieren
  KL_ACmax_Aktiv:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktion ACmax durch Kundeneingriff aktiviert
  KL_Defrost_Aktiv:
    Bits: 40|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktion Defrost durch Kundeneingriff aktiviert
  KL_ECO_Aktiv:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Funktion Eco durch Kundeneingriff aktiviert
  KL_Status_Klimastil:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Signal für aktuelles Programm des KBT
  KL_Kundeneingriff_Geblaese:
    Bits: 44|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Manueller Kundeneingriff in die Geblaeseregelung
  KL_Thermischer_Bedarf:
    Bits: 45|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Temperierungsbedarf zur Innenraumklimatisierung
  KL_Eps_Innen:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -50.0
    Unit: Unit_DegreCelsi
    Range: [-50.0 .. 50.0]
    Comment: mittlerer Wert der Klima Regelabweichung
  KL_Sonnenintensitaet:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_WattPerMeterSquar
    Range: [0.0 .. 1000.0]
    Comment: gemittelte eingestrahlte Leistung in W/m2

## Vehicle State


### 0x116 (278) - ESP_10
DLC: 8, Transmitter: Gateway
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

### 0x1A2 (418) - ESP_15
DLC: 8, Transmitter: Gateway
Description: MLBevo verlangsamte 100ms-Botschaft
Signals (22):
  ESP_15_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_15_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  HMS_P_beibehalten_GE:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Fordert das Getriebe auf in Stellung 'P' zu bleiben.
  HMS_Konsistenz_GE:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an, ob die Kommunikation und die Anforderungen plausibel sind.
  HMS_Konsistenz_IPA:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  HMS_Konsistenz_STA:
    Bits: 18|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an, ob die Kommunikation und die Anforderungen plausibel sind.
  HMS_Konsistenz_MO:
    Bits: 19|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an, ob die Kommunikation und die Anforderungen plausibel sind.
  HMS_halten_nicht_verfuegbar:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Rueckmeldung HMS-Teilfunktion 'Halten' nicht verfuegbar.
  NL_PRIM_STP_01_Status:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  VMM_Warnblinken:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Über dieses Signal wird ein Warnblinken von Funktionen des VMM (VehicleMotionManagers) angefordert.
  VZM_Status:
    Bits: 26|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Zeigt die Verfuegbarkeit und den Zustand der VZM Funktionen im ESC an
  HMS_Notstopp_Status:
    Bits: 29|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Statusinformation des Notstoppsystems an die beteiligten Systeme.
  HMS_parken_nicht_verfuegbar:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Rueckmeldung HMS-Teilfunktionen 'Parken' nicht verfuegbar.
  HMS_Anforderung_GE:
    Bits: 33|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Fordert vom Getriebe bestimmte Schaltstellungen an.
  HMS_Systemstatus:
    Bits: 36|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Zeigt den aktuellen Systemstatus des HMS an.
  HMS_Stillstand:
    Bits: 40|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Das Signal zeigt, basierend auf den vier ESP-Raddrehzahlinformatioen, gefiltert und/oder entprellt m...
  HMS_aktives_System:
    Bits: 43|5 (Intel (LE), unsigned)
    Range: [0.0 .. 31.0]
    Comment: Zeigt an, von welchem System die Anforderung gerade umgesetzt wird.
  HMS_Konsistenz_HVLM:
    Bits: 48|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an, ob die Kommunikation und die Anforderungen plausibel sind.
  HMS_Konsistenz_ACC:
    Bits: 49|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an, ob die Kommunikation und die Anforderungen plausibel sind.
  HMS_Fehlerstatus:
    Bits: 50|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  ESP_Index_Haltemoment:
    Bits: 53|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an, ob das Summen-Radmoment von ESP_Haltemoment ein Bremsmoment (bergab) oder ein Antriebsmome...
  ESP_Haltemoment:
    Bits: 54|10 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_NewtoMeter
    Range: [0.0 .. 10210.0]
    Comment: Zeigt in Abhängigkeit von ESP_Index_Haltemoment das notwendige Summen-Radmoment an (bergab: Bremsmom...

### 0x31B (795) - ESP_24
DLC: 8, Transmitter: Gateway
Signals (17):
  ESP_24_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_24_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Botschaft asynchron geroutet. Zählerwert kann springen.
  ESP_Lampe:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ABS_Lampe:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BK_Lampe_02:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  TC_Lampe:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_m_Raddrehz:
    Bits: 17|15 (Intel (LE), unsigned)
    Formula: raw * 0.002 + 0.0
    Unit: Unit_Hertz
    Range: [0.0 .. 65.278]
  ESP_Textanzeigen_03:
    Bits: 32|5 (Intel (LE), unsigned)
    Range: [0.0 .. 31.0]
  ESP_Meldungen:
    Bits: 37|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  ESP_Wegimp_VA:
    Bits: 40|11 (Intel (LE), unsigned)
    Range: [0.0 .. 2047.0]
  ESP_Fehlerstatus_Wegimp:
    Bits: 51|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_Wegimp_Ueberlauf:
    Bits: 52|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_QBit_Wegimp_VA:
    Bits: 53|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Gesetzt bei Ersatzwert, Init oder Fehler.
  ESP_HDC_Geschw_Farbe:
    Bits: 54|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Farbdarstellung der angezeigten HDC-Geschwindigkeit
  ESP_Off_Lampe:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESP_HDC_Regelgeschw:
    Bits: 56|7 (Intel (LE), unsigned)
    Formula: raw * 0.32 + 0.0
    Unit: Unit_KiloMeterPerHour
    Range: [0.32 .. 39.68]
  ESP_BKV_Warnung:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x3C0 (960) - Klemmen_Status_01
DLC: 4, Transmitter: Gateway
Description: MQB
Signals (19):
  Klemmen_Status_01_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  Klemmen_Status_01_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: 4bit Botschaftszaehler; wird in jeder Sendebotschaft inkrementiert
  RSt_Fahrerhinweise:
    Bits: 12|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Über das Signal fordert das BCM Fahrerhinweise im Kombi für die Funktion RemoteStart an.
  ZAS_Kl_S:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ZAS_Kl_15:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ZAS_Kl_X:
    Bits: 18|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ZAS_Kl_50_Startanforderung:
    Bits: 19|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BCM_Remotestart_Betrieb:
    Bits: 20|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt den Status des RemoteStarts an, d.h. KL15_RSt ist aktiviert und der Motorlauf ist im RSt-Betri...
  ZAS_Kl_Infotainment:
    Bits: 21|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BCM_Remotestart_KL15_Anf:
    Bits: 22|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt den Status der Aktivierung der KL15_RST durch das BCM an.
  BCM_Remotestart_MO_Start:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Nachdem das BCM die Remotestart MSG Freigabe sicher erkannt hat, fordert das BCM wiederum über diese...
  KST_Warn_P1_ZST_def:
    Bits: 24|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KST_Warn_P2_ZST_def:
    Bits: 25|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  KST_Fahrerhinweis_1:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisierung Fahrerhinweis 1 der Klemmensteuerung an das Kombi
  KST_Fahrerhinweis_2:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisierung Fahrerhinweis 2 der Klemmensteuerung an das Kombi
  BCM_Ausparken_Betrieb:
    Bits: 28|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung zur erneuten Lampenaufprüfung und Anzeige von Warnhinweisen nach einem pilotierten Auspa...
  KST_Fahrerhinweis_4:
    Bits: 29|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisierung Fahrerhinweis 4 der Klemmensteuerung an das Kombi
  KST_Fahrerhinweis_5:
    Bits: 30|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anzeigesignal für RemoteStart
  KST_Fahrerhinweis_6:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signal aktiviert die Kombi-Anzeigeinstrumente (Drehzahl, Tankinhalt, Kühlmitteltemperatur) während a...

### 0x65D (1629) - ESP_20
DLC: 8, Transmitter: Gateway
Description: neu im MQB
Signals (25):
  ESP_20_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_20_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Botschaft asynchron geroutet. Zählerwert kann springen.
  BR_Systemart:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_SpannungsAnf_02:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Forderung des ESP-SG's zur Spannungsanhebung.
  ESP_Zaehnezahl:
    Bits: 16|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_Charisma_FahrPr:
    Bits: 24|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESP_Charisma_Status:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESP_Wiederstart_Anz_01:
    Bits: 30|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für einen Wiederstart vom Bremsensteuergerät an. 
  ESP_Wiederstart_Anz_02:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für einen Wiederstart vom Bremsensteuergerät an. 
  ESP_Wiederstart_Anz_03:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für einen Wiederstart vom Bremsensteuergerät an. 
  ESP_Wiederstart_Anz_04:
    Bits: 33|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für einen Wiederstart vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_01:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_02:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_03:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_04:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_05:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_06:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_07:
    Bits: 40|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Stoppverbot_Anz_Std:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  ESP_Dachrelingsensor:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Erkennung auf Verbau des Dachrelingsensors und Dachquerträger.
  ESP_Stoppverbot_Anz_08:
    Bits: 44|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Dieses Signal gibt die detaillierte Begründung für ein Stoppverbot vom Bremsensteuergerät an.
  HDC_Charisma_FahrPr:
    Bits: 45|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  HDC_Charisma_Status:
    Bits: 49|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BR_QBit_Reifenumfang:
    Bits: 51|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Gesetzt bei Ersatzwert, Init oder Fehler.
  BR_Reifenumfang:
    Bits: 52|12 (Intel (LE), unsigned)
    Unit: Unit_MilliMeter
    Range: [0.0 .. 4095.0]

### 0x92DD54AB (2463978667) - ESP_25
DLC: 8, Transmitter: Gateway
Signals (7):
  ESP_25_CRC:
    Bits: 0|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  ESP_25_BZ:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  ESCGeneratorSollMoment:
    Bits: 16|12 (Intel (LE), unsigned)
    Formula: raw * 3.0 + -12279.0
    Range: [-12279.0 .. 0.0]
  TSMomBegrenzung:
    Bits: 28|12 (Intel (LE), unsigned)
    Formula: raw * 3.0 + -12279.0
    Range: [-12279.0 .. 0.0]
  ESC_Anf_Mindestreku:
    Bits: 48|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei Wert == 1 soll durch den Antrieb eine im Projekt definierte Mindestrekuperationsleistung des E-A...
  ESC_Status_Schaltkennfeld_sek:
    Bits: 50|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Das Signal qualifiziert Wunscheingriffe auf die Getriebeschaltstrategie der sekundären Achse! Signal...
  ESP_M_hyd_anteil_Bremse:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_NewtoMeter
    Range: [0.0 .. 16372.0]

### 0x96A954AE (2527679662) - ESP_49
DLC: 8, Transmitter: Gateway
Signals (7):
  ESC_Anf_Luefternachlauf:
    Bits: 42|6 (Intel (LE), unsigned)
    Formula: raw * 15.0 + 0.0
    Unit: Unit_Secon
    Range: [0.0 .. 945.0]
    Comment: Steuert den Nachlauf des Kühlerlüfters zur Bremsenkühlung nach Klemme 15 aus.
  ESC_Reku_Anzeige:
    Bits: 48|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  ESC_Anf_LadeVorhalt:
    Bits: 49|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Steuert den Ladevorhalt, um Rekuperation bei langer Bergabfahrt sicherzustellen.
  ESC_Textanzeigen_SysSpez:
    Bits: 53|5 (Intel (LE), unsigned)
    Range: [0.0 .. 31.0]
    Comment: Textanzeigen des Bremsregelsystems zur Anzeige von systemspezifischen Textmeldungen
  BRS_BFLS_Verbauzustand:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  BRS_Bremsfluessigkeit_Warnungen:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  ESC_Anf_Bremsenkuehlung:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Signal steuert das Oeffnen des Bremsenkuehlkanals und ggf. das Aktivieren eines unterstützenden Lüft...

### 0x97F60219 (2549482009) - ISO_RTM_OBDC_Resp
DLC: 8, Transmitter: Gateway
Description: Datenübertragung vom OBDC zum Datenanforderer OPR
Signals (1):
  ISO_RTM_OBDC_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Kommunikation zwischen Datenanforderer RTM (RealTimeMonitoring) und OBDC (Onboard Data Collector)

### 0x97F61902 (2549487874) - ISO_OBDC_RTM_Resp
DLC: 8, Transmitter: Sub_Gateway
Description: Datenübertragung vom OBDC zum Datenanforderer OPR
Signals (1):
  ISO_OBDC_RTM_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Kommunikation zwischen OBDC (Onboard Data Collector) und Datenanforderer RTM (RealTimeMonitoring)

### 0x97FE0242 (2550006338) - OBDC_TME_Resp
DLC: 8, Transmitter: TME
Signals (1):
  OBDC_TME_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE027B (2550006395) - OBDC_Hybrid_01_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_Hybrid_01_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE028F (2550006415) - OBDC_AWC_Resp
DLC: 8, Transmitter: AWC
Signals (1):
  OBDC_AWC_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE02AE (2550006446) - OBDC_BattRegelung_Resp
DLC: 8, Transmitter: BMS_NV
Signals (1):
  OBDC_BattRegelung_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FE02B4 (2550006452) - OBDC_TelemetrieSG_Resp
DLC: 8, Transmitter: Sub_Gateway
Signals (1):
  OBDC_TelemetrieSG_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE02B5 (2550006453) - OBDC_FCU_Resp
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (1):
  OBDC_FCU_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FE16C0 (2550011584) - DIA_HV_H2_Resp
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (1):
  DIA_HV_H2_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnosezugriff & Flashen Hochvolt Wasserheizer

### 0x97FE70D0 (2550034640) - OBDC_SSN_BJB_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_BJB_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten BJB

### 0x97FE70D1 (2550034641) - OBDC_SSN_CMC_01_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_01_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D2 (2550034642) - OBDC_SSN_CMC_02_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_02_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D3 (2550034643) - OBDC_SSN_CMC_03_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_03_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D4 (2550034644) - OBDC_SSN_CMC_04_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_04_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D5 (2550034645) - OBDC_SSN_CMC_05_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_05_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D6 (2550034646) - OBDC_SSN_CMC_06_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_06_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D7 (2550034647) - OBDC_SSN_CMC_07_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_07_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D8 (2550034648) - OBDC_SSN_CMC_08_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_08_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70D9 (2550034649) - OBDC_SSN_CMC_09_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_09_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70DA (2550034650) - OBDC_SSN_CMC_10_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_10_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70DB (2550034651) - OBDC_SSN_CMC_11_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_11_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70DC (2550034652) - OBDC_SSN_CMC_12_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_12_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70F1 (2550034673) - OBDC_SSN_CMC_13_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_13_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70F2 (2550034674) - OBDC_SSN_CMC_14_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_14_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

### 0x97FE70F3 (2550034675) - OBDC_SSN_CMC_15_Resp
DLC: 8, Transmitter: BMC_MLBevo
Signals (1):
  OBDC_SSN_CMC_15_Resp_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Response FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 1...

## Electric Motor


### 0x0A8 (168) - Motor_12
DLC: 8, Transmitter: Gateway
Description: Teil 2 neue Momentenschnittstelle MLB
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

### 0x11B (283) - Motor_35
DLC: 8, Transmitter: Gateway
Signals (2):
  MO_Status_Katheizen:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Status_MIN_Wunschdrehzahl:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: In 2.0 TFSI Motorprojekten EA888 Gen3 und EA888 EVO4 im MLBevo (AU536 und AU49x, in allen anderen Pr...

### 0x121 (289) - Motor_20
DLC: 8, Transmitter: Gateway
Signals (2):
  MO_Moment_im_Leerlauf:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_temporaere_Fahrerabwesenheit:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x131 (305) - Motor_Hybrid_03
DLC: 8, Transmitter: Gateway
Signals (2):
  MO_Zust_Antrieb:
    Bits: 16|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Signalisiert die Zuordnung der Antriebsverbräuche zu den Antriebsmaschinen für die Restreichweitenbe...
  Ladezustand_02:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]

### 0x154 (340) - Motor_28
DLC: 8, Transmitter: Gateway
Signals (3):
  MO_IstMoment_VKM:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -509.0
    Range: [-509.0 .. 509.0]
  MO_Fehlerstatus_Antrieb_FDR:
    Bits: 42|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Fehlerstatus Antrieb - FDR
  MO_Drehzahl_VM:
    Bits: 48|16 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 16383.0]

### 0x179 (377) - Motor_42
DLC: 8, Transmitter: Gateway
Signals (1):
  MO_Soll_NutzbarerSOC:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Der Sollwert für den Kunden-Nutzbaren-SOC bei einer SOC-steuernden Betriebsstrategie

### 0x18A (394) - Motor_Hybrid_11
DLC: 8, Transmitter: Gateway
Signals (1):
  MO_HYB_LowSpeedModus:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x32C (812) - Motor_17
DLC: 8, Transmitter: Gateway
Signals (6):
  MO_Prio_MAX_Wunschdrehzahl:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Prioritätsbit der Motorwunschdrehzahl  als MAX
  MO_Prio_MIN_Wunschdrehzahl:
    Bits: 13|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Prioritätsbit der Motorwunschdrehzahl als MIN
  MO_Luftpfad_aktiv:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Getriebeeingriff wird zusätzlich zum Zündungspfad auch über den Luftpfad umgesetzt, kommt zu MLB neu...
  MO_Drehzahlbeeinflussung:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 0.39 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 99.45]
    Comment: Faktor zur Anhebung der Fahrdrehzahl im Warmlauf
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
DLC: 8, Transmitter: Gateway
Signals (2):
  MO_Klima_Eingr:
    Bits: 33|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Eingriffe des Motor bei der Klima
  MO_BMS_NV_Anf_stuetzen:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Garantierte Unterstützung des Zusatzspeichers ist erforderlich. Zusatzspeicher muss an das Versorgun...

### 0x556 (1366) - Motor_31
DLC: 8, Transmitter: Gateway
Signals (5):
  MO_HV_Auszeit_Status:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status HV-Auszeit.
  MO_HV_Auszeit:
    Bits: 24|9 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_Minut
    Range: [0.0 .. 2036.0]
    Comment: Zeit seit letzter Hochvoltaktivität.
  MO_Verbau_StartSys_EM_HV:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Verbau_StartSys_RSG_NV:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_Ziel_SoC_Fremdladung:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.4 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Auf diesen SoC soll MHEV-Speicher geladen werden, wenn ein Ladegerät erkannt ist.

### 0x58F (1423) - Motor_24
DLC: 8, Transmitter: Gateway
Description: CAN-Botschaft
Signals (3):
  MO_Kuehlmittel_Temp_Rohwert:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
  MO_Oel_Temp_Radsatz:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -60.0
    Unit: Unit_DegreCelsi
    Range: [-60.0 .. 193.0]
    Comment: Öltemperatur in PDK Radsatz 
  MO_FSA_Sensorheizung:
    Bits: 44|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Aktivierung Sensorheizung FSA

### 0x640 (1600) - Motor_07
DLC: 8, Transmitter: Gateway
Description: ab MQB mit QBit Höheneinfo und MO_Versionsinfo_02
Signals (12):
  MO_QBit_Ansaugluft_Temp:
    Bits: 0|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_QBit_Oel_Temp:
    Bits: 1|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_QBit_Kuehlmittel_Temp:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  MO_KL15_Abschaltempfehlung:
    Bits: 3|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Kessy Abschaltempfehlung nach Verlassen des Fahrzeugs
  MO_HYB_Fehler_HV_Netz:
    Bits: 4|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Im HV-Bordnetz ist fehlerbedingt kein generatorischer Betrieb möglich. Das 12V-Bordnetz wird über de...
  MO_aktives_Getriebeheizen:
    Bits: 5|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Information für Getriebe, ob ein Ventil für Getriebheizen angesteuert werden muss
  MO_Ansaugluft_Temp:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: In Fahrzeugen ohne Außentemperatur vom Kombi kann aus der Ansauglufttemperatur auf die Außentemperat...
  MO_Oel_Temp:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -60.0
    Unit: Unit_DegreCelsi
    Range: [-60.0 .. 192.0]
    Comment: Realer Meßbereich ca. -40..150 Grad
  MO_Kuehlmittel_Temp:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Kuehlmitteltemperatur
  MO_Heizungspumpenansteuerung:
    Bits: 53|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  MO_SpannungsAnf:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Forderung des Motor-SGs zur Spannungsanhebung
  MO_Nachlaufzeit_Heizungspumpe:
    Bits: 58|6 (Intel (LE), unsigned)
    Formula: raw * 15.0 + 0.0
    Unit: Unit_Secon
    Range: [0.0 .. 945.0]
    Comment: Nachlaufzeit für Heizungspumpe für Turboladerschutz

### 0x641 (1601) - Motor_Code_01
DLC: 8, Transmitter: Gateway
Description: ab MQB mit CRC-8
Signals (1):
  MO_Faktor_Momente_02:
    Bits: 12|2 (Intel (LE), unsigned)
    Unit: Unit_NewtoMeter
    Range: [1.0 .. 3.0]

### 0x647 (1607) - Motor_09
DLC: 8, Transmitter: Gateway
Signals (2):
  MO_ITM_Kuehlmittel_Temp:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-45.75 .. 143.25]
  SCR_Reichweite:
    Bits: 16|15 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 32766.0]
    Comment: Gesamtreichweite in km der aktuellen SCR-Tankfüllung, sofern die Reichweite kleiner 2400 km ist. Bei...

### 0x670 (1648) - Motor_18
DLC: 8, Transmitter: Gateway
Description: Kombischnittstelle
Signals (3):
  MO_max_Ladedruck:
    Bits: 0|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Bar
    Range: [0.0 .. 6.3]
    Comment: maximal möglicher Ladedruck (zur Anpassung des Anzeigebereichs bei absoluter Ladedruckanzeige)
  MO_Drehzahl_Warnung:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Warnhinweis zum Kombi im Fall einer fehlerbehafteten Drehzahlbegrenzung
  MO_obere_Drehzahlgrenze:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [50.0 .. 12750.0]
    Comment: obere Drehzahlgrenze des Motors

### 0x92DD549E (2463978654) - Motor_FuelCell_02
DLC: 8, Transmitter: Gateway
Signals (1):
  MO_MaxStrom_Sekundaerkreis:
    Bits: 53|11 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 2046.0]
    Comment: Maximal zulässiger Strom des Brennstoffzellensystems Richtung Traktionsnetz

### 0x96A9545D (2527679581) - Motor_39
DLC: 8, Transmitter: Gateway
Signals (2):
  MO_Hysterese_Reakt_Charge:
    Bits: 26|9 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 2036.0]
    Comment: Signalisiert die Hysterese auf die MO_Abwurfschwelle_Charge ab welcher der Charge-Mode wieder reakti...
  MO_Abwurfschwelle_Charge:
    Bits: 44|10 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 4084.0]
    Comment: Signalisiert den Offset auf das Signal BMS_SOC_Max bei dem der Charge-Mode von der BS abgeworfen wir...

### 0x9A55550F (2589283599) - Motor_62
DLC: 8, Transmitter: Gateway
Signals (1):
  MO_Kilometerstand_Wakeup:
    Bits: 12|20 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 1048573.0]
    Comment: Odometer value sent at wake-up

## Other


### 0x040 (64) - Airbag_01
DLC: 8, Transmitter: Gateway
Signals (3):
  AB_Versorgungsspannung:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  AB_Deaktivierung_HV:
    Bits: 42|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Deaktivierung des HV-Systems und der HV-Teilnehmer im Crash (u.a. Hybrid- und Elektrofahrzeuge)
  SC_LowSpeedCrashErkannt:
    Bits: 55|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Crash erkannt bei niedriger Geschwindigkeit

### 0x073 (115) - EM1_HYB_12
DLC: 8, Transmitter: Gateway
Signals (4):
  EM1_IstMoment:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1023.5]
    Comment: Momentanwert: E-Maschinen Moment
  EM1_IstDrehzahl_alt:
    Bits: 24|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl vor 2,5ms zur Aussetzerdiagnose
  EM1_IstDrehzahl:
    Bits: 39|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl
  EM1_MaxZul_Drehzahl:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 80.0 + 0.0
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 20240.0]
    Comment: EM1_MaxZul_Drehzahl stellt die maximale Drehzahlgrenze für LE bzw. EM dar die im Freilauf zulässig i...

### 0x074 (116) - EM2_HYB_12
DLC: 8, Transmitter: Gateway
Signals (5):
  EM2_IstMoment:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1023.5]
    Comment: Momentanwert: E-Maschinen Moment
  EM2_IstDrehzahl_alt:
    Bits: 24|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl vor 2,5ms zur Aussetzerdiagnose
  EM2_IstDrehzahl:
    Bits: 39|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl
  EM2_offene_Klemme_Verfuegbar:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Information EM2: Open switches (freewheeling) available / Information EM2: Offene Klemme (Freilauf) ...
  EM2_MaxZul_Drehzahl:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 80.0 + 0.0
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 20240.0]
    Comment: EM2_MaxZul_Drehzahl stellt die maximale Drehzahlgrenze für LE bzw. EM dar die im Freilauf zulässig i...

### 0x075 (117) - EM3_HYB_12
DLC: 8, Transmitter: Gateway
Signals (5):
  EM3_IstMoment:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1023.5]
    Comment: Momentanwert: E-Maschinen Moment
  EM3_IstDrehzahl_alt:
    Bits: 24|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl vor 2,5ms zur Aussetzerdiagnose
  EM3_IstDrehzahl:
    Bits: 39|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl
  EM3_offene_Klemme_Verfuegbar:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Information EM3: Open switches (freewheeling) available / Information EM3: Offene Klemme (Freilauf) ...
  EM3_MaxZul_Drehzahl:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 80.0 + 0.0
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 20240.0]
    Comment: EM2_MaxZul_Drehzahl stellt die maximale Drehzahlgrenze für LE bzw. EM dar die im Freilauf zulässig i...

### 0x087 (135) - EM1_HYB_06
DLC: 8, Transmitter: Gateway
Signals (6):
  EM1_Fehler_ElAntriebAbschaltung:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EM1_Fehler_ElAntriebFreilauf:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler: EM im Freilauf; Leistungselektronik fordert Begrenzung der maximalen Drehzahl an, um Überspa...
  EM1_Max_Moment:
    Bits: 20|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM1_Min_Moment:
    Bits: 31|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM1_MaxDyn_Moment:
    Bits: 42|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM1_MinDyn_Moment:
    Bits: 53|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]

### 0x0A0 (160) - LH_EPS_04
DLC: 8, Transmitter: Gateway
Signals (3):
  EPS_Lenkmoment_AbsReduz:
    Bits: 12|4 (Intel (LE), unsigned)
    Formula: raw * 0.62 + 0.61
    Unit: Unit_NewtoMeter
    Range: [0.61 .. 8.05]
    Comment: Gibt den Absolutwert des aktuelle Handlenkmoment mit einer Auflösung von 0 ,62 Nm aus
  EPS_Motormoment:
    Bits: 23|8 (Intel (LE), unsigned)
    Formula: raw * 0.2 + -16.2
    Unit: Unit_NewtoMeter
    Range: [-16.0 .. 16.0]
    Comment: Motornahes Motor-Sollmoment
  EPS_Versorgungsspannung:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 5.05
    Unit: Unit_Volt
    Range: [5.05 .. 17.7]

### 0x0B1 (177) - Getriebe_17
DLC: 8, Transmitter: Gateway
Signals (1):
  GE_Strombedarf_erh:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]

### 0x0B3 (179) - Bremse_EV_01
DLC: 8, Transmitter: Gateway
Description: Schnittstelle EBKV - Motor
Signals (1):
  EBKV_Generatorsollmoment:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 1.5 + 0.0
    Unit: Unit_NewtoMeter
    Range: [0.0 .. 6120.0]
    Comment: Sollwert für Modifikation des verfügbaren Generatormoments

### 0x0B8 (184) - EM1_HYB_13
DLC: 8, Transmitter: Gateway
Signals (5):
  EM1_IstStrom:
    Bits: 24|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_Amper
    Range: [-1023.0 .. 1022.0]
    Comment: Momentanwert: Strom über PWR
  EM1_Fehler_ElAntriebFreilauf_Anf:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Wird gesetzt um den zwingenden Bedarf einer Umschaltung in den Freilauf anzuzeigen. Vergleiche Signa...
  EM1_Abregelung_Temperatur:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Leistungsbegrenzung aufgrund Übertemperatur der E-Maschine / des PWR
  EM1_Moment_HVVerbraucher:
    Bits: 44|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_NewtoMeter
    Range: [-511.0 .. 511.0]
    Comment: rechnerisch benötigtes Generator-Moment, um den Bedarf der el. Nebenverbraucher incl. Batterieladung...
  EM1_HV_betriebsbereit:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: 1: Ready_HV / 1: HV-Netz Spannung im Betriebsbereich

### 0x0C7 (199) - EM2_HYB_13
DLC: 8, Transmitter: Gateway
Signals (4):
  EM2_IstStrom:
    Bits: 24|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_Amper
    Range: [-1023.0 .. 1022.0]
    Comment: Momentanwert: Strom über PWR
  EM2_Fehler_ElAntriebFreilauf_Anf:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Wird gesetzt um den zwingenden Bedarf einer Umschaltung in den Freilauf anzuzeigen. Vergleiche Signa...
  EM2_Abregelung_Temperatur:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Leistungsbegrenzung aufgrund Übertemperatur der E-Maschine / des PWR
  EM2_HV_betriebsbereit:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: 1: Ready_HV / 1: HV-Netz Spannung im Betriebsbereich

### 0x0C8 (200) - EM3_HYB_13
DLC: 8, Transmitter: Gateway
Signals (4):
  EM3_IstStrom:
    Bits: 24|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_Amper
    Range: [-1023.0 .. 1022.0]
    Comment: Momentanwert: Strom über PWR
  EM3_Fehler_ElAntriebFreilauf_Anf:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Wird gesetzt um den zwingenden Bedarf einer Umschaltung in den Freilauf anzuzeigen. Vergleiche Signa...
  EM3_Abregelung_Temperatur:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Leistungsbegrenzung aufgrund Übertemperatur der E-Maschine / des PWR
  EM3_HV_betriebsbereit:
    Bits: 58|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: 1: Ready_HV / 1: HV-Netz Spannung im Betriebsbereich

### 0x111 (273) - EM2_HYB_06
DLC: 8, Transmitter: Gateway
Signals (6):
  EM2_Fehler_ElAntriebAbschaltung:
    Bits: 16|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  EM2_Fehler_ElAntriebFreilauf:
    Bits: 17|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler: EM im Freilauf; Leistungselektronik fordert Begrenzung der maximalen Drehzahl an, um Überspa...
  EM2_Max_Moment:
    Bits: 20|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM2_Min_Moment:
    Bits: 31|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM2_MaxDyn_Moment:
    Bits: 42|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM2_MinDyn_Moment:
    Bits: 53|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]

### 0x153 (339) - MSG_HYB_30
DLC: 8, Transmitter: Gateway
Description: CAN-Botschaft
Signals (9):
  MO_HVEM_Eskalation:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Eskalationsbit, mit diesem zusammen darf MO_HVEM_MaxLeistung < der minimal angeforderten Leistung se...
  MO_Fehler_Notentladung_Anf:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Aufforderung zur schnellstmöglichen Entladeung des HV-Zwischenkreises aufgrund Crash-Erkennung / Abr...
  MO_HVEM_MaxLeistung:
    Bits: 15|9 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 25450.0]
  MO_HVK_EmIstzustand:
    Bits: 24|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
  MO_HVK_AntriebFehlerstatus:
    Bits: 37|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  MO_HVK_AntriebZustand:
    Bits: 41|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  MO_HVK_EmFehlerstatus:
    Bits: 44|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  MO_MVK_AntriebFehlerstatus:
    Bits: 47|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  MO_MVK_AntriebZustand:
    Bits: 50|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]

### 0x17B (379) - FCU_02
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (3):
  FCU_HV_Anf:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung 'HV-Netz aktivieren'
  FCU_Klima_Eingr:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: beschränkt Kompressorleistung / Kompressorabwurf
  FCU_Fehler_Entladung_defekt:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Aktueller Fehlerstatus der Entladung

### 0x1F1 (497) - Kessy_11
DLC: 8, Transmitter: Gateway
Signals (1):
  KY_IDG_in_Reichweite:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Für elektrisch betätigte Türen und Klappen muss für eine Betätigung über Funk eine Reichweitenbegren...

### 0x20F (527) - NVEM_10
DLC: 8, Transmitter: Gateway
Signals (1):
  NVEM_DC_uSoll_NV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.025 + 10.6
    Unit: Unit_Volt
    Range: [10.6 .. 16.0]
    Comment: Sollspannung 12V Seite des DC Wandlers, feine Auflösung

### 0x25E (606) - FCU_Verbrauch_01
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (1):
  FCU_Energieaequivalent:
    Bits: 4|12 (Intel (LE), unsigned)
    Formula: raw * 30.0 + 0.0
    Unit: Unit_WattSeconPerGram
    Range: [0.0 .. 122820.0]
    Comment: BZ-System-Wirkungsgrad korrigierter Heizwert

### 0x2B1 (689) - MSG_TME_02
DLC: 8, Transmitter: TME
Signals (2):
  TME_Energie_Thermo:
    Bits: 36|14 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 63250.0]
    Comment: Energieverbrauch für die Temperierung der Bauteile (beim Laden)
  TME_Energie_Komfort:
    Bits: 50|14 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 63250.0]
    Comment: Energieverbrauch für die Vorkonditionierung des Innenraums (beim Laden)

### 0x30B (779) - Kombi_01
DLC: 8, Transmitter: Gateway
Description: MQB
Signals (2):
  KBI_NV_in_Anzeige:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Das Bit gibt dem NV die Information, ob NV-Bild im Kombi in der Anzeige ist. (Information ist notwen...
  KBI_Anzeigefehler_NV:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Über dieses Signal teilt das Kombiinstrument dem Nightvision-Steuergerät mit, dass ein Fehler vom Ko...

### 0x365 (869) - NVEM_05
DLC: 8, Transmitter: Gateway
Description: ab MQB
Signals (2):
  BEM_HYB_DC_uSollLV:
    Bits: 50|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 10.6
    Unit: Unit_Volt
    Range: [10.6 .. 16.0]
    Comment: Sollwert für die Bordnetzspannung im Tiefsetzbetrieb vom Traktionsnetz ins Bordnetz (Hybrid)
  BEM_HYB_DC_uMinLV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 25.3]
    Comment: Abregelgrenze: minimal erlaubte Bordnetzspannung im Hochsetzbetrieb (Hybrid)

### 0x37C (892) - EM1_HYB_11
DLC: 8, Transmitter: Gateway
Signals (1):
  EM1_EM_SollVolumenstrom:
    Bits: 58|6 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 30.5]

### 0x38E (910) - EM1_HYB_05
DLC: 8, Transmitter: Gateway
Description: CAN-Botschaft
Signals (4):
  EM1_Temperatur_ElMotor:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur der E-Maschine
  EM1_Temperatur_PWR:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des PWR
  EM1_Temperatur_Rotor:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des Rotors
  EM1_Temperatur_KW:
    Bits: 50|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Aktueller Ist-Wert Kuehlwassertemperatur Leistungselektronik / E-Machine 1

### 0x391 (913) - OBD_01
DLC: 8, Transmitter: Gateway
Signals (2):
  OBD_Eng_Cool_Temp:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 215.0]
  MO_QBit_Aussen_Temp:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: QBit der Aussentemperatur. Gesetzt bei Ersatzwert, Init oder Fehler.

### 0x394 (916) - WBA_03
DLC: 8, Transmitter: Gateway
Description: ersetzt WBA_02
Signals (1):
  GE_Tipschaltempf_verfuegbar:
    Bits: 44|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisierung, ob eine Hochschaltempfehlung im Tipmodus verfügbar ist oder nicht.

### 0x3A3 (931) - MVEM_01
DLC: 8, Transmitter: Gateway
Signals (5):
  MVEM_IstStrom_MVVerbraucher:
    Bits: 16|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -511.0
    Unit: Unit_Amper
    Range: [-511.0 .. 510.0]
  MVEM_DC_SollSpannung_MV:
    Bits: 26|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: Soll-Ausgangspannung des DC/DC-Wandlers MV-Spannungssseitig (48V-seitig).
  MVEM_DC_SollLeistung_MV:
    Bits: 38|8 (Intel (LE), unsigned)
    Formula: raw * 25.0 + -6325.0
    Unit: Unit_Watt
    Range: [-6325.0 .. 0.0]
  MVEM_EMG_MaxGenStrom:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 506.0]
    Comment: Maximaler Strom des RSGs im spannungsgeregeltem Betrieb. 
  MVEM_EMG_MaxGenSpannung:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: max. Spannung des 48V-RSG im generatorischen Betrieb.

### 0x3CE (974) - TSG_HFS_01
DLC: 8, Transmitter: Gateway
Signals (1):
  HFS_FH_Fang:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fangbereich ist 4mm von oberer Lippendichtung

### 0x3CF (975) - TSG_HBFS_01
DLC: 8, Transmitter: Gateway
Signals (1):
  HBFS_FH_Fang:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fangbereich ist 4mm von oberer Lippendichtung

### 0x3D0 (976) - TSG_FT_01
DLC: 8, Transmitter: Gateway
Description: ab MQB 7.05
Signals (4):
  SSR_Temp_Freigabe:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Freigabe des Seitenscheibenrollos. Bit wird  für die Deaktivierung des SSR bei Innenraumtemperaturen...
  FT_Sp_Heizung_Anf:
    Bits: 15|1 (Intel (LE), unsigned)
    Formula: raw * 100.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Kundenanforderung der Außenspiegelheizung Fahrertür
  FT_FH_Fang:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fangbereich ist 4mm von oberer Lippendichtung
  FT_SP_Heizung_Status:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status der Spiegelheizung

### 0x3D1 (977) - TSG_BT_01
DLC: 8, Transmitter: Gateway
Description: ab MQB 7.05
Signals (4):
  BT_Sp_Heizung_Anf:
    Bits: 15|1 (Intel (LE), unsigned)
    Formula: raw * 100.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Kundenanforderung der Außenspiegelheizung Beifahrertür
  BT_FH_Fang:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fangbereich ist 4mm von oberer Lippendichtung
  BT_SP_Heizung_aktiv:
    Bits: 45|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  BT_SP_Heizung_Status:
    Bits: 55|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status der Spiegelheizung

### 0x3F7 (1015) - TME_07
DLC: 8, Transmitter: TME
Signals (6):
  KL_HV_Energie_Anf:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 12600.0]
  TME_HVEM_HV_Anf:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung 'HV-Netz aktivieren' wegen Innenraumkonditionierung mit HV-Komponente
  TME_HVPTC_Istmodus:
    Bits: 33|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktuelle Ist-Betriebsart der HV-PTC
  TME_HVPTC_Fehlerstatus:
    Bits: 36|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: aktueller Fehlerstatus des HVPTC
  TME_HVK_EKK_SOLL:
    Bits: 45|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung an HVK den HV EKK abzuschalten.
  TME_Strom_Kuehlerluefter:
    Bits: 51|5 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 145.0]
    Comment: aktueller Kühlerlüfterstrom, +-20% 

### 0x3F9 (1017) - TME_08
DLC: 8, Transmitter: TME
Signals (3):
  TME_EM1_IstVolumenstrom:
    Bits: 34|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 14.5]
    Comment: Aktueller Ist-Wert des Vorlaufvolumenstromes LE/EM 1
  TME_EM2_IstVolumenstrom:
    Bits: 39|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 14.5]
    Comment: Aktueller Ist-Wert des Vorlaufvolumenstromes LE/EM 2
  TME_EM3_IstVolumenstrom:
    Bits: 44|5 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 14.5]
    Comment: Aktueller Ist-Wert des Vorlaufvolumenstromes LE/EM 3

### 0x3FE (1022) - Charisma_08
DLC: 8, Transmitter: Gateway
Signals (1):
  CHA_Ziel_FahrPr_BMS:
    Bits: 36|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Anforderung Ziel-Fahrprogramm BMS (nähere Spezifikation siehe Charisma-LH PAG)

### 0x450 (1104) - EM3_HYB_14
DLC: 8, Transmitter: Gateway
Signals (5):
  EM3_MaxPred_Moment:
    Bits: 12|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM3_MinPred_Moment:
    Bits: 23|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM3_MaxPred_Abknickdrehzahl:
    Bits: 34|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12700.0]
    Comment: rechnerischer Wert: Abknickdrehzahl für maximales motorisches Moment nach Definition Funktionslasten...
  EM3_MinPred_Abknickdrehzahl:
    Bits: 42|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12700.0]
    Comment: rechnerischer Wert: Abknickdrehzahl für minimales generatorisches Moment nach Definition Funktionsla...
  EM3_IstSpannung:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: Momentanwert: HV-Spannung am PWR

### 0x451 (1105) - EM2_HYB_14
DLC: 8, Transmitter: Gateway
Signals (5):
  EM2_MaxPred_Moment:
    Bits: 12|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM2_MinPred_Moment:
    Bits: 23|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM2_MaxPred_Abknickdrehzahl:
    Bits: 34|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12700.0]
    Comment: rechnerischer Wert: Abknickdrehzahl für maximales motorisches Moment nach Definition Funktionslasten...
  EM2_MinPred_Abknickdrehzahl:
    Bits: 42|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12700.0]
    Comment: rechnerischer Wert: Abknickdrehzahl für minimales generatorisches Moment nach Definition Funktionsla...
  EM2_IstSpannung:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: Momentanwert: HV-Spannung am PWR

### 0x452 (1106) - EM1_HYB_14
DLC: 8, Transmitter: Gateway
Signals (5):
  EM1_MaxPred_Moment:
    Bits: 12|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM1_MinPred_Moment:
    Bits: 23|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1022.0]
  EM1_MaxPred_Abknickdrehzahl:
    Bits: 34|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12700.0]
    Comment: rechnerischer Wert: Abknickdrehzahl für maximales motorisches Moment nach Definition Funktionslasten...
  EM1_MinPred_Abknickdrehzahl:
    Bits: 42|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MinutInver
    Range: [0.0 .. 12700.0]
    Comment: rechnerischer Wert: Abknickdrehzahl für minimales generatorisches Moment nach Definition Funktionsla...
  EM1_IstSpannung:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: Momentanwert: HV-Spannung am PWR

### 0x474 (1140) - Zentralrechner_01
DLC: 8, Transmitter: Gateway
Signals (2):
  ZR_SpannungSaeule:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Ladespannung an der Ladesäule am nächsten geplanten Ladestopp
  ZR_t_naechsteLadung:
    Bits: 56|8 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 255.0]
    Comment: Dauer bis zur nächsten Schnellladung in Minuten

### 0x488 (1160) - HVLM_06
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (6):
  HVLM_MaxLadeLeistung:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 250.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 150000.0]
    Comment: maximale DC Ladeleistung
  HVLM_MaxSpannung_DCLS:
    Bits: 22|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 600.0]
    Comment: maximale DC Ladespannung
  HVLM_IstStrom_DCLS:
    Bits: 32|9 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 400.0]
    Comment: aktueller DC Ladestrom
  HVLM_MaxStrom_DCLS:
    Bits: 41|9 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 400.0]
    Comment: maximaler DC Ladestrom
  HVLM_MinSpannung_DCLS:
    Bits: 50|9 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 400.0]
    Comment: minimale DC Ladespannung
  HVLM_MinStrom_DCLS:
    Bits: 59|5 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 29.0]
    Comment: minimaler DC Ladestrom

### 0x497 (1175) - Parkhilfe_01
DLC: 8, Transmitter: Gateway
Description: neue Struktur ab MQB ohne KD-Fehler
Signals (2):
  PDC_Tonausgabe_Front:
    Bits: 4|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  PDC_Tonausgabe_Heck:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]

### 0x503 (1283) - HVK_01
DLC: 8, Transmitter: Gateway
Signals (9):
  HVK_BMS_Sollmodus:
    Bits: 24|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  HVK_DCDC_Sollmodus:
    Bits: 27|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  HVK_HVPTC_Sollmodus:
    Bits: 33|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  HVK_HVLM_Sollmodus:
    Bits: 36|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Angeforderter Sollmodus des Lademanagers.
  HVK_HV_Netz_Warnungen:
    Bits: 39|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  HV_Bordnetz_aktiv:
    Bits: 41|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisiert ein aktives Hochvoltbordnetz. Für die Belastbarkeit des HV-Netzes müssen Strom- bzw. Le...
  HV_Bordnetz_Fehler:
    Bits: 42|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status ob im  HV-Netz ein Fehler detektiert wurde.
  HVK_AktiveEntladung_Anf:
    Bits: 45|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung zum aktiven Entladen des HV-Netzes, während der Abschaltung des HV-Netz.
  HVK_DCDC_EKK_Sollmodus:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Bedarfsgerechtes Aktivieren des 400V-DCDC-Wandlers bei Anforderung durch TME

### 0x504 (1284) - MVK_01
DLC: 8, Transmitter: Gateway
Signals (3):
  MVK_DCDC_Sollmodus_02:
    Bits: 12|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
  MVK_BMS_Sollmodus:
    Bits: 56|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
  MVK_DCDC_Sollmodus:
    Bits: 59|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]

### 0x521 (1313) - STH_01
DLC: 8, Transmitter: Gateway
Signals (3):
  STH_Zusatzheizung:
    Bits: 2|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  STH_Heizleistung:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 255.0]
    Comment: Heizleistung in %
  STH_Wassertemp:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 142.25]

### 0x53C (1340) - HVLM_04
DLC: 8, Transmitter: Ladegeraet_Konzern
Description: CAN-Botschaft
Signals (8):
  HVLM_Standklima_Timer_Status:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Signalisierung an das KBT ob mind. ein Timer zur Standklimatisierung programmiert ist
  HVLM_HVEM_MaxLeistung:
    Bits: 16|9 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 25450.0]
    Comment: Maximal zulässige Leistung, die das HVEM vom Lader nutzen darf. Wenn das Fzg nicht über einen Lader ...
  HVLM_Anf_Ladescreen:
    Bits: 26|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung für Ladescreen (BEV)
  HVLM_Ladeart:
    Bits: 27|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anzeige, ob Strom in das Fzg. fließt und wofür er verwendet wird.
  HVLM_HV_Anf:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: HV-Aktivierung Anforderung und Grund der Anforderung
  HVLM_Stecker_Status:
    Bits: 51|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status der Steckererkennung unabhängig vom Lademodus (AC oder DC)
  HVLM_LadeAnforderung:
    Bits: 53|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Mitteilung des Lademanagements an das MSG ob Stecker erkannt wurde, Laden gestartet, ein autonomer B...
  HVLM_MaxBatLadestromHV:
    Bits: 56|8 (Intel (LE), unsigned)
    Unit: Unit_Amper
    Range: [0.0 .. 253.0]
    Comment: Empfohlener HV Batterieladestrom bei einer geplanten Ladung

### 0x54B (1355) - Parkhilfe_04
DLC: 8, Transmitter: Gateway
Description: neue Struktur ab MQB ohne KD-Fehler
Signals (2):
  PDC_Charisma_Status:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Charisma Systemstatus des Charisma-Teilnehmers
  PDC_Charisma_FahrPr:
    Bits: 60|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Istwert der Kennline (siehe System-LAH MQB FPA)

### 0x552 (1362) - HVEM_05
DLC: 8, Transmitter: Gateway
Signals (6):
  HVEM_NVNachladen_Energie:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 25.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 6325.0]
    Comment: HV-Lademanagement benötigt die Energiemenge für das Laden der 12V Batterie, um die Ladezeit zu berec...
  HVEM_Nachladen_Anf:
    Bits: 32|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung für HV Laden bei gestecktem Stecker und deaktivierter Ladeanforderung
  HVEM_SollStrom_HV:
    Bits: 33|11 (Intel (LE), unsigned)
    Formula: raw * 0.2 + -204.4
    Unit: Unit_Amper
    Range: [-204.4 .. 204.6]
    Comment: Sollstrom Laden auf der HV-Seite
  HVEM_MaxSpannung_HV:
    Bits: 44|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: maximale Ladespannung an das Ladegerät bzw. DC-Ladestation
  HVEM_DCDC_Freigabe:
    Bits: 57|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Freigabe des DCDC wenn das Fzg nicht Fahrbereit ist.
  HVEM_FreigabeKlimatisierung:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Klimatisierung freigeben

### 0x55D (1373) - TME_11
DLC: 8, Transmitter: TME
Signals (5):
  TME_Klimadefizit:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Erkanntes Leistungsdefizit für die Klimatisierung
  TME_Verdampfertemperatur:
    Bits: 22|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -20.0
    Unit: Unit_DegreCelsi
    Range: [-20.0 .. 70.0]
    Comment: Istwert der Verdampfertemperatur  (Verdampfer hängt am TME C7PHEV)
  TME_Temp_KM_vor_LE:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Kuehlmittelvorlauftemperatur vor der Leistungselektronik
  TME_Kuehlmittel_Temp_Pumpe1_tNT:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Kühlwassertemperatur nach Batteriekreispumpe
  TME_IstSpannung_EKK_HV:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 508.0]
    Comment: Vom EKV gemessene HV-Spannung

### 0x55F (1375) - TME_10
DLC: 8, Transmitter: TME
Signals (11):
  TME_Zuheizer_Freigabe:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Freigabe-Signal für Verbrennungszuheizfunktion
  TME_AC_LED:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Steuerung der LED in der AC-Taste des KBTs
  TME_Heizstrategie:
    Bits: 8|4 (Intel (LE), unsigned)
    Range: [0.0 .. 15.0]
    Comment: Info über aktuelle Heizstrategie an KBT
  TME_AC_Rueckmeldung:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Status des Elektrischen Klimakompressors
  TME_Heizungspumpen_Anst:
    Bits: 20|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Anforderung Heizungspumpenansteuerung
  TME_Frischluftklappe_Soll:
    Bits: 24|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Sollvorgabe der Frischluftklappenstellung bei speziellem Heizbetrieb, z.B. Wärmepumpe
  TME_Freigabe_Luft_PTC:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Freigabe des Zuheizers im Klimagerät
  TME_Umluftklappe_Soll:
    Bits: 32|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Sollvorgabe der Umluftklappenstellung bei speziellem Heizbetrieb, z.B. Wärmepumpe
  TME_Nachlaufzeit_Heizp:
    Bits: 40|6 (Intel (LE), unsigned)
    Formula: raw * 15.0 + 0.0
    Unit: Unit_Secon
    Range: [0.0 .. 945.0]
    Comment: Nachlaufzeit für Heizungspumpe
  TME_Klima_Defizit:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 40.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 10000.0]
    Comment: Für Innennraumklimatisierung aktuell nicht verfügbare  Leistung (Defizit = Istleistung - Solleistung...
  TME_Temp_KM_vor_HWT:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 149.75]
    Comment: Kühlmitteltemperatur vor HWT

### 0x564 (1380) - LAD_01
DLC: 8, Transmitter: Ladegeraet_Konzern
Description: MLB, MXB, MLBevo
Signals (4):
  LAD_AC_Istspannung:
    Bits: 15|9 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 509.0]
    Comment: Istwert AC-Netzspannung (RMS) 
  LAD_IstSpannung_HV:
    Bits: 24|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: Ausgangsspannung Lader
  LAD_IstStrom_HV:
    Bits: 34|10 (Intel (LE), unsigned)
    Formula: raw * 0.2 + -102.0
    Unit: Unit_Amper
    Range: [-102.0 .. 102.2]
    Comment: Ausgangsstrom Lader
  LAD_Temperatur:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur Ladegerät

### 0x565 (1381) - HVLM_03
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (8):
  HVLM_HV_Abstellzeit:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 4.0 + 0.0
    Unit: Unit_Minut
    Range: [0.0 .. 1012.0]
    Comment: Zeitraum zwischen HV-Deaktiviert und HV-Aktiviert
  HVLM_Ladesystemhinweise:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  HVLM_MaxStrom_Netz:
    Bits: 24|7 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 62.5]
  HVLM_Stecker_Verriegeln:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung Steckerverriegelung
  HVLM_Start_Spannungsmessung_DCLS:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Meldung an das BMS, dass an der DC-Ladesäule eine Messspannung aufgeschaltet wird.
  HVLM_FreigabeKlimatisierung:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Klimatisierung freigeben
  HVLM_Ladetexte:
    Bits: 49|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Anzeige, ob AC- oder DC-Laden nicht möglich ist
  HVLM_IstSpannung_HV:
    Bits: 54|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: Ausgangsspannung des Ladegerätes und DC-Spannung der Ladesäule. Messung zwischen den DC HV-Leitungen...

### 0x577 (1399) - TME_09
DLC: 8, Transmitter: TME
Signals (6):
  TME_Leistung_Komfort_HV_stat:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 12650.0]
    Comment: HV-Leistungsaufnahme Thermomanagement im stationären Fall für die Innenraumklimatisierung
  TME_Leistung_Thermo_HV_stat:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 12650.0]
    Comment: HV-Leistungsaufnahme Thermomanagement im stationären Fall für das Kühlen/Heizen von Bauteilen
  TME_Energie_Komfort_NV:
    Bits: 20|14 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 63250.0]
    Comment: Energieverbrauch/-bedarf der 12V-Komponenten für die Innenraumklimatisierung 
  TME_Energie_Thermo_NV:
    Bits: 34|14 (Intel (LE), unsigned)
    Formula: raw * 5.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 63250.0]
    Comment: Energieverbrauch/-bedarf der 12V-Komponenten für das Kühlen/Heizen von der Batterie
  TME_Leistung_Komfort_NV_stat:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 2530.0]
    Comment: 12V-Leistungsaufnahme Thermomanagement im stationären Fall für die Innenraumklimatisierung
  TME_Leistung_Thermo_NV_stat:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 2530.0]
    Comment: 12V-Leistungsaufnahme Thermomanagement im stationären Fall für das Kühlen/Heizen von Bauteilen

### 0x59A (1434) - MSG_TME_01
DLC: 8, Transmitter: TME
Signals (1):
  TME_Leistungsanf_ThermoHeizen:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 12650.0]
    Comment: Leistungsanforderung Thermomanagement für das Heizen von Komponenten

### 0x5AC (1452) - HVEM_02
DLC: 8, Transmitter: Gateway
Signals (5):
  HVEM_IstStrom_HVVerbraucher:
    Bits: 0|12 (Intel (LE), unsigned)
    Formula: raw * 0.1 + -204.7
    Unit: Unit_Amper
    Range: [-204.7 .. 204.6]
    Comment: Iststromaufnahme der elektrischen Nebenaggregate aus dem HV Zwischenkreis
  HVEM_Energie_Klima_Vorgabe_HighR:
    Bits: 12|3 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 50.0]
  HVEM_Leistung_Klima_Vorgabe:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 12650.0]
    Comment: Leistungsvorgabe Thermo (für Kühlen und Heizen)
  HVEM_Nutzbare_Energie:
    Bits: 32|11 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 102200.0]
    Comment: Nutzbare Energie welche für Komfortfunktionen vom HVEM freigegeben wird.
  HVEM_Energie_Klima_Vorgabe:
    Bits: 43|8 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 12650.0]
    Comment: HVEM-Energievorgabe für Klima-Vorkonditionierung

### 0x5F5 (1525) - Reichweite_01
DLC: 8, Transmitter: Gateway
Signals (5):
  RW_Gesamt_Reichweite_Max_Anzeige:
    Bits: 0|11 (Intel (LE), unsigned)
    Range: [0.0 .. 2047.0]
  RW_Gesamt_Reichweite:
    Bits: 29|11 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 2044.0]
    Comment: Reichweite für primär und sekundär Reichweite (eventuell Gewichtet)
  RW_Prim_Reichweitenverbrauch:
    Bits: 40|11 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_None
    Range: [0.0 .. 204.4]
    Comment: Für primäre Reichweitenberechnung zugrunde gelegter Verbrauch
  RW_Prim_Reichweitenv_Einheit:
    Bits: 51|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Für primäre  Reichweitenberechnung (KBI_Prim_Reichweitenverbrauch) zugrunde gelegte Einheit
  RW_Primaer_Reichweite:
    Bits: 53|11 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 2044.0]
    Comment: Reichweite der primären Antriebsart

### 0x5F7 (1527) - Reichweite_02
DLC: 8, Transmitter: Gateway
Signals (7):
  RW_Reichweite_Einheit_Anzeige:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Einheit für Primaer-/Sekundaer-/Gesamt- Reichweite zur Anzeige im Kombi
  RW_Gesamt_Reichweite_Anzeige:
    Bits: 7|11 (Intel (LE), unsigned)
    Range: [0.0 .. 2044.0]
    Comment: Reichweite für primär und sekundär Reichweite (eventuell Gewichtet) zur Anzeige im Kombi
  RW_Primaer_Reichweite_Anzeige:
    Bits: 18|11 (Intel (LE), unsigned)
    Range: [0.0 .. 2044.0]
    Comment: Reichweite der primären Antriebsart zur Anzeige im Kombi
  RW_Sekundaer_Reichweite_Anzeige:
    Bits: 29|11 (Intel (LE), unsigned)
    Range: [0.0 .. 2044.0]
    Comment: Reichweite der sekundaeren Antriebsart zur Anzeige im Kombi
  RW_Sekundaer_Reichweite:
    Bits: 40|11 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 2044.0]
    Comment: Reichweite der sekundaeren Antriebsart
  RW_Sek_Reichweitenv_Einheit:
    Bits: 51|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Für sekundäre Reichweitenberechnung (KBI_Sek_Reichweitenverbrauch) zugrunde gelegte Einheit
  RW_Sek_Reichweitenverbrauch:
    Bits: 53|11 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_None
    Range: [0.0 .. 204.4]
    Comment: Für sekundäre Reichweitenberechnung zugrunde gelegter Verbrauch

### 0x643 (1603) - Einheiten_01
DLC: 8, Transmitter: Gateway
Description: ab MQB mit 8 Byte
Signals (1):
  KBI_Einheit_Temp:
    Bits: 6|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Einheit der Temperatur

### 0x64F (1615) - BCM1_04
DLC: 8, Transmitter: Gateway
Signals (2):
  BCM1_Spannungs_Anf:
    Bits: 48|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Forderung des BCM1-SG's zur Spannungsanhebung.
  BCM1_Status_Ladeanzeige:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x65A (1626) - BCM_01
DLC: 8, Transmitter: Gateway
Signals (1):
  BCM1_NV_Taster:
    Bits: 34|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: BCM1_NV_Taster gibt an, ob der NightVision-Taster im Lichtdrehschalter-Modul aktuell betaetigt wird.

### 0x663 (1635) - NVEM_02
DLC: 8, Transmitter: Gateway
Description: DLC=8 ab D4 gültig
Signals (4):
  BEM_Ladezustand:
    Bits: 24|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
  BEM_Batteriediagnose:
    Bits: 28|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Anzeige von Textwarnungen
  MIB_Stromsparmodus:
    Bits: 31|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  NVEM_Batterie_Service:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung einer Batterie-Service - Anzeige / Anforderung an die Werkstatt über MOD

### 0x679 (1657) - TME_12
DLC: 8, Transmitter: TME
Signals (5):
  TME_Temp_KM_nach_EM:
    Bits: 12|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.0]
    Comment: Momentanwert Kuehlmitteltemperatur nach E-Maschine
  TME_Klimaanforderung:
    Bits: 21|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Anforderungsgrund der Klimaapplikation durch die TME
  TME_Klimaleistung:
    Bits: 32|7 (Intel (LE), unsigned)
    Formula: raw * 0.04 + 0.0
    Unit: Unit_KiloWatt
    Range: [0.0 .. 5.0]
    Comment: Leistung des elektrischen Klimakompressors
  TME_Systemtemp:
    Bits: 42|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Systemtemperatur Antriebe für Kombi
  TME_Temperatur_Ruecklauf_VKM:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 149.75]
    Comment: Kühlmitteltemperatur Rücklauf VKM

### 0x67E (1662) - LAD_02
DLC: 8, Transmitter: Ladegeraet_Konzern
Description: MLB, MXB, MLBevo
Signals (6):
  LAD_Abregelung_Temperatur:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Abregelung aufgrund interner Übertemperatur im Ladegerät
  LAD_Abregelung_BuchseTemp:
    Bits: 14|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Reduzierung aufgrund zu hoher Temperatur der Ladebuchse
  LAD_MaxLadLeistung_HV:
    Bits: 16|9 (Intel (LE), unsigned)
    Formula: raw * 100.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 50900.0]
    Comment: Maximale Leistung Ladegerät bezogen auf die maximale Leistung Infrastruktur (Kabel, Ladestation) und...
  LAD_PRX_Stromlimit:
    Bits: 32|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: AC-Stromlimit aufgrund der PRX Kabelkodierung
  LAD_Stecker_Verriegelt:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Status Steckerverriegelung (Rücklesekontakt des Verrieglungsaktors)
  LAD_MaxLadLeistung_HV_Offset:
    Bits: 57|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Signal kann in Ergänzung zu LAD_MaxLadLeistung_HV (100W Skalierung) genutzt werden, um die Auflösege...

### 0x6A6 (1702) - Wischer_01
DLC: 8, Transmitter: Gateway
Description: MLBevo
Signals (1):
  Wischer1_Wischgeschwindigkeit:
    Bits: 8|6 (Intel (LE), unsigned)
    Formula: raw * 1.0 + 9.0
    Unit: Unit_MinutInver
    Range: [10.0 .. 70.0]
    Comment: aktuelle, durch Sensorik gemessene Wischgeschwindigkeit

### 0x6B7 (1719) - Kombi_02
DLC: 8, Transmitter: Gateway
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

### 0x92DD5472 (2463978610) - HVLM_10
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (9):
  HVLM_RtmWarnLadeverbindung:
    Bits: 31|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Charging connection fault
  HVLM_RtmWarnLadesystem:
    Bits: 34|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Electrical machine CAN communication fault
  HVLM_RtmWarnLadestatus:
    Bits: 37|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Warning of charging status fault
  HVLM_RtmWarnLadeKommunikation:
    Bits: 40|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Warning of charging communction fault
  HVLM_Ladeanzeige_Anf:
    Bits: 43|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
  HVLM_Ladeanzeige_Status:
    Bits: 44|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  HVLM_Ladeanzeige_Rampzeit:
    Bits: 46|4 (Intel (LE), unsigned)
    Formula: raw * 50.0 + 0.0
    Unit: Unit_MilliSecon
    Range: [0.0 .. 750.0]
    Comment: Zeit innerhalb der die gesendet Intensität für die Ladeanzeige erreicht werden soll
  HVLM_Ladeanzeige_Intens_Heck:
    Bits: 50|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: aktuelle Helligkeit für die Anzeige des Ladestatus über Heck-Fahrzeug-Lichtelemente
  HVLM_Ladeanzeige_Intens_Front:
    Bits: 57|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: aktuelle Helligkeit für die Anzeige des Ladestatus über Front-Fahrzeug-Lichtelemente 

### 0x92DD5477 (2463978615) - EM2_HYB_11
DLC: 8, Transmitter: Gateway
Signals (1):
  EM2_EM_SollVolumenstrom:
    Bits: 58|6 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 30.5]

### 0x92DD5478 (2463978616) - EM3_HYB_11
DLC: 8, Transmitter: Gateway
Signals (1):
  EM3_EM_SollVolumenstrom:
    Bits: 58|6 (Intel (LE), unsigned)
    Formula: raw * 0.5 + 0.0
    Unit: Unit_LiterPerMinut
    Range: [0.0 .. 30.5]
    Comment: Für die Kühlung der E-Maschine aktuell mindestens benötigter Kühlwasservolumenstrom

### 0x92DD5490 (2463978640) - LAD2_01
DLC: 8, Transmitter: Ladegeraet_2
Signals (3):
  LAD2_Abregelung_Temperatur:
    Bits: 15|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Abregelung auf Grund interner Übertemperatur im OBC2
  LAD2_Temperatur:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert Temperatur OBC2
  LAD2_IstSpannung_HV:
    Bits: 24|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: HV-Ausgangsspannung OBC2

### 0x92DD5491 (2463978641) - HVLM_11
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (3):
  HVLM_HVLB_SollSpannung_HVLS:
    Bits: 12|12 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1023.25]
    Comment: Vorgabe SollSpannung Eingangsseite vom HVLM an HVLB
  HVLM_HVLB_Status:
    Bits: 24|12 (Intel (LE), unsigned)
    Range: [0.0 .. 4095.0]
    Comment: Statusübermittlung von Ladesäulenspezifika
  HVLM_HVLB_SollModus:
    Bits: 36|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vorgabe SollModus vom HVLM an HVLB

### 0x92DD5492 (2463978642) - LAD_04
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (3):
  LAD_SollStrom_HV_Pfad1:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.3]
    Comment: HV-SollStrom für Rail1 von OBC2
  LAD_SollStrom_HV_Pfad2:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.3]
    Comment: HV-Sollstrom für Rail2 von OBC2
  LAD_SollStrom_HV_Pfad3:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.3]
    Comment: HV-Sollstrom für Rail3 von OBC2

### 0x92DD5493 (2463978643) - LAD_05
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (4):
  LAD_AC_MaxStrom_L1:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 102.1]
    Comment: Maximaler AC-Eingangsstrom auf Rail 1 für OBC2
  LAD_AC_MaxStrom_L2:
    Bits: 22|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 102.1]
    Comment: Maximaler AC-Eingangsstrom auf Rail 2 für OBC2
  LAD_AC_MaxStrom_L3:
    Bits: 32|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 102.1]
    Comment: Maximaler AC-Eingangsstrom auf Rail 3 für OBC2
  LAD_Max_LadeSpannung_LAD2:
    Bits: 43|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: max. voltage / maximal erlaubte Ladespannung

### 0x92DD5494 (2463978644) - LAD2_02
DLC: 8, Transmitter: Ladegeraet_2
Signals (6):
  LAD2_AC_Istspannung_L1:
    Bits: 13|9 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 509.0]
    Comment: AC-IstSpannung Rail1 von OBC2
  LAD2_AC_Istspannung_L2:
    Bits: 22|9 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 509.0]
    Comment: AC-IstSpannung Rail2 von OBC2
  LAD2_AC_Istspannung_L3:
    Bits: 31|9 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 509.0]
    Comment: AC-IstSpannung Rail3 von OBC2
  LAD2_IstStrom_HV_Pfad1:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.3]
    Comment: HV-IstStrom Rail1 von OBC2
  LAD2_IstStrom_HV_Pfad2:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.3]
    Comment: HV-IstStrom Rail2 von OBC2
  LAD2_IstStrom_HV_Pfad3:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.3]
    Comment: HV-IstStrom Rail3 von OBC2

### 0x92DD5495 (2463978645) - LAD2_03
DLC: 8, Transmitter: Ladegeraet_2
Signals (3):
  LAD2_AC_IstStrom_L1:
    Bits: 12|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 102.1]
    Comment: AC-IstStrom Rail1 von OBC2
  LAD2_AC_IstStrom_L2:
    Bits: 22|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 102.1]
    Comment: AC-IstStrom Rail2 von OBC2
  LAD2_AC_IstStrom_L3:
    Bits: 32|10 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 102.1]
    Comment: AC-IstStrom Rail3 von OBC2

### 0x92DD54A2 (2463978658) - Rezi_01
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (8):
  Rezi_Warnung_Uebertemperatur:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Warnung bei überschreiten einer definierten Temperaturschwelle, gesendet von Rezi
  Rezi_IstSpannung:
    Bits: 24|9 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1020.0]
    Comment: DC-Spannung des H2-Rezirkulationsgebläses, gesendet von Rezi
  Rezi_Fehler_Ueberspannung_HV:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Überspannung im HV-DC-Netz, gesendet von Rezi
  Rezi_Fehler_Ueberspannung_NV:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Überspannung im NV-Netz, gesendet von Rezi
  Rezi_Fehler_Uebertemperatur:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehlermeldung bei überschreiten einer definierten Temperaturschwelle, gesendet von Rezi
  Rezi_Fehler_Unterspannung_HV:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Unterspannung im HV-DC-Netz, gesendet von Rezi
  Rezi_IstDrehzahl:
    Bits: 40|14 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 32764.0]
    Comment: aktuelle Drehzahl des H2-Rezirkulationsgebläses, gesendet von Rezi
  Rezi_IstStrom:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.4]
    Comment: DC-Strom des H2-Rezirkulationsgebläses, gesendet von Rezi

### 0x92DD54A3 (2463978659) - KMP_01
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (9):
  KMP_Warnung_Uebertemperatur:
    Bits: 23|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Warnung Übertemperatur
  KMP_Ist_Spannung:
    Bits: 24|9 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1022.0]
    Comment: Spannung HV am der Kühlmittelpumpe
  KMP_Fehler_Ueberspannung_HV:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler Überspannung HV
  KMP_Fehler_Ueberspannung_12V:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler Ueberspannung 12V
  KMP_Fehler_Uebertemperatur:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler Uebertemperatur der Kühlmittelpumpe im FCEV ZP4
  KMP_Fehler_Unterspannung_HV:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler Unterspannung HV
  KMP_Fehler_Unterspannung_12V:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler Unterspannung 12V
  KMP_Ist_Drehzahl:
    Bits: 40|14 (Intel (LE), unsigned)
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 16383.0]
    Comment: Ist- Drehzahl der Kühlmittelpumpe
  KMP_Ist_Strom:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 25.5]
    Comment: Ist- Stromaufnahme der Kühlmittelpumpe auf HV Leitung

### 0x92DD54A4 (2463978660) - ETL_01
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (8):
  ETL_IstSpannung:
    Bits: 24|9 (Intel (LE), unsigned)
    Formula: raw * 2.0 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 1020.0]
    Comment: DC-Spannung des ETL, gesendet von ETL
  ETL_Fehler_Ueberspannung_HV:
    Bits: 35|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Ueberspannung im HV-DC-Netz, gesendet von ETL
  ETL_Fehler_Ueberspannung_NV:
    Bits: 36|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Ueberspannung im NV-Netz, gesendet von ETL
  ETL_Fehler_Uebertemperatur:
    Bits: 37|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Fehler sobald Temperatur eine kritische Fehlerschwelle ueberschreitet, gesendet von ETL
  ETL_Fehler_Unterspannung_HV:
    Bits: 38|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Unterspannung im HV-DC-Netz, gesendet von ETL
  ETL_Warnung_Uebertemperatur:
    Bits: 39|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Warnung sobald Temperaturschwelle überschritten wird, gesendet von ETL
  ETL_IstDrehzahl:
    Bits: 40|14 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 163820.0]
    Comment: Ist-Drehzahl, gesendet von ETL
  ETL_IstStrom:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -63.5
    Unit: Unit_Amper
    Range: [-63.5 .. 63.5]
    Comment: aktuell gezogener DC Strom, gesendet von ETL

### 0x92DD54C7 (2463978695) - ETL_02
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (3):
  ETL_Temperatur_Axiallager:
    Bits: 0|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Temperatur am Axiallager, gesendet von ETL
  ETL_Temperatur_LE:
    Bits: 8|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Temperatur Leistungselektronik, gesendet von ETL
  ETL_Temperatur_Stator:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Stator-Temperatur, gesendet von ETL

### 0x96A95410 (2527679504) - EM1_HYB_10
DLC: 8, Transmitter: LE_MLBevo
Signals (3):
  EM1_IstStrom:
    Bits: 14|11 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -1023.0
    Unit: Unit_Amper
    Range: [-1023.0 .. 1022.0]
    Comment: Momentanwert: Strom über PWR
  EM1_IstDrehzahl_02:
    Bits: 25|15 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -10000.0
    Unit: Unit_MinutInver
    Range: [-10000.0 .. 22765.0]
    Comment: Momentanwert: E-Maschinen Drehzahl
  EM1_IstMoment_02:
    Bits: 40|12 (Intel (LE), unsigned)
    Formula: raw * 0.5 + -1023.0
    Unit: Unit_NewtoMeter
    Range: [-1023.0 .. 1023.5]
    Comment: Momentanwert: E-Maschinen Moment

### 0x96A95414 (2527679508) - NVEM_06
DLC: 8, Transmitter: Gateway
Signals (6):
  NVEM_Min_Spannung:
    Bits: 16|7 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 10.6
    Unit: Unit_Volt
    Range: [10.6 .. 16.0]
    Comment: Mild_Hybrid: erforderliche Mindestspannung 12V - Netz
  NVEM_Min_Spannung_Start:
    Bits: 23|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 7.0
    Unit: Unit_Volt
    Range: [7.0 .. 13.1]
    Comment: Minimale einzuhaltende Spannung im RSG Wiederstart
  NVEM_Max_Spannung:
    Bits: 29|7 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 10.6
    Unit: Unit_Volt
    Range: [10.6 .. 16.0]
  NVEM_Energie_Klima_Vorgabe:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_WattHour
    Range: [0.0 .. 2530.0]
    Comment: NVEM-Energievorgabe für Klima-Vorkonditionierung
  NVEM_MV_DC_uMinLV:
    Bits: 48|7 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 3.5
    Unit: Unit_Volt
    Range: [3.5 .. 16.0]
    Comment: Abregelgrenze: minimal erlaubte Bordnetzspannung im Hochsetzbetrieb vom 12V-Bordnetz ins 48V-Netz
  NVEM_MV_DC_uSollLV:
    Bits: 55|7 (Intel (LE), unsigned)
    Formula: raw * 0.05 + 10.6
    Unit: Unit_Volt
    Range: [10.6 .. 16.0]
    Comment: Sollwert für die 12V-Bordnetzspannung im Tiefsetzbetrieb vom 48V-Netz ins 12V-Bordnetz

### 0x96A95415 (2527679509) - NVEM_07
DLC: 8, Transmitter: Gateway
Signals (6):
  NVEM_BMS_NV_Imax_RSG_Anf:
    Bits: 32|6 (Intel (LE), unsigned)
    Formula: raw * 20.0 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 1220.0]
    Comment: prädizierter RSG-Verbrauchsstrom während des 12V-Zustarts
  NVEM_BMS_NV_Umin_RSG_Anf:
    Bits: 38|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 7.0
    Unit: Unit_Volt
    Range: [7.0 .. 13.1]
    Comment: erforderliche Mindestspannung beim RSG-Zustart für die Berechnung des Signals BMS_NV_Qe_Li_Inhalt
  NVEM_BMS_NV_Nachlaufzeit_Vorgabe:
    Bits: 44|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 60.0]
    Comment: Zeitvorgabe bis zum Öffnen des Trennelements
  NVEM_BMS_NV_Umin_Anf:
    Bits: 50|6 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 9.0
    Unit: Unit_Volt
    Range: [9.0 .. 15.1]
    Comment: erforderliche Mindestspannung für die Berechnung des Signals BMS_NV_Qe_Li_Inhalt
  NVEM_BMS_NV_Impuls_Abfrage:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Trigger zur Lieferung aktueller Daten und Initialisierung der Schleppzeiger für die Signale UMinStar...
  NVEM_BMS_NV_Sollmodus:
    Bits: 62|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Anforderung Trennelement oeffnen/ schliessen

### 0x96A954A0 (2527679648) - EM2_HYB_05
DLC: 8, Transmitter: Gateway
Description: CAN-Botschaft
Signals (4):
  EM2_Temperatur_ElMotor:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur der E-Maschine
  EM2_Temperatur_PWR:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des PWR
  EM2_Temperatur_Rotor:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des Rotors
  EM2_Temperatur_KW:
    Bits: 50|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Aktueller Ist-Wert Kuehlwassertemperatur Leistungselektronik / E-Machine 2

### 0x96A954A1 (2527679649) - EM3_HYB_05
DLC: 8, Transmitter: Gateway
Signals (4):
  EM3_Temperatur_ElMotor:
    Bits: 16|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur der E-Maschine
  EM3_Temperatur_PWR:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des PWR
  EM3_Temperatur_Rotor:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Momentanwert: Temperatur des Rotors
  EM3_Temperatur_KW:
    Bits: 50|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Aktueller Ist-Wert Kuehlwassertemperatur Leistungselektronik / E-Machine 3

### 0x96A954E0 (2527679712) - EM1_HYB_27
DLC: 8, Transmitter: Gateway
Signals (2):
  EM1_RtmWarnUeberdrehzahl:
    Bits: 15|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal overspeed warning EM1
  EM1_RtmWarnUeberstrom:
    Bits: 18|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal over-current warning EM1

### 0x96A954E1 (2527679713) - EM2_HYB_27
DLC: 8, Transmitter: Gateway
Signals (2):
  EM2_RtmWarnUeberdrehzahl:
    Bits: 15|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal overspeed warning EM2
  EM2_RtmWarnUeberstrom:
    Bits: 18|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal over-current warning EM2

### 0x96A954E2 (2527679714) - EM3_HYB_27
DLC: 8, Transmitter: Gateway
Signals (2):
  EM3_RtmWarnUeberdrehzahl:
    Bits: 15|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal overspeed warning EM3
  EM3_RtmWarnUeberstrom:
    Bits: 18|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: RTM Signal over-current warning EM3

### 0x96A954E3 (2527679715) - pVS_02
DLC: 8, Transmitter: Gateway_PAG
Signals (7):
  pVS_EnergieAnfr_Zaehler:
    Bits: 24|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Fortlaufender Zähler für die Anfrage Energie durch die Funktion pVS.
  pVS_EnergieAnfr_SocStart:
    Bits: 28|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Energieanfrage zur Erstellung einer Ladekurve: Start-SoC
  pVS_EnergieAnfr_SocZiel:
    Bits: 35|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Energieanfrage zur Erstellung einer Ladekurve: Ziel-SOC
  pVS_Zielklimamodus_Range_pers:
    Bits: 42|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Vom Nutzer konfigurierter Zielklimamodus (personalisiert) welchen die Klimaanlage im Fahrprogramm Ra...
  pVS_Zielklimamodus_Range:
    Bits: 45|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Zielklimamodus welchen die Klimaanlage im Fahrprogramm Range annehmen soll, wenn Sie nicht bereits i...
  pVS_LadegrenzeAnfr_Zaehler:
    Bits: 48|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Fortlaufender Zähler für die Anfrage Ladegrenze für die Funktion pVS (Erzeugung einer Ladekurve)
  pVS_LadegrenzeAnfr_Leistung:
    Bits: 52|12 (Intel (LE), unsigned)
    Formula: raw * 200.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 818600.0]

### 0x96A954E4 (2527679716) - TME_ThermoPred_01
DLC: 8, Transmitter: TME
Signals (5):
  TME_Pred_KlimaSys_Anf_FP1:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Systemtrigger, dimensionslose Größe, wird hochgezählt sobald das Fahrprogramm 1 gewählt wird um im G...
  TME_Pred_KlimaKunde_Anf_FP1:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Systemtrigger, dimensionslose Größe, wird hochgezählt sobald das Fahrprogramm 1 gewählt wird um im G...
  TME_ThermoPred_P_stat_Klima_FP1:
    Bits: 26|11 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 20450.0]
    Comment: Prädizierte Leistung der statischen Phase der Innenraumklimatisierung im Fahrprogramm 1
  TME_ThermoPred_P_dyn_Klima_FP1:
    Bits: 37|11 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 20450.0]
    Comment: Prädizierte Leistung der dynamischen Phase der Innenraumklimatisierung im Fahrprogramm 1
  TME_ThermoPred_T_dyn_Klima_FP1:
    Bits: 56|8 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 253.0]
    Comment: Prädizierte Zeit der dynamischen Phase der Innenraumklimatisierung im Fahrprogramm 1

### 0x96A954E5 (2527679717) - TME_ThermoPred_02
DLC: 8, Transmitter: TME
Signals (5):
  TME_Pred_KlimaSys_Anf_FP2:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Systemtrigger, dimensionslose Größe, wird hochgezählt sobald das Fahrprogramm 2 gewählt wird um im G...
  TME_Pred_KlimaKunde_Anf_FP2:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Systemtrigger, dimensionslose Größe, wird hochgezählt sobald das Fahrprogramm 2 gewählt wird um im G...
  TME_ThermoPred_P_stat_Klima_FP2:
    Bits: 26|11 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 20450.0]
    Comment: Prädizierte Leistung der statischen Phase der Innenraumklimatisierung im Fahrprogramm 2
  TME_ThermoPred_P_dyn_Klima_FP2:
    Bits: 37|11 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 20450.0]
    Comment: Prädizierte Leistung der dynamischen Phase der Innenraumklimatisierung im Fahrprogramm 2
  TME_ThermoPred_T_dyn_Klima_FP2:
    Bits: 56|8 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 253.0]
    Comment: Prädizierte Zeit der dynamischen Phase der Innenraumklimatisierung im Fahrprogramm 2

### 0x96A954E6 (2527679718) - TME_ThermoPred_03
DLC: 8, Transmitter: TME
Signals (5):
  TME_Pred_KlimaSys_Anf_FP3:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Systemtrigger, dimensionslose Größe, wird hochgezählt sobald das Fahrprogramm 3 gewählt wird um im G...
  TME_Pred_KlimaKunde_Anf_FP3:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Systemtrigger, dimensionslose Größe, wird hochgezählt sobald das Fahrprogramm 3 gewählt wird um im G...
  TME_ThermoPred_P_stat_Klima_FP3:
    Bits: 26|11 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 20450.0]
    Comment: Prädizierte Leistung der statischen Phase der Innenraumklimatisierung im Fahrprogramm 3
  TME_ThermoPred_P_dyn_Klima_FP3:
    Bits: 37|11 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 20450.0]
    Comment: Prädizierte Leistung der dynamischen Phase der Innenraumklimatisierung im Fahrprogramm 3
  TME_ThermoPred_T_dyn_Klima_FP3:
    Bits: 56|8 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 253.0]
    Comment: Prädizierte Zeit der dynamischen Phase der Innenraumklimatisierung im Fahrprogramm 3

### 0x97F0007B (2549088379) - KN_Hybrid_01
DLC: 8, Transmitter: BMC_MLBevo
Signals (5):
  KN_BMS_ECUKnockOutTimer:
    Bits: 32|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 62.0]
    Comment: Ausgabe des ECUKnockOut-Timer in der Knotenbotschaft
  KN_BMS_BusKnockOut:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des BusKnockOut-Status in der Knotenbotschaft
  KN_BMS_BusKnockOutTimer:
    Bits: 40|8 (Intel (LE), unsigned)
    Range: [0.0 .. 254.0]
    Comment: Ausgabe des BusKnockOut-Timer in der Knotenbotschaft
  KN_BMS_ECUKnockOut:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
    Comment: Ausgabe des ECUKnockOut-Status in der Knotenbotschaft
  BMS_HYB_KD_Fehler:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Bei gesetztem Bit ist mindestens ein Kundendienstfehler eingetragen

### 0x97F000AE (2549088430) - KN_BattRegelung
DLC: 8, Transmitter: BMS_NV
Signals (1):
  BMS_NV_Lokalaktiv:
    Bits: 61|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Zeigt an ob das SG nach Klemme15 AUS und nach entsprechender MAX Aktivzeit noch aktiv war. LAH.DUM.9...

### 0x97F40219 (2549350937) - ISO_OBDC_RTM_Req
DLC: 8, Transmitter: Gateway
Description: Datenübertragung vom OBDC zum Datenanforderer OPR
Signals (1):
  ISO_OBDC_RTM_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Kommunikation zwischen OBDC (Onboard Data Collector) und Datenanforderer RTM (RealTimeMonitoring)

### 0x97F41902 (2549356802) - ISO_RTM_OBDC_Req
DLC: 8, Transmitter: Sub_Gateway
Description: Datenübertragung vom OBDC zum Datenanforderer OPR
Signals (1):
  ISO_RTM_OBDC_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Kommunikation zwischen Datenanforderer RTM (RealTimeMonitoring) und OBDC (Onboard Data Collector)

### 0x97FC0242 (2549875266) - OBDC_TME_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_TME_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC027B (2549875323) - OBDC_Hybrid_01_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_Hybrid_01_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC028F (2549875343) - OBDC_AWC_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_AWC_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC02AE (2549875374) - OBDC_BattRegelung_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_BattRegelung_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]

### 0x97FC02B4 (2549875380) - OBDC_TelemetrieSG_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_TelemetrieSG_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FC02B5 (2549875381) - OBDC_FCU_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_FCU_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: TP-Kanal für den Onboard Datensammler

### 0x97FC16C0 (2549880512) - DIA_HV_H2_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  DIA_HV_H2_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Diagnosezugriff & Flashen Hochvolt Wasserheizer

### 0x97FC70D0 (2549903568) - OBDC_SSN_BJB_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_BJB_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten BJB

### 0x97FC70D1 (2549903569) - OBDC_SSN_CMC_01_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_01_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D2 (2549903570) - OBDC_SSN_CMC_02_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_02_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D3 (2549903571) - OBDC_SSN_CMC_03_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_03_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D4 (2549903572) - OBDC_SSN_CMC_04_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_04_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D5 (2549903573) - OBDC_SSN_CMC_05_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_05_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D6 (2549903574) - OBDC_SSN_CMC_06_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_06_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D7 (2549903575) - OBDC_SSN_CMC_07_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_07_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D8 (2549903576) - OBDC_SSN_CMC_08_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_08_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70D9 (2549903577) - OBDC_SSN_CMC_09_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_09_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70DA (2549903578) - OBDC_SSN_CMC_10_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_10_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70DB (2549903579) - OBDC_SSN_CMC_11_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_11_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70DC (2549903580) - OBDC_SSN_CMC_12_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_12_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70F1 (2549903601) - OBDC_SSN_CMC_13_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_13_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70F2 (2549903602) - OBDC_SSN_CMC_14_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_14_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FC70F3 (2549903603) - OBDC_SSN_CMC_15_Req
DLC: 8, Transmitter: Gateway
Signals (1):
  OBDC_SSN_CMC_15_Req_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: Request FID- zu NA- Kommunikation zw. OnboardDiagnoseClient und DK2F Sub-System-Knoten CMC_01 ... 16

### 0x97FD0200 (2549940736) - OBDC_Funktionaler_Req_All
DLC: 8, Transmitter: Gateway
Description: VDC funktionaler Request auf CANHS
Signals (1):
  OBDC_Funktionaler_Req_All_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.84467440737096e+19]
    Comment: VDC funktionaler Request auf CANHS

### 0x9A555441 (2589283393) - DC_MV_Komponentenfehler
DLC: 8, Transmitter: DCDC_IHEV
Signals (32):
  DC_MV_Komponentenfehler_01:
    Bits: 0|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_02:
    Bits: 2|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_03:
    Bits: 4|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_04:
    Bits: 6|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_05:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_06:
    Bits: 10|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_07:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_08:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_09:
    Bits: 16|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_10:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_11:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_12:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_13:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_14:
    Bits: 26|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_15:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_16:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_17:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_18:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_19:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_20:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_21:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_22:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_23:
    Bits: 44|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_24:
    Bits: 46|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_25:
    Bits: 48|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_26:
    Bits: 50|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_27:
    Bits: 52|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_28:
    Bits: 54|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_29:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_30:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_31:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_VLD:
    Bits: 63|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Valid-Bit der OBD-Komponentenfehler. Das Bit wird beim Fehlerspeicher löschen zurückgenommen und ers...

### 0x9A555480 (2589283456) - MVEM_02
DLC: 8, Transmitter: Gateway
Signals (1):
  MVEM_MO_Sollspannung_Notbetrieb:
    Bits: 24|8 (Intel (LE), unsigned)
    Formula: raw * 0.25 + 0.0
    Unit: Unit_Volt
    Range: [0.0 .. 63.25]
    Comment: Im eingeschränkten Betrieb des 48V Systems und Fehlerfälle der 48V Batterie.

### 0x9A555487 (2589283463) - TME_14
DLC: 8, Transmitter: TME
Signals (4):
  TME_Mischventil_Heizkreis_Ist:
    Bits: 24|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Aktuelle Ventilstellung des MVW1
  TME_Temp_KM_v_Pumpe_Heizkreis:
    Bits: 32|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Temperatur des Kühlmediums vor der Heizkreispumpe P5
  TME_Temp_KM_n_Mischventil_Heizkr:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Temperatur des Kühlmediums nach MVW1 bei Verbindung von Heizkreis und HT-Kreis
  TME_KMP_Solldrehzahl:
    Bits: 50|14 (Intel (LE), unsigned)
    Unit: Unit_RevPerMinute
    Range: [0.0 .. 16381.0]
    Comment: Angeforderte Solldrehzahl der Kühlmittelpumpe der FCU

### 0x9A55549D (2589283485) - HVLM_08
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  HVLM_Dauer_Klima_02:
    Bits: 58|6 (Intel (LE), unsigned)
    Unit: Unit_Minut
    Range: [0.0 .. 60.0]
    Comment: Dauer der Innenraumklimatisierung.

### 0x9A555515 (2589283605) - HVLM_09
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (1):
  HVLM_AWC_Sollstrom:
    Bits: 55|9 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 50.9]
    Comment: Angeforderter Sollstrom für AWC Laden

### 0x9A555517 (2589283607) - AWC_01
DLC: 8, Transmitter: AWC
Signals (2):
  AWC_IstSpannung:
    Bits: 30|10 (Intel (LE), unsigned)
    Unit: Unit_Volt
    Range: [0.0 .. 1021.0]
    Comment: Ausgangsspannung AWC Lader
  AWC_IstStrom:
    Bits: 46|9 (Intel (LE), unsigned)
    Formula: raw * 0.1 + 0.0
    Unit: Unit_Amper
    Range: [0.0 .. 50.9]
    Comment: Ausgangsstrom AWC

### 0x9A555522 (2589283618) - AWC_02
DLC: 8, Transmitter: AWC
Signals (2):
  AWC_Ladeoption_Infrastruktur:
    Bits: 48|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Rückmeldung an das Lademanagement welche Ladeoption an der Infrastrukturseite ausgewählt wurde
  AWC_Verzoegerung_Temp:
    Bits: 62|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Verzögerung des Ladevorgangs aufgrund der Umgebungstemperatur außerhalb Ladespec.

### 0x9A555531 (2589283633) - AWC_03
DLC: 8, Transmitter: AWC
Signals (2):
  AWC_Abregelung_Temperatur:
    Bits: 12|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Abregelung aufgrund Übertemperatur AWC
  AWC_Temperatur:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -40.0
    Unit: Unit_DegreCelsi
    Range: [-40.0 .. 213.0]
    Comment: Aktuelle Temperatur des AWC

### 0x9A55553A (2589283642) - FCU_04
DLC: 8, Transmitter: FCU_MLBevo_FCEV
Signals (4):
  FCU_Heizungspumpenansteuerung:
    Bits: 12|4 (Intel (LE), unsigned)
    Formula: raw * 10.0 + 0.0
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Heizungspumpenansteuerung. Erforderlich für Leitfähigkeit-Spülfunktion
  FCU_Temp_Kuehlmittel_Stack_Ausl:
    Bits: 40|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Mapping von Sensor 3T2
  FCU_Temp_Kuehlmittel_Stack_Einl:
    Bits: 48|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Mapping von Sensor 3T1
  FCU_Temp_Kuehlmittel_KMP_Einl:
    Bits: 56|8 (Intel (LE), unsigned)
    Formula: raw * 0.75 + -48.0
    Unit: Unit_DegreCelsi
    Range: [-48.0 .. 141.75]
    Comment: Mapping von Sensor 3T3

### 0x9A55554D (2589283661) - HVLM_15
DLC: 8, Transmitter: Ladegeraet_Konzern
Signals (6):
  HVLM_PlanAnfr_Ladeart:
    Bits: 26|3 (Intel (LE), unsigned)
    Range: [0.0 .. 7.0]
    Comment: Anfrage Ladeart für Ladeplanerstellung
  HVLM_EnergieAnfr_SocStart:
    Bits: 29|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Energieanfrage für die Ladeplanung: Start-SoC
  HVLM_EnergieAnfr_SocZiel:
    Bits: 36|7 (Intel (LE), unsigned)
    Unit: Unit_PerCent
    Range: [0.0 .. 100.0]
    Comment: Energieanfrage für die Ladeplanung: Ziel-SOC
  HVLM_EnergieAnfr_Zaehler:
    Bits: 43|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Fortlaufender Zähler für die Anfrage Energie
  HVLM_LadegrenzeAnfr_Leistung:
    Bits: 48|12 (Intel (LE), unsigned)
    Formula: raw * 200.0 + 0.0
    Unit: Unit_Watt
    Range: [0.0 .. 818600.0]
    Comment: Anfrage Ladeleistung mit welcher bis zu SOC aus BMS_LadegrenzeAntw_SOC geladen werden kann
  HVLM_LadegrenzeAnfr_Zaehler:
    Bits: 60|4 (Intel (LE), unsigned)
    Unit: Unit_None
    Range: [0.0 .. 13.0]
    Comment: Fortlaufender Zähler für die Anfrage Ladegrenze

### 0x9A555566 (2589283686) - DC_MV_CALID_CVN
DLC: 8, Transmitter: DCDC_IHEV
Signals (1):
  DC_MV_CVN_VLD:
    Bits: 7|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Valid-Bit der Calibration-Verification-Number (CVN bzw. Checksumme)

### 0x9A55560F (2589283855) - DC_MV_Komponentenfehler_02
DLC: 8, Transmitter: DCDC_IHEV
Signals (31):
  DC_MV_Komponentenfehler_32:
    Bits: 0|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_33:
    Bits: 2|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_34:
    Bits: 4|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_35:
    Bits: 6|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_36:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_37:
    Bits: 10|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_38:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_39:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_40:
    Bits: 16|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_41:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_42:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_43:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_44:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_45:
    Bits: 26|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_46:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_47:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_48:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_49:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_50:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_51:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_52:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_53:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_54:
    Bits: 44|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_55:
    Bits: 46|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_56:
    Bits: 48|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_57:
    Bits: 50|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_58:
    Bits: 52|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_59:
    Bits: 54|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_60:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_61:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_62:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x9A555610 (2589283856) - DC_MV_Komponentenfehler_03
DLC: 8, Transmitter: DCDC_IHEV
Signals (31):
  DC_MV_Komponentenfehler_63:
    Bits: 0|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_64:
    Bits: 2|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_65:
    Bits: 4|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_66:
    Bits: 6|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_67:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_68:
    Bits: 10|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_69:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_70:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_71:
    Bits: 16|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_72:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_73:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_74:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_75:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_76:
    Bits: 26|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_77:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_78:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_79:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_80:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_81:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_82:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_83:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_84:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_85:
    Bits: 44|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_86:
    Bits: 46|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_87:
    Bits: 48|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_88:
    Bits: 50|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_89:
    Bits: 52|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_90:
    Bits: 54|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_91:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_92:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_93:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x9A555611 (2589283857) - DC_MV_Komponentenfehler_04
DLC: 8, Transmitter: DCDC_IHEV
Signals (31):
  DC_MV_Komponentenfehler_94:
    Bits: 0|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_95:
    Bits: 2|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_96:
    Bits: 4|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_97:
    Bits: 6|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_98:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_99:
    Bits: 10|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_100:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_101:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_102:
    Bits: 16|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_103:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_104:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_105:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_106:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_107:
    Bits: 26|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_108:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_109:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_110:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_111:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_112:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_113:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_114:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_115:
    Bits: 42|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_116:
    Bits: 44|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_117:
    Bits: 46|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_118:
    Bits: 48|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_119:
    Bits: 50|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_120:
    Bits: 52|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_121:
    Bits: 54|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_122:
    Bits: 56|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_123:
    Bits: 58|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_124:
    Bits: 60|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x9A555612 (2589283858) - DC_MV_Komponentenfehler_05
DLC: 8, Transmitter: DCDC_IHEV
Signals (21):
  DC_MV_Komponentenfehler_125:
    Bits: 0|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_126:
    Bits: 2|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_127:
    Bits: 4|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_128:
    Bits: 6|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_129:
    Bits: 8|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_130:
    Bits: 10|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_131:
    Bits: 12|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_132:
    Bits: 14|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_133:
    Bits: 16|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_134:
    Bits: 18|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_135:
    Bits: 20|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_136:
    Bits: 22|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_137:
    Bits: 24|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_138:
    Bits: 26|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_139:
    Bits: 28|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_140:
    Bits: 30|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_141:
    Bits: 32|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_142:
    Bits: 34|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_143:
    Bits: 36|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_144:
    Bits: 38|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]
  DC_MV_Komponentenfehler_145:
    Bits: 40|2 (Intel (LE), unsigned)
    Range: [0.0 .. 3.0]

### 0x9B000001 (2600468481) - NMH_Sekundaer_01
DLC: 8, Transmitter: LE2
Signals (1):
  NM_Sekundaer_01_NM_aktiv_Anf_HVK:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung HVK (Nachlaufursache, SG nicht im NM-Zustand 'Ready to Sleep')

### 0x9B000003 (2600468483) - NMH_Sekundaer_03
DLC: 8, Transmitter: LE1
Signals (1):
  NM_Sekundaer_03_NM_aktiv_Anf_HVK:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Anforderung HVK (Nachlaufursache, SG nicht im NM-Zustand 'Ready to Sleep')

### 0x9B000042 (2600468546) - NMH_TME
DLC: 8, Transmitter: TME
Description: CAN_NM_PDU MLBevo
Signals (1):
  NM_TME_NM_aktiv_HV_Auszeitber:
    Bits: 29|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Nachlauf aufgrund HV Auszeitberechnung 

### 0x9B0000AE (2600468654) - NMH_BattRegelung
DLC: 8, Transmitter: BMS_NV
Signals (1):
  NM_BattRegelung_NM_aktiv_NV:
    Bits: 27|1 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]
    Comment: Niedervoltspeicher_angefordert

### 0xC0000000 (3221225472) - VECTOR__INDEPENDENT_SIG_MSG
DLC: 0, Transmitter: Vector__XXX
Description: This is a message for not used signals, created by Vector CANdb++ DBC OLE DB Provider.
Signals (3):
  KBI_Kilometerstand:
    Bits: 0|20 (Intel (LE), unsigned)
    Unit: Unit_KiloMeter
    Range: [0.0 .. 1048573.0]
  BR_Eingriffsmoment:
    Bits: 0|10 (Intel (LE), unsigned)
    Formula: raw * 1.0 + -509.0
    Range: [-509.0 .. 509.0]
  XCP_BMS_MV_CRO_01_Data:
    Bits: 0|64 (Intel (LE), unsigned)
    Range: [0.0 .. 1.0]