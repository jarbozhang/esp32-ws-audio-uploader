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

    // Maximize power to keep external battery awake
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.setSleep(false);
    Serial.println("WiFi: Max TX power set, Power Save disabled");
}

String AppNetworkManager::stripLocalSuffix(const char* hostname) {
    String h = hostname;
    if (h.endsWith(".local")) {
        h = h.substring(0, h.length() - 6);
    }
    return h;
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

        // Strip .local suffix — MDNS.queryHost() expects bare hostname
        String hostBare = stripLocalSuffix(WS_HOST);
        Serial.printf("Resolving host: %s\n", hostBare.c_str());

        // Retry loop: up to 5 attempts with 1s backoff
        for (int attempt = 0; attempt < 5; attempt++) {
            ip = MDNS.queryHost(hostBare.c_str());
            if (ip != IPAddress()) break;
            Serial.printf("mDNS attempt %d failed, retrying...\n", attempt + 1);
            delay(1000);
        }
    }

    if (ip != IPAddress()) {
        Serial.print("Server IP: ");
        Serial.println(ip);
        _serverIP = ip;
        _ipResolved = true;
        _lastResolveTime = millis();

        // Connect WS
        _ws.begin(_serverIP, WS_PORT, WS_PATH);
        _ws.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
            this->webSocketEvent(type, payload, length);
        });
        _ws.setReconnectInterval(2000);
    } else {
        Serial.println("mDNS resolution failed after 5 attempts.");
    }
}

void AppNetworkManager::loop() {
    // Ensure WiFi
    if (_wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi lost, reconnecting...");
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (!_ipResolved) {
            // Initial resolution not yet done — attempt now
            resolveAndConnect();
        } else {
            // Periodic re-resolution: re-query mDNS in case server IP changed
            uint32_t now = millis();
            if (now - _lastResolveTime >= MDNS_RECHECK_INTERVAL_MS) {
                Serial.println("mDNS recheck: re-resolving host...");
                IPAddress ip;
                if (!ip.fromString(WS_HOST)) {
                    // Only re-resolve if WS_HOST is a hostname, not a direct IP
                    String hostBare = stripLocalSuffix(WS_HOST);
                    ip = MDNS.queryHost(hostBare.c_str());
                    if (ip != IPAddress() && ip != _serverIP) {
                        Serial.print("mDNS recheck: IP changed to ");
                        Serial.println(ip);
                        _serverIP = ip;
                        // Reconnect WebSocket with new IP
                        _ws.disconnect();
                        _ws.begin(_serverIP, WS_PORT, WS_PATH);
                    }
                }
                _lastResolveTime = now;
            }
        }
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
    Serial.println("DEBUG: [NM] WS Connected event received");
    if (_hookCallback) {
        Serial.println("DEBUG: [NM] Calling Connected hook");
        _hookCallback("Connected");
    }
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
