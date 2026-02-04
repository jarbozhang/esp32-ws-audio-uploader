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

// Keep-alive state
static unsigned long lastActivityMs = 0;
static unsigned long lastKeepaliveMs = 0;
static bool keepalivePulseActive = false;
static unsigned long pulseStartMs = 0;

static void updateActivity() {
    lastActivityMs = millis();
}

static void keepAliveLoop() {
    unsigned long now = millis();

    // Check if we're within idle timeout
    if ((now - lastActivityMs) > KEEPALIVE_IDLE_TIMEOUT_MS) {
        // Idle too long, allow power bank to sleep
        if (keepalivePulseActive) {
            digitalWrite(KEEPALIVE_PIN, LOW);
            keepalivePulseActive = false;
        }
        return;
    }

    // Handle active pulse
    if (keepalivePulseActive) {
        if ((now - pulseStartMs) >= KEEPALIVE_PULSE_DURATION_MS) {
            digitalWrite(KEEPALIVE_PIN, LOW);
            keepalivePulseActive = false;
        }
        return;
    }

    // Check if it's time for next pulse
    if ((now - lastKeepaliveMs) >= KEEPALIVE_PULSE_INTERVAL_MS) {
        digitalWrite(KEEPALIVE_PIN, HIGH);
        keepalivePulseActive = true;
        pulseStartMs = now;
        lastKeepaliveMs = now;
    }
}

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
__attribute__((weak)) void setup() {
    Serial.begin(115200);
    delay(200);

    AudioMgr.begin();

    NetworkMgr.setHookCallback(onHookEvent);
    NetworkMgr.begin();

    // External control buttons (active LOW with internal pull-up)
    pinMode(BTN_APPROVE_PIN,      INPUT_PULLUP);
    pinMode(BTN_REJECT_PIN,      INPUT_PULLUP);
    pinMode(BTN_SWITCH_MODEL_PIN, INPUT_PULLUP);
    pinMode(BTN_AUTO_APPROVE_PIN, INPUT_PULLUP);
}

__attribute__((weak)) void loop() {
    AudioMgr.update();
    NetworkMgr.loop();

    // --- External control buttons (edge-triggered, active LOW) ---
    {
        struct BtnDef {
            int pin;
            const char* label;
            void (AppNetworkManager::*handler)();
        };
        static const BtnDef buttons[] = {
            {BTN_APPROVE_PIN,      "Approve",        &AppNetworkManager::sendApprove},
            {BTN_REJECT_PIN,      "Reject",         &AppNetworkManager::sendReject},
            {BTN_SWITCH_MODEL_PIN, "SwitchModel",    &AppNetworkManager::sendSwitchModel},
            {BTN_AUTO_APPROVE_PIN, "ToggleAutoApprove", &AppNetworkManager::sendToggleAutoApprove},
        };
        static bool lastState[4] = {true, true, true, true}; // HIGH = not pressed
        static unsigned long lastPressMs[4] = {0, 0, 0, 0};

        for (int i = 0; i < 4; i++) {
            bool cur = digitalRead(buttons[i].pin);
            unsigned long now = millis();
            if (cur == LOW && lastState[i] == HIGH && (now - lastPressMs[i]) > 15) {
                Serial.printf("%s button pressed\n", buttons[i].label);
                lastPressMs[i] = now;
                if (NetworkMgr.isConnected()) {
                    (NetworkMgr.*(buttons[i].handler))();
                } else {
                    Serial.printf("%s button pressed but WS not connected\n", buttons[i].label);
                }
            }
            lastState[i] = cur;
        }
    }

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
