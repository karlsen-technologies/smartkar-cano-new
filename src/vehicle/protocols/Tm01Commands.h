#pragma once

#include <Arduino.h>

/**
 * Tm01Commands - TM_01 (0x5A7) command builder for horn/flash/lock operations
 * 
 * CAN ID: 0x5A7 (1447)
 * DLC: 8 bytes
 * 
 * This is a direct broadcast command that doesn't require BAP protocol.
 * The car responds immediately to these commands.
 * 
 * Byte 6 contains the command bits:
 *   bit 0 (0x01) = Horn
 *   bit 1 (0x02) = Lock
 *   bit 2 (0x04) = Unlock
 *   bit 3 (0x08) = Flash lights (CONFIRMED WORKING)
 *   bit 4 (0x10) = Panic (horn + flash)
 * 
 * Note: Lock/unlock may require the signature field (bytes 0-5) to be valid.
 * Flash is confirmed working without signature.
 */
namespace Tm01Commands {

// CAN ID for TM_01 message
constexpr uint32_t TM_01_CAN_ID = 0x5A7;

// Command bits in byte 6
constexpr uint8_t CMD_HORN   = 0x01;
constexpr uint8_t CMD_LOCK   = 0x02;
constexpr uint8_t CMD_UNLOCK = 0x04;
constexpr uint8_t CMD_FLASH  = 0x08;
constexpr uint8_t CMD_PANIC  = 0x10;

/**
 * Command types for TM_01
 */
enum class Command : uint8_t {
    NONE = 0,
    HORN = CMD_HORN,
    LOCK = CMD_LOCK,
    UNLOCK = CMD_UNLOCK,
    FLASH = CMD_FLASH,
    PANIC = CMD_PANIC
};

/**
 * Build a TM_01 command frame.
 * 
 * @param cmd The command to execute
 * @param data Output buffer (must be 8 bytes)
 */
inline void buildCommand(Command cmd, uint8_t* data) {
    // Clear all bytes
    memset(data, 0, 8);
    
    // Set command in byte 6
    data[6] = static_cast<uint8_t>(cmd);
}

/**
 * Build a horn command.
 */
inline void buildHornCommand(uint8_t* data) {
    buildCommand(Command::HORN, data);
}

/**
 * Build a lock command.
 */
inline void buildLockCommand(uint8_t* data) {
    buildCommand(Command::LOCK, data);
}

/**
 * Build an unlock command.
 */
inline void buildUnlockCommand(uint8_t* data) {
    buildCommand(Command::UNLOCK, data);
}

/**
 * Build a flash lights command.
 */
inline void buildFlashCommand(uint8_t* data) {
    buildCommand(Command::FLASH, data);
}

/**
 * Build a panic (horn + flash) command.
 */
inline void buildPanicCommand(uint8_t* data) {
    buildCommand(Command::PANIC, data);
}

/**
 * Get the CAN ID for TM_01 commands.
 */
inline uint32_t getCanId() {
    return TM_01_CAN_ID;
}

/**
 * Get command name for logging.
 */
inline const char* getCommandName(Command cmd) {
    switch (cmd) {
        case Command::HORN:   return "HORN";
        case Command::LOCK:   return "LOCK";
        case Command::UNLOCK: return "UNLOCK";
        case Command::FLASH:  return "FLASH";
        case Command::PANIC:  return "PANIC";
        default:              return "NONE";
    }
}

} // namespace Tm01Commands
