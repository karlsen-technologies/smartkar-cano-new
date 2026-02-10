#include "ModemManager.h"
#include "PowerManager.h"
#include "MqttManager.h"
#include "../util.h"

#include <Arduino.h>

// Set to 1 to see all AT commands in Serial output
#define MODEM_DEBUG 0

#if MODEM_DEBUG
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
#define MODEM_STREAM debugger
#else
#define MODEM_STREAM Serial1
#endif

ModemManager *ModemManager::_instance = nullptr;

ModemManager::ModemManager(PowerManager *powerManager)
    : powerManager(powerManager)
{
    modem = new TinyGsmSim7080Extended(MODEM_STREAM);
    _instance = this;
}

ModemManager::~ModemManager()
{
    if (modem)
    {
        delete modem;
        modem = nullptr;
    }
}

bool ModemManager::setup()
{
    Serial.println("[MODEM] Setting up modem manager");

    // Initialize Serial1 for modem communication
    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    // Configure modem control pins
    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    // Check if modem is already powered (wake from sleep)
    if (powerManager->isModemPowered())
    {
        Serial.println("[MODEM] Modem was powered, attempting hotstart");
        if (modem->testAT(100))
        {
            setState(ModemState::HOTSTART);
            didHotstart = true; // Mark that we did a hotstart
            enableInterrupt();
        }
        else
        {
            // Modem powered but not responding - power cycle needed
            Serial.println("[MODEM] Modem not responding, will need restart");
            powerManager->setModemPower(false);
            setState(ModemState::OFF);
            didHotstart = false;
        }
    }
    else
    {
        setState(ModemState::OFF);
        didHotstart = false;
    }

    Serial.println("[MODEM] Setup complete");
    return true;
}

void ModemManager::loop()
{
    // Process any pending interrupts first
    handleInterrupt();

    // State machine
    switch (state)
    {
    case ModemState::OFF:
        handleDisabledState();
        break;
    case ModemState::STARTING:
        handleStartingState();
        break;
    case ModemState::HOTSTART:
        handleHotstartState();
        break;
    case ModemState::CONFIGURING:
        handleConfiguringState();
        break;
    case ModemState::SEARCHING:
        handleSearchingState();
        break;
    case ModemState::REGISTERED:
        handleRegisteredState();
        break;
    case ModemState::UNREGISTERED:
        // Retry registration check
        if (millis_since(lastLoopTime) > UNREGISTERED_RETRY_INTERVAL)
        {
            handleSearchingState();
            lastLoopTime = millis();
        }
        break;
    case ModemState::DENIED:
        // Retry after longer delay
        if (millis_since(lastLoopTime) > DENIED_RETRY_INTERVAL)
        {
            handleSearchingState();
            lastLoopTime = millis();
        }
        break;
    case ModemState::CONNECTING:
        // Connection in progress, wait for interrupt
        break;
    case ModemState::CONNECTED:
        handleConnectedState();
        break;
    case ModemState::NO_SIM:
    case ModemState::MODEM_ERROR:
        // Terminal states - require manual intervention
        break;
    }
}

void ModemManager::prepareForSleep()
{
    Serial.println("[MODEM] Preparing for sleep");

    // Disable interrupt during sleep prep
    disableInterrupt();

    // Modem stays powered during sleep - RI pin will wake us
    // No need to do anything special here
}

bool ModemManager::isBusy()
{
    // Modem is busy during startup, configuration, and connection phases
    switch (state)
    {
    case ModemState::STARTING:
    case ModemState::HOTSTART:
    case ModemState::CONFIGURING:
    case ModemState::CONNECTING:
        return true;
    default:
        return false;
    }
}

bool ModemManager::isReady()
{
    return state == ModemState::CONNECTED;
}

bool ModemManager::enable()
{
    if (state != ModemState::OFF && state != ModemState::MODEM_ERROR)
    {
        Serial.println("[MODEM] Cannot enable - not in disabled/error state");
        return false;
    }

    Serial.println("[MODEM] Enabling modem");
    setState(ModemState::STARTING);

    // Report activity to prevent sleep during startup
    if (activityCallback)
        activityCallback();

    return true;
}

bool ModemManager::disable()
{
    Serial.println("[MODEM] Disabling modem");

    disableInterrupt();
    powerManager->setModemPower(false);
    setState(ModemState::OFF);

    return true;
}

