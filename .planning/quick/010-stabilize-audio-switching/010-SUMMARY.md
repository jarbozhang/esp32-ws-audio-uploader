# Quick Task 010 Summary: Stabilize Audio Switching

## Actions
- **Updated `src/AudioManager.cpp`**:
    - Added `delay(10)` after `M5.Mic.end()` and `M5.Speaker.begin()` in `playPendingBeeps`.
    - This is intended to give the I2S hardware time to release/acquire resources during the mode switch.

## Verification
- User should flash and check if the startup beep and connection beep are now audible.
- The logs proved the logic flow is correct, so this targets the hardware/driver layer.
