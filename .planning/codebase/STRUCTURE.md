# Codebase Structure

**Analysis Date:** 2026-02-02

## Directory Layout

```
esp32-ws-audio-uploader/
├── src/                    # Firmware source code
│   └── main.cpp           # Single entry point with all application logic
├── platformio.ini         # PlatformIO build configuration
├── README.md              # Project overview and protocol documentation
├── SPEC.md                # Reference to Obsidian spec document
├── LICENSE                # MIT license
└── .gitignore             # Excludes .pio and .vscode directories
```

## Directory Purposes

**src/:**
- Purpose: Arduino/C++ firmware source code
- Contains: Main application entry point and protocol implementation
- Key files: `main.cpp` - Complete application (93 lines)

## Key File Locations

**Entry Points:**
- `src/main.cpp`: Single C++ file containing `setup()` and `loop()` Arduino entry points

**Configuration:**
- `platformio.ini`: ESP32-S3 board selection, build flags, library dependencies

**Documentation:**
- `README.md`: Protocol specification and project purpose
- `SPEC.md`: Link to external Obsidian specification

**Build Artifacts:**
- `.pio/`: Generated build output (gitignored)
- `.vscode/`: IDE settings (gitignored)

## Naming Conventions

**Files:**
- `main.cpp`: Single source file (Arduino standard)
- `platformio.ini`: Standard PlatformIO config name

**Functions:**
- camelCase: `sendStart()`, `sendEnd()`, `webSocketEvent()`, `setup()`, `loop()`

**Variables:**
- Constants: UPPERCASE_SNAKE_CASE: `WIFI_SSID`, `WS_HOST`, `WS_PORT`, `SAMPLE_RATE`, `CHANNELS`, `BIT_DEPTH`, `AUTH_TOKEN`
- Static module: camelCase: `reqId`, `ws`

**Types:**
- Arduino/WebSocket types from libraries: `WStype_t`, `WebSocketsClient`

## Where to Add New Code

**New Feature (e.g., button input, I2S recording):**
- Primary code: Add functions to `src/main.cpp` (or create separate `.cpp/.h` pair if exceeds 200 lines)
- Tests: Create `test/` directory with corresponding test files (not yet present)

**New Module/Component:**
- Create separate `.h` header file in `src/` with declarations
- Create corresponding `.cpp` implementation file in `src/`
- Include from `main.cpp`

**Utilities/Helpers:**
- Shared message formatting: Add functions below `sendStart()`/`sendEnd()` in `src/main.cpp`
- WiFi utilities: Create `src/wifi_utils.h/cpp` if WiFi logic grows beyond setup
- Audio utilities: Create `src/audio_utils.h/cpp` for I2S and sample rate management

**Configuration:**
- Hardcoded parameters: Add static const globals at top of `src/main.cpp` (lines 6-20)
- Build-time settings: Add to `platformio.ini` build_flags or new ini env section

## Special Directories

**Build Output (.pio):**
- Purpose: PlatformIO build artifacts, compiled binaries, dependencies
- Generated: Yes (automatically by PlatformIO)
- Committed: No (gitignored)

**IDE Configuration (.vscode):**
- Purpose: Visual Studio Code workspace settings and extensions recommendations
- Generated: Manual (user-configured)
- Committed: No (gitignored)

## Extending the Codebase

**For Button Input (PTT):**
1. Add GPIO pin constant at top of `main.cpp`
2. Add `void initButton()` in `setup()` after WiFi
3. Add `bool readButton()` helper function
4. Add PTT state tracking logic in `loop()`

**For I2S Audio Recording:**
1. Create `src/audio_capture.h/cpp` for I2S driver abstraction
2. Include in `main.cpp` and call initialization in `setup()`
3. Add `uint8_t* readAudioFrame()` function
4. Replace delay in `loop()` with `if (pttPressed) { streamAudioFrames(); }`

**For Message Parsing:**
1. Create `src/ws_protocol.h/cpp` for JSON message handling
2. Add `void parseServerResult(const char* json)` function
3. Call from `webSocketEvent()` in TEXT case

---

*Structure analysis: 2026-02-02*
