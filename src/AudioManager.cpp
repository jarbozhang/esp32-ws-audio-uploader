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
        playPendingBeeps();
    }
}

BeepPattern AudioManager::patternFor(BeepKind k) {
    switch (k) {
    case BEEP_PERMISSION:
        return {2000, 80, 2, 60};
    case BEEP_FAILURE:
        return {800, 200, 3, 80};
    case BEEP_START:
        return {2400, 100, 1, 0}; // Single high pitch for start
    case BEEP_STOP:
    default:
        return {1800, 60, 2, 80};
    }
}

void AudioManager::queueBeep(BeepKind kind) {
    if (_recording) {
        if (kind == BEEP_STOP) _pendingStop++;
        else if (kind == BEEP_PERMISSION) _pendingPermission++;
        else if (kind == BEEP_FAILURE) _pendingFailure++;
        else if (kind == BEEP_START) _pendingStart++;
        return;
    }
    
    // If not recording, play immediately
    Serial.printf("AudioMgr: Playing beep kind %d immediate\n", kind);
    
    // Need to temporarily switch to speaker
    M5.Mic.end();
    M5.Speaker.begin();
    M5.Speaker.setVolume(128); // Ensure audible volume
    
    auto p = patternFor(kind);
    for (int i = 0; i < p.repeat; i++) {
        M5.Speaker.tone(p.freq, p.ms);
        delay(p.ms + p.gapMs);
    }
    
    M5.Speaker.end();
    M5.Mic.begin();
}

void AudioManager::playPendingBeeps() {
    if (!_pendingStop && !_pendingPermission && !_pendingFailure && !_pendingStart) return;

    // Switch to speaker
    M5.Mic.end();
    M5.Speaker.begin();
    M5.Speaker.setVolume(128);

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
