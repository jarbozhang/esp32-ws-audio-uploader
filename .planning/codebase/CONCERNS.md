# Codebase Concerns

**Analysis Date:** 2026-02-02

## Tech Debt

**Incomplete Scaffold - Core Audio Recording Not Implemented:**
- Issue: Main loop does not actually record from I2S microphone. Lines 86-90 in `src/main.cpp` show PTT + audio recording is still TODO.
- Files: `src/main.cpp` (lines 86-91)
- Impact: Device cannot perform its primary function - capturing and streaming audio. Current implementation only demonstrates WebSocket protocol handshake without actual audio capture. Any deployment will be non-functional.
- Fix approach: Implement I2S audio driver initialization in `setup()`, add PTT button polling in `loop()`, and stream PCM audio frames via `ws.sendBIN()` between `sendStart()` and `sendEnd()` calls.

**Hardcoded Configuration in Source Code:**
- Issue: WiFi SSID, password, and server IP are hardcoded in `src/main.cpp` (lines 6-10, 13) instead of stored in device flash or configuration partition.
- Files: `src/main.cpp` (lines 6-13)
- Impact: Firmware must be recompiled for each new network/server environment. Credentials are exposed in version control. No flexibility for field deployment or testing in different locations.
- Fix approach: Implement NVS (Non-Volatile Storage) or SPIFFS-based configuration system. Store WiFi and server details in persistent storage, with USB serial or web-based provisioning interface for field updates.

**String Concatenation for JSON Serialization:**
- Issue: JSON messages are built using `String()` concatenation in `sendStart()` (line 28-33) and `sendEnd()` (line 37-40), which is inefficient and error-prone.
- Files: `src/main.cpp` (lines 27-41)
- Impact: Difficult to maintain JSON structure, risk of malformed messages if values contain special characters. Memory fragmentation due to repeated string allocations on embedded device.
- Fix approach: Use a proper JSON library (e.g., ArduinoJson) to serialize messages. This improves code clarity and ensures valid JSON.

**Placeholder/Default Request ID:**
- Issue: Request ID is hardcoded as `"req-1"` (line 25) and never changed between sessions.
- Files: `src/main.cpp` (line 25)
- Impact: Cannot distinguish or correlate multiple recording sessions. ASR server cannot differentiate between requests. If client reconnects without changing reqId, server may return cached results or duplicate processing.
- Fix approach: Generate unique reqIds using timestamp, UUID, or incrementing counter. Reset or regenerate on each new recording session.

## Known Bugs

**Serial Monitor Assumption:**
- Symptoms: Code calls `Serial.println()` and `Serial.printf()` extensively (lines 46, 49, 53, 56, 69-76) but does not check if serial is properly initialized.
- Files: `src/main.cpp` (entire file uses Serial)
- Trigger: If USB serial is disabled at compile time or hardware issue occurs, output is lost silently. Build flag `-DARDUINO_USB_CDC_ON_BOOT=1` assumes CDC-over-USB is always available.
- Workaround: Ensure correct USB mode is selected in board settings. Serial monitor must be opened before device sends output.

**WiFi Connection Blocking:**
- Symptoms: `WiFi.begin()` (line 68) followed by synchronous blocking loop (lines 70-73) halts all execution until WiFi connects.
- Files: `src/main.cpp` (lines 68-76)
- Trigger: If WiFi network is unreachable or credentials are wrong, device will appear frozen during boot. User sees dots being printed and no timeout mechanism.
- Workaround: Network must be available and reachable before device boots. Manual restart required if WiFi fails.

**No WebSocket Reconnection Behavior Defined:**
- Symptoms: `ws.setReconnectInterval(2000)` (line 80) sets reconnect interval, but no custom reconnection handler defined. Default behavior may not suit long-running audio sessions.
- Files: `src/main.cpp` (line 80)
- Trigger: If server drops connection mid-recording, WebSocket library may not cleanly handle resumption. Audio stream will be incomplete.
- Workaround: Only deploy in stable network environments. Manual device restart needed if connection drops during recording.

## Security Considerations

**Hardcoded Authentication Token:**
- Risk: Auth token is hardcoded as `"change_me"` (line 13) in source code and committed to version control.
- Files: `src/main.cpp` (line 13)
- Current mitigation: Comment says "change_me" (developer reminder only, not enforced).
- Recommendations: Store token in NVS encrypted partition or environment variable. Implement token rotation mechanism. Never commit secrets to git - use .gitignore or environment variable substitution during build.

**No Input Validation on WebSocket Messages:**
- Risk: `webSocketEvent()` (lines 43-61) receives and prints payloads without validation. Malicious server could send oversized or malformed data causing buffer overflow or denial of service.
- Files: `src/main.cpp` (lines 43-61, especially lines 53 and 56)
- Current mitigation: None - raw `payload` pointer and `length` are used directly in `Serial.printf()`.
- Recommendations: Validate message length before printing. Use bounds checking on `length`. Parse JSON instead of printing raw payloads. Implement message size limits to prevent DoS.

