#pragma once

#include <Arduino.h>
#include <vector>

// WiFi Configuration
struct WiFiCredential {
    const char* ssid;
    const char* password;
};

// Add your WiFi networks here
static const std::vector<WiFiCredential> WIFI_NETWORKS = {
    {"YOUR_WIFI_SSID_1", "YOUR_WIFI_PASS_1"},
    {"YOUR_WIFI_SSID_2", "YOUR_WIFI_PASS_2"}
};

// WebSocket Configuration
// Mac mDNS hostname (no IP scanning). 
// Recommend configuring this via build_flags (e.g., -DWS_HOSTNAME=\"jiabos-macbook-pro-2.local\")
#ifndef WS_HOSTNAME
#define WS_HOSTNAME "jiabos-macbook-pro-2.local"
#endif

static const char *WS_HOST = WS_HOSTNAME;
static const uint16_t WS_PORT = 8765;
static const char *WS_PATH = "/ws";

static const char *AUTH_TOKEN = "change_me";

// Audio Configuration
static constexpr int SAMPLE_RATE = 16000;
static constexpr int CHANNELS = 1;
static constexpr int BIT_DEPTH = 16;
static constexpr const char *FORMAT = "pcm_s16le";

// Chunking: 20ms @16kHz => 320 samples => 640 bytes (s16)
static constexpr int CHUNK_SAMPLES = 320;
static constexpr int CHUNK_BYTES = CHUNK_SAMPLES * (BIT_DEPTH / 8) * CHANNELS;

// Recording duration cap (safety)
static constexpr uint32_t MAX_RECORD_MS = 8000;

// Control buttons (active LOW with INPUT_PULLUP)
#define BTN_APPROVE_PIN      5
#define BTN_REJECT_PIN       6
#define BTN_SWITCH_MODEL_PIN 7
#define BTN_AUTO_APPROVE_PIN 8
