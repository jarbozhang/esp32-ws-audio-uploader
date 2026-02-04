# Plan: Use Hook Callback for Connection Beep

The user wants to use the existing hook event mechanism (used for broadcast messages) to trigger the connection beep. This ensures consistency and proper context for audio playback.

## Tasks

### 1. Update NetworkManager
- **File:** `src/NetworkManager.cpp`
- **Action:**
    - Remove `#include "AudioManager.h"`.
    - In `webSocketEvent` case `WStype_CONNECTED`:
        - Remove `AudioMgr.queueBeep(BEEP_START);`.
        - Add `if (_hookCallback) _hookCallback("Connected");`.

### 2. Update Main Handler
- **File:** `src/main.cpp`
- **Action:**
    - In `onHookEvent(const char* eventName)`:
        - Add a check for `"Connected"`.
        - Call `AudioMgr.queueBeep(BEEP_START);` inside that check.

### 3. Verification
- **Action:** Compile to ensure no linking errors (AudioMgr should be available in main.cpp).
