# Plan: Deep Debug Beep Failure

To identify why the beep is not playing, we will instrument the code with debug logs at every step of the event chain and perform a startup test.

## Tasks

### 1. Instrument NetworkManager
- **File:** `src/NetworkManager.cpp`
- **Action:**
    - In `WStype_CONNECTED`, add `Serial.println("DEBUG: [NM] WS Connected event received");`.
    - Before calling `_hookCallback("Connected")`, add `Serial.println("DEBUG: [NM] Calling Connected hook");`.

### 2. Instrument Main Handler
- **File:** `src/main.cpp`
- **Action:**
    - In `onHookEvent`, add `Serial.printf("DEBUG: [Main] Hook event: %s\n", eventName);`.
    - Inside the "Connected" block, add `Serial.println("DEBUG: [Main] Queueing BEEP_START");`.
    - **Startup Test:** In `setup()`, immediately after `AudioMgr.begin()`, add:
        ```cpp
        Serial.println("DEBUG: [Setup] Testing startup beep...");
        AudioMgr.queueBeep(BEEP_START);
        ```

### 3. Instrument AudioManager
- **File:** `src/AudioManager.cpp`
- **Action:**
    - In `queueBeep`, add `Serial.printf("DEBUG: [Audio] Queueing kind %d. PendingStart: %d\n", kind, _pendingStart + 1);`.
    - In `playPendingBeeps`, add a check:
        ```cpp
        if (_pendingStop || _pendingPermission || _pendingFailure || _pendingStart) {
            Serial.printf("DEBUG: [Audio] Playing pending. Start=%d, Perm=%d, Fail=%d, Stop=%d\n", 
                _pendingStart, _pendingPermission, _pendingFailure, _pendingStop);
        }
        ```
    - In `update`, add a throttled log (once every second) if `_pendingStart > 0` to see if it's stuck in the queue:
        ```cpp
        static unsigned long lastLog = 0;
        if (_pendingStart > 0 && millis() - lastLog > 1000) {
             Serial.printf("DEBUG: [Audio] Update loop. Pending Start: %d\n", _pendingStart);
             lastLog = millis();
        }
        ```

### 4. Verify
- **Action:** Compile and instruct the user to check Serial logs.
