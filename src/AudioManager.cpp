#include "AudioManager.h"

AudioManager AudioMgr;

void AudioManager::begin() {
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

void AudioManager::update() {
    M5.update();
    
    // Handle pending beeps if not recording
    if (!_recording) {
        static unsigned long lastLog = 0;
        if (_pendingStart > 0 && millis() - lastLog > 1000) {
             Serial.printf("DEBUG: [Audio] Update loop. Pending Start: %d\n", _pendingStart);
             lastLog = millis();
        }
        playPendingBeeps();
    }
}

BeepPattern AudioManager::patternFor(BeepKind k) {
    switch (k) {
    case BEEP_PERMISSION:
        return {2000, 200, 2, 500};
    case BEEP_FAILURE:
        return {800, 200, 3, 500};
    case BEEP_START:
        return {2400, 200, 1, 0}; // Single high pitch for start
    case BEEP_STOP:
    default:
        return {1800, 60, 2, 500};
    }
}

void AudioManager::queueBeep(BeepKind kind) {
    Serial.printf("DEBUG: [Audio] Queueing kind %d. PendingStart before: %d\n", kind, _pendingStart);
    if (kind == BEEP_STOP) _pendingStop++;
    else if (kind == BEEP_PERMISSION) _pendingPermission++;
    else if (kind == BEEP_FAILURE) _pendingFailure++;
    else if (kind == BEEP_START) _pendingStart++;
}

void AudioManager::playPendingBeeps() {
    if (!_pendingStop && !_pendingPermission && !_pendingFailure && !_pendingStart) return;

    Serial.printf("DEBUG: [Audio] Playing pending. Start=%d, Perm=%d, Fail=%d, Stop=%d\n", 
                _pendingStart, _pendingPermission, _pendingFailure, _pendingStop);

    // Switch to speaker
    M5.Mic.end();
    delay(100); // Stabilize
    M5.Speaker.begin();
    delay(100); // Stabilize
    M5.Speaker.setVolume(64);
    Serial.printf("DEBUG: [Audio] Volume set to: %d\n", M5.Speaker.getVolume());

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

    // Priority: start > permission > failure > stop
    playN(BEEP_START, _pendingStart);
    playN(BEEP_PERMISSION, _pendingPermission);
    playN(BEEP_FAILURE, _pendingFailure);
    playN(BEEP_STOP, _pendingStop);

    _pendingStop = _pendingPermission = _pendingFailure = _pendingStart = 0;

    M5.Speaker.end();
    M5.Mic.begin();
}

uint8_t AudioManager::pendingBeeps(BeepKind kind) const {
    switch (kind) {
    case BEEP_STOP:       return _pendingStop;
    case BEEP_PERMISSION: return _pendingPermission;
    case BEEP_FAILURE:    return _pendingFailure;
    case BEEP_START:      return _pendingStart;
    default:              return 0;
    }
}

void AudioManager::startRecording() {
    _recording = true;
    _recordStartMs = millis();
    _pendingStop = _pendingPermission = _pendingFailure = _pendingStart = 0;
}

void AudioManager::stopRecording() {
    _recording = false;
}

bool AudioManager::recordOneChunk(int16_t* buf, size_t samples) {
    if (!_recording) return false;
    if (!M5.Mic.isEnabled()) return false;

    // Check timeout
    if (millis() - _recordStartMs > MAX_RECORD_MS) {
        stopRecording();
        return false;
    }

    return M5.Mic.record(buf, samples, SAMPLE_RATE);
}
