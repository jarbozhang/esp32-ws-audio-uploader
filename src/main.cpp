#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

// TODO: replace with your WiFi + server
static const char* WIFI_SSID = "YOUR_WIFI";
static const char* WIFI_PASS = "YOUR_PASS";

static const char* WS_HOST = "192.168.1.10"; // Mac IP
static const uint16_t WS_PORT = 8765;
static const char* WS_PATH = "/ws";

static const char* AUTH_TOKEN = "change_me";

WebSocketsClient ws;

// Audio settings (MVP constants)
static const int SAMPLE_RATE = 16000;
static const int CHANNELS = 1;
static const int BIT_DEPTH = 16;

// In MVP we don't actually record from I2S yet.
// This file is a scaffold: it shows the WebSocket protocol.

String reqId = "req-1";

void sendStart() {
  String msg = String("{\"type\":\"start\",\"token\":\"") + AUTH_TOKEN +
    "\",\"reqId\":\"" + reqId +
    "\",\"format\":\"pcm_s16le\",\"sampleRate\":" + SAMPLE_RATE +
    ",\"channels\":" + CHANNELS +
    ",\"bitDepth\":" + BIT_DEPTH +
    ",\"mode\":\"return_only\"}";
  ws.sendTXT(msg);
}

void sendEnd(uint32_t chunks) {
  String msg = String("{\"type\":\"end\",\"reqId\":\"") + reqId +
    "\",\"chunks\":" + chunks + "}";
  ws.sendTXT(msg);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WS disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WS connected");
      sendStart();
      break;
    case WStype_TEXT:
      Serial.printf("WS text: %.*s\n", (int)length, payload);
      break;
    case WStype_BIN:
      Serial.printf("WS bin len=%u\n", (unsigned)length);
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

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

  // TODO: implement PTT + I2S recording loop:
  // - wait for button
  // - sendStart
  // - stream PCM frames as ws.sendBIN()
  // - sendEnd
  delay(10);
}
