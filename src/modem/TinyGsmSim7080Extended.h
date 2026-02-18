/**
 * @file       TinyGsmSim7080Extended.h
 * @brief      Extended TinyGsmSim7080 with connection adoption support
 * @details    Adds public method to query modem connection state and sync with TinyGSM
 */

#ifndef TINYGSMCLIENTSIM7080EXTENDED_H
#define TINYGSMCLIENTSIM7080EXTENDED_H

#include <TinyGsmClient.h>

/**
 * Extended TinyGsmSim7080 that exposes connection state checking
 * for TCP connection persistence across ESP32 deep sleep
 */
class TinyGsmSim7080Extended : public TinyGsmSim7080
{
public:
    explicit TinyGsmSim7080Extended(Stream &stream) : TinyGsmSim7080(stream) {}

    /**
     * Check if a connection exists on the given mux and update TinyGSM state
     * This allows adopting TCP connections that persisted through deep sleep
     *
     * Calls the protected modemGetAvailable() which in turn calls modemGetConnected().
     * This checks if we have data available, but more importantly updates the
     * sock_connected flag for the given mux/socket.
     *
     * @param mux The socket mux number (default 0)
     */
    void adoptConnection(uint8_t mux = 0)
    {
        // Call the protected modemGetAvailable to update connection state
        modemGetAvailable(mux);
    }
};

#endif // TINYGSMCLIENTSIM7080EXTENDED_H
