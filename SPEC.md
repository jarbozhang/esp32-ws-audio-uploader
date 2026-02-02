# Spec (esp32-ws-audio-uploader)

This project is the ESP32 client side of the overall system.

Canonical spec lives in Obsidian:
`/Volumes/100.86.103.28/obsidian/20 Areas/Hardware/Claude Code/ESP32 + WebSocket 语音上传 + whisper.cpp ASR（Mac Node）- Spec.md`

## WebSocket

- URL: `ws://<mac-host>:8765/ws`
- Auth: send `token` in `start` message; must match server `AUTH_TOKEN`

### Start
```json
{
  "type": "start",
  "token": "...",
  "reqId": "...",
  "mode": "return_only",
  "format": "pcm_s16le",
  "sampleRate": 16000,
  "channels": 1,
  "bitDepth": 16
}
```

### Audio
Binary frames: raw PCM bytes.

Default audio format:
- 16kHz
- mono
- 16-bit signed little-endian (`pcm_s16le`)

### End
```json
{ "type": "end", "reqId": "..." }
```

## Hooks broadcast (server → esp32)

The Mac server may broadcast hook events:
```json
{ "type": "hook", "id": "uuid", "ts": 1730000000000, "hook_event_name": "Stop" }
```

ESP32 uses these to trigger beeps (queued while recording).
