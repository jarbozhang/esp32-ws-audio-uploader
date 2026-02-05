# Architecture

**Analysis Date:** 2026-02-05

## Pattern Overview

**Overall:** Modular event-driven embedded client with layered manager pattern

**Key Characteristics:**
- Separation of concerns: networking, audio, and main control logic split into dedicated managers
- Callback-based communication between layers (hook events, WebSocket messages)
- Push-to-Talk UI model driven by hardware button events
- Asynchronous beep queuing system to avoid blocking audio recording
- mDNS service discovery with periodic re-resolution for fault tolerance

## Layers

**Application Layer:**
- Purpose: Orchestrates main control flow, button handling, recording lifecycle, beep state
- Location: `src/main.cpp`
- Contains: `setup()`, `loop()`, button state machines, activity tracking, auto-shutdown logic
- Depends on: `AudioManager`, `NetworkManager`, M5Unified hardware API
- Used by: Arduino runtime (setup/loop paradigm)

**Network Management Layer:**
- Purpose: Handles WiFi connection, mDNS hostname resolution, WebSocket lifecycle, message protocol
- Location: `src/NetworkManager.h` (interface), `src/NetworkManager.cpp` (implementation)
- Contains: WiFi multi-AP connection, mDNS resolution with retry logic, WebSocket event dispatch, JSON message serialization
- Depends on: WiFiMulti, WebSocketsClient, ArduinoJson, Config constants
- Used by: Application layer (calls `begin()`, `loop()`, `sendStart/End/Audio()`, external control methods)

**Audio Management Layer:**
- Purpose: Encapsulates audio recording, mic/speaker switching, beep pattern sequencing
- Location: `src/AudioManager.h` (interface), `src/AudioManager.cpp` (implementation)
- Contains: Microphone recording buffer management, speaker output, beep frequency/pattern definitions, pending beep queue
- Depends on: M5Unified (Mic/Speaker API), Config audio constants
- Used by: Application layer (calls `begin()`, `update()`, `startRecording()`, `stopRecording()`, `recordOneChunk()`, `queueBeep()`)

**Configuration Layer:**
- Purpose: Centralized compile-time and environment-time constants
- Location: `src/Config.h`, `src/secrets.h` (gitignored)
- Contains: Audio parameters (sample rate, chunk size), WebSocket endpoint, authentication token, WiFi credentials, button pins, keep-alive timings
- Depends on: None (header-only, no runtime dependencies)
- Used by: All other layers

## Data Flow

**Recording Flow (User-initiated ASR):**

1. User presses M5 button A (polled in `loop()`)
2. `loop()` detects `M5.BtnA.wasPressed()` and calls `NetworkMgr.sendStart(reqId)`
3. `NetworkManager::sendStart()` constructs JSON start message with audio format metadata and sends via WebSocket text frame
4. `AudioMgr.startRecording()` sets `_recording = true` and resets pending beeps
5. In each `loop()` iteration: `AudioMgr.recordOneChunk(buf, CHUNK_SAMPLES)` reads 320 samples (20ms @ 16kHz) from mic via M5Unified
6. On success, `NetworkMgr.sendAudio(buf, CHUNK_BYTES)` sends 640 bytes as binary WebSocket frame
7. User releases button A → `M5.BtnA.wasReleased()` detected
8. `loop()` calls `AudioMgr.stopRecording()` (sets `_recording = false`) and `NetworkMgr.sendEnd(reqId)`
9. Any beeps queued during recording are now played: `AudioMgr.update()` → `playPendingBeeps()` → M5Speaker output
10. Mac server processes audio and returns `result` JSON message (handled in WebSocket event)

**Hook Event Flow (Server-initiated):**

