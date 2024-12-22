#include "Modem.h"
#include "Link/ConnectionManager.h"

#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);

Modem* Modem::_instance = nullptr;

Modem::Modem(PowerManager* powerManager) {
    this->powerManager = powerManager;
    this->modem = new TinyGsm(debugger);

    this->_instance = this;
}

Modem::~Modem() {
}

bool Modem::setup() {
    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);

    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    // If the modem is powered, attempt to reach it
    if (this->powerManager->isModemPowered() && this->modem->testAT(100)) {
        this->state = ModemState::MODEM_HOTSTART;
        this->enableInterrupt();
    } else {
        this->state = ModemState::MODEM_DISABLED;

        // If the modem was powered, turn it off
        if (this->powerManager->isModemPowered()) {
            this->powerManager->modemPower(false);
        }
    }

    Serial.println("[MODEM] Setup OK!");

    return true;
}

void Modem::enableInterrupt() {
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_3), []() {
        Modem::instance()->onModemInterrupt();
    }, RISING);
}

void Modem::disableInterrupt() {
    detachInterrupt(digitalPinToInterrupt(GPIO_NUM_3));
}

void Modem::onModemInterrupt() {
    Serial.println("[MODEM] Modem interrupt trigger!");
    this->hasModemInterrupt = true;
}

void Modem::loop() {
    ModemState loopStartState = this->state;

    if(this->justChangedState()) {
        switch (this->state)
        {
            case ModemState::MODEM_DISABLED:
                Serial.println("[MODEM] Changed state to disabled!");
                break;
            case ModemState::MODEM_STARTING:
                Serial.println("[MODEM] Changed state to starting!");
                break;
            case ModemState::MODEM_HOTSTART:
                Serial.println("[MODEM] Changed state to hot start!");
                break;
            case ModemState::MODEM_TIMEOUT:
                Serial.println("[MODEM] Changed state to timeout!");
                break;
            case ModemState::MODEM_NOSIM:
                Serial.println("[MODEM] Changed state to no sim!");
                break;
            case ModemState::MODEM_SEARCHING:
                Serial.println("[MODEM] Changed state to searching!");
                break;
            case ModemState::MODEM_REGISTERED:
                Serial.println("[MODEM] Changed state to registered!");
                break;
            case ModemState::MODEM_UNREGISTERED:
                Serial.println("[MODEM] Changed state to unregistered!");
                break;
            case ModemState::MODEM_DENIED:
                Serial.println("[MODEM] Changed state to denied!");
                break;
            case ModemState::MODEM_CONNECTED:
                Serial.println("[MODEM] Changed state to connected!");
                break;
            default:
                Serial.println("[MODEM] [WARN] Modem entered an unknown state!");
                break;
        }

        this->last_loop = 0;
    }

    // If we are enabling the modem, wait for it to be ready
    if (this->state == ModemState::MODEM_STARTING && this->justChangedState()) {
        // Set SMS mode to text
        this->modem->sendAT("+CMGF=1");
        if (this->modem->waitResponse() != 1) {
            Serial.println("[MODEM] Failed to configure SMS TEXT mode!");
            return;
        }

        // Configure the delivery mode for SMS
        this->modem->sendAT("+CNMI=2,2");
        if (this->modem->waitResponse() != 1) {
            Serial.println("[MODEM] Failed to configure delivery mode for SMS!");
            return;
        }

        // Lets configure the modem
        this->modem->setNetworkMode(2);
        this->modem->setPreferredMode(MODEM_NETWORK_CATM);
        this->modem->sendAT("+CGDCONT=1,\"IP\",\"hologram\"");
        if (this->modem->waitResponse() != 1) {
            this->disableModem();
            return;
        }

        // Configure the RI pin
        this->modem->sendAT("+CFGRI=1");
        if (this->modem->waitResponse() != 1) {
            Serial.println("[MODEM] Failed to configure the RI pin!");
            return;
        }

        this->checkModemState();
    } else if (this->state == ModemState::MODEM_HOTSTART && millis_since(this->last_loop) > MODEM_START_DELAY) {
        this->loop_modem_hotstart();
        this->last_loop = millis();
    } else if (this->state == ModemState::MODEM_SEARCHING && millis_since(this->last_loop) > MODEM_SEARCH_DELAY) {
        this->checkModemState();
        this->last_loop = millis();
    } else if (this->state == ModemState::MODEM_UNREGISTERED && millis_since(this->last_loop) > MODEM_UNREGISTERED_DELAY) {
        this->checkModemState();
        this->last_loop = millis();
    } else if (this->state == ModemState::MODEM_DENIED && millis_since(this->last_loop) > MODEM_DENIED_DELAY) {
        this->checkModemState();
        this->last_loop = millis();
    } else if (this->state == ModemState::MODEM_REGISTERED && millis_since(this->last_loop) > MODEM_REGISTERED_DELAY) {
        this->loop_modem_registered();
        this->last_loop = millis();
    } else if (this->state == ModemState::MODEM_CONNECTED && (this->justChangedState() || millis_since(this->last_loop) > MODEM_CONNECTED_DELAY)) {
        //this->loop_modem_connected();
        this->last_loop = millis();
    }

    // Update the previous state if the state at the start of the loop has changed
    if (loopStartState != this->previousLoopState) {
        this->previousLoopState = loopStartState;
    }
}

