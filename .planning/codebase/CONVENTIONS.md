# Coding Conventions

**Analysis Date:** 2026-02-02

## Naming Patterns

**Files:**
- C++ source files: `main.cpp` (snake_case)
- Configuration files: `platformio.ini` (lowercase with dots)

**Functions:**
- PascalCase for function names: `sendStart()`, `sendEnd()`, `webSocketEvent()`, `setup()`, `loop()`
- Callback functions follow WebSocket library conventions: `webSocketEvent()`
- Arduino lifecycle functions: `setup()`, `loop()` (required by Arduino framework)

**Variables:**
- camelCase for local and function parameters: `reqId`, `chunks`, `type`, `payload`, `length`
- SCREAMING_SNAKE_CASE for constants: `WIFI_SSID`, `WIFI_PASS`, `WS_HOST`, `WS_PORT`, `WS_PATH`, `AUTH_TOKEN`, `SAMPLE_RATE`, `CHANNELS`, `BIT_DEPTH`
- Global variables: camelCase: `ws`, `reqId`

**Types:**
- C++ standard types used directly: `String`, `uint8_t`, `size_t`, `uint16_t`, `uint32_t`, `WStype_t`

## Code Style

**Formatting:**
- No explicit formatter configured (no .prettierrc, eslint, or clang-format file)
- Observed style:
  - 2-space indentation (observed in switch statements, function bodies)
  - No trailing semicolons after closing braces of control structures
  - Braces on same line (1TBS style): `if (...) { ... }`
  - String concatenation using `+` operator (Arduino String class)

**Linting:**
- No linting configuration found
- Code style enforced implicitly by Arduino/PlatformIO framework conventions

## Import Organization

**Order:**
1. Arduino framework headers: `#include <Arduino.h>`
2. Standard library headers: `#include <WiFi.h>`
3. Third-party library headers: `#include <WebSocketsClient.h>`

**Path Aliases:**
- Not applicable (C++ includes, not module imports)

## Error Handling

**Patterns:**
- Blocking synchronous checks: `while (WiFi.status() != WL_CONNECTED)` with polling
- Serial logging for error states: `Serial.println("WS disconnected")`
- No exception handling observed (Arduino/embedded context)
- Callbacks handle error states via event type checking: `WStype_DISCONNECTED` case in `webSocketEvent()`
- No try-catch or error propagation mechanism (typical for embedded systems)

## Logging

**Framework:** Arduino Serial library (console output via USB CDC)

**Patterns:**
- Connection status: `Serial.println()` for major state changes (WiFi connected, WS connected/disconnected)
- Debug information: `Serial.printf()` for formatted output (WS messages, binary data length)
- Output format: Human-readable status messages with numeric values
- No structured logging or log levels

## Comments

**When to Comment:**
- TODO comments mark incomplete features: `// TODO: replace with your WiFi + server`, `// TODO: implement PTT + I2S recording loop`
- Inline comments explain non-obvious sections: `// In MVP we don't actually record from I2S yet.`
- Section comments delineate major configuration areas: `// Audio settings (MVP constants)`

**JSDoc/TSDoc:**
- Not applicable (C++, no JSDoc/TSDoc convention)
- Function documentation via inline comments when needed

## Function Design

**Size:**
- Small focused functions: `sendStart()` (9 lines), `sendEnd()` (5 lines), `webSocketEvent()` (16 lines)
- Arduino lifecycle functions: `setup()` (19 lines), `loop()` (7 lines)

**Parameters:**
- Callback functions dictated by library signature: `void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)`
- Helper functions take specific parameters: `sendEnd(uint32_t chunks)` for chunk count

**Return Values:**
- `void` return type for event handlers and lifecycle methods
- No return values used for error handling (event-driven model)

## Module Design

**Exports:**
- Single file architecture: `src/main.cpp` contains all application code
- Arduino framework entry points: `setup()` and `loop()` are implicitly exported/required
- Global WebSocket client instance: `WebSocketsClient ws` accessible to event handler and lifecycle functions

**Barrel Files:**
- Not applicable (single file Arduino sketch architecture)

## Configuration Management

**Constants:**
- Hardcoded credentials at top of file (MVP pattern): `WIFI_SSID`, `WIFI_PASS`, `AUTH_TOKEN`
- Audio format constants grouped together: `SAMPLE_RATE`, `CHANNELS`, `BIT_DEPTH`
- Server connection constants: `WS_HOST`, `WS_PORT`, `WS_PATH`

**Notes:**
- Comments indicate TODO for replacing hardcoded values with configuration
- No environment variable support or external configuration files detected

---

*Convention analysis: 2026-02-02*
