---
wave: 1
depends_on: []
files_modified:
  - src/main.cpp
  - src/Config.h
  - src/NetworkManager.h
  - src/NetworkManager.cpp
  - src/AudioManager.h
  - src/AudioManager.cpp
  - platformio.ini
autonomous: true
---

# Phase 2: Refactor and Connectivity Upgrade

**Goal:** Split the monolithic `main.cpp` into modular components and upgrade network logic to support multiple WiFi credentials and mDNS server discovery.

## Plan Breakdown

### 1. Refactor Audio Subsystem
**Goal:** Isolate M5Unified audio logic (Mic, Speaker, Beeps) into `AudioManager`.

- [ ] Create `src/AudioManager.h` and `src/AudioManager.cpp`.
- [ ] Move `BeepKind`, `BeepPattern`, and beep queue logic (`pendingStop`, etc.) to this class.
- [ ] Move `setupAudio()`, `recordOneChunkAndSend()`, and `loop()` (button handling) logic.
- [ ] Expose public methods:
    - `begin()`
    - `update()` (called in loop)
    - `queueBeep(BeepKind)`
    - `isRecording()`
    - `getAudioChunk()` (returns pointer/size for sending)

### 2. Refactor Network Subsystem
**Goal:** Isolate WiFi and WebSocket logic into `NetworkManager`.

- [ ] Create `src/NetworkManager.h` and `src/NetworkManager.cpp`.
- [ ] Implement `WiFiMulti` logic in `begin()` to support list of credentials.
- [ ] Move `WebSocketsClient` instance and event handling here.
- [ ] Implement mDNS resolution:
    - Add method `resolveHost(const char* hostname)` using `MDNS.queryHost()`. 
    - Call this before connecting WebSocket.
- [ ] Expose public methods:
    - `begin(const char* host, int port, const char* path)`
    - `loop()`
    - `sendAudio(uint8_t* data, size_t len)`
    - `sendJson(String json)`
    - `isConnected()`

### 3. Create Configuration Header
**Goal:** Centralize settings.

- [ ] Create `src/Config.h`.
- [ ] Define structs/vectors for WiFi credentials.
- [ ] Define macros/constants for `WS_HOSTNAME`, `WS_PORT`, `AUTH_TOKEN`.
- [ ] Define Audio constants (`SAMPLE_RATE`, etc.).

### 4. Update Main Orchestrator
**Goal:** Simplify `main.cpp` to just setup and loop managers.

- [ ] Include `Config.h`, `AudioManager.h`, `NetworkManager.h`.
- [ ] Instantiate global managers.
- [ ] `setup()`: Initialize Serial, Audio, Network.
- [ ] `loop()`: Call `Audio.update()`, `Network.loop()`. 
- [ ] Glue logic: Pass audio chunks from Audio -> Network; pass hook events from Network -> Audio.

## Verification

- [ ] **Compile:** Project builds without errors.
- [ ] **WiFi:** Connects to one of the configured WiFi networks.
- [ ] **mDNS:** Successfully resolves `jiabos-macbook-pro-2.local` (or configured host) to an IP.
- [ ] **WebSocket:** Connects to the resolved IP.
- [ ] **Audio:** Push-to-talk still works (sends data, receives beeps).