TinyGsmClient *ModemManager::createClient()
{
    if (!modem || state != ModemState::CONNECTED)
    {
        return nullptr;
    }
    return new TinyGsmClient(*modem);
}

String ModemManager::getSimCCID()
{
    if (simCCID.length() == 0 && modem)
    {
        simCCID = modem->getSimCCID();
    }
    return simCCID;
}

int16_t ModemManager::getSignalQuality()
{
    return cachedSignalQuality;
}

// State machine helpers

bool ModemManager::stateJustChanged()
{
    return previousState != state;
}

void ModemManager::setState(ModemState newState)
{
    if (state != newState)
    {
        Serial.printf("[MODEM] State: %d -> %d\r\n", (int)state, (int)newState);
        previousState = state;
        state = newState;
        stateEntryTime = millis();
        lastLoopTime = millis();

        // Reset cached signal quality when entering non-connected states
        if (newState == ModemState::OFF ||
            newState == ModemState::MODEM_ERROR ||
            newState == ModemState::NO_SIM)
        {
            cachedSignalQuality = 0;
        }
    }
}

unsigned long ModemManager::timeInState()
{
    return millis_since(stateEntryTime);
}

// State handlers

void ModemManager::handleDisabledState()
{
    // Nothing to do - wait for enable() call
}

void ModemManager::handleStartingState()
{
    if (!stateJustChanged())
        return;

    // Execute power-on sequence
    if (powerOnSequence())
    {
        setState(ModemState::CONFIGURING);
    }
    else
    {
        Serial.println("[MODEM] Power-on sequence failed");
        setState(ModemState::MODEM_ERROR);
    }
}

void ModemManager::handleHotstartState()
{
    if (!stateJustChanged())
        return;

    // Modem is already powered, check its state
    if (modem->testAT(500))
    {
        RegStatus regStatus = checkRegistration();

        if (regStatus == RegStatus::REGISTERED_HOME || regStatus == RegStatus::REGISTERED_ROAMING)
        {
            // Check if already connected to internet
            if (modem->isGprsConnected())
            {
                setState(ModemState::CONNECTED);
                if (activityCallback)
                    activityCallback();
            }
            else
            {
                setState(ModemState::REGISTERED);
            }
        }
        else
        {
            // Need to reconfigure
            setState(ModemState::CONFIGURING);
        }
    }
    else
    {
        // Modem not responding after hotstart
        setState(ModemState::STARTING);
    }
}

void ModemManager::handleConfiguringState()
{
    if (!stateJustChanged())
        return;

    if (sendInitCommands())
    {
        setState(ModemState::SEARCHING);
    }
    else
    {
        Serial.println("[MODEM] Configuration failed");
        setState(ModemState::MODEM_ERROR);
    }
}

void ModemManager::handleSearchingState()
{
    if (millis_since(lastLoopTime) < SEARCH_CHECK_INTERVAL && !stateJustChanged())
    {
        return;
    }
    lastLoopTime = millis();

    // Update cached signal quality
    if (modem)
    {
        cachedSignalQuality = modem->getSignalQuality();
    }

    RegStatus regStatus = checkRegistration();

    switch (regStatus)
    {
    case RegStatus::REGISTERED_HOME:
    case RegStatus::REGISTERED_ROAMING:
        setState(ModemState::REGISTERED);
        if (activityCallback)
            activityCallback();
        break;
    case RegStatus::SEARCHING:
        // Stay in searching state
        break;
    case RegStatus::DENIED:
        setState(ModemState::DENIED);
        break;
    case RegStatus::NOT_REGISTERED:
        setState(ModemState::UNREGISTERED);
        break;
    default:
        break;
    }
}