**No WiFi Security Configuration:**
- Risk: WiFi mode is set to `WIFI_STA` (line 67) but WiFi security settings are not explicitly configured beyond SSID/password.
- Files: `src/main.cpp` (line 67)
- Current mitigation: Relies on standard Arduino WiFi library defaults (usually WPA2).
- Recommendations: Explicitly set WiFi security mode (e.g., `WiFi.begin(SSID, PASS, CHANNEL, BSSID, WPA_SECURITY)`). Validate server certificate for WSS (WebSocket Secure) connections if not on trusted LAN.

**No Rate Limiting or DDoS Protection:**
- Risk: Device will accept and forward any WebSocket frames from server without rate limiting. No protection against malicious server flooding device with messages.
- Files: `src/main.cpp` (WebSocket library integration)
- Current mitigation: None.
- Recommendations: Implement message rate limiting in `webSocketEvent()`. Add timeout mechanisms for idle connections. Validate that audio chunks are within expected size range.

## Performance Bottlenecks

**Synchronous WiFi Connection Blocking Boot:**
- Problem: `while (WiFi.status() != WL_CONNECTED)` (line 70) blocks entire device startup. If WiFi is slow, device is unresponsive for several seconds.
- Files: `src/main.cpp` (lines 70-73)
- Cause: No async WiFi connection handling. Device waits for connection before proceeding to WebSocket setup.
- Improvement path: Implement async WiFi event handlers (e.g., `WiFi.onEvent()`) to allow device to continue initialization while WiFi connects in background. WebSocket connection can be attempted after WiFi is ready.

**String Allocation in Loop:**
- Problem: `loop()` calls `ws.loop()` which may allocate string buffers for WebSocket frame processing. Combined with delay, this could cause memory fragmentation on long-running device.
- Files: `src/main.cpp` (lines 84-91)
- Cause: No explicit memory management or pooling for WebSocket buffers.
- Improvement path: Profile heap usage during extended operation. Consider buffer pools for audio frames. Implement periodic garbage collection or reset.

**Fixed 10ms Loop Delay:**
- Problem: `delay(10)` (line 91) in main loop is arbitrary and may be too fast or too slow for PTT + audio streaming.
- Files: `src/main.cpp` (line 91)
- Cause: No dynamic adjustment based on actual audio buffer fill level or network conditions.
- Improvement path: Replace fixed delay with event-driven model. Trigger recording on button interrupt instead of polling. Adjust transmission rate based on network latency.

## Fragile Areas

**WebSocket Event Handler - Single Point of Failure:**
- Files: `src/main.cpp` (lines 43-61)
- Why fragile: `webSocketEvent()` handles all WebSocket events (connect, disconnect, text, binary) without error handling. If any event processing throws exception, entire WebSocket subsystem could crash. No retry logic or state recovery.
- Safe modification: Add try-catch style error handling (C++ exceptions or error codes). Log all event transitions. Test with server disconnections and malformed messages.
- Test coverage: No test coverage for event sequences (e.g., rapid connect/disconnect cycles, out-of-order messages).

**Startup Initialization Sequence:**
- Files: `src/main.cpp` (lines 63-81)
- Why fragile: Sequential dependency chain - Serial → WiFi → WebSocket. If any step hangs or fails, subsequent steps never execute. No timeout or fallback mechanism.
- Safe modification: Add explicit timeout for WiFi connection (e.g., 30 seconds max). Implement async initialization state machine. Allow partial startup (e.g., WebSocket reconnection even if initial connection fails).
- Test coverage: Not tested with missing WiFi, unavailable server, or hardware initialization failures.

**Audio Format Constants - No Validation:**
- Files: `src/main.cpp` (lines 18-20)
- Why fragile: Audio format parameters (16kHz, 16-bit mono) are hardcoded and embedded in JSON messages. If actual I2S hardware uses different format, messages will be misleading. No runtime validation that recorded audio matches advertised format.
- Safe modification: Read actual I2S configuration at runtime. Validate that recorded samples match advertised format before sending to server.
- Test coverage: Format mismatch between client claim and actual data is not detectable.

## Scaling Limits

**Single Recording Session Per Device:**
- Current capacity: Only one `reqId` ("req-1") - device assumes one concurrent recording session.
- Limit: If user presses PTT button while previous recording is still being streamed, behavior is undefined. No queue or session management.
- Scaling path: Implement queue for multiple recording requests. Assign unique reqIds per session. Handle overlapping transmissions gracefully.

**Memory for Audio Buffering:**
- Current capacity: No explicit audio buffer allocated. Real-time streaming to server with no local buffering.
- Limit: If network latency spikes, no buffer to absorb delay. Audio will underrun or drop frames.
- Scaling path: Implement circular audio buffer with size based on ESP32 available RAM (typically 300-500KB heap). Add latency compensation by buffering 0.5-1 second of audio before transmission.

