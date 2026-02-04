# Plan: Fix Beep Compile Error

The previous task introduced `BEEP_START` usage in `NetworkManager.cpp` which caused a compilation error because `BEEP_START` is not defined in `AudioManager.h`. I need to check the available beep types and either define `BEEP_START` or use an existing one.

## Tasks

### 1. Check AudioManager Definitions
- **File:** `src/AudioManager.h`
- **Action:** Read the file to see the `BeepType` enum definitions.

### 2. Update AudioManager or NetworkManager
- **Scenario A (Missing Definition):** If `BEEP_START` is missing but logical to have, add it to the `BeepType` enum in `src/AudioManager.h`.
- **Scenario B (Wrong Name):** If there is an existing beep that fits (e.g., `BEEP_WAKE`, `BEEP_CONNECT`), update `src/NetworkManager.cpp` to use that instead.
- **Preference:** Add `BEEP_START` if it represents a distinct "Connected/Ready" sound (e.g., a rising tone).

### 3. Verify Fix
- **Action:** Compile the project to ensure the error is resolved.
