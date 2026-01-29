#include "VehicleManager.h"
#include "../modules/CanManager.h"
#include "protocols/BapProtocol.h"

VehicleManager::VehicleManager(CanManager *canMgr)
    : canManager(canMgr), bodyDomain(state, this), batteryDomain(state, this), driveDomain(state, this), climateDomain(state, this), gpsDomain(state, this), rangeDomain(state, this), batteryControlChannel(state, this), profileManager(this)
{
    // Create mutex for thread-safe state access (Core 0 CAN task + Core 1 main loop)
    stateMutex = xSemaphoreCreateMutex();
    if (stateMutex == nullptr)
    {
        Serial.println("[VehicleManager] CRITICAL: Failed to create state mutex!");
    }

    // Register BAP channels with router
    bapRouter.registerChannel(&batteryControlChannel);
}

VehicleManager::~VehicleManager()
{
    if (stateMutex != nullptr)
    {
        vSemaphoreDelete(stateMutex);
        stateMutex = nullptr;
    }
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
    Serial.println("[VehicleManager]   - BapChannelRouter (BAP frame routing)");
    Serial.println("[VehicleManager]   - BatteryControlChannel (0x17332510 BAP RX)");
    Serial.println("[VehicleManager]   - Wake State Machine (integrated)");
    Serial.println("[VehicleManager]   - ChargingProfileManager (high-level charging/climate API)");
    Serial.println("[VehicleManager] Thread-safe state access enabled (CAN task on Core 0)");

    return true;
}

void VehicleManager::loop()
{
    // Clear initialization flag after first loop iteration
    // This allows CAN activity tracking after startup is complete
    if (canInitializing) {
        canInitializing = false;
    }
    
    // Update wake state machine (from main loop on Core 1)
    updateWakeStateMachine();

    // Update channel command state machines
    batteryControlChannel.loop();
    
    // Update profile manager state machine
    profileManager.loop();

    // Periodic statistics logging (from main loop on Core 1)
    if (millis() - lastLogTime > LOG_INTERVAL)
    {
        logStatistics();
        lastLogTime = millis();
    }
}

// =============================================================================
// State access (thread-safe via mutex)
// =============================================================================

VehicleState VehicleManager::getStateCopy()
{
    // Acquire mutex with 100ms timeout
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        VehicleState copy = state;
        xSemaphoreGive(stateMutex);
        return copy;
    }

    // Timeout - return potentially stale data
    Serial.println("[VehicleManager] WARNING: getStateCopy() mutex timeout, returning stale data");
    return state;
}

bool VehicleManager::isVehicleAwake()
{
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bool awake = state.isAwake();
        xSemaphoreGive(stateMutex);
        return awake;
    }

    // Timeout - return potentially stale data
    return state.isAwake();
}

uint32_t VehicleManager::getFrameCount()
{
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        uint32_t count = state.canFrameCount;
        xSemaphoreGive(stateMutex);
        return count;
    }

    // Timeout - return potentially stale data
    return state.canFrameCount;
}

// =============================================================================
// CAN Frame Processing (called from CAN task on Core 0)
// =============================================================================