1. Mac server broadcasts JSON hook event: `{"type": "hook", "id": "<uuid>", "hook_event_name": "PermissionRequest", ...}`
2. `NetworkManager::webSocketEvent()` (WStype_TEXT) deserializes message
3. Detects `type == "hook"` and calls `handleHookEvent(doc)`
4. `handleHookEvent()` extracts event name and checks for duplicate ID via `seenId()`
5. If new event, triggers `_hookCallback(eventName)` (callback set in `main.cpp`)
6. Callback `onHookEvent()` in `main.cpp` determines beep type based on event name:
   - "PermissionRequest" → `BEEP_PERMISSION`
   - "PostToolUseFailure" → `BEEP_FAILURE`
   - "Stop" → `BEEP_STOP`
   - "Notification" → `BEEP_PERMISSION` (same as permission)
   - "Connected" → `BEEP_START`
7. Calls `AudioMgr.queueBeep(kind)`
8. If recording: increments pending beep counter (e.g., `_pendingPermission++`)
9. If not recording: immediately plays beep (via `playPendingBeeps()`)
10. On recording stop, any queued beeps play with priority: START > PERMISSION > FAILURE > STOP

**Keep-Alive Pulse Flow:**

1. `loop()` calls `keepAliveLoop()` (defined in `main.cpp`)
2. Monitors elapsed time since `lastActivityMs` (updated on button press, recording, etc.)
3. If active (within `KEEPALIVE_IDLE_TIMEOUT_MS`): generates pulse every `KEEPALIVE_PULSE_INTERVAL_MS` (30s)
4. Pulse drives GPIO48 (RGB LED data pin) HIGH for `KEEPALIVE_PULSE_DURATION_MS` (100ms) to draw current from power bank
5. Prevents power bank auto-sleep during voice session

**WiFi & mDNS Resolution Flow:**

1. `NetworkMgr.begin()` calls `connectWiFi()` which uses `WiFiMulti.run()` to connect to first available SSID from `WIFI_NETWORKS`
2. On WiFi connected, `resolveAndConnect()` checks if `WS_HOST` is already an IP address
3. If hostname: strips `.local` suffix and calls `MDNS.queryHost(bare_hostname)` with retry loop (5 attempts, 1s backoff)
4. On successful resolution, stores `_serverIP` and `_ipResolved = true`, starts WebSocket connection
5. In `loop()`, periodic check every `MDNS_RECHECK_INTERVAL_MS` (5 minutes) re-queries mDNS
6. If IP changed, disconnects and reconnects WebSocket with new IP

**External Control Button Flow:**

1. Four GPIO pins configured (pins 5, 6, 7, 8) as INPUT_PULLUP
2. In `loop()`, edge-triggered detection: reads pin → detects LOW → checks debounce (>15ms since last press)
3. On valid press, calls corresponding `NetworkMgr` method:
   - Pin 5 → `sendApprove()`
   - Pin 6 → `sendReject()`
   - Pin 7 → `sendSwitchModel()`
   - Pin 8 → `sendToggleAutoApprove()`
4. Each sends JSON command frame: `{"type": "command", "action": "..."}`

## Key Abstractions

**Manager Pattern:**
- Purpose: Encapsulates subsystem lifecycle and behavior
- Examples: `AudioManager`, `AppNetworkManager`
- Pattern: Single global instance (extern), `begin()` initializes, `loop()` or `update()` runs per frame, method-based API

**BeepKind & BeepPattern:**
- Purpose: Type-safe beep enumeration and frequency/duration specification
- Location: `src/AudioManager.h`
- Pattern: Enum maps beep types to frequency patterns via `patternFor(BeepKind)` lookup

**Hook Event Deduplication:**
- Purpose: Prevent duplicate beep playback from same server event
- Location: `src/NetworkManager.h` (members `_recentIds[]`, `_recentIdx`)
- Pattern: Circular 16-element buffer storing recently seen hook event IDs; checked on every event reception

**WebSocket Message Protocol:**
- Purpose: ASR and control communication with Mac server
- Patterns:
  - **Text frames (JSON):** `start`, `end`, `command`, responses
  - **Binary frames:** Raw PCM audio data (20ms chunks, 16-bit signed, mono, 16kHz)
  - **Hook broadcast:** Server sends `{"type": "hook", "hook_event_name": "...", "id": "..."}`

