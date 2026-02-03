---
wave: 1
depends_on: []
files_modified:
  - tests/README.md
  - tests/test_main.cpp
  - scripts/mock_server.py
  - platformio.ini
autonomous: true
---

# Phase 3: Generate Testing Methods

**Goal:** Establish comprehensive testing strategies, including a manual test plan, on-device unit tests, and a mock server for protocol verification.

## Plan Breakdown

### 1. Create Test Plan Documentation
**Goal:** Document manual testing procedures for E2E scenarios.

- [ ] Create `tests/README.md`.
- [ ] Define "Manual Verification Checklist":
    - [ ] **Connectivity:** WiFi status, mDNS resolution, WS connection.
    - [ ] **Audio:** Record > 3s, verify server receipt, verify audio clarity.
    - [ ] **Latency:** Time from button release to beep.
    - [ ] **Hook Events:** Verify beep patterns for Permission/Failure/Stop.
- [ ] Document how to run Unity tests and the mock server.

### 2. Implement Mock ASR Server
**Goal:** Create a lightweight Python server to verify ESP32 protocol compliance without the heavy ASR backend.

- [ ] Create `scripts/mock_server.py`.
- [ ] Implement WebSocket server using `websockets` library (asyncio).
- [ ] Handle `start` message: validate JSON fields.
- [ ] Handle binary audio: calculate duration/size, log receipt.
- [ ] Handle `end` message: send mock `ack` and `result`.
- [ ] Add CLI feature to broadcast hook events (e.g., press 'p' to send PermissionRequest).

### 3. Configure On-Device Testing (Unity)
**Goal:** Enable running Unity tests on the ESP32.

- [ ] Update `platformio.ini`:
    - [ ] Add `test_build_src = yes` (to access app code).
    - [ ] Add `test_ignore = test_desktop`.
- [ ] Create `tests/test_main.cpp`:
    - [ ] `test_config_loading`: Verify default WiFi list size > 0.
    - [ ] `test_network_manager_state`: Verify initial state is disconnected.
    - [ ] `test_audio_manager_state`: Verify initial state is not recording.

## Verification

- [ ] **Documentation:** `tests/README.md` exists and covers manual scenarios.
- [ ] **Mock Server:** `python scripts/mock_server.py` starts and accepts connections (verify with `wscat` or actual device).
- [ ] **Unit Tests:** `pio test -e esp32-s3` compiles and runs (may fail if hardware not present, but build must pass).
