# Plan: Increase Audio Delays and Volume

Previous attempts to fix the beep issue confirmed that the logic is correct, but the device remains silent. This suggests the audio hardware (I2S bus) or driver state needs more time or configuration to switch effectively between Microphone and Speaker modes.

## Tasks

### 1. Update `AudioManager.cpp`
- **File:** `src/AudioManager.cpp`
- **Action:**
    - Increase the stabilization delays from `10ms` to `100ms` in `playPendingBeeps`.
    - Increase volume from `128` to `255` (max) to ensure it's not a volume scaling issue.
    - Add a debug log to print the actual volume set: `Serial.printf("DEBUG: [Audio] Volume set to: %d\n", M5.Speaker.getVolume());`.

### 2. Verify
- **Action:** Compile and test.

