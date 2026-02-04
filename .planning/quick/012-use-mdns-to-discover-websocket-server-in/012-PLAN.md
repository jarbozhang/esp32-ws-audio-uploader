---
phase: quick
plan: 012
type: execute
wave: 1
depends_on: []
files_modified:
  - src/NetworkManager.cpp
  - src/Config.h
  - src/secrets.h.template
autonomous: true
---

<objective>
Fix mDNS hostname resolution so ESP32 can reliably discover the WebSocket server by hostname.

Purpose: The existing mDNS code has a bug - `MDNS.queryHost()` expects hostname WITHOUT the `.local` suffix, but the config uses `"hostname.local"`. Also needs retry logic for reliability.

Output: ESP32 reliably resolves Mac hostname and connects even when Mac's IP changes.
</objective>

<execution_context>
@/Users/jiabozhang/.claude/get-shit-done/workflows/execute-plan.md
@/Users/jiabozhang/.claude/get-shit-done/templates/summary.md
</execution_context>

<context>
@.planning/STATE.md
@src/NetworkManager.cpp
@src/NetworkManager.h
@src/Config.h
@src/secrets.h.template
</context>

<tasks>

<task type="auto">
  <name>Task 1: Fix mDNS hostname parsing and add retry logic</name>
  <files>src/NetworkManager.cpp, src/Config.h</files>
  <action>
  1. In `resolveAndConnect()`, strip `.local` suffix from hostname before calling `MDNS.queryHost()`:
     - If `WS_HOST` ends with `.local`, remove it before passing to queryHost
     - `MDNS.queryHost("my-macbook")` NOT `MDNS.queryHost("my-macbook.local")`

  2. Add retry loop with backoff for mDNS resolution (up to 5 attempts, 1s between):
     ```cpp
     IPAddress resolvedIP;
     for (int attempt = 0; attempt < 5; attempt++) {
         resolvedIP = MDNS.queryHost(hostWithoutLocal);
         if (resolvedIP != IPAddress()) break;
         Serial.printf("mDNS attempt %d failed, retrying...\n", attempt + 1);
         delay(1000);
     }
     ```

  3. Add a helper function `stripLocalSuffix(const char* hostname)` that returns hostname without `.local`:
     ```cpp
     String stripLocalSuffix(const char* hostname) {
         String h = hostname;
         if (h.endsWith(".local")) {
             h = h.substring(0, h.length() - 6);
         }
         return h;
     }
     ```

  4. Add periodic re-resolution: In `loop()`, if connected but hasn't re-resolved in 5 minutes, re-query mDNS (in case server IP changed). Store `_lastResolveTime` and check against it.

  5. In Config.h, add constant for re-resolution interval:
     ```cpp
     static constexpr uint32_t MDNS_RECHECK_INTERVAL_MS = 300000; // 5 minutes
     ```
  </action>
  <verify>
  - Compile with `pio run` succeeds
  - Serial output shows: "Resolving host: my-macbook" (without .local)
  - After 5 failed attempts, appropriate error message shown
  </verify>
  <done>
  - mDNS resolution works with hostnames configured as "hostname.local" in secrets.h
  - Retry logic prevents single-shot failures from blocking connection
  - Periodic re-resolution handles server IP changes
  </done>
</task>

<task type="auto">
  <name>Task 2: Update secrets.h.template documentation</name>
  <files>src/secrets.h.template</files>
  <action>
  Update the comment for WS_HOSTNAME to clarify:
  - Can use either "hostname" or "hostname.local" (code handles both)
  - Can also use direct IP address if mDNS is problematic
  - Add example showing how to find Mac hostname: `scutil --get LocalHostName`

  Example comment:
  ```cpp
  // WebSocket server hostname for mDNS discovery.
  // Can be either:
  //   - "my-macbook.local" (full mDNS name)
  //   - "my-macbook" (just hostname, .local added automatically)
  //   - "192.168.1.100" (direct IP, skips mDNS)
  //
  // To find your Mac's hostname: scutil --get LocalHostName
  // Or check System Settings > General > About > Name
  ```
  </action>
  <verify>
  - File compiles when copied to secrets.h
  - Comments are clear and helpful
  </verify>
  <done>
  - secrets.h.template has clear documentation for hostname configuration
  </done>
</task>

<task type="auto">
  <name>Task 3: Test and verify mDNS discovery</name>
  <files>N/A (testing)</files>
  <action>
  1. Build and upload to ESP32: `pio run -t upload`
  2. Open serial monitor: `pio device monitor`
  3. Verify serial output shows:
     - "Resolving host: [hostname-without-local]"
     - "Server IP: [resolved-ip]"
     - "WS connected"
  4. If Mac's hostname is configured in secrets.h, verify connection works
  5. Test fallback: temporarily set WS_HOSTNAME to a direct IP and verify it skips mDNS
  </action>
  <verify>
  - Serial output confirms mDNS resolution succeeds
  - WebSocket connection establishes
  - Direct IP fallback works when configured
  </verify>
  <done>
  - ESP32 successfully discovers and connects to WebSocket server via mDNS
  - Fallback to direct IP works when mDNS hostname is an IP address
  </done>
</task>

</tasks>

<verification>
1. `pio run` compiles without errors
2. Serial monitor shows successful mDNS resolution
3. WebSocket connects after mDNS resolution
4. If server restarts with new IP, ESP32 re-discovers within 5 minutes
</verification>

<success_criteria>
- ESP32 reliably discovers WebSocket server via mDNS hostname
- Works with both "hostname" and "hostname.local" format
- Falls back gracefully to direct IP when configured
- Re-resolves periodically to handle IP changes
</success_criteria>

<output>
After completion, create `.planning/quick/012-use-mdns-to-discover-websocket-server-in/012-SUMMARY.md`
</output>