void VehicleManager::onCanFrame(uint32_t canId, const uint8_t *data, uint8_t dlc, bool extended)
{
    // Acquire mutex with 10ms timeout (CAN task runs at high priority)
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(10)) != pdTRUE)
    {
        // Mutex timeout - skip this frame to avoid blocking CAN task
        // This should be rare; indicates main loop is holding mutex too long
        unhandledFrames++;
        return;
    }

    // Count every frame, but only mark activity after initialization
    // This prevents spurious "vehicle awake" detection during CAN startup
    if (!canInitializing) {
        state.markCanActivity();
    }

    // Extended frames: only BAP
    if (extended)
    {
        // Route all BAP frames (0x1733xxxx range) to BAP router
        if ((canId & 0xFFFF0000) == 0x17330000)
        {
            if (bapRouter.processFrame(canId, data, dlc))
            {
                bapFrames++;
            }
            else
            {
                unhandledFrames++;
            }
        }
        else
        {
            unhandledFrames++;
        }
        xSemaphoreGive(stateMutex);
        return;
    }

    // Standard frames: O(1) switch routing
    switch (canId)
    {
    case 0x0FD:
    case 0x3C0:
    case 0x6B2:
        if (driveDomain.processFrame(canId, data, dlc))
            driveFrames++;
        else
            unhandledFrames++;
        break;
    case 0x3D0:
    case 0x3D1:
    case 0x583:
        if (bodyDomain.processFrame(canId, data, dlc))
            bodyFrames++;
        else
            unhandledFrames++;
        break;
    case 0x484:
    case 0x485:
    case 0x486:
        if (gpsDomain.processFrame(canId, data, dlc))
            gpsFrames++;
        else
            unhandledFrames++;
        break;
    case 0x483:
    case 0x59E:
    case 0x5CA:
        if (batteryDomain.processFrame(canId, data, dlc))
            batteryFrames++;
        else
            unhandledFrames++;
        break;
    case 0x5E1:
    case 0x66E:
        if (climateDomain.processFrame(canId, data, dlc))
            climateFrames++;
        else
            unhandledFrames++;
        break;
    case 0x5F5:
    case 0x5F7:
        if (rangeDomain.processFrame(canId, data, dlc))
            rangeFrames++;
        else
            unhandledFrames++;
        break;
    default:
        unhandledFrames++;
        break;
    }

    xSemaphoreGive(stateMutex);
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
        // Debug logging (disabled - uncomment for CAN frame debugging)
        // Serial.printf("[VehicleManager] TX: ID:0x%03lX [%d]", canId, dlc);
        // for (int i = 0; i < dlc; i++)
        // {
        //     Serial.printf(" %02X", data[i]);
        // }
        // Serial.println();
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
    Serial.printf("[VehicleManager] Charging: %s (source:%s), Balancing: %s\r\n",
                  batt.charging ? "YES" : "no",
                  batt.chargingSource == DataSource::BAP ? "BAP" : batt.chargingSource == DataSource::CAN_STD ? "CAN"
                                                                                                              : "none",
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
    Serial.printf("[VehicleManager] Temps: inside=%.1f°C (source:%s) outside=%.1f°C\r\n",
                  clim.insideTemp,
                  clim.insideTempSource == DataSource::BAP ? "BAP" : "CAN",
                  clim.outsideTemp);
    Serial.printf("[VehicleManager] Climate: %s (source:%s) heat:%d cool:%d vent:%d defrost:%d time:%dmin\r\n",
                  clim.climateActive ? "ACTIVE" : "off",
                  clim.climateActiveSource == DataSource::BAP ? "BAP" : "CAN",
                  clim.heating, clim.cooling, clim.ventilation, clim.autoDefrost, clim.climateTimeMin);

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
    bapRouter.getAssemblerStats(shortMsgs, longMsgs, longStarts, longConts, contErrors, pendingOverflows, staleReplacements, pendingCount, maxPendingCount);
    Serial.printf("[VehicleManager] BAP Assembler: short=%lu long=%lu (starts=%lu conts=%lu) errors: cont=%lu overflow=%lu stale=%lu pending=%u max=%u\r\n",
                  shortMsgs, longMsgs, longStarts, longConts, contErrors, pendingOverflows, staleReplacements, pendingCount, maxPendingCount);

    // BAP command statistics
    uint32_t cmdQueued, cmdCompleted, cmdFailed;
    batteryControlChannel.getCommandStats(cmdQueued, cmdCompleted, cmdFailed);
    Serial.printf("[VehicleManager] BAP Commands: queued=%lu completed=%lu failed=%lu (success rate: %.1f%%)\r\n",
                  cmdQueued, cmdCompleted, cmdFailed,
                  cmdQueued > 0 ? (cmdCompleted * 100.0f / cmdQueued) : 100.0f);

    // BAP status (consolidated into main state)
    const PlugState &plug = snapshot.plug;
    const BatteryState &batt2 = snapshot.battery;
    const ClimateState &clim2 = snapshot.climate;

    if (plug.isValid())
    {
        Serial.printf("[VehicleManager] BAP Plug: %s (supply:%s lock:%d)\r\n",
                      plug.plugStateStr(),
                      plug.hasSupply() ? "yes" : "no",
                      plug.lockState);
    }

    if (batt2.socSource == DataSource::BAP || batt2.chargingDetailsUpdate > 0)
    {
        Serial.printf("[VehicleManager] BAP Charge: SOC=%.0f%% mode=%d status=%d amps=%d target=%d%% time=%dmin\r\n",
                      batt2.soc,
                      batt2.chargingMode,
                      batt2.chargingStatus,
                      batt2.chargingAmps,
                      batt2.targetSoc,
                      batt2.remainingTimeMin);
    }

    // BAP Climate detail (only show when BAP is source)
    if (clim2.climateActiveSource == DataSource::BAP && clim2.climateActive)
    {
        Serial.printf("[VehicleManager] BAP Climate Detail: heat:%d cool:%d vent:%d defrost:%d temp:%.1f°C time:%dmin\r\n",
                      clim2.heating, clim2.cooling, clim2.ventilation, clim2.autoDefrost,
                      clim2.insideTemp, clim2.climateTimeMin);
    }

    Serial.println("[VehicleManager] ======================");
}

