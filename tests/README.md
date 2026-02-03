# Testing Guide

## Manual Verification Checklist

### Connectivity
- [ ] **WiFi Connection**: Verify LED status or Serial logs "WiFi connected".
- [ ] **mDNS Resolution**: Verify Serial logs "Resolved IP: ...".
- [ ] **WebSocket Connection**: Verify Serial logs "WS connected".

### Audio
- [ ] **Recording**: Hold BtnA for > 3 seconds. Verify "Recording start".
- [ ] **Server Receipt**: Verify mock server or real server logs binary frame reception.
- [ ] **Audio Clarity**: Inspect saved PCM file (if using real server) in Audacity (16kHz, S16LE, Mono).

### Latency
- [ ] **Round Trip**: Measure time from releasing BtnA to hearing the "Stop" beep (simulated or real). Target < 1s.

### Hook Events
- [ ] **PermissionRequest**: Trigger event (press 'p' in mock server). Verify "High-High" beep.
- [ ] **PostToolUseFailure**: Trigger event (press 'f' in mock server). Verify "Low-Low-Low" beep.
- [ ] **Stop**: Trigger event (press 's' in mock server). Verify "Mid-Mid" beep.

## Running Unit Tests (On-Device)

Requires ESP32 device connected via USB.

```bash
pio test -e esp32-s3
```

## Running Mock Server

Requires Python 3.8+ and `websockets`.

```bash
pip install websockets
python scripts/mock_server.py
```

**Controls:**
- `p`: Send PermissionRequest hook
- `f`: Send PostToolUseFailure hook
- `s`: Send Stop hook
- `q`: Quit
