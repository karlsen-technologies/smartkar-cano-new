# Core EV Signals for e-Golf

Extracted from VW DBC files. These signals use standard 11-bit CAN IDs
and are likely available on the comfort CAN bus via gateway routing.

**Note:** Signal positions use DBC notation: `start_bit|length@byte_order`
- `@1` = Intel/Little-endian (LSB first)
- `@0` = Motorola/Big-endian (MSB first)
- Formula: `physical_value = raw_value * scale + offset`


## Battery Management (BMS)

### 0x191 (401) - BMS_01
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Core battery voltage/current/SOC

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_01_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| BMS_01_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| BMS_IstStrom_02 | 12|12@LE,u | 1.0 | -2047.0 | Unit_Amper | Momentanwert: Batteriestrom
Stromrichtun... |
| BMS_IstSpannung | 24|12@LE,u | 0.25 | 0.0 | Unit_Volt | actual voltage of the battery / Momentan... |
| BMS_Spannung_ZwKr | 36|11@LE,u | 0.5 | 0.0 | Unit_Volt | Momentanwert: 
Spannung an den Batterie-... |
| BMS_SOC_HiRes | 47|11@LE,u | 0.05 | 0.0 | Unit_PerCent | State of Charge / aktueller Ladezustand ... |
| BMS_IstStrom_02_OffsetVZ | 58|1@LE,u | 1.0 | 0.0 |  | Vorzeichen des Offsets, bzw. der Nachkom... |
| BMS_IstStrom_02_Offset | 60|4@LE,u | 0.0625 | 0.0 | Unit_Amper | Nachkommastelle des BMS_IstStrom_02 |

### 0x1A1 (417) - BMS_02
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Charge/discharge current limits

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_MaxDyn_LadeStrom_02_Offset | 8|4@LE,u | 0.0625 | 0.0 | Unit_Amper | Nachkommastelle BMS_MaxDyn_LadeStrom_02 |
| BMS_MaxDyn_EntladeStrom_02 | 12|11@LE,u | 1.0 | 0.0 | Unit_Amper | max current (discharge)
maximal zulässig... |
| BMS_MaxDyn_LadeStrom_02 | 23|11@LE,u | 1.0 | 0.0 | Unit_Amper | max current (charge)
maximal zulässiger ... |
| BMS_Min_EntladeSpannung | 34|10@LE,u | 1.0 | 0.0 | Unit_Volt | min. voltage / minimal erlaubte Batterie... |
| BMS_MinDyn_EntladeSpannung | 44|10@LE,u | 1.0 | 0.0 | Unit_Volt | min. voltage during discharge with curre... |
| BMS_MinDyn_LadeSpannung | 54|10@LE,u | 1.0 | 0.0 | Unit_Volt | min. voltage during charge with current ... |

