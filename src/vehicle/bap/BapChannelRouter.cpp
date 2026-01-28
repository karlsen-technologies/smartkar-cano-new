#include "BapChannelRouter.h"

BapChannelRouter::BapChannelRouter() {
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        channels[i] = nullptr;
    }
}

bool BapChannelRouter::registerChannel(BapChannel* channel) {
    if (!channel || channelCount >= MAX_CHANNELS) {
        return false;
    }
    
    channels[channelCount++] = channel;
    return true;
}

bool BapChannelRouter::processFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) {
    // EARLY FILTER: Check if any registered channel cares about this CAN ID
    // This avoids wasting CPU cycles on frame assembly for BAP channels we don't use
    // (there are 20+ BAP channels on the bus, but we only care about a few)
    BapChannel* channel = findChannel(canId);
    if (!channel) {
        // No channel registered for this CAN ID - ignore silently
        return false;
    }
    
    totalFramesProcessed++;
    
    // Attempt to assemble message (handles short + long multi-frame)
    BapProtocol::BapMessage msg;
    bool messageComplete = frameAssembler.processFrame(data, dlc, msg);
    
    if (!messageComplete) {
        // Frame was part of a long message, but not yet complete
        // This is normal - not an error
        return true;  // Frame was processed (accepted)
    }
    
    // We have a complete message - route it to the channel
    bool handled = channel->processMessage(msg);
    if (handled) {
        completeMessagesRouted++;
        
        // Update specific message type counters
        if (msg.payloadLen <= 6) {
            shortMessagesRouted++;
        } else {
            longMessagesRouted++;
        }
    }
    
    return handled;
}

BapChannel* BapChannelRouter::findChannel(uint32_t canId) {
    for (uint8_t i = 0; i < channelCount; i++) {
        if (channels[i] && channels[i]->handlesCanId(canId)) {
            return channels[i];
        }
    }
    return nullptr;
}

void BapChannelRouter::getStats(uint32_t& totalFrames, uint32_t& completeMessages,
                                uint32_t& shortMessages, uint32_t& longMessages) const {
    totalFrames = totalFramesProcessed;
    completeMessages = completeMessagesRouted;
    shortMessages = shortMessagesRouted;
    longMessages = longMessagesRouted;
}

void BapChannelRouter::getAssemblerStats(uint32_t& shortMsgs, uint32_t& longMsgs,
                                        uint32_t& longStarts, uint32_t& longConts,
                                        uint32_t& contErrors, uint32_t& pendingOverflows,
                                        uint32_t& staleReplacements, uint8_t& pendingCount,
                                        uint8_t& maxPendingCount) const {
    shortMsgs = frameAssembler.shortMessagesDecoded;
    longMsgs = frameAssembler.longMessagesDecoded;
    longStarts = frameAssembler.longStartFrames;
    longConts = frameAssembler.longContFrames;
    contErrors = frameAssembler.continuationErrors;
    pendingOverflows = frameAssembler.pendingOverflows;
    staleReplacements = frameAssembler.staleReplacements;
    pendingCount = frameAssembler.getPendingCount();
    maxPendingCount = frameAssembler.maxPendingCount;
}

void BapChannelRouter::reset() {
    frameAssembler.reset();
}
