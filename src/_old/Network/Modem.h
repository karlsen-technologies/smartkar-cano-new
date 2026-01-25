#pragma once

#include "PowerManager.h"

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
#include "util.h"

#define MODEM_START_DELAY SECONDS(1)
#define MODEM_SEARCH_DELAY SECONDS(1)
#define MODEM_UNREGISTERED_DELAY SECONDS(1)
#define MODEM_DENIED_DELAY SECONDS(30)
#define MODEM_REGISTERED_DELAY SECONDS(1)
#define MODEM_CONNECTED_DELAY SECONDS(10)

enum class ModemState {
    MODEM_READY,        // Initial state before setup
    MODEM_DISABLED,     // Modem is powered off
    MODEM_STARTING,     // Modem is powering on and configuring
    MODEM_HOTSTART,     // Modem was already powered (wake from sleep)
    MODEM_TIMEOUT,      // Reserved: timeout during network search
    MODEM_NOSIM,        // No SIM card detected
    MODEM_SEARCHING,    // Searching for network
    MODEM_REGISTERED,   // Registered to network, not yet connected
    MODEM_UNREGISTERED, // Lost network registration
    MODEM_DENIED,       // Network registration denied
    MODEM_CONNECTED,    // Connected to internet (GPRS/LTE)
};

class Modem {

    public:
        Modem(PowerManager* powerManager);
        ~Modem();

        enum {
            MODEM_NETWORK_CATM = 1,
            MODEM_NETWORK_NB_IOT,
            MODEM_NETWORK_CATM_NBIOT,
        };

        enum ModemState state = ModemState::MODEM_READY;

        static Modem* instance() { return Modem::_instance; }

        bool setup();
        void loop();

        TinyGsm* getModem() { return this->modem; }

        bool enableModem();
        bool disableModem();

        void enableInterrupt();
        void disableInterrupt();
        void IRAM_ATTR onModemInterrupt();
        void handleModemInterrupt();

        String getSimCCID();

    private:
        static Modem* _instance;
        TinyGsm* modem = nullptr;
        PowerManager* powerManager = nullptr;

        bool hasModemInterrupt = false;

        unsigned long last_loop = 0;

        ModemState previousLoopState = ModemState::MODEM_READY;

        String simCCID = "";

        bool justChangedState() { return this->previousLoopState != this->state; };

        void checkModemState();

        void loop_modem_hotstart();
        void loop_modem_registered();
        void loop_modem_connected();
};