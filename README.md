# esp32-ws-audio-uploader

ESP32-S3 firmware that records audio from an I2S microphone (M5Stack Atom EchoS3R via M5Unified) and uploads it to a Mac WebSocket ASR server.

Companion server:
- https://github.com/jarbozhang/mac-whisper-ws-asr-server

## What it does

- Connects to WiFi
- Connects to Mac server via WebSocket: `ws://<mac-host>:8765/ws`
- Push-to-talk:
  - Hold BtnA to record
  - Streams audio as binary PCM frames
  - Releases BtnA to stop and request ASR
- Listens for broadcast hook events from Claude Code (forwarded by the Mac server) and plays notification **beeps**.

## Configuration

Edit `src/main.cpp`:
- `WIFI_SSID`, `WIFI_PASS`
- `AUTH_TOKEN` (must match Mac server `AUTH_TOKEN`)

Mac address:
- Default uses mDNS hostname: `jiabos-macbook-pro-2.local`
- You can override at build time:
  - PlatformIO build flag: `-DWS_HOSTNAME=\"your-mac.local\"`

## Protocol (ASR)

ESP32 → Mac
- Connect to `ws://<mac-host>:8765/ws`
- Send JSON `start` (with token, reqId, audio format params)
- Stream audio chunks as binary frames (raw PCM)
- Send JSON `end`

Mac → ESP32
- Receive JSON `ack`
- Receive JSON `result` with `text`

Audio format (current default):
- `format`: `pcm_s16le`
- `sampleRate`: 16000
- `channels`: 1
- `bitDepth`: 16

Chunking (recommended):
- 20ms @ 16kHz mono s16 => 320 samples => 640 bytes per frame

## Protocol (Hooks broadcast)

Mac server can broadcast compacted Claude Code hook events as JSON text frames:

```json
{ "type": "hook", "id": "uuid", "ts": 1730000000000, "hook_event_name": "Stop", "session_id": "..." }
```

ESP32 beep rules (current):
- `PermissionRequest` → beep (needs approval)
- `PostToolUseFailure` → beep (failure)
- `Stop` → beep (Claude finished output)

Important: Atom EchoS3R mic and speaker cannot be used simultaneously.
This firmware **does not interrupt recording**; it queues beeps and plays them after recording stops.

## Build / Flash (PlatformIO)

This repo uses PlatformIO + Arduino framework.

### Prerequisites

PlatformIO requires **Python 3.10–3.13**. If your system Python doesn't meet this (e.g. Homebrew's externally-managed env or Anaconda base < 3.10), create a dedicated environment first:

```bash
# Anaconda / Miniconda
conda create -n pio python=3.12 -y
conda activate pio
pip install platformio pyyaml          # pyyaml is required by the espressif32 platform builder
```

> `pyyaml` is pulled in implicitly by the ESP32 Arduino framework builder but is not declared as a PlatformIO dependency — install it manually if you see `ModuleNotFoundError: No module named 'yaml'`.

### First build

The first `pio run` automatically downloads:
- **espressif32** platform + Xtensa toolchain
- **Arduino ESP32** framework (IDF 5.5)
- Libraries: WebSockets 2.7.3, M5Unified 0.2.13 (+M5GFX), ArduinoJson 7.4.2

```bash
pio run                  # compile only (downloads everything on first run)
pio run -t upload        # compile + flash firmware to device
pio device monitor       # open serial monitor (115200 baud)
```

### Verified build output (as of 2026-02-03)

| Resource | Used | Total |
|----------|------|-------|
| RAM | 50 KB (15.4%) | 328 KB |
| Flash | 1.3 MB (40.2%) | 3.3 MB |

### Known compiler warnings (harmless)

- `WebSocketsClient.cpp`: `flush()` deprecated — upstream library issue, no functional impact.
- `main.cpp`: `StaticJsonDocument` deprecated in ArduinoJson v7 — replace with `JsonDocument` if desired, no runtime impact.

## Notes

Spec source (Obsidian):
`/Volumes/100.86.103.28/obsidian/20 Areas/Hardware/Claude Code/ESP32 + WebSocket 语音上传 + whisper.cpp ASR（Mac Node）- Spec.md`
