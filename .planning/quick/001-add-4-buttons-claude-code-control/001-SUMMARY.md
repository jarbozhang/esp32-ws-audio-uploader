---
phase: quick-001
plan: 01
subsystem: ui
tags: [gpio, websocket, esp32, button, arduino]

requires:
  - phase: 02-refactor-and-connectivity
    provides: NetworkManager with WebSocket send pattern and mDNS connectivity
provides:
  - 4 external GPIO buttons (pins 5/6/7/8) mapped to Claude Code control commands
  - WebSocket JSON command methods: sendApprove, sendReject, sendSwitchModel, sendToggleAutoApprove
affects: [claude-code-control, ws-protocol]

tech-stack:
  added: []
  patterns:
    - "Table-driven button dispatch: BtnDef struct array with member-function pointer, single polling loop"
    - "Edge-triggered debounce: static lastState[] + millis()-based 15ms cooldown"

key-files:
  created: []
  modified:
    - src/Config.h
    - src/NetworkManager.h
    - src/NetworkManager.cpp
    - src/main.cpp

key-decisions:
  - "Used INPUT_PULLUP + detect LOW for active-low external mechanical switches"
  - "Table-driven dispatch (BtnDef array + member function pointer) instead of 4 independent if-blocks, reduces code duplication and makes adding buttons a single-line change"
  - "15ms debounce via millis() cooldown rather than delay(), keeps loop non-blocking"

patterns-established:
  - "Button polling pattern: edge-detect HIGH->LOW with millis()-based debounce in loop(), table-driven handler dispatch"

duration: 4min
completed: 2026-02-03
---

# Quick Task 001: Add 4 Buttons for Claude Code Control Summary

**GPIO 5/6/7/8 buttons wired to approve/reject/switch_model/toggle_auto_approve WebSocket commands via table-driven edge-triggered polling with 15ms debounce**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-03T08:16:08Z
- **Completed:** 2026-02-03T08:20:24Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments
- Defined 4 GPIO control-button pins in Config.h (pins 5, 6, 7, 8)
- Added 4 WebSocket command methods to AppNetworkManager following the existing sendStart/sendEnd JSON pattern
- Implemented edge-triggered button polling in loop() with 15ms debounce, dispatched via a compact table of structs containing pin, label, and member-function pointer
- All button presses log to Serial for hardware debug; WS-not-connected state is also warned

## Task Commits

Each task was committed atomically:

1. **Task 1: GPIO pin defs + NetworkManager command methods** - `7a9a80a` (feat)
2. **Task 2: Button init in setup() and edge-triggered polling in loop()** - `75d7561` (feat)

**Plan metadata:** (see below)

## Files Created/Modified
- `src/Config.h` - Added BTN_APPROVE_PIN(5), BTN_REJECT_PIN(6), BTN_SWITCH_MODEL_PIN(7), BTN_AUTO_APPROVE_PIN(8)
- `src/NetworkManager.h` - Declared sendApprove, sendReject, sendSwitchModel, sendToggleAutoApprove
- `src/NetworkManager.cpp` - Implemented 4 methods, each serializing `{"type":"command","action":"<name>"}` and sending via `_ws.sendTXT`
- `src/main.cpp` - Added 4 pinMode(INPUT_PULLUP) in setup(); added table-driven button polling block in loop()

## Decisions Made
- Used INPUT_PULLUP with LOW-active detection, matching typical external mechanical switch wiring without requiring external pull-down resistors.
- Chose table-driven dispatch (BtnDef struct with member function pointer) over 4 copy-pasted if-blocks. Adding or reordering buttons requires only one line in the array.
- Debounce implemented via millis()-based cooldown (15ms) rather than delay(), keeping loop() non-blocking and preserving audio recording responsiveness.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
- `pio` CLI was not on PATH; located at `/Users/jiabozhang/.platformio/penv/bin/pio`. Used absolute path for all builds. Not a code issue.
- Pre-existing `StaticJsonDocument` deprecation warnings from ArduinoJson v7 (on sendStart/sendEnd and the new methods alike). Not introduced by this change; outside scope of this task.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Firmware compiles and links successfully. Ready for `pio run -t upload` to device.
- The WebSocket server side (Python) will need to handle the new `{"type":"command","action":"*"}` messages to actually invoke Claude Code CLI commands. That is a server-side concern, not firmware.
- If button wiring uses external pull-down resistors instead of relying on internal pull-ups, change `INPUT_PULLUP` to `INPUT` in main.cpp setup block.

---
*Phase: quick-001*
*Completed: 2026-02-03*