**WebSocket Library - Single Connection:**
- Current capacity: One WebSocket client (`ws` global variable). No support for multiple servers or failover.
- Limit: If primary server becomes unavailable, no automatic failover. Manual restart required.
- Scaling path: Implement list of backup servers. Implement automatic server failover logic in WebSocket event handler.

## Dependencies at Risk

**WebSockets Library (links2004/WebSockets@^2.4.1):**
- Risk: Dependency uses caret versioning (`^2.4.1`), which allows minor/patch updates automatically. Library may change behavior in updates without explicit version bump. Library is community-maintained third-party package.
- Impact: Unexpected behavior after PlatformIO updates packages. Possible breaking changes in WebSocket protocol handling.
- Migration plan: Pin to exact version (`2.4.1`) to ensure reproducible builds. Monitor library repository for security updates. Plan for alternate WebSocket library (e.g., embedded within firmware).

**Arduino Framework Version (Espressif32):**
- Risk: `platform = espressif32` does not specify version. PlatformIO will pull latest version during builds, which may introduce breaking changes to WiFi or I2S APIs.
- Impact: Code may fail to compile with new framework versions.
- Migration plan: Pin platform version (e.g., `platform = espressif32@6.3.0`). Test regularly with new releases.

**WiFi Availability Assumption:**
- Risk: Code assumes WiFi network with specific SSID always exists. No fallback or offline mode.
- Impact: Device cannot operate without WiFi. Cannot be tested in environments without network.
- Migration plan: Implement offline mode or BLE provisioning for network-less setup. Consider mesh networking for robustness.

## Missing Critical Features

**No Button/PTT Input Handling:**
- Problem: Core feature (push-to-talk recording) has no implementation. Lines 86-90 are placeholder comments.
- Blocks: Device cannot initiate recording. User has no way to control when audio is captured.
- Status: Critical blocker for any real-world use. Must implement GPIO input handling for button, debouncing, and state transitions.

**No I2S Audio Capture:**
- Problem: Device does not initialize I2S peripheral or read audio samples from microphone. Comment on line 22-23 explicitly states "we don't actually record from I2S yet".
- Blocks: Device captures no audio. WebSocket messages are sent empty (no audio data).
- Status: Critical blocker. Must implement I2S driver initialization, microphone hardware interface, sample buffering, and PCM encoding.

**No Audio Format Negotiation:**
- Problem: Audio format is hardcoded (16kHz, 16-bit, mono) with no negotiation with server. If server requires different format, no mechanism to adapt.
- Blocks: Cannot work with servers that expect different formats (e.g., 8kHz for speech recognition, stereo for music).
- Status: Medium priority. Negotiate format during `start` handshake or detect server capabilities.

**No Response Parsing:**
- Problem: `webSocketEvent()` receives response text (lines 52-54) but does not parse JSON or extract results. Serial.printf just prints raw payload.
- Blocks: Device cannot extract ASR results from server. Results are logged but never processed or displayed.
- Status: Critical blocker for real use. Must parse JSON response, extract `text` field, and display or store result.

**No Network Error Recovery:**
- Problem: If WiFi or WebSocket connection fails, device does not attempt to recover. No reconnection state machine.
- Blocks: Transient network issues require manual device restart.
- Status: High priority. Implement exponential backoff reconnection strategy.

## Test Coverage Gaps

**No Unit Tests:**
- What's not tested: JSON message formatting, WebSocket event handling, audio format parameters, WiFi connection logic.
- Files: `src/main.cpp` (entire file has no test coverage).
- Risk: Regressions in message formatting go unnoticed. Protocol changes break silently.
- Priority: High - Core protocol logic should have unit tests for JSON serialization, message sequences, and error cases.

**No Integration Tests:**
- What's not tested: Full flow from WiFi connection → WebSocket connection → message exchange → response reception.
- Files: `src/main.cpp` (lines 63-81 initialization sequence, lines 43-61 event handling).
- Risk: Device may connect to WebSocket but fail to exchange messages. Silent failures in protocol handshake.
- Priority: High - Mock server tests needed to verify protocol compliance.

**No Hardware Tests:**
- What's not tested: I2S microphone integration, GPIO button polling, actual audio format output.
- Files: Not yet implemented in source.
- Risk: When I2S recording is implemented, integration with specific hardware (microphone, button pins) may fail or produce low-quality audio.
- Priority: Critical - Must be tested with actual ESP32-S3 + microphone hardware before deployment.

**No Network Resilience Tests:**
- What's not tested: WiFi disconnection recovery, server connection drop handling, network latency impact on audio streaming.
- Files: `src/main.cpp` (lines 70-73 WiFi loop, line 80 reconnect interval).
- Risk: Device may hang or produce incomplete audio streams in real network conditions.
- Priority: High - Test WiFi drops, server outages, network latency spikes.

---

*Concerns audit: 2026-02-02*
