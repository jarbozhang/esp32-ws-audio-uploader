---
quick: 002
name: externalize-wifi-credentials-to-config-file
subsystem: infra
tags: [esp32, arduino, secrets, gitignore, wifi, security]

requires:
  - phase: quick-001
    provides: GPIO button wiring and WebSocket command methods

provides:
  - secrets.h pattern: WiFi credentials and auth token isolated in gitignored file
  - secrets.h.template: committed onboarding reference with documented placeholders
  - Config.h cleaned: no credential literals in version-controlled code

affects: [any future phase touching WiFi config, auth token, or mDNS hostname]

tech-stack:
  added: []
  patterns:
    - "secrets.h / secrets.h.template pattern for ESP32 credential management"
    - "#ifndef WS_HOSTNAME fallback chain: build_flags > secrets.h > Config.h default"

key-files:
  created:
    - src/secrets.h (gitignored, holds actual credentials)
    - src/secrets.h.template (committed, placeholder values with setup instructions)
  modified:
    - src/Config.h (removed credential definitions, added #include "secrets.h")
    - .gitignore (added src/secrets.h exclusion rule)

key-decisions:
  - "WiFiCredential struct moved to secrets.h alongside WIFI_NETWORKS -- keeps struct and its only consumer co-located"
  - "WS_HOSTNAME uses layered #ifndef: secrets.h defines it, Config.h fallback only fires if neither secrets.h nor build_flags provides it"
  - "AUTH_TOKEN kept as static const char* (matches existing usage in NetworkManager.cpp) rather than switching to #define"

patterns-established:
  - "secrets.h / secrets.h.template: credential files that are gitignored with a committed template for onboarding"

duration: 4min
completed: 2026-02-03
---

# Quick Task 002: Externalize WiFi Credentials to secrets.h

**WiFi credentials (WIFI_NETWORKS, AUTH_TOKEN) and mDNS hostname extracted from Config.h into a gitignored secrets.h, with secrets.h.template committed as the onboarding guide**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-03T08:43:36Z
- **Completed:** 2026-02-03T08:48:10Z
- **Tasks:** 3
- **Files modified:** 4

## Accomplishments

- WiFiCredential struct, WIFI_NETWORKS vector, and AUTH_TOKEN moved to src/secrets.h (gitignored)
- src/secrets.h.template committed with placeholder values and inline comments explaining each field
- Config.h reduced to non-sensitive config only; includes secrets.h for credentials
- .gitignore updated to exclude src/secrets.h; project compiles cleanly

## Task Commits

Each task was committed atomically:

1. **Task 1: Create secrets.h + secrets.h.template** - `1e422de` (feat)
2. **Task 2: Update Config.h to include secrets.h** - `3132a9e` (feat)
3. **Task 3: Update .gitignore to exclude secrets.h** - `a3e5a32` (chore)

**Plan metadata:** (see final commit below)

## Files Created/Modified

- `src/secrets.h` - Gitignored; holds WiFiCredential struct, WIFI_NETWORKS, WS_HOSTNAME, AUTH_TOKEN
- `src/secrets.h.template` - Committed template; placeholder values with setup commentary
- `src/Config.h` - Removed all credential definitions; now includes secrets.h and retains audio/GPIO/WS port config
- `.gitignore` - Added `src/secrets.h` exclusion rule with explanatory comment

## Decisions Made

- WiFiCredential struct placed in secrets.h alongside WIFI_NETWORKS to keep the type and its only instantiation co-located. Config.h no longer needs `<vector>`.
- WS_HOSTNAME uses a layered `#ifndef` chain: build_flags override first, then secrets.h, then the Config.h fallback default. This preserves the existing build_flags workflow documented in the original comment.
- AUTH_TOKEN stays as `static const char*` to match the existing usage pattern in NetworkManager.cpp without requiring call-site changes.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Verification check 7 (`grep AUTH_TOKEN src/Config.h`) triggered on the `#include "secrets.h"` comment line that mentions AUTH_TOKEN for documentation purposes. No actual credential definition remains in Config.h. False positive from a literal grep; all success criteria met.

## User Setup Required

New clones must run:
```
cp src/secrets.h.template src/secrets.h
```
Then edit `src/secrets.h` to fill in actual WiFi SSIDs, passwords, WS_HOSTNAME, and AUTH_TOKEN before building.

## Next Phase Readiness

- Credential isolation complete. Any future phase can reference secrets.h for sensitive values without risk of committing them.
- AUTH_TOKEN and WS_HOSTNAME are available project-wide via secrets.h include chain.

---
*Quick task: 002-externalize-wifi-credentials-to-config-file*
*Completed: 2026-02-03*