void Modem::checkModemState() {

    String data;
    this->modem->sendAT("+CEREG?");
    this->modem->waitResponse(1000, "+CEREG:");
    this->modem->waitResponse(100, data, AT_NL);
    data.trim();

    int status = data.substring(data.indexOf(',') + 1, data.indexOf(',', data.indexOf(',') + 1)).toInt();

    switch (status) {
        case REG_SEARCHING:
            this->state = ModemState::MODEM_SEARCHING;
            break;
        case REG_DENIED:
            this->state = ModemState::MODEM_DENIED;
            break;
        case REG_UNREGISTERED:
            this->state = ModemState::MODEM_UNREGISTERED;
            break;
        case REG_OK_HOME:
        case REG_OK_ROAMING:
            this->state = ModemState::MODEM_REGISTERED;
            break;
    }

    if (this->state == ModemState::MODEM_REGISTERED && this->modem->isGprsConnected()) {
        this->state = ModemState::MODEM_CONNECTED;
        return;
    }
}

void Modem::handleModemInterrupt() {

    if (this->hasModemInterrupt == false) {
        return;
    }

    Serial.println("[MODEM] Modem interrupt!");

    int urc = this->modem->waitResponse(15L, "+APP", "+CMT:", "+CA");

    if (urc == 1) {
        int state = this->modem->waitResponse("ACTIVE", "DEACTIVE");
        this->modem->waitResponse(AT_NL);
        if (state == 1) {
            Serial.println("[MODEM] Modem connected to internet!");
            this->state = ModemState::MODEM_CONNECTED;
        } else {
            Serial.println("[MODEM] Modem disconnected from internet!");
            this->checkModemState();
        }
    } else if (urc == 2) {
        String message;

        this->modem->waitResponse(100, AT_NL);
        this->modem->waitResponse(100, message, AT_NL);

        message.trim();

        Serial.println("[MODEM] Received SMS message!");
        Serial.println(message);
    } else if (urc == 3) {
        Serial.println("[MODEM] Received TCP layer URC!");
        ConnectionManager::instance()->handleTCPInterrupt();
    }

    this->hasModemInterrupt = false;

    Serial.println("[MODEM] Modem interrupt handled!");
}

void Modem::loop_modem_hotstart() {
    // Check if the modem responds to us
    if (this->modem->testAT(500)) {
        // Modem has responded, lets get its current state
        this->checkModemState();

        // If the modem is not connected, we can switch to the starting state to perform the necessary configuration
        if (this->state != ModemState::MODEM_CONNECTED) {
            this->state = ModemState::MODEM_STARTING;
            return;
        }
    } else {
        // Modem did not respond, lets switch to the starting state
        this->state = ModemState::MODEM_STARTING;
    }
}

void Modem::loop_modem_registered() {

    if (this->justChangedState()) {

        // Configure the modem to use eDRX
        Serial.println("[MODEM] Configuring eDRX!");
        this->modem->sendAT("+CEDRXS=1,4,\"0001\"");
        if (this->modem->waitResponse() != 1) {
            return;
        }

        // Connect to the internet
        this->modem->sendAT("+CNACT=0,2");
        if (modem->waitResponse() != 1) {
            return;
        }

    }

    // Lets start by checking the modem state, this also checks if we are already connected to the internet
    /*this->checkModemState();

    // If we are no longer registered, no need to continue
    if (this->state != ModemState::MODEM_REGISTERED) {
        return;
    }

    // We are registered, but not connected to the internet
    // Lets attempt to connect to the internet by activating the PDP context
    modem->sendAT("+CNACT=0,1");
    if (modem->waitResponse() != 1) {
        return;
    }*/
}

void Modem::loop_modem_connected() {
    // If we just switched to being connected, open the UDP server.
    // However, if we just switched from hotstart, the UDP server is already running.
    if (this->justChangedState() && this->previousLoopState != ModemState::MODEM_HOTSTART) {
        /*this->modem->sendAT("+CASERVER=0,0,\"UDP\",6000,1");
        if (this->modem->waitResponse() != 1) {
            Serial.println("[MODEM] Failed to open UDP server!");
            return;
        }*/
    }
    
    // We are connected to the internet, but we need to detect if we are still connected
    this->checkModemState();
}


bool Modem::enableModem() {

    // We can only enable the modem if we are in the correct state
    if (this->state != ModemState::MODEM_TIMEOUT && this->state != ModemState::MODEM_DISABLED) {
        return false;
    }

    this->state = ModemState::MODEM_STARTING;

    // Ensure the power to the modem is on
    this->powerManager->modemPower(true);

    // Give modem some time to get power
    delay(100);

    // Toggle the modem power pin for 1 second
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    // After the modem has been powered, we should wait for it to be ready
    // This will take at least 2.5 seconds
    delay(2500);

    // Initialize the modem connection, this will fail if the SIM is not available
    if (! this->modem->init()) {
        Serial.println("[MODEM] Failed to initialize modem!");
        Serial.println("[MODEM] Are we sure a SIM is inserted?");
        this->state = ModemState::MODEM_NOSIM;
        return false;
    }

    this->modem->waitResponse("SMS Ready");
    this->modem->waitResponse(AT_NL);

    this->enableInterrupt();

    return true;
}

bool Modem::disableModem() {

    // We can only disable the modem if we are in the correct state
    if (this->state != ModemState::MODEM_STARTING && this->state != ModemState::MODEM_REGISTERED && this->state != ModemState::MODEM_SEARCHING && this->state != ModemState::MODEM_UNREGISTERED && this->state != ModemState::MODEM_DENIED) {
        return false;
    }

    // Turn the power to the modem off
    this->powerManager->modemPower(false);

    this->state = ModemState::MODEM_DISABLED;

    this->disableInterrupt();

    return true;
}

String Modem::getSimCCID() {
    if (this->simCCID.length() == 0) {
        this->simCCID = this->modem->getSimCCID();
    }

    return this->simCCID;
}