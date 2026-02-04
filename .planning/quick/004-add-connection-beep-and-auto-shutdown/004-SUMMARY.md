# Quick Task 004 Summary: Add Connection Beep and Auto Shutdown

## Actions
- **Connection Beep:** Updated `src/NetworkManager.cpp` to play `BEEP_START` upon successful WebSocket connection.
- **Auto Shutdown:** Updated `src/main.cpp` to track user activity (buttons, recording) and turn off WiFi after 30 minutes of inactivity.
    - Added `checkAutoShutdown()` function.
    - Added `updateActivity()` calls on button presses.
    - Shutdown behavior: Disconnect WiFi, turn mode to OFF, play stop beep, and enter infinite loop (effectively sleeping until power cut or reset).

## Verification
- Code structure is consistent.
- `BEEP_START` is reused from `AudioMgr`.
- `AUTO_SHUTDOWN_MS` set to 30 minutes.