// =============================================================================
// Wake State Machine
// =============================================================================

bool VehicleManager::requestWake()
{
    // If already waking or awake, don't restart
    if (wakeState == WakeState::WAKING || wakeState == WakeState::AWAKE)
    {
        Serial.printf("[VehicleManager] Wake already in progress/complete (state=%s)\r\n",
                      getWakeStateName());
        return true;
    }

    // If vehicle already awake (via CAN activity), skip wake sequence
    // Thread-safe check with mutex
    bool vehicleAwake = false;
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        vehicleAwake = state.isAwake();
        xSemaphoreGive(stateMutex);
    }

    if (vehicleAwake)
    {
        Serial.println("[VehicleManager] Vehicle already awake (CAN active)");
        setWakeState(WakeState::AWAKE);
        startKeepAlive();
        return true;
    }

    // Request wake
    Serial.println("[VehicleManager] Wake requested");
    setWakeState(WakeState::WAKE_REQUESTED);
    lastWakeActivity = millis();
    wakeAttempts++;

    return true;
}

bool VehicleManager::isAwake() const
{
    return wakeState == WakeState::AWAKE;
}

const char *VehicleManager::getWakeStateName() const
{
    switch (wakeState)
    {
    case WakeState::ASLEEP:
        return "ASLEEP";
    case WakeState::WAKE_REQUESTED:
        return "WAKE_REQUESTED";
    case WakeState::WAKING:
        return "WAKING";
    case WakeState::AWAKE:
        return "AWAKE";
    default:
        return "UNKNOWN";
    }
}

