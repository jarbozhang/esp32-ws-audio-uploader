# Quick Task 008 Summary: Refactor AudioManager to always queue beeps

## Actions
- **Updated `src/AudioManager.cpp`**:
    - Modified `AudioManager::queueBeep` to always queue beeps by incrementing the respective `_pendingX` counters.
    - Removed the conditional logic for immediate playback (the `if (_recording)` block and its `else` branch).
    - Removed the `Serial.printf` debug message related to immediate playback.

## Rationale
- This ensures all beeps are processed and played through the `AudioMgr.update()` function in the main loop, providing a more consistent and reliable playback mechanism, especially for `BEEP_START` during WebSocket connection. This aligns the behavior of all beeps with the working "broadcast message" beep logic.

## Verification
- Compilation should pass.
- The `BEEP_START` sound should now play reliably upon WebSocket connection, as it will be handled asynchronously in the main loop like other queued beeps.
