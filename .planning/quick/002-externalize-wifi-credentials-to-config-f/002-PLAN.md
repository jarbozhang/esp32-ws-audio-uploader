---
quick: 002
name: externalize-wifi-credentials-to-config-file
type: execute
files_modified:
  - src/secrets.h
  - src/secrets.h.template
  - src/Config.h
  - .gitignore
autonomous: true

must_haves:
  truths:
    - "WiFi credentials and auth tokens are NOT in committed source code"
    - "New clones can create their own secrets.h from template"
    - "Existing code compiles without modification"
  artifacts:
    - path: "src/secrets.h"
      provides: "Actual credentials (gitignored)"
    - path: "src/secrets.h.template"
      provides: "Template with placeholder values"
    - path: ".gitignore"
      contains: "secrets.h"
  key_links:
    - from: "src/Config.h"
      to: "src/secrets.h"
      via: "#include"
---

<objective>
Externalize WiFi credentials and sensitive configuration to a separate secrets.h file that is gitignored, preventing credential exposure when pushing to GitHub.

Purpose: Security - keep credentials out of version control
Output: secrets.h (gitignored) + secrets.h.template (committed) + updated Config.h
</objective>

<context>
Current state: src/Config.h contains hardcoded WiFi credentials (WIFI_NETWORKS), WS_HOSTNAME, and AUTH_TOKEN.

Arduino/ESP32 convention: Use secrets.h pattern - a header file containing sensitive values that is gitignored, with a .template version committed for reference.
</context>

<tasks>

<task type="auto">
  <name>Task 1: Create secrets.h with actual credentials + template version</name>
  <files>src/secrets.h, src/secrets.h.template</files>
  <action>
    1. Create src/secrets.h containing:
       - Header guard (#pragma once)
       - WiFiCredential struct definition (copy from Config.h)
       - WIFI_NETWORKS vector with current placeholder values
       - AUTH_TOKEN define
       - WS_HOSTNAME define (the mDNS hostname)

    2. Create src/secrets.h.template with the same structure but clearly marked placeholder values:
       - WIFI_NETWORKS with {"YOUR_SSID", "YOUR_PASSWORD"} entries
       - AUTH_TOKEN = "your_auth_token_here"
       - WS_HOSTNAME = "your-hostname.local"
       - Include comments explaining what each value should be
  </action>
  <verify>Both files exist: `ls -la src/secrets.h src/secrets.h.template`</verify>
  <done>secrets.h contains the actual struct/values, secrets.h.template has documented placeholders</done>
</task>

<task type="auto">
  <name>Task 2: Update Config.h to include secrets.h</name>
  <files>src/Config.h</files>
  <action>
    1. Add `#include "secrets.h"` near the top (after Arduino.h)
    2. Remove the WiFiCredential struct definition (now in secrets.h)
    3. Remove the WIFI_NETWORKS vector definition (now in secrets.h)
    4. Remove the AUTH_TOKEN definition (now in secrets.h)
    5. Keep the #ifndef WS_HOSTNAME / #define WS_HOSTNAME block but update it:
       - It should check if WS_HOSTNAME is already defined (from secrets.h or build_flags)
       - Only provide a default fallback if not defined
    6. Keep all other non-sensitive config (audio settings, pins, etc.)
  </action>
  <verify>`pio run` compiles successfully</verify>
  <done>Config.h includes secrets.h and no longer contains credentials inline</done>
</task>

<task type="auto">
  <name>Task 3: Update .gitignore to exclude secrets.h</name>
  <files>.gitignore</files>
  <action>
    Add to .gitignore:
    ```
    # Secrets (credentials)
    src/secrets.h
    ```

    Note: secrets.h.template should NOT be gitignored - it gets committed as documentation.
  </action>
  <verify>`git check-ignore src/secrets.h` returns the path (ignored); `git check-ignore src/secrets.h.template` returns nothing (not ignored)</verify>
  <done>secrets.h is gitignored, secrets.h.template is not</done>
</task>

</tasks>

<verification>
1. `pio run` - Project compiles successfully
2. `git check-ignore src/secrets.h` - Returns path (file is ignored)
3. `git check-ignore src/secrets.h.template` - Returns nothing (file can be committed)
4. `grep -q "WIFI_NETWORKS" src/secrets.h` - Credentials are in secrets.h
5. `grep -q "WIFI_NETWORKS" src/Config.h` returns nothing - No credentials in Config.h
</verification>

<success_criteria>
- WiFi credentials (WIFI_NETWORKS) moved to src/secrets.h
- AUTH_TOKEN moved to src/secrets.h
- WS_HOSTNAME moved to src/secrets.h (with fallback in Config.h)
- src/secrets.h is gitignored
- src/secrets.h.template committed with placeholder values and documentation
- Project compiles without errors
- Config.h includes secrets.h and contains no hardcoded credentials
</success_criteria>

<output>
After completion, create `.planning/quick/002-externalize-wifi-credentials-to-config-f/002-SUMMARY.md`
</output>
