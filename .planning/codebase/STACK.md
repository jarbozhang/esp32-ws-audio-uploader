# Technology Stack

**Analysis Date:** 2026-02-02

## Languages

**Primary:**
- C++ - Firmware code for ESP32-S3 microcontroller

**Secondary:**
- Arduino (C++ dialect) - Sketches and platform-specific APIs

## Runtime

**Environment:**
- ESP32-S3 microcontroller (Xtensa LX7 dual-core processor)
- Arduino framework

**Package Manager:**
- PlatformIO - Embedded systems development platform and dependency manager

## Frameworks

**Core:**
- Arduino Framework - Provides hardware abstraction for ESP32

**Connectivity:**
- WiFi (built-in to ESP32)
- WebSockets Client Library - `links2004/WebSockets@^2.4.1`

**Audio:**
- I2S (Inter-IC Sound) - Hardware protocol for microphone communication (planned, not yet implemented)

## Key Dependencies

**Critical:**
- `links2004/WebSockets@^2.4.1` - WebSocket client library for ESP32, enables bidirectional communication with WebSocket ASR server

**Hardware Libraries:**
- WiFi - Built-in ESP32 WiFi capability
- Serial - Built-in UART for debugging

## Configuration

**Environment:**
- Configured via `platformio.ini`
- WiFi credentials: SSID and password (currently placeholders in source)
- WebSocket server address and port hardcoded in source
- Authentication token hardcoded in source

**Build:**
- `platformio.ini` - PlatformIO project configuration
  - Platform: espressif32
  - Board: esp32-s3-devkitc-1
  - Monitor speed: 115200 baud
  - Build flags: `-DARDUINO_USB_CDC_ON_BOOT=1` (enables USB serial over CDC)

## Platform Requirements

**Development:**
- PlatformIO CLI or VS Code PlatformIO extension
- USB connection to ESP32-S3 development board

**Hardware:**
- ESP32-S3-DevKitC-1 board
- I2S microphone (planned for future implementation)
- WiFi network access

**Production:**
- ESP32-S3 module or development board
- WiFi connectivity to network
- Access to Mac WebSocket ASR server at known IP and port 8765

## Audio Configuration

**Specifications:**
- Sample Rate: 16 kHz
- Channels: 1 (mono)
- Bit Depth: 16-bit
- Format: PCM signed 16-bit little-endian (pcm_s16le)

---

*Stack analysis: 2026-02-02*