void VehicleManager::updateWakeStateMachine()
{
    unsigned long now = millis();
    unsigned long elapsed = now - wakeStateStartTime;

    // Always track vehicle state based on CAN activity (thread-safe read)
    bool vehicleHasCanActivity = false;
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        vehicleHasCanActivity = state.isAwake();
        xSemaphoreGive(stateMutex);
    }

    // Update keep-alive management
    if (keepAliveActive)
    {
        // Stop keep-alive after timeout (no commands for 5 minutes)
        if (now - lastWakeActivity > KEEPALIVE_TIMEOUT)
        {
            Serial.println("[VehicleManager] Keep-alive timeout (5 min since last command)");
            stopKeepAlive();
            // Don't force ASLEEP - let vehicle naturally go to sleep
        }

        // Send keep-alive frame every 500ms while active
        if (now - lastKeepAlive >= KEEPALIVE_INTERVAL)
        {
            sendKeepAliveFrame();
            lastKeepAlive = now;
        }
    }

    // Process state transitions
    switch (wakeState)
    {
    case WakeState::ASLEEP:
        // Check if vehicle woke up naturally (door open, key fob, etc.)
        if (vehicleHasCanActivity)
        {
            Serial.println("[VehicleManager] Vehicle woke up (CAN activity detected)");
            setWakeState(WakeState::AWAKE);
        }
        break;

    case WakeState::WAKE_REQUESTED:
        // Send wake sequence
        Serial.println("[VehicleManager] Initiating wake sequence...");

        // Send wake frame
        if (!sendWakeFrame())
        {
            Serial.println("[VehicleManager] Failed to send wake frame");
            setWakeState(WakeState::ASLEEP);
            wakeFailed++;
            break;
        }

        // Start keep-alive immediately
        startKeepAlive();

        // Small delay before BAP init
        delay(100);

        // Send BAP init
        if (!sendBapInitFrame())
        {
            Serial.println("[VehicleManager] Failed to send BAP init frame");
            stopKeepAlive();
            setWakeState(WakeState::ASLEEP);
            wakeFailed++;
            break;
        }

        // Transition to WAKING
        setWakeState(WakeState::WAKING);
        break;

    case WakeState::WAKING:
        // Check if vehicle has woken up (CAN activity)
        if (vehicleHasCanActivity)
        {
            // Wait for BAP initialization
            if (elapsed >= BAP_INIT_WAIT)
            {
                Serial.printf("[VehicleManager] Vehicle awake after %lums\r\n", elapsed);
                setWakeState(WakeState::AWAKE);
            }
        }
        // Check for wake timeout
        else if (elapsed > WAKE_TIMEOUT)
        {
            Serial.println("[VehicleManager] Wake timeout - no CAN activity");
            stopKeepAlive();
            setWakeState(WakeState::ASLEEP);
            wakeFailed++;
        }
        break;

    case WakeState::AWAKE:
        // Vehicle went to sleep naturally (no CAN activity)
        if (!vehicleHasCanActivity)
        {
            Serial.println("[VehicleManager] Vehicle went to sleep (no CAN activity)");
            stopKeepAlive();
            setWakeState(WakeState::ASLEEP);
        }
        break;
    }
}

bool VehicleManager::sendWakeFrame()
{
    uint8_t wakeData[4] = {0x40, 0x00, 0x01, 0x1F};
    Serial.println("[VehicleManager] Sending wake frame (0x17330301)");
    return sendCanFrame(CAN_ID_WAKE, wakeData, 4, true);
}

bool VehicleManager::sendBapInitFrame()
{
    uint8_t bapInitData[8] = {0x67, 0x10, 0x41, 0x84, 0x14, 0x00, 0x00, 0x00};
    Serial.println("[VehicleManager] Sending BAP init frame (0x1B000067)");
    return sendCanFrame(CAN_ID_BAP_INIT, bapInitData, 8, true);
}

bool VehicleManager::sendKeepAliveFrame()
{
    uint8_t keepAliveData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (sendCanFrame(CAN_ID_KEEPALIVE, keepAliveData, 8, false))
    {
        keepAlivesSent++;
        return true;
    }

    Serial.println("[VehicleManager] Failed to send keep-alive frame");
    return false;
}

void VehicleManager::startKeepAlive()
{
    if (!keepAliveActive)
    {
        Serial.println("[VehicleManager] Starting keep-alive (500ms interval)");
        keepAliveActive = true;
        lastKeepAlive = millis();
        lastWakeActivity = millis();

        // Send first keep-alive immediately
        sendKeepAliveFrame();
    }
}

void VehicleManager::stopKeepAlive()
{
    if (keepAliveActive)
    {
        Serial.println("[VehicleManager] Stopping keep-alive");
        keepAliveActive = false;
    }
}

void VehicleManager::setWakeState(WakeState newState)
{
    if (wakeState != newState)
    {
        Serial.printf("[VehicleManager] Wake: %s -> %s\r\n",
                      getWakeStateName(),
                      [this, newState]()
                      {
                          WakeState temp = wakeState;
                          wakeState = newState;
                          const char *name = getWakeStateName();
                          wakeState = temp;
                          return name;
                      }());
        wakeState = newState;
        wakeStateStartTime = millis();
    }
}
