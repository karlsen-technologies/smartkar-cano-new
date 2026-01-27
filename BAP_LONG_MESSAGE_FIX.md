# BAP Long Message Parsing Fix

## Problem Summary

Long BAP messages are not being parsed correctly. The issue is in the extraction and matching of the message index field in `BapProtocol.cpp`.

## Root Cause

The current code extracts the message index incorrectly:

```cpp
// WRONG - extracts bits 4-5 unshifted, giving 0x00/0x10/0x20/0x30
header.messageIndex = firstByte & 0x30;
```

## Correct Byte 0 Format

According to the BAP protocol documentation, Byte 0 of long frames has this structure:

```
Byte 0: [Long:1][Cont:1][Group:2][Index:4]
         Bit 7   Bit 6   Bits 5-4  Bits 3-0

Long (Bit 7):     1 = Long message, 0 = Short message
Cont (Bit 6):     0 = Start frame, 1 = Continuation frame
Group (Bits 5-4): Message group (0-3), allows 4 concurrent streams
Index (Bits 3-0): Sequence counter (0-15), wraps after 15
```

### Group Values

| Group | Start Byte0 | Continuation Range |
|-------|-------------|-------------------|
| 0     | 0x80        | 0xC0 - 0xCF       |
| 1     | 0x90        | 0xD0 - 0xDF       |
| 2     | 0xA0        | 0xE0 - 0xEF       |
| 3     | 0xB0        | 0xF0 - 0xFF       |

### Key Facts

- START frames always have Index = 0
- First continuation frame also has Index = 0
- Index increments with each subsequent continuation
- Index wraps from 15 back to 0 (stays within same group)
- Maximum 4 concurrent messages per CAN ID (one per group)

## Fix Plan

### 1. Update `BapProtocol.h`

#### 1.1 Update `BapHeader` struct

Rename `messageIndex` to `group`, add `index` field:

```cpp
struct BapHeader {
    uint8_t opcode = 0;
    uint8_t deviceId = 0;
    uint8_t functionId = 0;
    bool isLong = false;
    bool isContinuation = false;
    uint8_t group = 0;          // Bits 5-4 (0-3)
    uint8_t index = 0;          // Bits 3-0 (0-15)
    uint8_t totalLength = 0;
    // ...
};
```

#### 1.2 Update `PendingMessage` struct

```cpp
struct PendingMessage {
    bool active = false;
    uint8_t group = 0;              // Message group (0-3)
    uint8_t nextExpectedIndex = 0;  // Next continuation index expected (0-15)
    // ... rest unchanged
};
```

#### 1.3 Update `findPendingMessage` signature

```cpp
int8_t findPendingMessage(uint8_t group, uint8_t index);
```

### 2. Update `BapProtocol.cpp`

#### 2.1 Fix `decodeHeader()`

```cpp
// For long start frame:
header.group = (firstByte >> 4) & 0x03;  // Bits 5-4
header.index = firstByte & 0x0F;          // Bits 3-0

// For long continuation frame:
header.group = (firstByte >> 4) & 0x03;  // Bits 5-4
header.index = firstByte & 0x0F;          // Bits 3-0
```

#### 2.2 Update `findPendingMessage()`

Match by group AND expected index:

```cpp
int8_t BapFrameAssembler::findPendingMessage(uint8_t group, uint8_t index) {
    for (int8_t i = pendingCount - 1; i >= 0; i--) {
        if (pending[i].active && 
            pending[i].group == group &&
            pending[i].nextExpectedIndex == index &&
            pending[i].assembledLength < pending[i].expectedLength) {
            return i;
        }
    }
    return -1;
}
```

#### 2.3 Update continuation handling

After appending continuation data, increment expected index with wrap:

```cpp
pm.nextExpectedIndex = (pm.nextExpectedIndex + 1) & 0x0F;  // Wrap at 16
```

### 3. Update encoding functions

~~Update `encodeLongStart()` and `encodeLongContinuation()` to use group/index terminology.~~

**COMPLETED** - Encoding functions now use correct bit format:
- `encodeLongStart()`: `0x80 | ((group & 0x03) << 4)` for start frames
- `encodeLongContinuation()`: `0xC0 | ((group & 0x03) << 4) | (index & 0x0F)` for continuation frames

## Test Data

Trace file has 78 frames for channel 0x17332510, resulting in 22 complete BAP messages.

Example message #4 (11 continuations):
```
START:  80 4F ...  -> group=0, index=0
C0 ...             -> group=0, index=0
C1 ...             -> group=0, index=1
...
CA ...             -> group=0, index=10
```

Example message #5 (4 continuations, group 1):
```
START:  90 1F ...  -> group=1, index=0
D0 ...             -> group=1, index=0
D1 ...             -> group=1, index=1
D2 ...             -> group=1, index=2
D3 ...             -> group=1, index=3
```

## Files to Modify

- [x] `src/vehicle/protocols/BapProtocol.h` - Decoding structs and encoding declarations
- [x] `src/vehicle/protocols/BapProtocol.cpp` - Decoding and encoding implementations

## Status: COMPLETE

All parsing and encoding functions now correctly handle the BAP long message control byte format:
- `decodeHeader()` correctly extracts group (bits 5-4) and index (bits 3-0)
- `findPendingMessage()` matches by group and expected index
- `processFrame()` increments nextExpectedIndex after each continuation
- `encodeLongStart()` encodes group in bits 5-4
- `encodeLongContinuation()` encodes group in bits 5-4 and index in bits 3-0

## Verification

After fix, all 22 messages from the trace should decode correctly.
