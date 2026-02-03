# Phase 3 Research: Generate Testing Methods

**Phase Goal:** Establish comprehensive testing strategies and methods for the refactored system.

## 1. Testing Strategy Overview

Given the embedded nature of the project (ESP32) and the refactored modular structure, a mix of manual, unit, and integration testing is appropriate.

**Levels of Testing:**
1.  **Unit Testing (Host-based):** Test pure logic modules (like `Config`, state machines in `NetworkManager` or `AudioManager` if decoupled from hardware) on the development machine (Mac).
2.  **Hardware-in-the-Loop (HIL) / Device Testing:** Test hardware-dependent features (WiFi, Audio I2S, WebSocket) on the ESP32 itself.
3.  **System/E2E Testing:** Verify the full flow: Button Press -> Audio -> WebSocket -> Server -> ASR -> Response -> Beep.

## 2. Unit Testing (Host)

**Framework:** `Unity` (Standard PlatformIO/Arduino testing framework).

**Feasibility:**
-   **Config:** Easy to test struct parsing/defaults.
-   **Managers:** Difficult without mocking Arduino/ESP32 APIs.
-   **Strategy:** Skip extensive host-based unit testing for now unless we extract pure logic classes. Focus on device testing.

## 3. Device Testing (On-Target)

**Framework:** `Unity` running on ESP32.

**Test Cases:**
-   **Network:**
    -   Connect to WiFi (verify `WiFi.status() == WL_CONNECTED`).
    -   Resolve mDNS (verify IP is not null).
    -   Connect WebSocket (verify `isConnected()` returns true).
-   **Audio:**
    -   Verify `M5.Mic.begin()` returns success.
    -   Record a short chunk and verify non-zero data (simple energy check).
    -   Verify `queueBeep` updates state.

## 4. Manual / E2E Testing Procedures

**Audio Quality:**
-   **Method:** Record audio, send to server, save raw PCM on server, analyze with Audacity.
-   **Checklist:** No glitches, correct sample rate/speed, clear voice.

**Connectivity:**
-   **Method:** Monitor Serial output during boot.
-   **Checklist:** WiFi connects, mDNS resolves `jiabos-macbook-pro-2.local`, WebSocket connects.

**Latency:**
-   **Method:** Measure time from "Button Release" to "Beep Start" (using stopwatch or logs).

## 5. Automation Strategy

-   **Serial Monitor Parsing:** A Python script can monitor ESP32 serial output for specific log keywords ("Recording start", "WS connected") to automate pass/fail checks for integration scenarios.
-   **Mock Server:** A simple Python WebSocket server can replace the Mac ASR server to verify ESP32 protocol compliance (handshake, binary data receipt, JSON formats) without running the full heavy ASR model.

## 6. Deliverables for this Phase

-   **Test Plan Document:** Detailed steps for manual verification.
-   **Test Scripts:**
    -   `tests/test_main.cpp`: Unity tests for ESP32.
    -   `scripts/mock_server.py`: Python script to simulate ASR server.
-   **Documentation:** How to run tests.
