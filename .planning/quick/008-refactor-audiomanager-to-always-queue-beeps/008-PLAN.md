# Plan: Refactor AudioManager to always queue beeps

The current `AudioManager::queueBeep` method has a conditional logic that attempts immediate playback if `_recording` is false. This can cause issues with timing and stability, especially during WebSocket connection events. To ensure consistency and reliability, especially for the `BEEP_START` event, we will refactor `queueBeep` to always queue the beep, letting `AudioMgr.update()` (which calls `playPendingBeeps()`) handle all actual audio output in the main loop context.

## Tasks

### 1. Refactor `AudioManager::queueBeep`
- **File:** `src/AudioManager.cpp`
- **Action:**
    - Remove the `if (_recording)` block and the subsequent `else` block that performs immediate playback.
    - Modify the function so that it always increments the respective `_pendingX` counter based on `BeepKind`.
    - Remove the `Serial.printf` debug message that was specific to immediate playback, as it will no longer be relevant in this context.

### 2. Verify
- **Action:** Compile the project to ensure no errors.
