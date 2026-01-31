#pragma once

#include <Arduino.h>
#include <cstdint>

/**
 * ActivityTracker - Tracks CAN bus activity for sleep management
 * 
 * Lightweight tracker that monitors when CAN frames are received.
 * Used by WakeController and domains to determine if vehicle is active.
 * 
 * Thread-safe: Can be called from CAN task (Core 0) and main loop (Core 1)
 * Uses volatile members for thread safety without mutex overhead.
 */
class ActivityTracker {
public:
    /**
     * Construct the activity tracker.
     */
    ActivityTracker();

    /**
     * Initialize the tracker.
     * @return true if initialization succeeded
     */
    bool setup();

    /**
     * Notify of CAN activity (called on every frame).
     * Thread-safe: can be called from CAN task.
     */
    void onCanActivity();

    /**
     * Check if CAN bus is currently active.
     * @param timeoutMs How long since last activity is still considered active (default: 5000ms)
     * @return true if activity within timeout
     */
    bool isActive(uint32_t timeoutMs = 5000) const;

    /**
     * Get time of last CAN activity.
     * @return millis() timestamp of last frame
     */
    unsigned long getLastActivityTime() const { return lastActivity; }

    /**
     * Get total count of CAN frames seen.
     * @return frame count
     */
    uint32_t getFrameCount() const { return frameCount; }

    /**
     * Reset frame count (for testing/debugging).
     */
    void resetFrameCount() { frameCount = 0; }

private:
    // Volatile for thread safety (accessed from CAN task and main loop)
    volatile unsigned long lastActivity = 0;
    volatile uint32_t frameCount = 0;
};
