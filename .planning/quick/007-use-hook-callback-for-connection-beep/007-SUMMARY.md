# Quick Task 007 Summary: Use Hook Callback for Connection Beep

## Actions
- **Updated `src/NetworkManager.cpp`**: Removed direct dependency on `AudioManager.h`. Replaced `AudioMgr.queueBeep(BEEP_START)` with `_hookCallback("Connected")`.
- **Updated `src/main.cpp`**: Added handling for the "Connected" event in `onHookEvent`, which triggers `AudioMgr.queueBeep(BEEP_START)`.

## Rationale
- This approach mimics the working logic for broadcast messages (PermissionRequest, etc.).
- It decouples `NetworkManager` from `AudioManager`, ensuring `NetworkManager` only deals with networking events and delegates application responses (like UI/Audio) to the main controller via callbacks.

## Verification
- Compilation should pass.
- User should verify that the beep now plays on connection.
