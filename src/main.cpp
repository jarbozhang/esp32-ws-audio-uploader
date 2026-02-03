#include <Arduino.h>
#include <M5Unified.h>
#include "Config.h"
#include "AudioManager.h"
#include "NetworkManager.h"

static String makeReqId() {
  return String("req-") + String((uint32_t)ESP.getEfuseMac(), HEX) + String("-") + String(millis());
}

static String currentReqId;
static int16_t audioBuf[CHUNK_SAMPLES];

void onHookEvent(const char* eventName) {
    if (!strcmp(eventName, "PermissionRequest")) {
        AudioMgr.queueBeep(BEEP_PERMISSION);
    } else if (!strcmp(eventName, "PostToolUseFailure")) {
        AudioMgr.queueBeep(BEEP_FAILURE);
    } else if (!strcmp(eventName, "Stop")) {
        AudioMgr.queueBeep(BEEP_STOP);
    }
}

#ifndef PIO_UNIT_TEST
void setup() {
    Serial.begin(115200);
    delay(200);

    AudioMgr.begin();
    
    NetworkMgr.setHookCallback(onHookEvent);
    NetworkMgr.begin();
}

void loop() {
    AudioMgr.update();
    NetworkMgr.loop();

    // Button handling
    // M5.update() is called inside AudioMgr.update()
    
    if (!AudioMgr.isRecording() && M5.BtnA.wasPressed()) {
        if (!NetworkMgr.isConnected()) {
            Serial.println("BtnA pressed but WS not connected");
        } else {
            Serial.println("Recording start");
            AudioMgr.startRecording();
            currentReqId = makeReqId();
            NetworkMgr.sendStart(currentReqId);
        }
    }

    if (AudioMgr.isRecording()) {
        // Stop conditions
        if (M5.BtnA.wasReleased()) {
             Serial.println("Recording stop (Btn released)");
             AudioMgr.stopRecording();
             NetworkMgr.sendEnd(currentReqId);
        } else {
             // Record and send
             if (AudioMgr.recordOneChunk(audioBuf, CHUNK_SAMPLES)) {
                 NetworkMgr.sendAudio((uint8_t*)audioBuf, CHUNK_BYTES);
             } else {
                 // Check if it stopped implicitly (timeout)
                 if (!AudioMgr.isRecording()) {
                     Serial.println("Recording stop (Timeout)");
                     NetworkMgr.sendEnd(currentReqId);
                 }
             }
        }
    }

    delay(1);
}
#endif
