# External Integrations

**Analysis Date:** 2026-02-02

## APIs & External Services

**WebSocket ASR Server:**
- Service: Mac-based Whisper.cpp ASR (Automatic Speech Recognition) server
- What it's used for: Receives PCM audio from ESP32 and returns speech-to-text results
- Repository: `https://github.com/jarbozhang/mac-whisper-ws-asr-server`
- Protocol: WebSocket over TCP/IP
- Connection Details:
  - Host: Configurable (currently hardcoded as `192.168.1.10`)
  - Port: `8765`
  - Path: `/ws`

## Network Configuration

**WiFi:**
- Connection Type: WiFi STA (Station mode)
- SSID: Configurable (placeholder: `YOUR_WIFI`)
- Password: Configurable (placeholder: `YOUR_PASS`)
- IP Address: Obtained via DHCP after connection

## Authentication & Identity

**Auth Provider:**
- Custom token-based authentication
- Implementation: Bearer token sent in WebSocket `start` message
- Environment: Auth token hardcoded in source as `AUTH_TOKEN` (currently set to `change_me`)
- Token Usage: Included in JSON payload during WebSocket handshake

## WebSocket Protocol

**Incoming (from Server):**
- JSON message with type `result` containing recognized text
- Connection status messages (connected/disconnected events)

**Outgoing (to Server):**
- JSON `start` message with:
  - `type`: "start"
  - `token`: Authentication token
  - `reqId`: Request ID identifier
  - `format`: Audio format ("pcm_s16le")
  - `sampleRate`: 16000
  - `channels`: 1
  - `bitDepth`: 16
  - `mode`: "return_only" (only return text, no intermediate results)
- Binary frames: Raw PCM audio data (s16le format)
- JSON `end` message with:
  - `type`: "end"
  - `reqId`: Request ID
  - `chunks`: Number of audio chunks sent

## Data Storage

**Databases:**
- Not used - stateless real-time processing

**File Storage:**
- Local filesystem: Not used
- All data transmitted over WebSocket in real-time

**Caching:**
- Not used

## Monitoring & Observability

**Error Tracking:**
- Not integrated - Local error handling only

**Logs:**
- Serial console output via USB CDC (115200 baud)
- Debug messages printed to Serial for WiFi status, WebSocket events, and audio metrics

## Hardware Integration

**I2S Microphone:**
- Currently planned but not implemented (MVP scaffold)
- Will be connected via I2S pins on ESP32-S3
- Audio capture specifications: 16 kHz sample rate, 16-bit depth, mono

**Serial Communication:**
- USB CDC (USB-to-Serial) for debugging and monitoring
- Baud rate: 115200

## Environment Configuration

**Required env vars (currently hardcoded):**
- WiFi SSID and password
- WebSocket server IP address
- WebSocket server port (8765)
- Authentication token

**Configuration Location:**
- `src/main.cpp` - Lines 6-13 contain connection parameters and credentials
- Planned: Move to external configuration file or EEPROM storage

## Webhooks & Callbacks

**Incoming Webhooks:**
- WebSocket text frames containing ASR results
- WebSocket events (CONNECTED, DISCONNECTED, TEXT, BIN)

**Outgoing Webhooks:**
- Not used - Unidirectional push to WebSocket server only

## Companion Infrastructure

**Mac-side Server:**
- URL: `https://github.com/jarbozhang/mac-whisper-ws-asr-server`
- Technology: Node.js WebSocket server with Whisper.cpp integration
- Handles: Speech-to-text transcription using OpenAI's Whisper model

---

*Integration audit: 2026-02-02*
