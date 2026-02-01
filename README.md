# esp32-ws-audio-uploader

ESP32-S3 firmware that records audio from an I2S microphone and uploads it to a Mac WebSocket ASR server.

This is the companion client for:
- https://github.com/jarbozhang/mac-whisper-ws-asr-server

## Protocol
- Connect to `ws://<mac-ip>:8765/ws`
- Send JSON `start` (with token, reqId, audio format params)
- Stream audio chunks as binary frames (raw PCM s16le)
- Send JSON `end`
- Receive JSON `result` with `text`

## Notes
- MVP implementation uses Arduino + WebSocketsClient
- Audio format: 16kHz, 16-bit, mono PCM (little-endian)

