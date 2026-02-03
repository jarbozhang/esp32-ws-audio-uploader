#include "NetworkManager.h"

NetworkManager NetworkMgr;

void NetworkManager::begin() {
    // Setup WiFi Multi
    for (const auto& cred : WIFI_NETWORKS) {
        _wifiMulti.addAP(cred.ssid, cred.password);
    }

    Serial.println("Connecting to WiFi...");
    connectWiFi();
    
    // Initial connection attempt
    resolveAndConnect();
}

void NetworkManager::connectWiFi() {
    while (_wifiMulti.run() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
}

void NetworkManager::resolveAndConnect() {
    if (!MDNS.begin("esp32-client")) {
        Serial.println("Error setting up MDNS responder!");
    }
    
    Serial.printf("Resolving host: %s\n", WS_HOST);
    // MDNS.queryHost blocks
    IPAddress ip = MDNS.queryHost(WS_HOST);
    
    if (ip != IPAddress()) {
        Serial.print("Resolved IP: ");
        Serial.println(ip);
        _serverIP = ip;
        _ipResolved = true;
        
        // Connect WS
        _ws.begin(_serverIP, WS_PORT, WS_PATH);
        // We need to bind the member function, but WebSocketsClient uses a C-style callback or std::function
        // The library supports std::function so we can use a lambda capture
        _ws.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
            this->webSocketEvent(type, payload, length);
        });
        _ws.setReconnectInterval(2000);
    } else {
        Serial.println("mDNS resolution failed.");
    }
}

void NetworkManager::loop() {
    // Ensure WiFi
    if (_wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi lost, reconnecting...");
    }
    
    // Ensure mDNS/WS
    if (WiFi.status() == WL_CONNECTED && !_ipResolved) {
         // Retry resolution occasionally? 
         // For simplicity, just retry now
         resolveAndConnect();
    }

    _ws.loop();
}

bool NetworkManager::isConnected() {
    return _wsConnected;
}

void NetworkManager::sendStart(String reqId) {
    StaticJsonDocument<256> doc;
    doc["type"] = "start";
    doc["token"] = AUTH_TOKEN;
    doc["reqId"] = reqId;
    doc["mode"] = "return_only";
    doc["format"] = FORMAT;
    doc["sampleRate"] = SAMPLE_RATE;
    doc["channels"] = CHANNELS;
    doc["bitDepth"] = BIT_DEPTH;

    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void NetworkManager::sendEnd(String reqId) {
    StaticJsonDocument<128> doc;
    doc["type"] = "end";
    doc["reqId"] = reqId;
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void NetworkManager::sendAudio(uint8_t* data, size_t len) {
    if (_wsConnected) {
        _ws.sendBIN(data, len);
    }
}

bool NetworkManager::seenId(const String &id) {
  if (!id.length()) return false;
  for (auto &s : _recentIds) {
    if (s == id) return true;
  }
  _recentIds[_recentIdx++ % 16] = id;
  return false;
}

void NetworkManager::handleHookEvent(const JsonDocument &doc) {
  const char *idc = doc["id"] | "";
  String id = String(idc);
  if (id.length() && seenId(id)) return;

  const char *ev = doc["hook_event_name"] | "";
  if (_hookCallback) {
      _hookCallback(ev);
  }
}

void NetworkManager::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    _wsConnected = false;
    Serial.println("WS disconnected");
    break;
  case WStype_CONNECTED:
    _wsConnected = true;
    Serial.println("WS connected");
    break;
  case WStype_TEXT: {
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

    Serial.printf("WS json: %s\n", s.c_str());
    break;
  }
  default:
    break;
  }
}
