#include "ActivityTracker.h"

ActivityTracker::ActivityTracker() {
}

bool ActivityTracker::setup() {
    Serial.println("[ActivityTracker] Initialized");
    Serial.println("[ActivityTracker]   - Tracks CAN bus activity");
    Serial.println("[ActivityTracker]   - Default timeout: 5000ms");
    lastActivity = millis();
    frameCount = 0;
    return true;
}

void ActivityTracker::onCanActivity() {
    // Called on every CAN frame from CAN task (Core 0)
    // Keep this VERY fast - just update counters
    lastActivity = millis();
    frameCount++;
}

bool ActivityTracker::isActive(uint32_t timeoutMs) const {
    unsigned long now = millis();
    unsigned long elapsed = now - lastActivity;
    return elapsed < timeoutMs;
}
