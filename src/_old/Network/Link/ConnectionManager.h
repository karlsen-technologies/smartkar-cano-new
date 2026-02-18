#pragma once

#include "Network/Modem.h"
#include "util.h"

#define CONNECTION_SERVER_PORT 4589

enum class LinkState {
    LINK_DISCONNECTED,
    LINK_AUTHENTICATING,
    LINK_CONNECTED,
    LINK_REJECTED,
    // Reserved for future use:
    // LINK_CONNECTING - for async connection handling
    // LINK_OFFLINE - for explicit offline mode
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

};