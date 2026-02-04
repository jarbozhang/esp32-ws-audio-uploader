#include "NetworkManager.h"

AppNetworkManager NetworkMgr;

void AppNetworkManager::begin() {
    // Setup WiFi Multi
    for (const auto& cred : WIFI_NETWORKS) {
        _wifiMulti.addAP(cred.ssid, cred.password);
    }

    Serial.println("Connecting to WiFi...");
    connectWiFi();
    
    // Initial connection attempt
    resolveAndConnect();
}

void AppNetworkManager::connectWiFi() {
    while (_wifiMulti.run() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
}

void AppNetworkManager::resolveAndConnect() {
    IPAddress ip;

    // Check if WS_HOST is already an IP address
    if (ip.fromString(WS_HOST)) {
        Serial.printf("Using direct IP: %s\n", WS_HOST);
    } else {
        // Need mDNS resolution for hostname
        if (!MDNS.begin("esp32-client")) {
            Serial.println("Error setting up MDNS responder!");
        }

        Serial.printf("Resolving host: %s\n", WS_HOST);
        ip = MDNS.queryHost(WS_HOST);
    }

    if (ip != IPAddress()) {
        Serial.print("Server IP: ");
        Serial.println(ip);
        _serverIP = ip;
        _ipResolved = true;

        // Connect WS
        _ws.begin(_serverIP, WS_PORT, WS_PATH);
        _ws.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
            this->webSocketEvent(type, payload, length);
        });
        _ws.setReconnectInterval(2000);
    } else {
        Serial.println("mDNS resolution failed.");
    }
}

void AppNetworkManager::loop() {
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

bool AppNetworkManager::isConnected() {
    return _wsConnected;
}

void AppNetworkManager::sendStart(String reqId) {
    StaticJsonDocument<256> doc;
    doc["type"] = "start";
    doc["token"] = AUTH_TOKEN;
    doc["reqId"] = reqId;
    doc["mode"] = "paste";
    doc["format"] = FORMAT;
    doc["sampleRate"] = SAMPLE_RATE;
    doc["channels"] = CHANNELS;
    doc["bitDepth"] = BIT_DEPTH;

    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void AppNetworkManager::sendEnd(String reqId) {
    StaticJsonDocument<128> doc;
    doc["type"] = "end";
    doc["reqId"] = reqId;
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void AppNetworkManager::sendApprove() {
    StaticJsonDocument<64> doc;
    doc["type"] = "command";
    doc["action"] = "approve";
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void AppNetworkManager::sendReject() {
    StaticJsonDocument<64> doc;
    doc["type"] = "command";
    doc["action"] = "reject";
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void AppNetworkManager::sendSwitchModel() {
    StaticJsonDocument<64> doc;
    doc["type"] = "command";
    doc["action"] = "switch_model";
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void AppNetworkManager::sendToggleAutoApprove() {
    StaticJsonDocument<64> doc;
    doc["type"] = "command";
    doc["action"] = "toggle_auto_approve";
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void AppNetworkManager::sendAudio(uint8_t* data, size_t len) {
    if (_wsConnected) {
        _ws.sendBIN(data, len);
    }
}

bool AppNetworkManager::seenId(const String &id) {
  if (!id.length()) return false;
  for (auto &s : _recentIds) {
    if (s == id) return true;
  }
  _recentIds[_recentIdx++ % 16] = id;
  return false;
}

void AppNetworkManager::handleHookEvent(const JsonDocument &doc) {
  const char *idc = doc["id"] | "";
  String id = String(idc);
  if (id.length() && seenId(id)) return;

  const char *ev = doc["hook_event_name"] | "";
  if (_hookCallback) {
      _hookCallback(ev);
  }
}

void AppNetworkManager::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
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
