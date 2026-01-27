#include "VehicleManager.h"
#include "../modules/CanManager.h"

VehicleManager::VehicleManager(CanManager* canMgr)
    : canManager(canMgr)
    , bodyDomain(state, this)
    , batteryDomain(state, this)
    , driveDomain(state, this)
    , climateDomain(state, this)
    , gpsDomain(state, this)
    , rangeDomain(state, this)
    , bapDomain(state, this)
{
}

bool VehicleManager::setup() {
    Serial.println("[VehicleManager] Initializing vehicle domains...");
    
    // Initialize state
    state = VehicleState();
    
    Serial.println("[VehicleManager] Domains initialized:");
    Serial.println("[VehicleManager]   - BodyDomain (0x3D0, 0x3D1, 0x583)");
    Serial.println("[VehicleManager]   - BatteryDomain (0x5CA, 0x59E)");
    Serial.println("[VehicleManager]   - DriveDomain (0x3C0, 0x0FD, 0x6B2)");
    Serial.println("[VehicleManager]   - ClimateDomain (0x66E, 0x5E1)");
    Serial.println("[VehicleManager]   - GpsDomain (0x484, 0x485, 0x486)");
    Serial.println("[VehicleManager]   - RangeDomain (0x5F5, 0x5F7)");
    Serial.println("[VehicleManager]   - BapDomain (0x17332510 BAP RX)");
    
    return true;
}

void VehicleManager::loop() {
    // Periodic statistics logging
    if (millis() - lastLogTime > LOG_INTERVAL) {
        logStatistics();
        lastLogTime = millis();
    }
}

void VehicleManager::onCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended) {
    // Mark activity
    state.markCanActivity();
    
    // Track all CAN IDs for debugging
    trackCanId(canId, dlc);
    
    // Extended frames go to BAP domain
    if (extended) {
        if (bapDomain.handlesCanId(canId)) {
            if (bapDomain.processFrame(canId, data, dlc)) {
                bapFrames++;
            }
        }
        return;  // Don't route extended frames to other domains
    }
    
    // Route to domains based on CAN ID
    bool processed = false;
    
    // Body domain (doors, locks, windows)
    if (bodyDomain.handlesCanId(canId)) {
        processed = bodyDomain.processFrame(canId, data, dlc);
        if (processed) bodyFrames++;
    }
    
    // Battery domain (SOC, voltage, current, charging)
    if (!processed && batteryDomain.handlesCanId(canId)) {
        processed = batteryDomain.processFrame(canId, data, dlc);
        if (processed) batteryFrames++;
    }
    
    // Drive domain (ignition, speed, odometer)
    if (!processed && driveDomain.handlesCanId(canId)) {
        processed = driveDomain.processFrame(canId, data, dlc);
        if (processed) driveFrames++;
    }
    
    // Climate domain (temperatures)
    if (!processed && climateDomain.handlesCanId(canId)) {
        processed = climateDomain.processFrame(canId, data, dlc);
        if (processed) climateFrames++;
    }
    
    // GPS domain (CAN-based GPS from infotainment)
    if (!processed && gpsDomain.handlesCanId(canId)) {
        processed = gpsDomain.processFrame(canId, data, dlc);
        if (processed) gpsFrames++;
    }
    
    // Range domain (range estimation from cluster)
    if (!processed && rangeDomain.handlesCanId(canId)) {
        processed = rangeDomain.processFrame(canId, data, dlc);
        if (processed) rangeFrames++;
    }
    
    if (!processed) {
        unhandledFrames++;
    }
}

void VehicleManager::trackCanId(uint32_t canId, uint8_t dlc) {
    // Check if we've seen this ID before
    for (size_t i = 0; i < numSeenIds; i++) {
        if (seenCanIds[i] == canId) {
            seenIdCounts[i]++;
            return;
        }
    }
    
    // New ID - add it if we have space
    if (numSeenIds < MAX_TRACKED_IDS) {
        seenCanIds[numSeenIds] = canId;
        seenIdCounts[numSeenIds] = 1;
        seenIdDlcs[numSeenIds] = dlc;
        numSeenIds++;
    }
}