### 0x39D (925) - BMS_03
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Voltage limits

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_Leerlaufspannung | 0|10@LE,u | 1.0 | 0.0 | Unit_Volt | Leerlaufspannung der Gesamtbatterie, Ope... |
| BMS_Max_LadeSpannung | 12|10@LE,u | 1.0 | 0.0 | Unit_Volt | max. voltage / maximal erlaubte Batterie... |
| BMS_MaxPred_EntladeStrom_02 | 22|11@LE,u | 1.0 | 0.0 | Unit_Amper | dynamic el. limit, max. current (dischar... |
| BMS_MaxPred_LadeStrom_02 | 33|11@LE,u | 1.0 | 0.0 | Unit_Amper | dynamic el. limit, max. current (charge)... |
| BMS_MinPred_EntladeSpannung | 44|10@LE,u | 1.0 | 0.0 | Unit_Volt | static el. limit, min. battery voltage (... |
| BMS_MinPred_LadeSpannung | 54|10@LE,u | 1.0 | 0.0 | Unit_Volt | static el. limit, min. battery voltage (... |

### 0x2AF (687) - BMS_05
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Additional battery data

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_As_Entladezaehler | 21|10@LE,u | 1.0 | 0.0 | Unit_AmperSecon | Signal zur Berechnung des Lastprofils du... |
| BMS_As_Entladezaehler_Ueberlauf | 31|1@LE,u | 1.0 | 0.0 |  | Wird beim ersten Überlauf von BMS_As_Ent... |
| BMS_Rekuperation | 32|15@LE,u | 10.0 | 0.0 | Unit_WattSecond | Zähler: Anzeigesignal: Energiezufuhr in ... |
| BMS_Rekuperation_Ueberlauf | 47|1@LE,u | 1.0 | 0.0 |  | Rekuperationleistungssignal BMS_Rekupera... |
| BMS_Verbrauch | 48|15@LE,u | 10.0 | 0.0 | Unit_WattSecond | Zähler: Energieentnahme aus der HV-Batte... |
| BMS_Verbrauch_Ueberlauf | 63|1@LE,u | 1.0 | 0.0 |  | Verbrauchssignal mindest 1x übergelaufen |

### 0x59E (1438) - BMS_06
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Temperature

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_Temperierung_Anf | 0|3@LE,u | 1.0 | 0.0 |  | Temperierungsbedarf der HV-Batterie |
| BMS_Status_Ventil | 3|3@LE,u | 1.0 | 0.0 |  | Status des 3-2-Wege-Ventils (betätigt/un... |
| BMS_Temp_Epsilon | 8|4@LE,u | 1.0 | -15.0 | Unit_DegreCelsi | Das Signal beschreibt die Differenz zwis... |
| BMS_Soll_PWM_Pumpe | 12|4@LE,u | 10.0 | 0.0 | Unit_PerCent | Ansteuerungs-Sollwert der Batterie-Kühlm... |
| BMS_Temperatur | 16|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi | battery temperature / Momentanwert: Temp... |
| BMS_IstVorlaufTemperatur | 24|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi | Aktuelle Vorlauftemperatur der Batterie |
| BMS_Fehler_NTKreis | 36|1@LE,u | 1.0 | 0.0 |  | Fehler im NTKreis, HV-Batterie wird nich... |
| BMS_Fehlerstatus_EWP_02 | 37|3@LE,u | 1.0 | 0.0 |  | Fehlerstatus der elektrischen Wasserpump... |
| BMS_SollVolumenstrom | 40|6@LE,u | 1.0 | 0.0 | Unit_LiterPerMinut | Soll Volumenstrom für Kühlkreislauf Batt... |
| BMS_Batterie_Voko_Anf | 46|1@LE,u | 1.0 | 0.0 |  | Anforderung des BMC die Bauteilvorkondit... |
| BMS_SollVorlauftemperatur | 48|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi | Soll_Vorlauftemperatur_Kühlmedium in °C |
| BMS_IstRuecklaufTemperatur_02 | 56|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi | Aktuelle Ruecklauftemperatur der Batteri... |

### 0x5A2 (1442) - BMS_04
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Mode and capacity

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_04_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| BMS_04_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| BMS_Status_ServiceDisconnect | 13|1@LE,u | 1.0 | 0.0 |  | Status des Service Disconnect, ob dieser... |
| BMS_Status_Spgfreiheit | 14|2@LE,u | 1.0 | 0.0 |  | 0=Init (ohne Funktion)
1=BMS Zwischenkre... |
| BMS_OBD_Lampe_Anf | 16|1@LE,u | 1.0 | 0.0 |  | Anforderung MIL (MalfunctionIndicationLa... |
| BMS_IstModus | 17|3@LE,u | 1.0 | 0.0 |  | Aktueller Betriebsmodus der Batterie
0: ... |
| BMS_Fehlerstatus | 20|3@LE,u | 1.0 | 0.0 |  | aktueller Fehlerstatus der Batterie |
| BMS_Kapazitaet_02 | 23|11@LE,u | 0.2 | 0.0 | Unit_AmperHour | Nutzbare Kapazität der Batterie,
oder al... |
| BMS_Soll_SOC_HiRes | 53|11@LE,u | 0.05 | 0.0 | Unit_PerCent | Der Sollwert für den HV-Batterie-SOC bei... |

### 0x5CA (1482) - BMS_07
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Energy content

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_07_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| BMS_07_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| BMS_Energieinhalt | 12|11@LE,u | 50.0 | 0.0 | Unit_WattHour | aktuell nutzbarer Energieinhalt der HV-B... |
| BMS_Ladevorgang_aktiv | 23|1@LE,u | 1.0 | 0.0 |  | Signalisiert aktiven Ladevorgang, Ladege... |
| BMS_Batteriediagnose | 24|3@LE,u | 1.0 | 0.0 |  | Anzeige von Textwarnungen |
| BMS_Freig_max_Performanz | 27|2@LE,u | 1.0 | 0.0 |  | Freigabe der Komponente, dass eine maxim... |
| BMS_Balancing_Aktiv | 30|2@LE,u | 1.0 | 0.0 |  | Signal zeigt an, ob mindestens eine Zell... |
| BMS_MaxEnergieinhalt | 32|11@LE,u | 50.0 | 0.0 | Unit_WattHour | maximaler nutzbarer Energieinhalt der HV... |
| BMS_Ausgleichsladung_Anf | 43|1@LE,u | 1.0 | 0.0 |  | BMS wünscht eine Ausgleichsladung
Flag d... |
| BMS_Gesamtst_Spgfreiheit | 44|2@LE,u | 1.0 | 0.0 |  | 0 = Funktion nicht aktiviert
1 = HV-Syst... |
| BMS_RIso_Ext | 46|12@LE,u | 5.0 | 0.0 | Unit_KiloOhm | Minimum des Isolationswiderstands zwisch... |
| BMS_KundenWarnung | 58|2@LE,u | 1.0 | 0.0 |  | Kunden Warnung aufgrund eines Batteriefe... |
| BMS_Fehler_KuehlkreislaufLeckage | 60|2@LE,u | 1.0 | 0.0 |  | Bei einer Leckage im Kühlkreislauf wird ... |

### 0x509 (1289) - BMS_10
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Usable SOC/energy

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_Energieinhalt_HiRes | 0|15@LE,u | 4.0 | 0.0 | Unit_WattHour | aktuell nutzbarer Energieinhalt der HV-B... |
| BMS_MaxEnergieinhalt_HiRes | 15|15@LE,u | 4.0 | 0.0 | Unit_WattHour | maximal nutzbarer Energieinhalt der HV-B... |
| BMS_NutzbarerSOC | 30|8@LE,u | 0.5 | 0.0 | Unit_PerCent | Aktueller relativer Ladezustand der HV-B... |
| BMS_NutzbarerEnergieinhalt | 38|12@LE,u | 25.0 | 0.0 | Unit_WattHour | Restenergieinhalt der Traktionsbatterie,... |
| BMS_Pred_Entladeperformance | 50|7@LE,u | 1.0 | 0.0 | Unit_PerCent | Indikator Stromintegral in Entladerichtu... |
| BMS_Pred_Ladeperformance | 57|7@LE,u | 1.0 | 0.0 | Unit_PerCent | Indikator Stromintegral in Laderichtung.... |

### 0x578 (1400) - BMS_DC_01
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: DC fast charging

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_DC_01_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| BMS_DC_01_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| BMS_Status_DCLS | 12|2@LE,u | 1.0 | 0.0 |  | Status der Spannungsüberwachung an der D... |
| BMS_DCLS_Spannung | 14|10@LE,u | 1.0 | 0.0 | Unit_Volt | DC-Spannung der Ladesäule.
Messung zwisc... |
| BMS_DCLS_MaxLadeStrom | 24|9@LE,u | 1.0 | 0.0 | Unit_Amper | maximaler zulässiger DC-Ladestrom |


## DC-DC Converter

### 0x2AE (686) - DCDC_01
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: HV/LV voltages and currents

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| DCDC_01_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| DCDC_01_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| DC_IstSpannung_HV | 12|12@LE,u | 0.25 | 0.0 | Unit_Volt | Momentanwert:  Hochspannung am DC/DC Wan... |
| DC_IstStrom_HV_02 | 24|10@LE,u | 0.4 | -204.0 | Unit_Amper | Aktueller Strom des DC/DC-Wandlers Höher... |
| DC_IstStrom_NV | 34|10@LE,u | 1.0 | -511.0 | Unit_Amper | Momentanwert: Strom vom DC/DC in das Bor... |
| DC_IstSpannung_NV | 56|8@LE,u | 0.1 | 0.0 | Unit_Volt | Momentanwert: Bordnetzspannung am DC/DC ... |

### 0x3F4 (1012) - DCDC_02
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Additional DCDC data

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| DCDC_02_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| DCDC_02_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| DC_HYB_iAktReserveLV | 12|10@LE,u | 1.0 | -511.0 | Unit_Amper | Momentanwert: Stromreserve vom DC/DC-Wan... |
| DC_Verbrauch_Ueberlauf | 23|1@LE,u | 1.0 | 0.0 |  | Überlauf-Bit für das Signal DC_Verbrauch |
| DC_Verbrauch | 24|10@LE,u | 1.0 | 0.0 | Unit_WattSecond | Energieverbrauch vom DCDC für Versorgung... |
| DC_Verlustleistung | 34|6@LE,u | 5.0 | 0.0 | Unit_Watt | Aktuelle Verlustleistung des DC/DC-Wandl... |
| DC_HYB_Auslastungsgrad | 56|8@LE,u | 0.4 | 0.0 | Unit_PerCent | Momentanwert: Auslastung DC/DC (DFM Sign... |

### 0x5CD (1485) - DCDC_03
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: DCDC mode and temperature

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| DCDC_03_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| DCDC_03_BZ | 8|4@LE,u | 1.0 | 0.0 |  |  |
| DC_Fehlerstatus | 16|3@LE,u | 1.0 | 0.0 |  | aktueller Fehlerstatus des DCDC-Wandler: |
| DC_Peakstrom_verfuegbar | 19|1@LE,u | 1.0 | 0.0 |  | Peakstrom verfügbar/nicht verfügbar |
| DC_Abregelung_Temperatur | 20|1@LE,u | 1.0 | 0.0 |  | Abregelung aufgrund Übertemperatur im Le... |
| DC_IstModus_02 | 21|3@LE,u | 1.0 | 0.0 |  | aktuelle Betriebsart des DC/DC Wandlers;... |
| DC_HV_EKK_IstModus | 28|3@LE,u | 1.0 | 0.0 |  | Aktuelle Betriebsart des DC/DC Wandlers ... |
| DC_Status_Spgfreiheit_HV | 46|2@LE,u | 1.0 | 0.0 |  | HV DC/DC-Wandler Status HV-Spannungsfrei... |
| DC_IstSpannung_EKK_HV | 48|8@LE,u | 2.0 | 0.0 | Unit_Volt | Ausgangsspannung des Kombiwandlers am 40... |
| DC_Temperatur | 56|8@LE,u | 1.0 | -40.0 | Unit_DegreCelsi | Momentanwert: Temperatur des DC/DC Wandl... |

### 0x3B1 (945) - DC_Hybrid_01
- DLC: 8 bytes
- Source: vw_mqb_2010.dbc
- Purpose: Hybrid DC-DC

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| DC_HYB_iAktLV | 12|10@LE,u | 1.0 | -511.0 | Unit_Amper |  |
| DC_HYB_iAktReserveLV | 22|10@LE,u | 1.0 | -511.0 | Unit_Amper |  |
| DC_HYB_uAktLV | 32|8@LE,u | 0.1 | 0.0 | Unit_Volt |  |
| DC_HYB_LangsRegelung | 40|1@LE,u | 1.0 | 0.0 |  |  |
| DC_HYB_Abregelung_Temperatur | 41|1@LE,u | 1.0 | 0.0 |  |  |
| DC_HYB_Fehler_RedLeistung | 42|1@LE,u | 1.0 | 0.0 |  |  |
| DC_HYB_Fehler_intern | 43|1@LE,u | 1.0 | 0.0 |  |  |
| DC_HYB_Fehler_Spannung | 44|1@LE,u | 1.0 | 0.0 |  |  |
| DC_HYB_Auslastungsgrad | 56|8@LE,u | 0.4 | 0.0 | Unit_PerCent |  |


## Climate/HVAC

### 0x3B5 (949) - Klima_11
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Climate system status

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| KL_Drehz_Anh | 0|1@LE,u | 1.0 | 0.0 |  | Drehzahlanhebung vom Motor angefordert

 |
| KL_Vorwarn_Komp_ein | 1|1@LE,u | 1.0 | 0.0 |  | Vorwarnung Kompressor ein; 140ms vorher;... |
| KL_AC_Schalter | 2|1@LE,u | 1.0 | 0.0 |  | AC Schalter
 |
| KL_Komp_Moment_alt | 3|1@LE,u | 1.0 | 0.0 |  |  |
| KL_Vorwarn_Zuheizer_ein | 6|1@LE,u | 1.0 | 0.0 |  | Vorwarnung Zuheizerbetrieb
 |
| KL_Zustand | 7|1@LE,u | 1.0 | 0.0 |  | Beschreibt den Zustand des Klima-/Heizun... |
| KL_Kompressorkupplung_linear | 8|8@LE,u | 20.0 | 0.0 | Unit_MilliAmper | Sollwert zur Ansteuerung der Kompressork... |
| KL_Charisma_FahrPr | 16|4@LE,u | 1.0 | 0.0 |  | MLBevo: Aktuelles Fahrprogramm des Chari... |
| KL_Charisma_Status | 20|2@LE,u | 1.0 | 0.0 |  | Charisma Systemstatus des Charisma-Teiln... |
| KL_nachtr_Stopp_Anf | 22|1@LE,u | 1.0 | 0.0 |  | Dieses Signal dient dem Motor als Trigge... |
| KL_T_Charge | 23|1@LE,u | 1.0 | 0.0 |  | Signal für Tastenbetätigung Charge-Taste... |
| KL_Last_Kompr | 24|8@LE,u | 0.25 | 0.0 | Unit_NewtoMeter | Aktuelles Verlustmoment der Klimaanlage ... |
| KL_Spannungs_Anf | 32|2@LE,u | 1.0 | 0.0 |  | Forderung des Klima-SG's zur Spannungsan... |
| KL_Thermomanagement | 34|2@LE,u | 1.0 | 0.0 |  | Stufen des Thermomanagements;
Bei Klemme... |
| KL_StartStopp_Info | 36|2@LE,u | 1.0 | 0.0 |  | Stopp-Freigabe und Start-Anforderung an ... |
| KL_Freilauf_Info | 38|2@LE,u | 1.0 | 0.0 |  | Freilauf Informationssignal von Klima zu... |
| KL_Anf_KL | 40|8@LE,u | 0.4 | 0.0 | Unit_PerCent | Anforderung Kühlerlüfter der Klimaanlage... |
| KL_el_Zuheizer_Stufe | 48|3@LE,u | 1.0 | 0.0 |  | Vorgabe für Motorsteuergeraet PTC-Stufe;... |
| KL_Ausstattung_Klima | 51|3@LE,u | 1.0 | 0.0 |  | Austtattungsart der Klimabedienung |
| KL_Variante_Standheizung | 54|2@LE,u | 1.0 | 0.0 |  | Identifikation der Variante der Standhei... |

### 0x659 (1625) - Klimakomp_01
- DLC: 8 bytes
- Source: vw_mqb_2010.dbc
- Purpose: Electric AC compressor

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| EKL_KD_Fehler | 15|1@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_SCI_com_stat | 16|2@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_output_stat | 18|2@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_main_stat | 20|1@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_ovld_stat | 21|3@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_Inv_stat | 24|2@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_photo_temp_stat | 30|2@LE,u | 1.0 | 0.0 |  |  |
| EKL_Comp_photo_temp | 32|8@LE,u | 1.0 | 0.0 | Unit_DegreCelsi |  |
| EKL_Comp_current | 40|8@LE,u | 0.1 | 0.0 | Unit_Amper |  |
| EKL_Comp_rev_stat | 48|8@LE,u | 50.0 | 0.0 | Unit_MinutInver |  |

### 0x668 (1640) - Klima_12
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Climate temperatures

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| KL_LRH_Taster | 0|1@LE,u | 1.0 | 0.0 |  | Tasterbetätigung Lenkradheizung |
| KL_LRH_Stufe | 1|2@LE,u | 1.0 | 0.0 |  | Eingestellte Temperaturstufe der Lenkrad... |
| HSH_Taster | 3|2@LE,u | 1.0 | 0.0 |  | An-/Abforderung Heckscheibenheizung über... |
| FSH_Taster | 5|1@LE,u | 1.0 | 0.0 |  | Tasterbetätigung Frontscheibenheizung
 |
| KL_Zuheizer_Freigabe | 6|1@LE,u | 1.0 | 0.0 |  | Freigabe Zuheizer
 |
| KL_Beschlagsgefahr | 7|1@LE,u | 1.0 | 0.0 |  | Beschlagsgefahr auf der Frontscheibe im ... |
| KL_SIH_Soll_li | 8|3@LE,u | 1.0 | 0.0 |  | Einstellung Sitzheizung Fahrer (0...6) |
| KL_SIH_Soll_re | 11|3@LE,u | 1.0 | 0.0 |  | Einstellung Sitzheizung Beifahrer (0...6... |
| KRH_Soll_li | 14|2@LE,u | 1.0 | 0.0 |  | Ausgabe des Sollwertes für die Kopfraumh... |
| KL_SIL_Soll_li | 16|3@LE,u | 1.0 | 0.0 |  | Einstellung Sitzlüftung Fahrer (0...6) |
| KL_SIL_Soll_re | 19|3@LE,u | 1.0 | 0.0 |  | Einstellung Sitzlüftung Beifahrer (0...6... |
| KRH_Soll_re | 22|2@LE,u | 1.0 | 0.0 |  | Ausgabe des Sollwertes für die Kopfraumh... |
| KL_Geblspng_Soll | 24|8@LE,u | 0.05 | 1.45 | Unit_Volt | Ausgabe des Sollwertes für die Spannung ... |
| KL_Geblspng_Fond_Soll | 32|8@LE,u | 0.05 | 1.45 | Unit_Volt | Ausgabe des Sollwertes für die Spannung ... |
| KL_I_Geblaese | 40|8@LE,u | 0.25 | 0.0 | Unit_Amper | Stromaufnahme Gebläse |
| KL_Kompressorstrom_soll | 48|10@LE,u | 1.0 | 0.0 |  | Sollvorgabe Klima-Kompressorstrom |
| KL_Umluftklappe_Status | 58|4@LE,u | 1.0 | 0.0 |  | Status Umluftklappe |
| KL_PTC_Verbauinfo | 62|2@LE,u | 1.0 | 0.0 |  | Im Klima- bzw. Heizungsbedienteil wird d... |

### 0x66E (1646) - Klima_03
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Climate control

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| KL_STL_aktiv | 0|1@LE,u | 1.0 | 0.0 |  | Betriebsstatus Standlueftung
 |
| KL_STH_aktiv | 1|1@LE,u | 1.0 | 0.0 |  | Betriebsstatus Standheizung
 |
| KL_Solarluefter_aktiv | 2|1@LE,u | 1.0 | 0.0 |  | Betriebsstatus Solarluefter
 |
| KL_Umluft_Taste | 3|1@LE,u | 1.0 | 0.0 |  | Betaetigung Umlufttaste
 |
| KL_Geblaese_Fond_Status | 4|4@LE,u | 1.0 | 0.0 |  | Status Geblaese









 |
| KL_STH_Ansteuerung | 8|2@LE,u | 1.0 | 0.0 |  | Ansteuerung STH

 |
| KL_STH_Betriebsdauer | 10|6@LE,u | 1.0 | 0.0 | Unit_Minut | Betriebsdauer STH |
| KL_Magnetventil | 16|1@LE,u | 1.0 | 0.0 |  | Anforderung des Bypassventils. |
| KL_WaPu | 17|1@LE,u | 1.0 | 0.0 |  | Ansteuerung der Standheizungswasserpumpe |
| KL_Geblaese_Status | 18|4@LE,u | 1.0 | 0.0 |  | Status Geblaese









 |
| KL_Restwaerme_aktiv | 22|1@LE,u | 1.0 | 0.0 |  | Signal zur Aktivierung/Deaktivierung der... |
| KL_Kompressorkupplung | 23|1@LE,u | 1.0 | 0.0 |  | Signal zur Aktivierung/Deaktivierung der... |
| KL_BCmE_Livetip_Freigabe | 24|1@LE,u | 1.0 | 0.0 |  | Status der Klimaanlage. 
Zeigt an, ob de... |
| KL_HYB_ASV_hinten_schliessen_Anf | 25|1@LE,u | 1.0 | 0.0 |  | Anforderung der Klima an das BMS, das Ab... |
| KL_HYB_ASV_vorne_schliessen_Anf | 26|1@LE,u | 1.0 | 0.0 |  | Anforderung der Klima an das BMS, das Ab... |
| KL_ErwVK_Zusatzrelais | 27|1@LE,u | 1.0 | 0.0 |  | Steuerung Zusatzrelais für Erweiterte Vo... |
| KL_Status_Beduftung | 28|1@LE,u | 1.0 | 0.0 |  | Status der Beduftung |
| KL_Status_Ionisator_Front | 29|1@LE,u | 1.0 | 0.0 |  | Status des Ionisator Front |
| KL_STH_Timer_Status | 30|1@LE,u | 1.0 | 0.0 |  | Status des Standheizungstimers (für VW62... |
| KL_ErwVK_Anf | 31|1@LE,u | 1.0 | 0.0 |  | Erweiterte Voko angefordertert / aktiv |
| KL_Innen_Temp | 32|8@LE,u | 0.5 | -50.0 | Unit_DegreCelsi | Innentemperatur

 |
| KL_I_Geblaese_Fond | 40|8@LE,u | 0.25 | 0.0 | Unit_Amper | Stromaufnahme Gebläse Heck |
| KL_Anf_AussenspiegelHzg | 48|1@LE,u | 1.0 | 0.0 |  | Anforderung Außenspiegelheizung seitens ... |
| KL_Heizkreisventilstellung | 49|2@LE,u | 1.0 | 0.0 |  | Bei MQB PHEV: Stellung Heizkreisventil M... |
| KL_FSH_Automatikbetrieb | 51|1@LE,u | 1.0 | 0.0 |  | Ermöglicht eine Unterscheidung von Anfor... |
| KL_SIH_LEH_Verteilung_VL | 52|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der H... |
| KL_SIH_LEL_Verteilung_VL | 55|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der L... |
| KL_SIH_LEH_Verteilung_VR | 58|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der H... |
| KL_SIH_LEL_Verteilung_VR | 61|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der L... |

### 0x671 (1649) - Klima_06
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Climate settings

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| KL_ZZ_Minute | 0|6@LE,u | 1.0 | 0.0 | Unit_Minut | Zielzeittimeranzeige im Display der Funk... |
| KL_ZZ_Status_Timer | 6|2@LE,u | 1.0 | 0.0 |  | Timerstatusanzeige im Display der Funkfe... |
| KL_ZZ_Monat | 8|4@LE,u | 1.0 | 0.0 | Unit_Month | Zielzeittimeranzeige im Display der Funk... |
| KL_ZZ_Stunde | 16|5@LE,u | 1.0 | 0.0 | Unit_Hours | Zielzeittimeranzeige im Display der Funk... |
| KL_ZZ_Betriebsmodus | 21|3@LE,u | 1.0 | 0.0 |  | Betriebsdauerberechnung anhand Aussentem... |
| KL_ZZ_Tag | 24|5@LE,u | 1.0 | 0.0 | Unit_Day | Zielzeittimeranzeige im Display der Funk... |
| KL_Betriebsmodus_FH | 29|3@LE,u | 1.0 | 0.0 |  | Synchronisation des Betriebsmodus zwisch... |
| KL_Stopp_Wiederstart_Anz_01 | 32|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_02 | 33|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_03 | 34|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_04 | 35|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_05 | 36|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_06 | 37|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_07 | 38|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_08 | 39|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_09 | 40|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_Std1 | 41|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |
| KL_Stopp_Wiederstart_Anz_Std2 | 42|1@LE,u | 1.0 | 0.0 |  | Dieses Signal gibt die detaillierte Begr... |

### 0x5A1 (1441) - Klima_13
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Additional climate

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| KL_Temp_Soll_hl | 7|5@LE,u | 0.5 | 15.5 | Unit_DegreCelsi | Eingestellte Solltemperatur hinten links... |
| KL_Temp_Soll_hr | 12|5@LE,u | 0.5 | 15.5 | Unit_DegreCelsi | Eingestellte Solltemperatur hinten recht... |
| KL_Temp_Soll_vl | 17|5@LE,u | 0.5 | 15.5 | Unit_DegreCelsi | Eingestellte Solltemperatur vorne links ... |
| KL_Temp_Soll_vr | 22|5@LE,u | 0.5 | 15.5 | Unit_DegreCelsi | Eingestellte Solltemperatur vorne rechts... |
| KL_SIH_Stufe_hl | 27|3@LE,u | 1.0 | 0.0 |  | Eingestellte Sitzheizungsstufe hinten li... |
| KL_SIH_Stufe_hr | 30|3@LE,u | 1.0 | 0.0 |  | Eingestellte Sitzheizungsstufe hinten re... |
| KL_SIL_Stufe_hl | 33|3@LE,u | 1.0 | 0.0 |  | Eingestellte Sitzlüftungsstufe hinten li... |
| KL_SIL_Stufe_hr | 36|3@LE,u | 1.0 | 0.0 |  | Eingestellte Sitzlüftungsstufe hinten re... |
| KL_SIL_LEL_Verteilung_HR | 39|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der L... |
| KL_SIL_LEL_Verteilung_HL | 42|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der L... |
| KL_SIH_LEH_Verteilung_HR | 45|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der H... |
| KL_PTC_linear_Anf | 48|6@LE,u | 50.0 | 0.0 | Unit_Watt | Kundenanforderung des PTC (linear) |
| KL_SIH_LEH_Verteilung_HL | 54|3@LE,u | 1.0 | 0.0 |  | Vom Kunden eingestellte Aufteilung der H... |
| KL_Frischluftklappe_Status | 57|7@LE,u | 1.0 | 0.0 | Unit_PerCent | Aktuelle Position der Frischluftklappe |


## Vehicle State

### 0x3C0 (960) - Klemmen_Status_01
- DLC: 4 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Ignition/terminal status

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| Klemmen_Status_01_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Für MLB:
'Berechnung siehe Lastenheft 'K... |
| Klemmen_Status_01_BZ | 8|4@LE,u | 1.0 | 0.0 |  | 4bit Botschaftszaehler; wird in jeder Se... |
| RSt_Fahrerhinweise | 12|4@LE,u | 1.0 | 0.0 |  | Über das Signal fordert das BCM Fahrerhi... |
| ZAS_Kl_S | 16|1@LE,u | 1.0 | 0.0 |  | Klemme S: S-Kontakt (Schluessel steckt)
 |
| ZAS_Kl_15 | 17|1@LE,u | 1.0 | 0.0 |  | Klemme 15: Zuendung (SW-Kl.15)
 |
| ZAS_Kl_X | 18|1@LE,u | 1.0 | 0.0 |  | Klemme 75
 |
| ZAS_Kl_50_Startanforderung | 19|1@LE,u | 1.0 | 0.0 |  | Klemme 50: Startwunsch Fahrer
 |
| BCM_Remotestart_Betrieb | 20|1@LE,u | 1.0 | 0.0 |  | Zeigt den Status des RemoteStarts an, d.... |
| ZAS_Kl_Infotainment | 21|1@LE,u | 1.0 | 0.0 |  | Separate Klemmeninformation für das Info... |
| BCM_Remotestart_KL15_Anf | 22|1@LE,u | 1.0 | 0.0 |  | Zeigt den Status der Aktivierung der KL1... |
| BCM_Remotestart_MO_Start | 23|1@LE,u | 1.0 | 0.0 |  | Nachdem das BCM die Remotestart MSG Frei... |
| KST_Warn_P1_ZST_def | 24|1@LE,u | 1.0 | 0.0 |  | Kombi Prio Warnung Klemmensteuerung 
Zün... |
| KST_Warn_P2_ZST_def | 25|1@LE,u | 1.0 | 0.0 |  | Kombi Prio Warnung Klemmensteuerung
Zünd... |
| KST_Fahrerhinweis_1 | 26|1@LE,u | 1.0 | 0.0 |  | Signalisierung Fahrerhinweis 1 der Klemm... |
| KST_Fahrerhinweis_2 | 27|1@LE,u | 1.0 | 0.0 |  | Signalisierung Fahrerhinweis 2 der Klemm... |
| BCM_Ausparken_Betrieb | 28|1@LE,u | 1.0 | 0.0 |  | Anforderung zur erneuten Lampenaufprüfun... |
| KST_Fahrerhinweis_4 | 29|1@LE,u | 1.0 | 0.0 |  | Signalisierung Fahrerhinweis 4 der Klemm... |
| KST_Fahrerhinweis_5 | 30|1@LE,u | 1.0 | 0.0 |  | Anzeigesignal für RemoteStart |
| KST_Fahrerhinweis_6 | 31|1@LE,u | 1.0 | 0.0 |  | Signal aktiviert die Kombi-Anzeigeinstru... |

### 0x0FD (253) - ESP_21
- DLC: 8 bytes
- Source: vw_mqb_2010.dbc
- Purpose: Vehicle speed

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| CHECKSUM | 0|8@LE,u | 1.0 | 0.0 |  |  |
| COUNTER | 8|4@LE,u | 1.0 | 0.0 |  |  |
| BR_Eingriffsmoment | 12|10@LE,u | 1.0 | -509.0 | Unit_NewtoMeter |  |
| ESP_v_Signal | 32|16@LE,u | 0.01 | 0.0 | Unit_KiloMeterPerHour |  |
| ASR_Tastung_passiv | 48|1@LE,u | 1.0 | 0.0 |  |  |
| ESP_Tastung_passiv | 49|1@LE,u | 1.0 | 0.0 |  |  |
| ESP_Systemstatus | 50|1@LE,u | 1.0 | 0.0 |  |  |
| ASR_Schalteingriff | 51|2@LE,u | 1.0 | 0.0 |  |  |
| ESP_Haltebestaetigung | 53|1@LE,u | 1.0 | 0.0 |  |  |
| ESP_QBit_v_Signal | 55|1@LE,u | 1.0 | 0.0 |  |  |
| ABS_Bremsung | 56|1@LE,u | 1.0 | 0.0 |  |  |
| ASR_Anf | 57|1@LE,u | 1.0 | 0.0 |  |  |
| MSR_Anf | 58|1@LE,u | 1.0 | 0.0 |  |  |
| EBV_Eingriff | 59|1@LE,u | 1.0 | 0.0 |  |  |
| EDS_Eingriff | 60|1@LE,u | 1.0 | 0.0 |  |  |
| ESP_Eingriff | 61|1@LE,u | 1.0 | 0.0 |  |  |
| ESP_ASP | 62|1@LE,u | 1.0 | 0.0 |  |  |
| ESP_Anhaltevorgang_ACC_aktiv | 63|1@LE,u | 1.0 | 0.0 |  |  |

### 0x6B2 (1714) - Diagnose_01
- DLC: 8 bytes
- Source: vw_mqb_2010.dbc
- Purpose: Odometer, time

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| DGN_Verlernzaehler | 0|8@LE,u | 1.0 | 0.0 |  |  |
| KBI_Kilometerstand | 8|20@LE,u | 1.0 | 0.0 | Unit_KiloMeter |  |
| UH_Jahr | 28|7@LE,u | 1.0 | 2000.0 | Unit_Year |  |
| UH_Monat | 35|4@LE,u | 1.0 | 0.0 | Unit_Month |  |
| UH_Tag | 39|5@LE,u | 1.0 | 0.0 | Unit_Day |  |
| UH_Stunde | 44|5@LE,u | 1.0 | 0.0 | Unit_Hours |  |
| UH_Minute | 49|6@LE,u | 1.0 | 0.0 | Unit_Minut |  |
| UH_Sekunde | 55|6@LE,u | 1.0 | 0.0 | Unit_Secon |  |
| Kombi_02_alt | 62|1@LE,u | 1.0 | 0.0 |  |  |
| Uhrzeit_01_alt | 63|1@LE,u | 1.0 | 0.0 |  |  |


## Charger

### 0x564 (1380) - LAD_01
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Charger status

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| LAD_01_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Ab MQB und MLBevo:
'Berechnung siehe Las... |
| LAD_01_BZ | 8|4@LE,u | 1.0 | 0.0 |  | freilaufender Botschaftszähler |
| LAD_IstModus | 12|3@LE,u | 1.0 | 0.0 |  | Betriebsmodus des Ladegerätes |
| LAD_AC_Istspannung | 15|9@LE,u | 1.0 | 0.0 | Unit_Volt | Istwert AC-Netzspannung (RMS)  |
| LAD_IstSpannung_HV | 24|10@LE,u | 1.0 | 0.0 | Unit_Volt | Ausgangsspannung Lader |
| LAD_IstStrom_HV | 34|10@LE,u | 0.2 | -102.0 | Unit_Amper | Ausgangsstrom Lader |
| LAD_Status_Spgfreiheit | 44|2@LE,u | 1.0 | 0.0 |  | 0 = Init (ohne Funktion)
1 = Ladegerät H... |
| LAD_Temperatur | 48|8@LE,u | 1.0 | -40.0 | Unit_DegreCelsi | Momentanwert: Temperatur Ladegerät |
| LAD_Verlustleistung | 56|8@LE,u | 20.0 | 0.0 | Unit_Watt | Momentanwert: Verlustleistung Ladegerät |

### 0x67E (1662) - LAD_02
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Charger data

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| LAD_02_CRC | 0|8@LE,u | 1.0 | 0.0 |  | Ab MQB und MLBevo:
'Berechnung siehe Las... |
| LAD_02_BZ | 8|4@LE,u | 1.0 | 0.0 |  | freilaufender Botschaftszähler |
| LAD_Abregelung_Temperatur | 12|1@LE,u | 1.0 | 0.0 |  | Abregelung aufgrund interner Übertempera... |
| LAD_Abregelung_IU_Ein_Aus | 13|1@LE,u | 1.0 | 0.0 |  | Abregelung aufgrund Strom oder Spannung ... |
| LAD_Abregelung_BuchseTemp | 14|1@LE,u | 1.0 | 0.0 |  | Reduzierung aufgrund zu hoher Temperatur... |
| LAD_MaxLadLeistung_HV | 16|9@LE,u | 100.0 | 0.0 | Unit_Watt | Maximale Leistung Ladegerät bezogen auf ... |
| LAD_PRX_Stromlimit | 32|3@LE,u | 1.0 | 0.0 |  | AC-Stromlimit aufgrund der PRX Kabelkodi... |
| LAD_CP_Erkennung | 35|1@LE,u | 1.0 | 0.0 |  | Status Control Pilot Überwachung (Erkenn... |
| LAD_Stecker_Verriegelt | 36|1@LE,u | 1.0 | 0.0 |  | Status Steckerverriegelung (Rücklesekont... |
| LAD_Kuehlbedarf | 38|2@LE,u | 1.0 | 0.0 |  | Kühlbedarf des Ladegerätes |
| LAD_MaxLadLeistung_HV_Offset | 57|2@LE,u | 1.0 | 0.0 |  | Signal kann in Ergänzung zu LAD_MaxLadLe... |
| LAD_Warnzustand | 62|1@LE,u | 1.0 | 0.0 |  | Sammelwarnung Ladegerät
0 keine Warnbedi... |
| LAD_Fehlerzustand | 63|1@LE,u | 1.0 | 0.0 |  | Sammelfehler Ladegerät - kein Laden mögl... |


## Range

### 0x5F5 (1525) - Reichweite_01
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Range estimate

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| RW_Gesamt_Reichweite_Max_Anzeige | 0|11@LE,u | 1.0 | 0.0 |  | Angabe der Maximalen Anzeige der gesamte... |
| RW_Reservewarnung_2_aktiv | 16|2@LE,u | 1.0 | 0.0 |  | Ansteuerung der Reservewarnung im Kombii... |
| RW_RWDS_Lastprofil | 18|11@LE,u | 1.0 | 0.0 | Unit_Amper | Lastprofil zur Bestimmung der notwendige... |
| RW_Gesamt_Reichweite | 29|11@LE,u | 1.0 | 0.0 | Unit_KiloMeter | Reichweite für primär und sekundär Reich... |
| RW_Prim_Reichweitenverbrauch | 40|11@LE,u | 0.1 | 0.0 | Unit_None | Für primäre Reichweitenberechnung zugrun... |
| RW_Prim_Reichweitenv_Einheit | 51|2@LE,u | 1.0 | 0.0 |  | Für primäre  Reichweitenberechnung (KBI_... |
| RW_Primaer_Reichweite | 53|11@LE,u | 1.0 | 0.0 | Unit_KiloMeter | Reichweite der primären Antriebsart |

### 0x5F7 (1527) - Reichweite_02
- DLC: 8 bytes
- Source: MLBevo_Gen2_MLBevo_HCAN_KMatrix_V8.18.01F_20190718_SE.dbc
- Purpose: Range details

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| RW_Tendenz | 0|3@LE,u | 1.0 | 0.0 |  | Signalisierung der RW-Tendenz auf Basis ... |
| RW_Texte | 3|2@LE,u | 1.0 | 0.0 |  | Signal zum Steuern von reichweitenbezoge... |
| RW_Reservewarnung_aktiv | 5|1@LE,u | 1.0 | 0.0 |  | Ansteuerung der Reservewarnung im Kombii... |
| RW_Reichweite_Einheit_Anzeige | 6|1@LE,u | 1.0 | 0.0 |  | Einheit für Primaer-/Sekundaer-/Gesamt- ... |
| RW_Gesamt_Reichweite_Anzeige | 7|11@LE,u | 1.0 | 0.0 |  | Reichweite für primär und sekundär Reich... |
| RW_Primaer_Reichweite_Anzeige | 18|11@LE,u | 1.0 | 0.0 |  | Reichweite der primären Antriebsart zur ... |
| RW_Sekundaer_Reichweite_Anzeige | 29|11@LE,u | 1.0 | 0.0 |  | Reichweite der sekundaeren Antriebsart z... |
| RW_Sekundaer_Reichweite | 40|11@LE,u | 1.0 | 0.0 | Unit_KiloMeter | Reichweite der sekundaeren Antriebsart |
| RW_Sek_Reichweitenv_Einheit | 51|2@LE,u | 1.0 | 0.0 |  | Für sekundäre Reichweitenberechnung (KBI... |
| RW_Sek_Reichweitenverbrauch | 53|11@LE,u | 0.1 | 0.0 | Unit_None | Für sekundäre Reichweitenberechnung zugr... |


## Hybrid Battery

### 0x65C (1628) - BMS_Hybrid_01
- DLC: 8 bytes
- Source: vw_mqb_2010.dbc
- Purpose: Hybrid battery

| Signal | Bits | Scale | Offset | Unit | Description |
|--------|------|-------|--------|------|-------------|
| BMS_HYB_ASV_hinten_Status | 13|1@LE,u | 1.0 | 0.0 |  |  |
| BMS_HYB_ASV_vorne_Status | 14|1@LE,u | 1.0 | 0.0 |  |  |
| BMS_HYB_KD_Fehler | 15|1@LE,u | 1.0 | 0.0 |  |  |
| BMS_HYB_BattFanSpd | 16|4@LE,u | 10.0 | 0.0 | Unit_PerCent |  |
| BMS_HYB_VentilationReq | 20|1@LE,u | 1.0 | 0.0 |  |  |
| BMS_HYB_Spuelbetrieb_Status | 21|1@LE,u | 1.0 | 0.0 |  |  |
| BMS_HYB_Kuehlung_Anf | 22|2@LE,u | 1.0 | 0.0 |  |  |
| BMS_HYB_Temp_vor_Verd | 24|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi |  |
| BMS_HYB_Temp_nach_Verd | 32|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi |  |
| BMS_Temperatur | 40|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi |  |
| BMS_Temperatur_Ansaugluft | 48|8@LE,u | 0.5 | -40.0 | Unit_DegreCelsi |  |
| BMS_IstSpannung_HV | 56|8@LE,u | 1.0 | 100.0 | Unit_Volt |  |

