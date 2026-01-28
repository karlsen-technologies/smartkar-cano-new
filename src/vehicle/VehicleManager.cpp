#include "VehicleManager.h"
#include "../modules/CanManager.h"
#include "protocols/BapProtocol.h"

VehicleManager::VehicleManager(CanManager *canMgr)
    : canManager(canMgr), bodyDomain(state, this), batteryDomain(state, this), driveDomain(state, this), climateDomain(state, this), gpsDomain(state, this), rangeDomain(state, this), bapDomain(state, this), profileManager(this)
{
}

VehicleManager::~VehicleManager()
{
}

bool VehicleManager::setup()
{
    Serial.println("[VehicleManager] Initializing vehicle domains...");

    // Initialize state
    state = VehicleState();

    Serial.println("[VehicleManager] Domains initialized:");
    Serial.println("[VehicleManager]   - BodyDomain (0x3D0, 0x3D1, 0x583)");
    Serial.println("[VehicleManager]   - BatteryDomain (0x5CA, 0x59E, 0x483)");
    Serial.println("[VehicleManager]   - DriveDomain (0x3C0, 0x0FD, 0x6B2)");
    Serial.println("[VehicleManager]   - ClimateDomain (0x66E, 0x5E1)");
    Serial.println("[VehicleManager]   - GpsDomain (0x484, 0x485, 0x486)");
    Serial.println("[VehicleManager]   - RangeDomain (0x5F5, 0x5F7)");
    Serial.println("[VehicleManager]   - BapDomain (0x17332510 BAP RX)");
    Serial.println("[VehicleManager]   - ChargingProfileManager (high-level charging/climate API)");
    Serial.println("[VehicleManager] Thread-safe state access enabled (CAN task on Core 0)");

    return true;
}

void VehicleManager::loop()
{
    // Periodic statistics logging (from main loop on Core 1)
    if (millis() - lastLogTime > LOG_INTERVAL)
    {
        logStatistics();
        lastLogTime = millis();
    }
}

// =============================================================================
// State access (lock-free - may read slightly stale data)
// =============================================================================

VehicleState VehicleManager::getStateCopy()
{
    // Direct copy - CAN task may be updating, but individual fields are atomic
    return state;
}

bool VehicleManager::isVehicleAwake()
{
    return state.isAwake();
}

uint32_t VehicleManager::getFrameCount()
{
    return state.canFrameCount;
}

// =============================================================================
// CAN Frame Processing (called from CAN task on Core 0)
// =============================================================================

void VehicleManager::onCanFrame(uint32_t canId, const uint8_t *data, uint8_t dlc, bool extended)
{
    // Count every frame
    state.markCanActivity();

    // Extended frames: only BAP
    if (extended)
    {
        if (canId == BapProtocol::CAN_ID_BATTERY_RX && bapDomain.processFrame(canId, data, dlc))
        {
            bapFrames++;
        }
        else
        {
            unhandledFrames++;
        }
        return;
    }

    // Standard frames: O(1) switch routing
    switch (canId)
    {
    case 0x0FD: case 0x3C0: case 0x6B2:
        if (driveDomain.processFrame(canId, data, dlc)) driveFrames++;
        else unhandledFrames++;
        break;
    case 0x3D0: case 0x3D1: case 0x583:
        if (bodyDomain.processFrame(canId, data, dlc)) bodyFrames++;
        else unhandledFrames++;
        break;
    case 0x484: case 0x485: case 0x486:
        if (gpsDomain.processFrame(canId, data, dlc)) gpsFrames++;
        else unhandledFrames++;
        break;
    case 0x483: case 0x59E: case 0x5CA:
        if (batteryDomain.processFrame(canId, data, dlc)) batteryFrames++;
        else unhandledFrames++;
        break;
    case 0x5E1: case 0x66E:
        if (climateDomain.processFrame(canId, data, dlc)) climateFrames++;
        else unhandledFrames++;
        break;
    case 0x5F5: case 0x5F7:
        if (rangeDomain.processFrame(canId, data, dlc)) rangeFrames++;
        else unhandledFrames++;
        break;
    default:
        unhandledFrames++;
        break;
    }
}

