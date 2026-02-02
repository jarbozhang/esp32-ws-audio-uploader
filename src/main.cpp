#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#include <ArduinoJson.h>
#include <M5Unified.h>

// ========= User config =========
static const char *WIFI_SSID = "YOUR_WIFI";
static const char *WIFI_PASS = "YOUR_PASS";

// Mac mDNS hostname (no IP scanning). Recommend configuring this via build_flags
// (e.g., -DWS_HOSTNAME=\"jiabos-macbook-pro-2.local\") to avoid hardcoding.
#ifndef WS_HOSTNAME
#define WS_HOSTNAME "jiabos-macbook-pro-2.local"
#endif

static const char *WS_HOST = WS_HOSTNAME;
static const uint16_t WS_PORT = 8765;
static const char *WS_PATH = "/ws";

static const char *AUTH_TOKEN = "change_me";
// ===============================

// Audio format (must match mac-whisper-ws-asr-server expectations)
static constexpr int SAMPLE_RATE = 16000;
static constexpr int CHANNELS = 1;
static constexpr int BIT_DEPTH = 16;
static constexpr const char *FORMAT = "pcm_s16le";

// Chunking: 20ms @16kHz => 320 samples => 640 bytes (s16)
static constexpr int CHUNK_SAMPLES = 320;
static constexpr int CHUNK_BYTES = CHUNK_SAMPLES * (BIT_DEPTH / 8) * CHANNELS;

// Recording duration cap (safety)
static constexpr uint32_t MAX_RECORD_MS = 8000;

WebSocketsClient ws;

enum BeepKind {
  BEEP_STOP,
  BEEP_PERMISSION,
  BEEP_FAILURE,
};

struct BeepPattern {
  uint16_t freq;
  uint16_t ms;
  uint8_t repeat;
  uint16_t gapMs;
};

static BeepPattern patternFor(BeepKind k) {
  switch (k) {
  case BEEP_PERMISSION:
    return {2000, 80, 2, 60};
  case BEEP_FAILURE:
    return {800, 200, 3, 80};
  case BEEP_STOP:
  default:
    return {1800, 60, 2, 80};
  }
}

static volatile bool wsConnected = false;
static bool recording = false;
static uint32_t recordStartMs = 0;

// Pending beeps are queued while recording and played after stop.
static uint8_t pendingStop = 0;
static uint8_t pendingPermission = 0;
static uint8_t pendingFailure = 0;

// Simple de-dup ring for hook ids
static String recentIds[16];
static uint8_t recentIdx = 0;
static bool seenId(const String &id) {
  if (!id.length()) return false;
  for (auto &s : recentIds) {
    if (s == id) return true;
  }
  recentIds[recentIdx++ % 16] = id;
  return false;
}

static void queueBeep(BeepKind kind) {
  if (recording) {
    if (kind == BEEP_STOP) pendingStop++;
    else if (kind == BEEP_PERMISSION) pendingPermission++;
    else if (kind == BEEP_FAILURE) pendingFailure++;
    return;
  }
  // if not recording, play immediately
  auto p = patternFor(kind);
  M5.Mic.end();
  M5.Speaker.begin();
  for (int i = 0; i < p.repeat; i++) {
    M5.Speaker.tone(p.freq, p.ms);
    delay(p.ms + p.gapMs);
  }
  M5.Speaker.end();
  M5.Mic.begin();
}

static void playPendingBeeps() {
  if (!pendingStop && !pendingPermission && !pendingFailure) return;

  // Switch to speaker
  M5.Mic.end();
  M5.Speaker.begin();

  auto playN = [&](BeepKind k, uint8_t n) {
    auto p = patternFor(k);
    for (uint8_t j = 0; j < n; j++) {
      for (uint8_t i = 0; i < p.repeat; i++) {
        M5.Speaker.tone(p.freq, p.ms);
        delay(p.ms + p.gapMs);
      }
      delay(120);
    }
  };

  // Priority: permission > failure > stop
  playN(BEEP_PERMISSION, pendingPermission);
  playN(BEEP_FAILURE, pendingFailure);
  playN(BEEP_STOP, pendingStop);

  pendingStop = pendingPermission = pendingFailure = 0;

  M5.Speaker.end();
  M5.Mic.begin();
}

