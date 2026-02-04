# Plan: Stabilize Audio Switching

The debug logs confirm that the beep logic is executed correctly, but the device remains silent. This points to a hardware resource conflict or timing issue when switching between Microphone and Speaker modes using `M5Unified`. To address this, we will introduce delays during the mode switch to allow the I2S bus and hardware to stabilize.

## Tasks

### 1. Stabilize `playPendingBeeps`
- **File:** `src/AudioManager.cpp`
- **Action:**
    - Insert `delay(10);` after `M5.Mic.end()`.
    - Insert `delay(10);` after `M5.Speaker.begin()`.
    - Keep `M5.Speaker.setVolume(128)`.
    - Keep debug logs for now to verify this specific attempt, then we can clean them up in a later task.

### 2. Verify
- **Action:** Compile and ask the user to test. If sound plays, the timing was the issue.