void ModemManager::handleRegisteredState()
{
    if (!stateJustChanged())
    {
        // Periodic check that we're still registered
        if (millis_since(lastLoopTime) > REGISTERED_CHECK_INTERVAL)
        {
            // Update cached signal quality
            if (modem)
            {
                cachedSignalQuality = modem->getSignalQuality();
            }

            RegStatus regStatus = checkRegistration();
            if (regStatus != RegStatus::REGISTERED_HOME && regStatus != RegStatus::REGISTERED_ROAMING)
            {
                setState(ModemState::SEARCHING);
                return;
            }
            lastLoopTime = millis();
        }
        return;
    }

    // Just entered registered state - configure eDRX and connect
    Serial.println("[MODEM] Configuring eDRX");
    modem->sendAT("+CEDRXS=1,4,\"0001\"");
    if (modem->waitResponse() != 1)
    {
        Serial.println("[MODEM] eDRX config failed (non-fatal)");
    }

    // Activate data connection
    Serial.println("[MODEM] Activating data connection");
    setState(ModemState::CONNECTING);

    modem->sendAT("+CNACT=0,2");
    if (modem->waitResponse() != 1)
    {
        Serial.println("[MODEM] Data activation command failed");
        setState(ModemState::REGISTERED);
        return;
    }

    // Wait for +APP PDP: ACTIVE interrupt or check status
    // The interrupt handler will move us to CONNECTED state
}

void ModemManager::handleConnectedState()
{
    // Periodic connectivity check
    if (millis_since(lastLoopTime) < CONNECTED_CHECK_INTERVAL)
    {
        return;
    }
    lastLoopTime = millis();

    // Update cached signal quality
    if (modem)
    {
        cachedSignalQuality = modem->getSignalQuality();
    }

    // Verify we're still connected
    if (!modem->isGprsConnected())
    {
        Serial.println("[MODEM] Lost data connection");
        RegStatus regStatus = checkRegistration();
        if (regStatus == RegStatus::REGISTERED_HOME || regStatus == RegStatus::REGISTERED_ROAMING)
        {
            setState(ModemState::REGISTERED);
        }
        else
        {
            setState(ModemState::SEARCHING);
        }
    }
}

// Modem operations

bool ModemManager::powerOnSequence()
{
    Serial.println("[MODEM] Starting power-on sequence");

    // Ensure modem power rail is on
    powerManager->setModemPower(true);
    delay(100);

    // Toggle modem power pin (active high pulse)
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    // Wait for modem to boot
    delay(2500);

    // Initialize modem
    if (!modem->init())
    {
        Serial.println("[MODEM] Init failed - check SIM card");
        setState(ModemState::NO_SIM);
        return false;
    }

    // Wait for SMS Ready indication
    modem->waitResponse(5000, "SMS Ready");

    enableInterrupt();

    Serial.println("[MODEM] Power-on sequence complete");
    return true;
}

bool ModemManager::sendInitCommands()
{
    Serial.println("[MODEM] Sending init commands");

    // Set SMS mode to text
    modem->sendAT("+CMGF=1");
    if (modem->waitResponse() != 1)
    {
        Serial.println("[MODEM] SMS text mode failed");
        return false;
    }

    // Configure SMS delivery mode
    modem->sendAT("+CNMI=2,2");
    if (modem->waitResponse() != 1)
    {
        Serial.println("[MODEM] SMS delivery config failed");
        return false;
    }

    // Set network mode (auto)
    modem->setNetworkMode(2);

    // Set preferred mode to CAT-M
    modem->setPreferredMode(1); // CAT-M

    // Set APN
    modem->sendAT("+CGDCONT=1,\"IP\",\"hologram\"");
    if (modem->waitResponse() != 1)
    {
        Serial.println("[MODEM] APN config failed");
        return false;
    }

    // Configure RI pin for URC notification
    modem->sendAT("+CFGRI=1");
    if (modem->waitResponse() != 1)
    {
        Serial.println("[MODEM] RI config failed");
        return false;
    }

    Serial.println("[MODEM] Init commands complete");
    return true;
}

RegStatus ModemManager::checkRegistration()
{
    String data;
    modem->sendAT("+CEREG?");
    modem->waitResponse(1000, "+CEREG:");
    modem->waitResponse(100, data, "\r\n");
    data.trim();

    // Parse: +CEREG: <n>,<stat>
    int commaIndex = data.indexOf(',');
    if (commaIndex < 0)
        return RegStatus::UNKNOWN;

    int status = data.substring(commaIndex + 1, data.indexOf(',', commaIndex + 1)).toInt();
    return static_cast<RegStatus>(status);
}

bool ModemManager::activateDataConnection()
{
    modem->sendAT("+CNACT=0,2");
    return modem->waitResponse() == 1;
}

// Interrupt handling