bool VehicleManager::sendCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc, bool extended) {
    if (!canManager) {
        Serial.println("[VehicleManager] No CAN manager - cannot send");
        return false;
    }
    
    if (!canManager->isRunning()) {
        Serial.println("[VehicleManager] CAN not running - cannot send");
        return false;
    }
    
    // Build TWAI message
    twai_message_t message;
    message.identifier = canId;
    message.extd = extended ? 1 : 0;
    message.rtr = 0;
    message.data_length_code = dlc;
    memcpy(message.data, data, dlc);
    
    // Send with 100ms timeout
    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(100));
    
    if (result == ESP_OK) {
        Serial.printf("[VehicleManager] TX: ID:0x%03lX [%d]", canId, dlc);
        for (int i = 0; i < dlc; i++) {
            Serial.printf(" %02X", data[i]);
        }
        Serial.println();
        return true;
    } else if (result == ESP_ERR_TIMEOUT) {
        Serial.println("[VehicleManager] TX timeout - no ACK");
        return false;
    } else {
        Serial.printf("[VehicleManager] TX failed: %s\r\n", esp_err_to_name(result));
        return false;
    }
}

void VehicleManager::logStatistics() {
    Serial.println("[VehicleManager] === Vehicle Status ===");
    Serial.printf("[VehicleManager] CAN frames: %lu (body:%lu batt:%lu drv:%lu clim:%lu gps:%lu rng:%lu bap:%lu unhandled:%lu)\r\n", 
        state.canFrameCount, bodyFrames, batteryFrames, driveFrames, climateFrames, gpsFrames, rangeFrames, bapFrames, unhandledFrames);
    
    // Show seen CAN IDs (most important for debugging!)
    Serial.printf("[VehicleManager] Unique CAN IDs seen: %u\r\n", numSeenIds);
    Serial.println("[VehicleManager] ID (DLC) -> count  [expected by domain]");
    for (size_t i = 0; i < numSeenIds && i < 20; i++) {  // Show first 20
        const char* domain = "";
        if (bodyDomain.handlesCanId(seenCanIds[i])) domain = " <-- BODY";
        else if (batteryDomain.handlesCanId(seenCanIds[i])) domain = " <-- BATTERY";
        else if (driveDomain.handlesCanId(seenCanIds[i])) domain = " <-- DRIVE";
        else if (climateDomain.handlesCanId(seenCanIds[i])) domain = " <-- CLIMATE";
        else if (gpsDomain.handlesCanId(seenCanIds[i])) domain = " <-- GPS";
        else if (rangeDomain.handlesCanId(seenCanIds[i])) domain = " <-- RANGE";
        else if (bapDomain.handlesCanId(seenCanIds[i])) domain = " <-- BAP";
        
        Serial.printf("[VehicleManager]   0x%03lX (DLC=%u) -> %lu%s\r\n", 
            seenCanIds[i], seenIdDlcs[i], seenIdCounts[i], domain);
    }
    if (numSeenIds > 20) {
        Serial.printf("[VehicleManager]   ... and %u more IDs\r\n", numSeenIds - 20);
    }
    
    Serial.printf("[VehicleManager] Vehicle awake: %s\r\n", state.isAwake() ? "YES" : "NO");
    
    // Body status
    const BodyState& body = state.body;
    Serial.printf("[VehicleManager] Lock: %s (raw: b2=0x%02X b7=0x%02X)\r\n", 
        body.centralLock == LockState::LOCKED ? "LOCKED" :
        body.centralLock == LockState::UNLOCKED ? "UNLOCKED" : "UNKNOWN",
        body.zv02_byte2, body.zv02_byte7);
    
    Serial.printf("[VehicleManager] Driver door: %s %s, window: %.0f%% (updated: %lu)\r\n",
        body.driverDoor.open ? "OPEN" : "closed",
        body.driverDoor.locked ? "locked" : "unlocked",
        body.driverDoor.windowPercent(),
        body.driverDoor.lastUpdate);
    
    Serial.printf("[VehicleManager] Passenger door: %s %s, window: %.0f%% (updated: %lu)\r\n",
        body.passengerDoor.open ? "OPEN" : "closed",
        body.passengerDoor.locked ? "locked" : "unlocked",
        body.passengerDoor.windowPercent(),
        body.passengerDoor.lastUpdate);
    
    // Battery status
    const BatteryState& batt = state.battery;
    uint32_t bms07, bms06;
    batteryDomain.getFrameCounts(bms07, bms06);
    Serial.printf("[VehicleManager] Battery frames: 0x5CA:%lu 0x59E:%lu\r\n", bms07, bms06);
    Serial.printf("[VehicleManager] Battery: energy=%.0f/%.0fWh (%.0f%%) temp=%.1f°C\r\n",
        batt.energyWh, batt.maxEnergyWh, 
        batt.maxEnergyWh > 0 ? (batt.energyWh / batt.maxEnergyWh * 100.0f) : 0.0f,
        batt.temperature);
    Serial.printf("[VehicleManager] Charging: %s, Balancing: %s\r\n",
        batt.chargingActive ? "YES" : "no",
        batt.balancingActive ? "YES" : "no");
    
    // Drive status
    const DriveState& drv = state.drive;
    const char* ignStr;
    switch (drv.ignition) {
        case IgnitionState::OFF: ignStr = "OFF"; break;
        case IgnitionState::ACCESSORY: ignStr = "ACCESSORY"; break;
        case IgnitionState::ON: ignStr = "ON"; break;
        case IgnitionState::START: ignStr = "START"; break;
        default: ignStr = "UNKNOWN"; break;
    }
    Serial.printf("[VehicleManager] Ignition: %s (key:%d ign:%d start:%d) (updated: %lu)\r\n", 
        ignStr, drv.keyInserted, drv.ignitionOn, drv.startRequested, drv.ignitionUpdate);
    Serial.printf("[VehicleManager] Speed: %.1f km/h (updated: %lu)\r\n", drv.speedKmh, drv.speedUpdate);
    Serial.printf("[VehicleManager] Odometer: %lu km (updated: %lu)\r\n", drv.odometerKm, drv.odometerUpdate);
    Serial.printf("[VehicleManager] Vehicle time: %04u-%02u-%02u %02u:%02u:%02u\r\n",
        drv.year, drv.month, drv.day, drv.hour, drv.minute, drv.second);
    
    // Climate status
    const ClimateState& clim = state.climate;
    Serial.printf("[VehicleManager] Temps: inside=%.1f°C outside=%.1f°C\r\n", 
        clim.insideTemp, clim.outsideTemp);
    Serial.printf("[VehicleManager] Standby heat:%d vent:%d (updated: %lu)\r\n", 
        clim.standbyHeatingActive, clim.standbyVentActive, clim.klimaUpdate);
    
    // GPS status
    const CanGpsState& gps = state.gps;
    Serial.printf("[VehicleManager] GPS: %s (%d sats, HDOP: %.1f)\r\n",
        gps.fixTypeStr(), gps.satellites, gps.hdop);
    if (gps.hasFix()) {
        Serial.printf("[VehicleManager] Position: %.6f, %.6f alt:%.0fm heading:%.1f°\r\n",
            gps.latitude, gps.longitude, gps.altitude, gps.heading);
    }
    
    // Range status
    const RangeState& rng = state.range;
    if (rng.isValid()) {
        Serial.printf("[VehicleManager] Range: %d km (electric: %d km) %s\r\n",
            rng.totalRangeKm, rng.electricRangeKm, rng.tendencyStr());
        Serial.printf("[VehicleManager] Consumption: %.1f %s, Reserve:%s\r\n",
            rng.consumption, rng.consumptionUnit == 0 ? "kWh/100km" : "km/kWh", 
            rng.reserveWarning ? "YES" : "no");
    }
    
    // BAP status (from passive listening to BAP protocol)
    const BapPlugState& plug = state.bapPlug;
    const BapChargeState& charge = state.bapCharge;
    const BapClimateState& bapClim = state.bapClimate;
    if (plug.isValid() || charge.isValid() || bapClim.isValid()) {
        Serial.printf("[VehicleManager] BAP Plug: %s (supply:%s lock:%d)\r\n",
            plug.plugStateStr(),
            plug.hasSupply() ? "yes" : "no",
            plug.lockState);
        Serial.printf("[VehicleManager] BAP Charge: SOC=%d%% mode=%s status=%s time=%dmin\r\n",
            charge.socPercent,
            charge.chargeModeStr(),
            charge.chargeStatusStr(),
            charge.remainingTimeMin);
        if (bapClim.isValid()) {
            Serial.printf("[VehicleManager] BAP Climate: %s (heat:%d cool:%d temp:%.1f°C)\r\n",
                bapClim.climateActive ? "ACTIVE" : "off",
                bapClim.heating, bapClim.cooling, bapClim.currentTempC);
        }
    }
    
    Serial.println("[VehicleManager] ======================");
}
