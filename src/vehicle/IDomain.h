#pragma once

#include <cstdint>

/**
 * IDomain - Interface for all domain managers
 * 
 * Domain managers are responsible for a specific functional area of the vehicle
 * (e.g., Battery, Climate, Body, Drive, etc.). They:
 * - Process standard CAN frames (11-bit IDs)
 * - Subscribe to BAP channel callbacks for extended frames (29-bit IDs)
 * - Maintain domain-specific state
 * - Provide clean public API for external consumers
 * - Support wake management and busy states
 * 
 * Key Principles:
 * - Domains do NOT process BAP frames directly (channels handle that)
 * - Domain state is owned by the domain, not shared
 * - Processing should be fast (<1-2ms on CAN thread)
 * - No business logic on CAN thread, just data copying
 * 
 * Thread Safety:
 * - processCanFrame() called from CAN task (Core 0) with mutex held
 * - loop() called from main loop (Core 1)
 * - All state access must be thread-safe
 */
class IDomain {
public:
    virtual ~IDomain() = default;

    /**
     * Get domain name for logging/debugging.
     * @return Domain name (e.g., "Battery", "Climate", "Body")
     */
    virtual const char* getName() const = 0;

    /**
     * Initialize the domain.
     * Called once during VehicleManager setup.
     * @return true if initialization succeeded
     */
    virtual bool setup() = 0;

    /**
     * Process periodic tasks.
     * Called from main loop on Core 1.
     * Use for command state machines, timeouts, etc.
     */
    virtual void loop() = 0;

    /**
     * Process a standard CAN frame (11-bit ID).
     * Called from CAN task on Core 0 with mutex held.
     * MUST be fast (<1-2ms) to avoid blocking CAN thread.
     * 
     * @param canId Standard CAN ID (11-bit)
     * @param data Frame data (max 8 bytes)
     * @param dlc Data length code (1-8)
     */
    virtual void processCanFrame(uint32_t canId, const uint8_t* data, uint8_t dlc) = 0;

    /**
     * Called when vehicle wake sequence completes.
     * Optional: Override if domain needs to perform actions after wake.
     */
    virtual void onWakeComplete() {}

    /**
     * Check if domain is busy with an operation.
     * Used by wake manager to wait for operations to complete before sleep.
     * @return true if domain has pending operations
     */
    virtual bool isBusy() const { return false; }
};