void ModemManager::enableInterrupt()
{
    attachInterrupt(digitalPinToInterrupt(BOARD_MODEM_RI_PIN), []()
                    {
        if (ModemManager::instance()) {
            ModemManager::instance()->onInterrupt();
        } }, RISING);
}

void ModemManager::disableInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(BOARD_MODEM_RI_PIN));
}

void IRAM_ATTR ModemManager::onInterrupt()
{
    hasInterrupt = true;
}

void ModemManager::handleInterrupt()
{
    if (!hasInterrupt)
        return;
    hasInterrupt = false;

    // Check for URCs we need to handle ourselves (not handled by TinyGSM)
    // TinyGSM's handleURCs() handles: +CADATAIND, +CASTATE, +CARECV, time updates, etc.
    int urc = modem->waitResponse(100L, "+APP", "+CMT:", "+SMSUB:", "+SMSTATE:");

    if (urc == 1)
    {
        // +APP PDP: ACTIVE/DEACTIVE - data connection state change
        int appState = modem->waitResponse("ACTIVE", "DEACTIVE");
        modem->waitResponse("\r\n");

        if (appState == 1)
        {
            Serial.println("[MODEM] Data connection active");
            setState(ModemState::CONNECTED);
        }
        else
        {
            Serial.println("[MODEM] Data connection deactivated");
            RegStatus regStatus = checkRegistration();
            if (regStatus == RegStatus::REGISTERED_HOME || regStatus == RegStatus::REGISTERED_ROAMING)
            {
                setState(ModemState::REGISTERED);
            }
            else
            {
                setState(ModemState::SEARCHING);
            }
        }
        if (activityCallback)
            activityCallback();
    }
    else if (urc == 2)
    {
        // +CMT: SMS received
        String message;
        modem->waitResponse(100, "\r\n");
        modem->waitResponse(100, message, "\r\n");
        message.trim();

        Serial.println("[MODEM] SMS received:");
        Serial.println(message);

        if (activityCallback)
            activityCallback();
    }
    else if (urc == 3)
    {
        // +SMSUB: "<topic>","<payload>" - MQTT message received
        String line;
        modem->waitResponse(100, line, "\r\n");
        line.trim();

        Serial.println("[MODEM] MQTT message received:");
        Serial.println(line);

        // Parse +SMSUB: "<topic>","<payload>"
        // Format: +SMSUB: "topic","payload"
        int firstQuote = line.indexOf('"');
        int secondQuote = line.indexOf('"', firstQuote + 1);
        int thirdQuote = line.indexOf('"', secondQuote + 1);
        int fourthQuote = line.indexOf('"', thirdQuote + 1);

        if (firstQuote != -1 && secondQuote != -1 && thirdQuote != -1 && fourthQuote != -1)
        {
            String topic = line.substring(firstQuote + 1, secondQuote);
            String payload = line.substring(thirdQuote + 1, fourthQuote);

            // Forward to MqttManager
            if (MqttManager::instance())
            {
                MqttManager::instance()->handleMessage(topic, payload);
            }
        }
        else
        {
            Serial.println("[MODEM] ERROR: Failed to parse MQTT message");
        }

        if (activityCallback)
            activityCallback();
    }
    else if (urc == 4)
    {
        // +SMSTATE: <state> - MQTT connection state changed
        String line;
        modem->waitResponse(100, line, "\r\n");
        line.trim();

        Serial.print("[MODEM] MQTT state changed: ");
        Serial.println(line);

        if (activityCallback)
            activityCallback();
    }
    else
    {
        // No match for +APP, +CMT, +SMSUB, or +SMSTATE - let TinyGSM handle it via maintain()
        // This processes +CADATAIND, +CASTATE, +CARECV, time updates, etc.
        // handleURCs() will set got_data=true and sock_connected as needed
        modem->maintain();
    }
}
    }
}
    else
    {
        // No match for +APP or +CMT - let TinyGSM handle it via maintain()
        // This processes +CADATAIND, +CASTATE, +CARECV, time updates, etc.
        // handleURCs() will set got_data=true and sock_connected as needed
        modem->maintain();

        // Notify LinkManager to check for data/connection changes
        if (LinkManager::instance())
        {
            LinkManager::instance()->handleTCPInterrupt();
        }
        if (activityCallback)
            activityCallback();
    }
}
