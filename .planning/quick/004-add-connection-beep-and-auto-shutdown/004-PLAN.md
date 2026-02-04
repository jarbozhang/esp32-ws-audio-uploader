# Plan: Add Connection Beep and Auto Shutdown

The user wants two specific UX improvements:
1.  **Audio Feedback:** Play a beep when the WebSocket connection is established.
2.  **Power Management:** Turn off WiFi after 30 minutes of inactivity to allow the external battery bank to power down.

## Tasks

### 1. Implement Connection Beep
- **File:** `src/NetworkManager.cpp`
- **Action:**
    - In `webSocketEvent`, under `WStype_CONNECTED`, call `AudioMgr.queueBeep(BEEP_START)` (or a new beep type if needed, but `BEEP_START` seems appropriate for "ready").
    - Ensure `AudioManager.h` is included if not already (it likely is or needs to be via `AppNetworkManager` context, or we pass a callback).
    - *Correction:* `NetworkManager` doesn't directly know about `AudioMgr`. `NetworkManager` has a hook callback system, but that's for incoming JSON events.
    - **Refinement:** We should probably expose a "Connection State" callback or just use the existing `AudioMgr` global if it's already a singleton used in `main.cpp`.
    - Checking `src/main.cpp`: `AudioMgr` is a global instance. `NetworkManager.cpp` does *not* include `AudioManager.h` currently.
    - **Decision:** Include `AudioManager.h` in `NetworkManager.cpp` and call `AudioMgr.queueBeep(BEEP_START)` on connection.

### 2. Implement Inactivity Shutdown
- **File:** `src/main.cpp`
- **Action:**
    - Define a timeout constant: `const unsigned long AUTO_SHUTDOWN_MS = 30 * 60 * 1000;` (30 mins).
    - Track `lastActivityMs`. This needs to be updated whenever:
        - A button is pressed.
        - Audio is recorded.
        - WebSocket message is received/sent (optional, but "user interaction" is usually physical). Let's stick to physical interaction (buttons/recording) or maybe significant network events (commands received).
    - In `loop()`, check `if (millis() - lastActivityMs > AUTO_SHUTDOWN_MS)`.
    - If timeout reached:
        - `WiFi.disconnect(true);` (Turn off WiFi).
        - `WiFi.mode(WIFI_OFF);`
        - Play a "shutdown" beep (optional but good).
        - Enter a deep sleep or just hang in a low-power loop (if we want the battery to cut power, dropping current is key). `esp_deep_sleep_start()` is probably best as it drops consumption to micro-amps.

### 3. Integration
- **File:** `src/main.cpp`
- **Action:**
    - Ensure `lastActivityMs` is updated on all button presses and recording events.
    - Currently `lastActivityMs` exists in `main.cpp` for the keep-alive pulse. Reuse/adapt this variable.

## Execution Steps
1.  **Modify `src/NetworkManager.cpp`**: Add `AudioManager.h` include and play beep on `WStype_CONNECTED`.
2.  **Modify `src/main.cpp`**: Implement the 30-minute timeout logic using the existing `lastActivityMs` (or ensuring it's updated correctly) and trigger deep sleep/WiFi off.
