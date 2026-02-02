# Architecture

**Analysis Date:** 2026-02-02

## Pattern Overview

**Overall:** Embedded Client-Server WebSocket Protocol Implementation

**Key Characteristics:**
- Single-file firmware scaffold for ESP32-S3 microcontroller
- Client initiates WebSocket connection to remote ASR server
- State machine driven by WebSocket lifecycle events
- Synchronous blocking I/O with periodic polling loop

## Layers

**Hardware Abstraction:**
- Purpose: Manage WiFi and WebSocket communication at the Arduino library level
- Location: Arduino SDK + links2004/WebSockets library
- Contains: WiFi connectivity, WebSocket client lifecycle
- Depends on: ESP32 hardware peripherals
- Used by: Application logic in `src/main.cpp`

**Application Layer:**
- Purpose: Control audio recording workflow and protocol messaging
- Location: `src/main.cpp`
- Contains: WebSocket event handling, message serialization, device setup
- Depends on: Arduino WiFi and WebSocket libraries
- Used by: No dependencies (entry point)

## Data Flow

**Connection Establishment:**

1. `setup()` initializes Serial debug output
2. WiFi begins connection to configured SSID with static credentials
3. Waits in blocking loop for WiFi connection success
4. WebSocket client begins connection to `ws://{WS_HOST}:{WS_PORT}{WS_PATH}`
5. Registers event callback and reconnection interval (2000ms)

**Audio Protocol Sequence (Planned):**

1. WebSocket connects → `webSocketEvent()` called with `WStype_CONNECTED`
2. `sendStart()` serializes JSON with audio format params (16kHz, 16-bit, mono PCM)
3. Audio data streamed as binary frames via `ws.sendBIN()`
4. `sendEnd()` signals completion with chunk count
5. Server responds with JSON result containing transcribed text
6. Result processed in `webSocketEvent()` handler

**State Management:**
- Request ID (`reqId`) maintained as global static string for request tracking
- WebSocket connection state managed by library (automatic reconnection)
- No explicit state machine; event-driven through callbacks

## Key Abstractions

**WebSocket Protocol:**
- Purpose: Abstract network communication and connection lifecycle
- Examples: `links2004/WebSockets@^2.4.1` library
- Pattern: Event callback pattern (`webSocketEvent()`)

**Message Serialization:**
- Purpose: Construct protocol messages for start, end, and audio data
- Examples: `src/main.cpp` (lines 27-41) - `sendStart()`, `sendEnd()`
- Pattern: String concatenation with JSON structure

**Audio Configuration:**
- Purpose: Define audio capture parameters for server
- Examples: `SAMPLE_RATE=16000`, `CHANNELS=1`, `BIT_DEPTH=16`
- Pattern: Static const globals at module level (lines 17-20)

## Entry Points

**setup():**
- Location: `src/main.cpp` (line 63)
- Triggers: ESP32 boot/reset
- Responsibilities: Initialize serial, connect WiFi, establish WebSocket connection

**loop():**
- Location: `src/main.cpp` (line 83)
- Triggers: Continuous polling after setup completes
- Responsibilities: Poll WebSocket for messages, handle button input (TODO), stream audio (TODO)

**webSocketEvent():**
- Location: `src/main.cpp` (line 43)
- Triggers: WebSocket connection/disconnection, message received
- Responsibilities: Log connection state, send start message on connect, log received messages

## Error Handling

**Strategy:** Minimal error handling in scaffold phase

**Patterns:**
- WiFi connection: Blocking loop with delay, no timeout
- WebSocket: Automatic reconnection via library (2000ms interval)
- Protocol: No validation or error recovery (MVP scaffold)
- Serial debug: Limited logging via `Serial.println()`

## Cross-Cutting Concerns

**Logging:** Serial UART output at 115200 baud (Arduino default). Limited debug output shows connection state and received messages.

**Validation:** Not implemented. Static credentials and hardcoded audio parameters. TODO comments indicate missing validation for button input and I2S recording.

**Authentication:** Basic token-based via `AUTH_TOKEN` string sent in start message. Static const, not validated by server in current scaffold.

---

*Architecture analysis: 2026-02-02*
