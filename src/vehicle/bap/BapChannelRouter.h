#pragma once

#include <Arduino.h>
#include "BapChannel.h"
#include "../protocols/BapProtocol.h"

/**
 * BapChannelRouter - Routes CAN frames to appropriate BAP channels
 * 
 * Responsibilities:
 * - Receive raw CAN frames from VehicleManager
 * - Early filter: Check if any channel cares about the CAN ID (saves CPU)
 * - Use BapFrameAssembler to handle multi-frame message assembly
 * - Route complete messages to registered channels by CAN ID
 * - Maintain routing table of active channels
 * 
 * Performance Optimization:
 * The bus has 20+ BAP channels active, but we typically only care about 2-3.
 * We check if a channel is registered for the CAN ID BEFORE running the frame
 * assembler, avoiding wasted CPU cycles and complexity in the assembler state.
 * 
 * Architecture:
 * ```
 * CAN Frame → BapChannelRouter::processFrame()
 *     ↓
 * findChannel(canId) → early filter (return false if no channel registered)
 *     ↓
 * BapFrameAssembler::processFrame()  (handles multi-frame assembly)
 *     ↓ (if complete)
 * Complete BapMessage
 *     ↓
 * channel->processMessage(msg)
 * ```
 * 
 * Usage:
 * ```cpp
 * BapChannelRouter router;
 * BatteryControlChannel batteryChannel;
 * 
 * router.registerChannel(&batteryChannel);
 * 
 * // In CAN receive callback:
 * if (router.processFrame(canId, data, dlc)) {
 *     // Frame was handled by a channel
 * }
 * ```
 */
class BapChannelRouter {
public:
    static constexpr uint8_t MAX_CHANNELS = 8;
    
    BapChannelRouter();
    
    /**
     * Register a BAP channel with the router
     * 
     * @param channel Pointer to channel instance (must remain valid)
     * @return true if registered successfully, false if no space
     */
    bool registerChannel(BapChannel* channel);
    
    /**
     * Process an incoming CAN frame
     * 
     * Attempts to assemble BAP message and route to appropriate channel.
     * Handles both short (single-frame) and long (multi-frame) messages.
     * 
     * @param canId CAN identifier
     * @param data Frame data
     * @param dlc Data length code
     * @return true if frame was processed (even if incomplete), false if not a BAP frame
     */
    bool processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc);
    
    /**
     * Get statistics for debugging
     */
    void getStats(uint32_t& totalFrames, uint32_t& completeMessages,
                  uint32_t& shortMessages, uint32_t& longMessages) const;
    
    /**
     * Get assembler statistics
     */
    void getAssemblerStats(uint32_t& shortMsgs, uint32_t& longMsgs,
                          uint32_t& longStarts, uint32_t& longConts,
                          uint32_t& contErrors, uint32_t& pendingOverflows,
                          uint32_t& staleReplacements, uint8_t& pendingCount,
                          uint8_t& maxPendingCount) const;
    
    /**
     * Reset router state (clears pending messages)
     */
    void reset();
    
    /**
     * Get number of registered channels
     */
    uint8_t getChannelCount() const { return channelCount; }
    
private:
    // Registered channels
    BapChannel* channels[MAX_CHANNELS];
    uint8_t channelCount = 0;
    
    // Frame assembler - handles short/long message abstraction
    BapProtocol::BapFrameAssembler frameAssembler;
    
    // Statistics
    volatile uint32_t totalFramesProcessed = 0;
    volatile uint32_t completeMessagesRouted = 0;
    volatile uint32_t shortMessagesRouted = 0;
    volatile uint32_t longMessagesRouted = 0;
    
    /**
     * Find channel that handles a given CAN ID
     * @return Channel pointer or nullptr if not found
     */
    BapChannel* findChannel(uint32_t canId);
};
