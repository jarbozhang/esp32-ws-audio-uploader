# Phase 2 Research: Refactor and Advanced Connectivity

**Phase Goal:** Refactor monolithic code, implement multi-WiFi support, and add mDNS service discovery.

## 1. Code Refactoring Strategy

**Current State:**
- Single `src/main.cpp` (~200 lines).
- Mixes logic: Audio (M5Unified), Network (WiFi/WebSocket), Config, Business Logic (Beeps, State).

**Recommended Structure:**
- `src/Config.h`: Centralized configuration (WiFi lists, Tokens, Pins).
- `src/NetworkManager.h/cpp`: Handles WiFi connection (Multi-WiFi), mDNS, and WebSocket lifecycle.
- `src/AudioManager.h/cpp`: Handles Mic/Speaker exclusion, recording, and beep generation.
- `src/main.cpp`: Orchestration only (setup, loop).

**Arduino/PlatformIO Constraints:**
- Must use `.cpp` and `.h` files in `src/`.
- Avoid complex dependency injection; use singleton pattern or global instances for managers on embedded.

## 2. Multi-WiFi Implementation

**Library:** `WiFiMulti` (Standard ESP32 Arduino Library).

**Usage:**
```cpp
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

void setup() {
    wifiMulti.addAP("SSID1", "PASS1");
    wifiMulti.addAP("SSID2", "PASS2");
}

void loop() {
    if(wifiMulti.run() == WL_CONNECTED) {
        // Connected
    }
}
```

**Pros:** Handles scanning and connecting to the strongest known AP automatically.
**Cons:** Blocking behavior in `run()` can be tuned.

## 3. mDNS Service Discovery

**Goal:** Resolve `hostname.local` to IP address automatically.

**Library:** `ESPmDNS` (Standard ESP32 Arduino Library).

**Implementation:**
1. **Resolution (Client Mode):**
   - The ESP32 needs to *find* the Mac server.
   - Use `MDNS.begin("esp32-client")` to start mDNS responder (optional but good).
   - Use `MDNS.queryHost(serverHostname)` to resolve IP.

**Caveat:** `MDNS.queryHost` is blocking.
- **Strategy:** Resolve IP *once* during setup or connection phase, then connect WebSocket to that IP. Re-resolve if connection fails.

## 4. Configuration Management

**Requirement:** "Support list of WiFi credentials... [and] mDNS hostname... configurable".

**Approach:**
- Move hardcoded strings to `src/Config.h`.
- Use `std::vector<std::pair<const char*, const char*>>` or simple struct array for WiFi credentials.

## 5. Risks & Mitigations

- **Blocking Calls:** WiFi connection and mDNS resolution are blocking.
  - *Mitigation:* Perform these during `setup()` or a specific "Connecting" state, indicated by a specific LED color or Beep pattern.
- **Memory:** Splitting files adds minimal overhead.
- **Refactoring Bug:** Breaking audio/ws timing.
  - *Mitigation:* Test audio loop strictly after refactoring.
