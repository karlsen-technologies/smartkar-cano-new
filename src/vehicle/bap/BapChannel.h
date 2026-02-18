#pragma once

#include <Arduino.h>
#include "../protocols/BapProtocol.h"

// Forward declaration
class VehicleManager;

/**
 * BapChannel - Base interface for BAP communication channels
 * 
 * Each BAP channel represents a specific device (LSG) that communicates
 * using the BAP protocol. Examples include:
 * - Battery Control (Device 0x25)
 * - Door Locking (Device 0x0D)
 * - ENI Gateway (Device 0x37)
 * 
 * The BAP protocol layer handles message framing and assembly, while
 * channel implementations handle device-specific functionality.
 * 
 * Architecture:
 * ```
 * CAN Frame → BapChannelRouter → BapChannel (this interface)
 *                                      ↓
 *                          BatteryControlChannel, DoorLockingChannel, etc.
 * ```
 */
class BapChannel {
public:
    virtual ~BapChannel() = default;
    
    // =========================================================================
    // Channel identification
    // =========================================================================
    
    /**
     * Get the device ID (LSG ID) for this channel
     * Example: 0x25 for Battery Control
     */
    virtual uint8_t getDeviceId() const = 0;
    
    /**
     * Get the CAN ID for sending commands to FSG (TO device)
     * Example: 0x17332501 for Battery Control commands
     */
    virtual uint32_t getTxCanId() const = 0;
    
    /**
     * Get the CAN ID for receiving responses from FSG (FROM device)
     * Example: 0x17332510 for Battery Control responses
     */
    virtual uint32_t getRxCanId() const = 0;
    
    /**
     * Check if this channel handles a specific CAN ID
     * Used by BapChannelRouter to route incoming frames
     */
    virtual bool handlesCanId(uint32_t canId) const = 0;
    
    /**
     * Get channel name for debugging/logging
     */
    virtual const char* getName() const = 0;
    
    // =========================================================================
    // Message processing
    // =========================================================================
    
    /**
     * Process a complete BAP message
     * 
     * Called by BapChannelRouter after message assembly is complete.
     * The channel should parse the message and update its state.
     * 
     * @param msg Complete BAP message (short or reassembled long)
     * @return true if message was handled successfully
     */
    virtual bool processMessage(const BapProtocol::BapMessage& msg) = 0;
    
protected:
    /**
     * Send a BAP frame to the vehicle CAN bus
     * 
     * Helper method for channels to send commands. Uses the channel's
     * TX CAN ID to send the frame.
     * 
     * @param mgr VehicleManager instance
     * @param data Frame data (8 bytes)
     * @param len Frame length (typically 8)
     * @return true if frame was sent successfully
     */
    bool sendBapFrame(VehicleManager* mgr, const uint8_t* data, uint8_t len);
};
