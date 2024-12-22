#pragma once

#include "Network/Modem.h"
#include "util.h"

#define CONNECTION_SERVER_PORT 6000

#define SERVER_HEALTH_CHECK_INTERVAL SECONDS(60)

enum class LinkState {
    LINK_DISCONNECTED,
    LINK_CONNECTING,
    LINK_AUTHENTICATING,
    LINK_CONNECTED,
    LINK_REJECTED,
    LINK_OFFLINE
};


class ConnectionManager {

    public:
        ConnectionManager(Modem* modemManager);
        ~ConnectionManager();

        LinkState state = LinkState::LINK_DISCONNECTED;

        static ConnectionManager* instance() { return ConnectionManager::_instance; }

        bool setup();
        void loop();
        bool prepareForSleep();

        void handleTCPInterrupt();

    private:
        static ConnectionManager* _instance;
        Modem* modemManager = nullptr;
        TinyGsmClient* client = nullptr;
        

        unsigned long last_loop = 0;
        unsigned long last_server_health_check = 0;
        unsigned long last_ping = 0;

};