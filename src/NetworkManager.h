#pragma once

#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "Config.h"

// Callback for received hook events
typedef std::function<void(const char* eventName)> HookCallback;

class AppNetworkManager {
public:
    void begin();
    void loop();
    
    bool isConnected();
    
    void sendStart(String reqId);
    void sendEnd(String reqId);
    void sendAudio(uint8_t* data, size_t len);
    
    void setHookCallback(HookCallback cb) { _hookCallback = cb; }

private:
    void connectWiFi();
    void resolveAndConnect();
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    void handleHookEvent(const JsonDocument &doc);
    bool seenId(const String &id);

    WiFiMulti _wifiMulti;
    WebSocketsClient _ws;
    bool _wsConnected = false;
    
    // mDNS resolved IP
    IPAddress _serverIP;
    bool _ipResolved = false;

    HookCallback _hookCallback;
    
    // De-dup
    String _recentIds[16];
    uint8_t _recentIdx = 0;
};

extern AppNetworkManager NetworkMgr;
