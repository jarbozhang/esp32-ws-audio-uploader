# Plan: Debug Connection Beep

The user reports that the beep does not play upon WebSocket connection, despite the code apparently being in place.

## Potential Causes
1.  **Main Loop Blocking:** `AudioMgr.queueBeep` might rely on `AudioMgr.update()` being called frequently. If `NetworkMgr.loop()` or other tasks block the main loop immediately after connection, the beep logic (which might be async or state-machine based) might not execute.
2.  **AudioMgr State:** `AudioMgr` might be in a state where it can't play beeps (e.g., thinking it's recording, or M5Speaker not initialized correctly).
3.  **Speaker vs Mic Conflict:** `AudioManager::queueBeep` switches from Mic to Speaker. If this transition fails or isn't given enough time/cycles, it might be silent.
4.  **Immediate Beep Logic:** `queueBeep` plays immediately if *not* recording using `M5.Speaker.tone` followed by `delay`. This *should* block and play sound. If it's silent, maybe the speaker isn't really enabled or the volume is zero.

## Tasks

### 1. Verify AudioMgr.update() and loop integration
- **File:** `src/main.cpp`
- **Action:** Check `loop()` to ensure `AudioMgr.update()` is called. (It is, based on previous context).

### 2. Force Speaker Config/Volume
- **File:** `src/AudioManager.cpp`
- **Action:**
    - In `queueBeep`, ensure we set a non-zero volume if M5Unified requires it.
    - Add Serial logs in `queueBeep` to confirm it is actually entered and the `kind` matches `BEEP_START`.
    - Check if `M5.Speaker.begin()` needs config.

### 3. Add Diagnostics
- **File:** `src/NetworkManager.cpp`
- **Action:** Add `Serial.println("Queueing BEEP_START...");` before calling `AudioMgr.queueBeep`.

### 4. Direct Tone Test (Fallback)
- **File:** `src/NetworkManager.cpp`
- **Action:** If `AudioMgr` is complex, try a direct `M5.Speaker.tone()` in `NetworkManager` just to prove the hardware works (though this breaks encapsulation, it's a good debug step). *Better:* Modify `AudioManager::queueBeep` to print debug info.

## Refined Plan
1.  **Instrument `AudioManager::queueBeep`**: Add `Serial.printf("Playing beep kind: %d\n", kind);` and check if it's hitting the "not recording" path.
2.  **Force Volume**: Explicitly set `M5.Speaker.setVolume(255);` (or a reasonable value) inside `queueBeep` before playing. M5Unified speaker volume defaults can sometimes be low or 0.
3.  **Check `BEEP_START` pattern**: Ensure the frequency (2400Hz) and duration (100ms) are audible.

Let's start by adding debug prints and ensuring volume is set.