bool VehicleManager::sendCanFrame(uint32_t canId, const uint8_t *data, uint8_t dlc, bool extended)
{
    if (!canManager)
    {
        Serial.println("[VehicleManager] No CAN manager - cannot send");
        return false;
    }

    if (!canManager->isRunning())
    {
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

    if (result == ESP_OK)
    {
        Serial.printf("[VehicleManager] TX: ID:0x%03lX [%d]", canId, dlc);
        for (int i = 0; i < dlc; i++)
        {
            Serial.printf(" %02X", data[i]);
        }
        Serial.println();
        return true;
    }
    else if (result == ESP_ERR_TIMEOUT)
    {
        Serial.println("[VehicleManager] TX timeout - no ACK");
        return false;
    }
    else
    {
        Serial.printf("[VehicleManager] TX failed: %s\r\n", esp_err_to_name(result));
        return false;
    }
}

void VehicleManager::logStatistics()
{
    // Get a consistent snapshot of state
    VehicleState snapshot = getStateCopy();

    // Get CanManager's count for comparison
    uint32_t canMgrCount = canManager ? canManager->getMessageCount() : 0;
    uint32_t canMgrMissed = canManager ? canManager->getMissedCount() : 0;

    Serial.println("[VehicleManager] === Vehicle Status ===");

    // Frame loss analysis
    uint32_t processedByDomains = bodyFrames + batteryFrames + driveFrames + climateFrames + gpsFrames + rangeFrames + bapFrames + unhandledFrames;
    Serial.printf("[VehicleManager] CanManager received: %lu (TWAI missed: %lu)\r\n", canMgrCount, canMgrMissed);
    Serial.printf("[VehicleManager] VehicleManager processed: %lu (domains: %lu)\r\n", snapshot.canFrameCount, processedByDomains);
    if (canMgrCount > snapshot.canFrameCount)
    {
        Serial.printf("[VehicleManager] FRAME LOSS: %lu frames lost between CanManager and VehicleManager (mutex timeout?)\r\n",
                      canMgrCount - snapshot.canFrameCount);
    }

    Serial.printf("[VehicleManager] Domain breakdown: body:%lu batt:%lu drv:%lu clim:%lu gps:%lu rng:%lu bap:%lu unhandled:%lu\r\n",
                  bodyFrames, batteryFrames, driveFrames, climateFrames, gpsFrames, rangeFrames, bapFrames, unhandledFrames);

    Serial.printf("[VehicleManager] Vehicle awake: %s\r\n", snapshot.isAwake() ? "YES" : "NO");

    // Body status
    const BodyState &body = snapshot.body;
    Serial.printf("[VehicleManager] Lock: %s (raw: b2=0x%02X b7=0x%02X)\r\n",
                  body.centralLock == LockState::LOCKED ? "LOCKED" : body.centralLock == LockState::UNLOCKED ? "UNLOCKED"
                                                                                                             : "UNKNOWN",
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
    const BatteryState &batt = snapshot.battery;
    uint32_t bms07, bms06, motorHybrid06;
    batteryDomain.getFrameCounts(bms07, bms06, motorHybrid06);
    Serial.printf("[VehicleManager] Battery frames: 0x5CA:%lu 0x59E:%lu 0x483:%lu\r\n", bms07, bms06, motorHybrid06);
    Serial.printf("[VehicleManager] Battery: energy=%.0f/%.0fWh (%.0f%%) temp=%.1f°C power=%.2fkW\r\n",
                  batt.energyWh, batt.maxEnergyWh,
                  batt.maxEnergyWh > 0 ? (batt.energyWh / batt.maxEnergyWh * 100.0f) : 0.0f,
                  batt.temperature, batt.powerKw);
    Serial.printf("[VehicleManager] Charging: %s, Balancing: %s\r\n",
                  batt.chargingActive ? "YES" : "no",
                  batt.balancingActive ? "YES" : "no");

    // Drive status
    const DriveState &drv = snapshot.drive;
    const char *ignStr;
    switch (drv.ignition)
    {
    case IgnitionState::OFF:
        ignStr = "OFF";
        break;
    case IgnitionState::ACCESSORY:
        ignStr = "ACCESSORY";
        break;
    case IgnitionState::ON:
        ignStr = "ON";
        break;
    case IgnitionState::START:
        ignStr = "START";
        break;
    default:
        ignStr = "UNKNOWN";
        break;
    }
    Serial.printf("[VehicleManager] Ignition: %s (key:%d ign:%d start:%d) (updated: %lu)\r\n",
                  ignStr, drv.keyInserted, drv.ignitionOn, drv.startRequested, drv.ignitionUpdate);
    Serial.printf("[VehicleManager] Speed: %.1f km/h (updated: %lu)\r\n", drv.speedKmh, drv.speedUpdate);
    Serial.printf("[VehicleManager] Odometer: %lu km (updated: %lu)\r\n", drv.odometerKm, drv.odometerUpdate);
    Serial.printf("[VehicleManager] Vehicle time: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                  drv.year, drv.month, drv.day, drv.hour, drv.minute, drv.second);

    // Climate status
    const ClimateState &clim = snapshot.climate;
    Serial.printf("[VehicleManager] Temps: inside=%.1f°C outside=%.1f°C\r\n",
                  clim.insideTemp, clim.outsideTemp);
    Serial.printf("[VehicleManager] Standby heat:%d vent:%d (updated: %lu)\r\n",
                  clim.standbyHeatingActive, clim.standbyVentActive, clim.klimaUpdate);

    // GPS status
    const CanGpsState &gps = snapshot.gps;
    Serial.printf("[VehicleManager] GPS: %s (%d sats, HDOP: %.1f)\r\n",
                  gps.fixTypeStr(), gps.satellites, gps.hdop);
    if (gps.hasFix())
    {
        Serial.printf("[VehicleManager] Position: %.6f, %.6f alt:%.0fm heading:%.1f°\r\n",
                      gps.latitude, gps.longitude, gps.altitude, gps.heading);
    }

    // Range status
    const RangeState &rng = snapshot.range;
    if (rng.isValid())
    {
        Serial.printf("[VehicleManager] Range: %d km (electric: %d km) %s\r\n",
                      rng.totalRangeKm, rng.electricRangeKm, rng.tendencyStr());
        Serial.printf("[VehicleManager] Consumption: %.1f %s, Reserve:%s\r\n",
                      rng.consumption, rng.consumptionUnit == 0 ? "kWh/100km" : "km/kWh",
                      rng.reserveWarning ? "YES" : "no");
    }

    // BAP assembler debug stats
    uint32_t shortMsgs, longMsgs, longStarts, longConts, contErrors, pendingOverflows, staleReplacements;
    uint8_t pendingCount, maxPendingCount;
    bapDomain.getAssemblerStats(shortMsgs, longMsgs, longStarts, longConts, contErrors, pendingOverflows, staleReplacements, pendingCount, maxPendingCount);
    Serial.printf("[VehicleManager] BAP Assembler: short=%lu long=%lu (starts=%lu conts=%lu) errors: cont=%lu overflow=%lu stale=%lu pending=%u max=%u\r\n",
        shortMsgs, longMsgs, longStarts, longConts, contErrors, pendingOverflows, staleReplacements, pendingCount, maxPendingCount);

    // BAP status
    const BapPlugState &plug = snapshot.bapPlug;
    const BapChargeState &charge = snapshot.bapCharge;
    const BapClimateState &bapClim = snapshot.bapClimate;
    if (plug.isValid() || charge.isValid() || bapClim.isValid())
    {
        Serial.printf("[VehicleManager] BAP Plug: %s (supply:%s lock:%d)\r\n",
                      plug.plugStateStr(),
                      plug.hasSupply() ? "yes" : "no",
                      plug.lockState);
        Serial.printf("[VehicleManager] BAP Charge: SOC=%d%% mode=%s status=%s time=%dmin\r\n",
                      charge.socPercent,
                      charge.chargeModeStr(),
                      charge.chargeStatusStr(),
                      charge.remainingTimeMin);
        if (bapClim.isValid())
        {
            Serial.printf("[VehicleManager] BAP Climate: %s (heat:%d cool:%d temp:%.1f°C)\r\n",
                          bapClim.climateActive ? "ACTIVE" : "off",
                          bapClim.heating, bapClim.cooling, bapClim.currentTempC);
        }
    }

    Serial.println("[VehicleManager] ======================");
}
