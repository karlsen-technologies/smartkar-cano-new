#include "VehicleManager.h"
#include "../modules/CanManager.h"
#include "protocols/BapProtocol.h"

VehicleManager::VehicleManager(CanManager *canMgr)
    : canManager(canMgr), 
      batteryControlChannel(this), 
      profileManager(this), 
      batteryManager(this), 
      climateManager(this), 
      bodyManager(this), 
      driveManager(this), 
      gpsManager(this), 
      rangeManager(this),
      wakeController(canMgr)
{
    // Create mutex for thread-safe state access (Core 0 CAN task + Core 1 main loop)
    stateMutex = xSemaphoreCreateMutex();
    if (stateMutex == nullptr)
    {
        Serial.println("[VehicleManager] CRITICAL: Failed to create state mutex!");
    }
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
    
    // Initialize services
    Serial.println("[VehicleManager] Initializing services...");
    activityTracker.setup();
    wakeController.setup();
    
    // Initialize domain managers
    Serial.println("[VehicleManager] === Domain Manager Initialization ===");
    batteryManager.setup();
    climateManager.setup();
    bodyManager.setup();
    driveManager.setup();
    gpsManager.setup();
    rangeManager.setup();
    Serial.println("[VehicleManager] === All Managers Initialized ===");

    Serial.println("[VehicleManager] Domain managers initialized:");
    Serial.println("[VehicleManager]   - BatteryManager (0x5CA, 0x59E, 0x483 + BAP)");
    Serial.println("[VehicleManager]   - ClimateManager (0x66E, 0x5E1 + BAP)");
    Serial.println("[VehicleManager]   - BodyManager (0x3D0, 0x3D1, 0x583)");
    Serial.println("[VehicleManager]   - DriveManager (0x3C0, 0x0FD, 0x6B2)");
    Serial.println("[VehicleManager]   - GpsManager (0x484, 0x485, 0x486)");
    Serial.println("[VehicleManager]   - RangeManager (0x5F5, 0x5F7)");
    Serial.println("[VehicleManager]   - BatteryControlChannel (0x17332510 BAP RX)");
    Serial.println("[VehicleManager]   - Wake State Machine (integrated)");
    Serial.println("[VehicleManager]   - ChargingProfileManager (high-level charging/climate API)");
    Serial.println("[VehicleManager] Thread-safe state access enabled (CAN task on Core 0)");

    return true;
}

