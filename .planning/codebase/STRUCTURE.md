# Codebase Structure

**Analysis Date:** 2026-02-05

## Directory Layout

```
esp32-ws-audio-uploader/
├── src/                              # C++ source code
│   ├── main.cpp                     # Application entry point, control flow, button handling
│   ├── AudioManager.h               # Audio manager interface
│   ├── AudioManager.cpp             # Audio manager implementation
│   ├── NetworkManager.h             # Network manager interface
│   ├── NetworkManager.cpp           # Network manager implementation
│   ├── Config.h                     # Centralized compile-time constants
│   ├── secrets.h                    # Runtime secrets (gitignored)
│   └── secrets.h.template           # Template for secrets.h
├── test/
│   └── test_main.cpp                # Unity framework unit tests
├── scripts/
│   └── mock_server.py               # Python mock WebSocket server for testing
├── .planning/                       # Planning and specification documents
├── platformio.ini                   # PlatformIO build configuration
├── README.md                        # User documentation and quick start
├── SPEC.md                          # WebSocket protocol specification
├── LICENSE                          # License file
├── requirements.txt                 # Python dependencies for mock server
└── .gitignore                       # Git ignore rules
```

## Directory Purposes

**src/ directory:**
- Purpose: Contains all C++ source files for ESP32 firmware
- Contains: Application code, manager classes, configuration, secrets
- Key files: `main.cpp` (182 lines), `AudioManager.cpp` (113 lines), `NetworkManager.cpp` (251 lines)
- Total: ~726 lines of C++ code across 7 files

**test/ directory:**
- Purpose: On-device unit tests using Unity framework
- Contains: Test cases for config loading, network/audio manager state
- Run with: `pio test -e esp32-s3`

**scripts/ directory:**
- Purpose: Testing utilities for development and verification
- Contains: `mock_server.py` — lightweight Python WebSocket server simulating Mac ASR backend
- Run with: `python scripts/mock_server.py`

**.planning/ directory:**
- Purpose: GSD-managed planning documents and phase specifications
- Contains: Phase plans, summaries, codebase analysis (ARCHITECTURE.md, STRUCTURE.md, etc.)

## Key File Locations

**Entry Points:**
- `src/main.cpp`: Arduino entry point with `setup()` (lines 87-105) and `loop()` (lines 107-181)

**Core Managers:**
- `src/AudioManager.h`: Interface defining beep queueing, recording control (47 lines)
- `src/AudioManager.cpp`: Implementation of microphone capture and beep sequencing (113 lines)
- `src/NetworkManager.h`: Interface for WiFi, mDNS, WebSocket, message protocol (56 lines)
- `src/NetworkManager.cpp`: Implementation of network connectivity and message dispatch (251 lines)

**Configuration:**
- `src/Config.h`: Audio parameters (sample rate, chunk size), WebSocket endpoint, button pins, keep-alive timings (47 lines)
- `src/secrets.h`: Auth token, WiFi credentials (gitignored, 30 lines)
- `src/secrets.h.template`: Template showing required format for secrets

**Build Configuration:**
- `platformio.ini`: PlatformIO build config, target board (esp32-s3-devkitc-1), library dependencies (WebSockets, M5Unified, ArduinoJson)

**Documentation:**
- `README.md`: Features, configuration, protocol overview, build instructions
- `SPEC.md`: Detailed WebSocket message protocol specification

**Testing:**
- `test/test_main.cpp`: Config validation and state machine tests (52 lines)
- `scripts/mock_server.py`: Test server for protocol validation
- `requirements.txt`: Python dependencies (websockets library)

## Naming Conventions

**Files:**
- Source files: `CamelCase.cpp` for implementations, `CamelCase.h` for headers
- Configuration: `Config.h`, `secrets.h`
- Test files: `test_*.cpp` following Arduino/PlatformIO convention
- Entry point: `main.cpp` (Arduino standard)

**Directories:**
- `src/` — Arduino/PlatformIO standard directory for source code
- `test/` — Arduino/PlatformIO standard directory for tests
- `scripts/` — Project-specific helper scripts
- `.planning/` — GSD planning documents (dot-prefixed to separate from source)

**Code Symbols (C++):**
- **Constants:** All-caps with underscores (e.g., `SAMPLE_RATE`, `MAX_RECORD_MS`, `KEEPALIVE_PULSE_INTERVAL_MS`)
- **Global variables (file scope):** Lower camelCase with `static` keyword (e.g., `currentReqId`, `lastActivityMs`)
- **Class member variables:** Lower camelCase with underscore prefix (e.g., `_recording`, `_wsConnected`, `_serverIP`)
- **Functions:** camelCase (e.g., `updateActivity()`, `makeReqId()`, `sendStart()`, `recordOneChunk()`)
- **Classes:** PascalCase (e.g., `AudioManager`, `AppNetworkManager`)
- **Enums:** PascalCase (e.g., `BeepKind`, `WStype_t` from library)
- **Structs:** PascalCase (e.g., `BeepPattern`)

## Where to Add New Code

**New Recording Feature:**
- Primary logic: `src/AudioManager.cpp` (add method to AudioManager class)
- Configuration: `src/Config.h` (add audio constant if needed)
- Integration: `src/main.cpp` (call new AudioManager method in `loop()`)

**New Hook Event Type:**
- Server message definition: Update Mac server codebase
- Handler: Add case in `onHookEvent()` callback in `src/main.cpp` (lines 72-84)
- Beep type: Add enum value to `BeepKind` in `src/AudioManager.h`
- Beep pattern: Add case in `patternFor()` in `src/AudioManager.cpp`

**New External Control Command:**
- Protocol method: Add to `AppNetworkManager` class in `src/NetworkManager.h` (e.g., `sendNewCommand()`)
- Implementation: Add JSON construction in `src/NetworkManager.cpp`
- Button binding: Add to button array in `src/main.cpp` `loop()` function (lines 114-143)