static String makeReqId() {
  // good-enough unique id on embedded
  return String("req-") + String((uint32_t)ESP.getEfuseMac(), HEX) + String("-") + String(millis());
}

static String currentReqId;

static void sendStart() {
  currentReqId = makeReqId();
  StaticJsonDocument<256> doc;
  doc["type"] = "start";
  doc["token"] = AUTH_TOKEN;
  doc["reqId"] = currentReqId;
  doc["mode"] = "return_only";
  doc["format"] = FORMAT;
  doc["sampleRate"] = SAMPLE_RATE;
  doc["channels"] = CHANNELS;
  doc["bitDepth"] = BIT_DEPTH;

  String out;
  serializeJson(doc, out);
  ws.sendTXT(out);
}

static void sendEnd() {
  StaticJsonDocument<128> doc;
  doc["type"] = "end";
  doc["reqId"] = currentReqId;
  String out;
  serializeJson(doc, out);
  ws.sendTXT(out);
}

static void handleHookEvent(const JsonDocument &doc) {
  const char *idc = doc["id"] | "";
  String id = String(idc);
  if (id.length() && seenId(id)) return;

  const char *ev = doc["hook_event_name"] | "";
  if (!strcmp(ev, "PermissionRequest")) {
    queueBeep(BEEP_PERMISSION);
    return;
  }
  if (!strcmp(ev, "PostToolUseFailure")) {
    queueBeep(BEEP_FAILURE);
    return;
  }
  if (!strcmp(ev, "Stop")) {
    queueBeep(BEEP_STOP);
    return;
  }
}

static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    wsConnected = false;
    Serial.println("WS disconnected");
    break;
  case WStype_CONNECTED:
    wsConnected = true;
    Serial.println("WS connected");
    break;
  case WStype_TEXT: {
    // payload isn't null-terminated
    String s;
    s.reserve(length + 1);
    for (size_t i = 0; i < length; i++) s += (char)payload[i];

    StaticJsonDocument<1024> doc;
    auto err = deserializeJson(doc, s);
    if (err) {
      Serial.printf("WS text (non-json): %s\n", s.c_str());
      return;
    }

    const char *t = doc["type"] | "";
    if (!strcmp(t, "hook")) {
      handleHookEvent(doc);
      return;
    }

    // Log other responses (ack/result/error)
    Serial.printf("WS json: %s\n", s.c_str());
    break;
  }
  default:
    break;
  }
}

static void setupAudio() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // Mic noise filter tweak from docs example
  auto micCfg = M5.Mic.config();
  micCfg.noise_filter_level = (micCfg.noise_filter_level + 8) & 255;
  M5.Mic.config(micCfg);

  // Start with mic enabled, speaker disabled
  M5.Speaker.end();
  M5.Mic.begin();
}

static bool recordOneChunkAndSend() {
  static int16_t buf[CHUNK_SAMPLES];

  if (!M5.Mic.isEnabled()) return false;
  if (!wsConnected) return false;

  bool ok = M5.Mic.record(buf, CHUNK_SAMPLES, SAMPLE_RATE);
  if (!ok) return false;

  ws.sendBIN((uint8_t *)buf, CHUNK_BYTES);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  setupAudio();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi OK, IP=");
  Serial.println(WiFi.localIP());

  ws.begin(WS_HOST, WS_PORT, WS_PATH);
  ws.onEvent(webSocketEvent);
  ws.setReconnectInterval(2000);
}

void loop() {
  ws.loop();
  M5.update();

  // Push-to-talk: hold button to record
  if (!recording && M5.BtnA.wasPressed()) {
    if (!wsConnected) {
      Serial.println("BtnA pressed but WS not connected");
    } else {
      Serial.println("Recording start");
      recording = true;
      recordStartMs = millis();
      // Optional: keep pending from earlier; currently reset on each recording.
      pendingStop = pendingPermission = pendingFailure = 0;
      sendStart();
    }
  }

  if (recording) {
    // Stop conditions
    if (M5.BtnA.wasReleased() || (millis() - recordStartMs) > MAX_RECORD_MS) {
      Serial.println("Recording stop");
      recording = false;
      sendEnd();

      // After stop, play any queued beeps (A strategy)
      playPendingBeeps();
    } else {
      recordOneChunkAndSend();
    }
  }

  delay(1);
}