void VehicleManager::loop()
{
    // Update wake state machine using services
    wakeController.loop(activityTracker.isActive());
    
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
// State Access
// =============================================================================

bool VehicleManager::isVehicleAwake()
{
    return activityTracker.isActive(5000);  // Consider awake if activity within 5s
}

uint32_t VehicleManager::getFrameCount()
{
    return activityTracker.getFrameCount();
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

    // Count every frame and mark activity
    activityTracker.onCanActivity();

    // Extended frames: only BAP
    if (extended)
    {
        // Route all BAP frames (0x1733xxxx range) to BatteryControlChannel
        if ((canId & 0xFFFF0000) == 0x17330000)
        {
            if (batteryControlChannel.processFrame(canId, data, dlc))
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

    // Standard frames: O(1) switch routing to domain managers
    switch (canId)
    {
    case 0x0FD:
    case 0x3C0:
    case 0x6B2:
        driveManager.processCanFrame(canId, data, dlc);
        driveFrames++;
        break;
    case 0x3D0:
    case 0x3D1:
    case 0x583:
        bodyManager.processCanFrame(canId, data, dlc);
        bodyFrames++;
        break;
    case 0x484:
    case 0x485:
    case 0x486:
        gpsManager.processCanFrame(canId, data, dlc);
        gpsFrames++;
        break;
    case 0x483:
    case 0x59E:
    case 0x5CA:
        batteryManager.processCanFrame(canId, data, dlc);
        batteryFrames++;
        break;
    case 0x5E1:
    case 0x66E:
        climateManager.processCanFrame(canId, data, dlc);
        climateFrames++;
        break;
    case 0x5F5:
    case 0x5F7:
        rangeManager.processCanFrame(canId, data, dlc);
        rangeFrames++;
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
    // Get frame counts from ActivityTracker
    uint32_t totalFrameCount = activityTracker.getFrameCount();

    // Get CanManager's count for comparison
    uint32_t canMgrCount = canManager ? canManager->getMessageCount() : 0;
    uint32_t canMgrMissed = canManager ? canManager->getMissedCount() : 0;

    Serial.println("[VehicleManager] === Vehicle Status ===");

    // Frame loss analysis
    uint32_t processedByDomains = bodyFrames + batteryFrames + driveFrames + climateFrames + gpsFrames + rangeFrames + bapFrames + unhandledFrames;
    Serial.printf("[VehicleManager] CanManager received: %lu (TWAI missed: %lu)\r\n", canMgrCount, canMgrMissed);
    Serial.printf("[VehicleManager] ActivityTracker: %lu frames | Domains processed: %lu\r\n", totalFrameCount, processedByDomains);
    if (canMgrCount > totalFrameCount)
    {
        Serial.printf("[VehicleManager] FRAME LOSS: %lu frames lost between CanManager and VehicleManager (mutex timeout?)\r\n",
                      canMgrCount - totalFrameCount);
    }

    Serial.printf("[VehicleManager] Domain breakdown: body:%lu batt:%lu drv:%lu clim:%lu gps:%lu rng:%lu bap:%lu unhandled:%lu\r\n",
                  bodyFrames, batteryFrames, driveFrames, climateFrames, gpsFrames, rangeFrames, bapFrames, unhandledFrames);

    Serial.printf("[VehicleManager] Vehicle awake: %s\r\n", activityTracker.isActive() ? "YES" : "NO");

    // BodyManager stats
    {
        uint32_t driverDoorFrames, passengerDoorFrames, lockStatusFrames;
        bodyManager.getFrameCounts(driverDoorFrames, passengerDoorFrames, lockStatusFrames);
        const BodyManager::State& bodyState = bodyManager.getState();
        Serial.printf("[VehicleManager] BodyManager: frames=0x3D0:%lu 0x3D1:%lu 0x583:%lu\r\n",
                      driverDoorFrames, passengerDoorFrames, lockStatusFrames);
        Serial.printf("[VehicleManager] Body: locked:%s driver_door:%s passenger_door:%s\r\n",
                      bodyState.isLocked() ? "YES" : "no",
                      bodyState.driverDoor.open ? "OPEN" : "closed",
                      bodyState.passengerDoor.open ? "OPEN" : "closed");
    }
    
    // BatteryManager stats
    {
        uint32_t bms07, bms06, motorHybrid06, plugCallbacks, chargeCallbacks;
        batteryManager.getFrameCounts(bms07, bms06, motorHybrid06);
        batteryManager.getCallbackCounts(plugCallbacks, chargeCallbacks);
        const BatteryManager::State& battState = batteryManager.getState();
        Serial.printf("[VehicleManager] BatteryManager: frames=0x5CA:%lu 0x59E:%lu 0x483:%lu callbacks=plug:%lu charge:%lu\r\n",
                      bms07, bms06, motorHybrid06, plugCallbacks, chargeCallbacks);
        Serial.printf("[VehicleManager] Battery: SOC=%.0f%% (source:%s) energy=%.0f/%.0fWh plugged:%s charging:%s\r\n",
                      battState.soc,
                      battState.socSource == DataSource::BAP ? "BAP" : battState.socSource == DataSource::CAN_STD ? "CAN" : "none",
                      battState.energyWh, battState.maxEnergyWh,
                      battState.plugState.isPlugged() ? "YES" : "no",
                      battState.charging ? "YES" : "no");
    }

    // DriveManager stats
    {
        uint32_t klemmenFrames, esp21Frames, diagnoseFrames;
        driveManager.getFrameCounts(klemmenFrames, esp21Frames, diagnoseFrames);
        const DriveManager::State& driveState = driveManager.getState();
        Serial.printf("[VehicleManager] DriveManager: frames=0x3C0:%lu 0x0FD:%lu 0x6B2:%lu\r\n",
                      klemmenFrames, esp21Frames, diagnoseFrames);
        const char* ignStr = driveState.ignition == IgnitionState::OFF ? "OFF" :
                            driveState.ignition == IgnitionState::ACCESSORY ? "ACCESSORY" :
                            driveState.ignition == IgnitionState::ON ? "ON" :
                            driveState.ignition == IgnitionState::START ? "START" : "UNKNOWN";
        Serial.printf("[VehicleManager] Drive: ignition:%s speed:%.1fkm/h odometer:%lukm\r\n",
                      ignStr, driveState.speedKmh, driveState.odometerKm);
    }

    // ClimateManager stats
    {
        uint32_t klima03, klimaSensor02, climateCallbacks;
        climateManager.getFrameCounts(klima03, klimaSensor02);
        climateCallbacks = climateManager.getCallbackCount();
        const ClimateManager::State& climState = climateManager.getState();
        Serial.printf("[VehicleManager] ClimateManager: frames=0x66E:%lu 0x5E1:%lu callbacks=%lu\r\n",
                      klima03, klimaSensor02, climateCallbacks);
        Serial.printf("[VehicleManager] Climate: inside=%.1f°C (source:%s) outside=%.1f°C active:%s\r\n",
                      climState.insideTemp,
                      climState.insideTempSource == DataSource::BAP ? "BAP" : climState.insideTempSource == DataSource::CAN_STD ? "CAN" : "none",
                      climState.outsideTemp,
                      climState.climateActive ? "YES" : "no");
    }
    
    // GpsManager stats
    {
        uint32_t navData01Frames, navData02Frames, navPos01Frames;
        gpsManager.getFrameCounts(navData01Frames, navData02Frames, navPos01Frames);
        const GpsManager::State& gpsState = gpsManager.getState();
        Serial.printf("[VehicleManager] GpsManager: frames=0x484:%lu 0x485:%lu 0x486:%lu\r\n",
                      navData01Frames, navData02Frames, navPos01Frames);
        Serial.printf("[VehicleManager] GPS: fix:%s sats:%d pos:%.6f,%.6f\r\n",
                      gpsState.fixTypeStr(), gpsState.satellites,
                      gpsState.latitude, gpsState.longitude);
    }
    
    // RangeManager stats
    {
        uint32_t reichweite01Frames, reichweite02Frames;
        rangeManager.getFrameCounts(reichweite01Frames, reichweite02Frames);
        const RangeManager::State& rangeState = rangeManager.getState();
        Serial.printf("[VehicleManager] RangeManager: frames=0x5F5:%lu 0x5F7:%lu\r\n",
                      reichweite01Frames, reichweite02Frames);
        Serial.printf("[VehicleManager] Range: total:%dkm electric:%dkm display:%dkm tendency:%s\r\n",
                      rangeState.totalRangeKm, rangeState.electricRangeKm,
                      rangeState.displayRangeKm, rangeState.tendencyStr());
    }

    // BAP Plug status (from BatteryManager)
    const BatteryManager::State& battState = batteryManager.getState();
    if (battState.plugState.isValid())
    {
        Serial.printf("[VehicleManager] BAP Plug: %s (supply:%s lock:%d)\r\n",
                      battState.plugState.plugStateStr(),
                      battState.plugState.hasSupply() ? "yes" : "no",
                      battState.plugState.lockState);
    }

    // BAP Charge detail (from BatteryManager)
    if (battState.socSource == DataSource::BAP || battState.chargingUpdate > 0)
    {
        Serial.printf("[VehicleManager] BAP Charge: SOC=%.0f%% mode=%d status=%d amps=%d target=%d%% time=%dmin\r\n",
                      battState.soc,
                      battState.chargingMode,
                      battState.chargingStatus,
                      battState.chargingAmps,
                      battState.targetSoc,
                      battState.remainingTimeMin);
    }

    // BAP Climate detail (from ClimateManager)
    const ClimateManager::State& climState = climateManager.getState();
    if (climState.climateActiveSource == DataSource::BAP && climState.climateActive)
    {
        Serial.printf("[VehicleManager] BAP Climate Detail: heat:%d cool:%d vent:%d defrost:%d temp:%.1f°C time:%dmin\r\n",
                      climState.heating, climState.cooling, climState.ventilation, climState.autoDefrost,
                      climState.insideTemp, climState.climateTimeMin);
    }

    Serial.println("[VehicleManager] ======================");
}