**New Network Message Type:**
- Protocol handler: Add case in `AppNetworkManager::webSocketEvent()` in `src/NetworkManager.cpp` (WStype_TEXT branch, around line 227)
- Message builder: Add method to `AppNetworkManager` class (e.g., `sendNewMessageType()`)
- JSON serialization: Use ArduinoJson pattern: create `StaticJsonDocument<size>`, populate fields, `serializeJson(doc, out)`, `_ws.sendTXT(out)`

**New WiFi Configuration:**
- Credentials: Add entry to `WIFI_NETWORKS` vector in `secrets.h`
- Multi-AP support: Already handled by `WiFiMulti` — just add more entries

**New Timing Parameter:**
- Add constant to `src/Config.h` with documentation comment
- Reference in appropriate manager implementation
- Add test case to `test/test_main.cpp` if it affects observable behavior

**Test Cases:**
- File: `test/test_main.cpp`
- Framework: Unity (see existing `TEST_ASSERT_*` macros)
- Globals: Tests run against global `AudioMgr`, `NetworkMgr` instances (already instantiated in main.cpp)
- Run: `pio test -e esp32-s3`

## Key Abstractions and Patterns

**Manager Pattern (Singleton):**
- Global instances: `extern AudioManager AudioMgr;` and `extern AppNetworkManager NetworkMgr;`
- Initialization: `begin()` called from `setup()`
- Update loop: `update()` or `loop()` called from Arduino `loop()`
- Public API: Methods for state control and data access

**Callback Pattern:**
- Hook events: `NetworkMgr.setHookCallback(onHookEvent)` registers callback
- Callback signature: `void (const char* eventName)`
- Used to decouple network layer from application layer beep logic

**WebSocket Event Handler:**
- Inline lambda: `_ws.onEvent([this](WStype_t type, ...) { this->webSocketEvent(...); })`
- Allows member function access while satisfying library callback interface

**Beep Queueing:**
- Counters: `_pendingStart`, `_pendingPermission`, `_pendingFailure`, `_pendingStop` (uint8_t)
- Queued during recording: prevents blocking audio input
- Played after recording stops: `playPendingBeeps()` processes in priority order
- Pattern lookup: `patternFor(BeepKind)` returns frequency/duration struct

**JSON Message Pattern:**
- Text messages: Use `StaticJsonDocument<N>` for fixed-size allocation
- Populate: Direct field assignment (e.g., `doc["type"] = "start"`)
- Serialize: `serializeJson(doc, out)` produces string
- Send: `_ws.sendTXT(out)` or `_ws.sendBIN(data, len)` for audio

**Edge-Triggered Button State Machine:**
- Per-button tracking: `static bool lastState[4]` and `static unsigned long lastPressMs[4]`
- Transition detection: `cur == LOW && lastState[i] == HIGH` detects falling edge
- Debounce: `(now - lastPressMs[i]) > 15` requires 15ms minimum
- Handler dispatch: Member function pointers in `BtnDef` array

**Circular Buffer (Hook Event De-dup):**
- Storage: `_recentIds[16]` and `_recentIdx` (modulo 16)
- Insert: `_recentIds[_recentIdx++ % 16] = id`
- Lookup: Simple linear search (16 elements)

## Important Build & Deployment Paths

**Build:**
- Configuration: `platformio.ini` specifies `board = esp32-s3-devkitc-1`, libraries in `lib_deps`
- Artifact: `.pio/build/esp32-s3/firmware.bin` (compiled firmware)
- Run: `pio run` (compile only), `pio run -t upload` (compile and flash)

**Debugging:**
- Serial output: `pio device monitor` (115200 baud)
- Debug statements: `Serial.printf()` and `Serial.println()` throughout codebase

**Testing:**
- Unit tests: `test/test_main.cpp` runs on device
- Run: `pio test -e esp32-s3`
- Mock server: `python scripts/mock_server.py` (separate terminal for integration testing)

## Size and Complexity Profile

| File | Lines | Purpose | Complexity |
|------|-------|---------|-----------|
| `src/main.cpp` | 182 | Entry point, control flow, button handling | High (polyphonic event handling) |
| `src/NetworkManager.cpp` | 251 | WiFi, mDNS, WebSocket, protocol | High (state machine, mDNS retry) |
| `src/AudioManager.cpp` | 113 | Recording, beep sequencing | Medium (beep pattern lookup, queue) |
| `src/Config.h` | 47 | Constants | Low |
| `src/NetworkManager.h` | 56 | Interface, class definition | Low |
| `src/AudioManager.h` | 47 | Interface, class definition | Low |
| `src/secrets.h` | 30 | Credentials | N/A (sensitive) |
| `test/test_main.cpp` | 52 | Unit tests | Medium |

**Total:** ~726 lines C++ across 7 files, ~52 lines tests

## Special Directories

**`.planning/` directory:**
- Purpose: GSD orchestrator-managed planning and specification documents
- Generated: Partially (new documents written by GSD tools)
- Committed: Yes (planning documents should be tracked)
- Contents: `ARCHITECTURE.md`, `STRUCTURE.md`, phase plans, roadmap

**`.git/` directory:**
- Purpose: Git version control metadata
- Generated: Yes (automatically by git)
- Committed: No (automatically ignored)

**`.claude/` directory:**
- Purpose: Claude editor local configuration
- Generated: Yes (automatically)
- Committed: No (gitignored)

**`.pio/` directory:**
- Purpose: PlatformIO build artifacts, dependencies, cache
- Generated: Yes (automatically on first build)
- Committed: No (in .gitignore)

---

*Structure analysis: 2026-02-05*
