#pragma once

#include <M5Unified.h>
#include "Config.h"

enum BeepKind {
    BEEP_STOP,
    BEEP_PERMISSION,
    BEEP_FAILURE,
    BEEP_START,
};

struct BeepPattern {
    uint16_t freq;
    uint16_t ms;
    uint8_t repeat;
    uint16_t gapMs;
};

class AudioManager {
public:
    void begin();
    void update();
    void queueBeep(BeepKind kind);
    bool isRecording() const { return _recording; }
    
    // Returns true if a full chunk is ready in buf
    bool recordOneChunk(int16_t* buf, size_t samples);
    
    void startRecording();
    void stopRecording();

    // 查询待处理蜂鸣计数（用于测试）
    uint8_t pendingBeeps(BeepKind kind) const;

private:
    BeepPattern patternFor(BeepKind k);
    void playPendingBeeps();

    bool _recording = false;
    uint32_t _recordStartMs = 0;
    
    // Pending beeps are queued while recording and played after stop.
    uint8_t _pendingStop = 0;
    uint8_t _pendingPermission = 0;
    uint8_t _pendingFailure = 0;
    uint8_t _pendingStart = 0;
};

extern AudioManager AudioMgr;
