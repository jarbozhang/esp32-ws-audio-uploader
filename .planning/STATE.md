# 项目状态

## Current Position
- **Current Milestone**: v0.1 Initial
- **Next Phase**: Phase 4 (TBD)
- **Overall Status**: Phase 3 Completed (Tests Compiled); Quick-001 to Quick-009 Completed; Quick-010 Completed (Stabilize audio switching)
- **Last activity**: 2026-02-04 - Completed quick-010 (Stabilize audio switching)

## Quick Tasks
- quick-001 Completed: 4 external GPIO buttons (pins 5/6/7/8) wired to WebSocket approve/reject/switch_model/toggle_auto_approve commands
  - Summary: `.planning/quick/001-add-4-buttons-claude-code-control/001-SUMMARY.md`
  - Decisions: INPUT_PULLUP active-LOW; table-driven dispatch; 15ms millis() debounce

## Quick Tasks (continued)
- quick-002 Completed: WiFi credentials (WIFI_NETWORKS, AUTH_TOKEN, WS_HOSTNAME) moved from Config.h to gitignored secrets.h; secrets.h.template committed as onboarding guide
  - Summary: `.planning/quick/002-externalize-wifi-credentials-to-config-f/002-SUMMARY.md`
  - Decisions: WiFiCredential struct co-located with WIFI_NETWORKS in secrets.h; layered #ifndef for WS_HOSTNAME (build_flags > secrets.h > fallback); AUTH_TOKEN stays static const char*
- quick-003 Completed: WiFi TX power set to max and sleep mode disabled to prevent battery bank standby
  - Summary: `.planning/quick/003-check-wifi-power-settings/003-SUMMARY.md`
  - Decisions: `WiFi.setTxPower(WIFI_POWER_19_5dBm)` and `WiFi.setSleep(false)`
- quick-004 Completed: Connection beep and 30-minute auto shutdown (WiFi off)
  - Summary: `.planning/quick/004-add-connection-beep-and-auto-shutdown/004-SUMMARY.md`
  - Decisions: Play `BEEP_START` on WS connect; `WiFi.mode(WIFI_OFF)` after 30 mins inactivity
- quick-005 Completed: Fix compile error for BEEP_START
  - Summary: `.planning/quick/005-fix-beep-compile-error/005-SUMMARY.md`
  - Decisions: Added `BEEP_START` to `BeepKind` enum and implemented its pattern (2400Hz, 100ms) in `AudioManager`.
- quick-006 Completed: Debug connection beep silence
  - Summary: `.planning/quick/006-debug-beep-on-connect/006-SUMMARY.md`
  - Decisions: Added explicit `M5.Speaker.setVolume(128)` and serial debug logs to `AudioManager`.
- quick-007 Completed: Refactor connection beep to use callback
  - Summary: `.planning/quick/007-use-hook-callback-for-connection-beep/007-SUMMARY.md`
  - Decisions: Decoupled `NetworkManager` from `AudioMgr`; fires "Connected" hook event instead.
- quick-008 Completed: Refactor AudioManager beep queueing
  - Summary: `.planning/quick/008-refactor-audiomanager-to-always-queue-beeps/008-SUMMARY.md`
  - Decisions: `queueBeep` now always queues, removing immediate playback logic.
- quick-009 Completed: Deep debug instrumentation for beep failure
  - Summary: `.planning/quick/009-deep-debug-beep-failure/009-SUMMARY.md`
  - Decisions: Added comprehensive `DEBUG:` logs and startup beep test.
- quick-010 Completed: Stabilize Mic/Speaker switching
  - Summary: `.planning/quick/010-stabilize-audio-switching/010-SUMMARY.md`
  - Decisions: Added `delay(10)` after `Mic.end` and `Speaker.begin` to fix potential I2S race condition.

## Accumulated Context

### Roadmap Evolution
- Quick-002 Completed: Externalize WiFi credentials to secrets.h (gitignored)
- Quick-001 Completed: 4 GPIO control buttons for Claude Code CLI
- Phase 3 Completed: Generate Testing Methods (with compilation fixes)
- Phase 3 added: Generate Testing Methods
- Phase 2 Completed: Refactor and Advanced Connectivity
- Phase 2 added: Refactor and Advanced Connectivity (Multi-WiFi, mDNS, Code Split)
- Roadmap initialized with "v0.1 Initial" milestone and "Phase 1: Basic Setup and Connectivity"

### Session Continuity
- Last session: 2026-02-03T08:48:10Z
- Stopped at: Completed quick-002
- Resume file: None
