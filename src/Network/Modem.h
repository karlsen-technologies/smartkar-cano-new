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
    MODEM_READY, // 0
    MODEM_DISABLED , // 1
    MODEM_STARTING, // 2
    MODEM_HOTSTART, // 3
    MODEM_TIMEOUT, // 4 Currently unsued, needs to be implemented
    MODEM_NOSIM, // 5
    MODEM_SEARCHING, // 6
    MODEM_REGISTERED, // 7
    MODEM_UNREGISTERED, // 8
    MODEM_DENIED, // 9
    MODEM_CONNECTED, // 10
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
        void onModemInterrupt();
        void handleModemInterrupt();

        String getSimCCID();

    private:
        static Modem* _instance;
        TinyGsm* modem = nullptr;
        PowerManager* powerManager = nullptr;

        bool hasModemInterrupt = false;

        unsigned long last_loop = 0;
        int modemStartAttempts = 0;

        ModemState previousLoopState = ModemState::MODEM_READY;

        String simCCID = "";

        bool justChangedState() { return this->previousLoopState != this->state; };

        void checkModemState();

        void loop_modem_hotstart();
        void loop_modem_registered();
        void loop_modem_connected();
};