**Activity Tracking:**
- Purpose: Trigger auto-shutdown and keep-alive pulse
- Location: `src/main.cpp` globals `lastActivityMs`, functions `updateActivity()`, `checkAutoShutdown()`
- Pattern: Timestamp on user action (button press, recording), compare in timeout check

## Entry Points

**setup() function:**
- Location: `src/main.cpp` lines 87-105
- Triggers: Arduino runtime at power-on
- Responsibilities:
  - Initialize serial console (115200 baud)
  - Call `AudioMgr.begin()` (initializes M5Unified, mic enabled, speaker disabled)
  - Queue startup beep
  - Call `NetworkMgr.begin()` (WiFi connection, mDNS resolve, WebSocket init)
  - Register hook callback with `NetworkMgr.setHookCallback(onHookEvent)`
  - Configure external control button pins as INPUT_PULLUP
  - Record activity timestamp

**loop() function:**
- Location: `src/main.cpp` lines 107-181
- Triggers: Arduino runtime repeats after each iteration with 1ms delay
- Responsibilities:
  - Call `AudioMgr.update()` (M5.update(), pending beep playback)
  - Call `NetworkMgr.loop()` (WiFi/mDNS re-check, WebSocket event handling)
  - Check auto-shutdown timeout
  - Poll external control buttons (edge-triggered)
  - Poll M5 button A (start/stop recording)
  - If recording: read audio chunk and send via WebSocket
  - Minimal delay (1ms) to maintain responsiveness

## Error Handling

**Strategy:** Defensive with logging, graceful degradation, non-blocking

**Patterns:**

- **WiFi Connection Lost:**
  - `NetworkMgr.loop()` detects via `_wifiMulti.run() != WL_CONNECTED`
  - Logs "WiFi lost, reconnecting..." and attempts immediate reconnect
  - Does not block; continues polling

- **WebSocket Disconnection:**
  - Detected via `WStype_DISCONNECTED` event callback
  - Sets `_wsConnected = false`
  - Auto-reconnect via WebSockets library with 2s interval
  - UI shows "not connected" if user presses button

- **mDNS Resolution Failure:**
  - Retry loop with 5 attempts, 1s backoff in initial resolution
  - If all fail, logs error and skips WebSocket init
  - Next `loop()` attempt re-triggers resolution
  - Falls back to direct IP if user provides IP-based `WS_HOST`

- **JSON Deserialization Error:**
  - `deserializeJson()` returns non-zero error code
  - Logs raw text frame and continues (non-fatal)
  - Prevents protocol errors from crashing device

- **Audio Recording Timeout:**
  - `recordOneChunk()` detects `MAX_RECORD_MS` (8 seconds) elapsed
  - Forces `stopRecording()` to prevent indefinite recording
  - Sends `end` message to server

- **Button Press Without Connection:**
  - `loop()` checks `NetworkMgr.isConnected()` before calling `sendStart()`
  - Logs warning if button pressed while disconnected
  - Does not queue recording

## Cross-Cutting Concerns

**Logging:**
- Uses `Serial.printf()` and `Serial.println()` throughout
- Key events logged: WiFi connection, mDNS resolution, WebSocket connect/disconnect, hook events, recording start/stop
- Debug prefixes: `[Main]`, `[Audio]`, `[NM]` (NetworkManager) identify component

**Validation:**
- WiFi credentials check: `WIFI_NETWORKS` must have at least one entry (tested in `test_main.cpp`)
- WebSocket connection check before sending: `isConnected()` guard
- JSON message validation: deserialize error check, field null-checking with `| ""` defaults
- Hook event ID validation: reject if `seenId()` detects duplicate
- Button debounce: require 15ms between edge transitions

**Authentication:**
- `AUTH_TOKEN` from `secrets.h` included in every `start` message
- Server-side verification of token; client has no validation logic
- Token protected by gitignore (not committed to repo)

**State Consistency:**
- `_recording` flag prevents simultaneous start/stop
- `_wsConnected` flag prevents sending when disconnected
- `_ipResolved` flag tracks mDNS readiness
- Pending beep counters prevent overflow (uint8_t saturation safe)

---

*Architecture analysis: 2026-02-05*
