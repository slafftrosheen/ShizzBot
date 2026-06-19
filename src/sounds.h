// ============================================================
// ShizzBot - Sound Effects Engine
// Uses M5Unified Speaker with Non-Blocking State Machine
// ============================================================
#pragma once
#include <M5Unified.h>

struct ToneDef {
    uint32_t freq;
    uint32_t duration;
};

class SoundEngine {
public:
    void init() {
        M5.Speaker.begin();
        M5.Speaker.setVolume(128); // 0-255
    }

    void playSequence(const ToneDef* seq, size_t len) {
        _currentSeq = seq;
        _seqLen = len;
        _seqIdx = 0;
        _nextActionMs = 0;
    }

    void playStartup() {
        static const ToneDef seq[] = {
            {440, 100}, {0, 50},
            {554, 100}, {0, 50},
            {659, 100}, {0, 50},
            {880, 200}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playHorn() {
        static const ToneDef seq[] = {
            {1000, 300}, {0, 100},
            {1000, 300}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playLowBattery() {
        static const ToneDef seq[] = {
            {400, 200}, {0, 100},
            {300, 200}, {0, 100},
            {200, 400}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playError() {
        static const ToneDef seq[] = {
            {150, 500}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void update() {
        if (!_currentSeq || _seqIdx >= _seqLen) return;
        
        unsigned long now = millis();
        if (now >= _nextActionMs) {
            ToneDef t = _currentSeq[_seqIdx++];
            if (t.freq > 0) {
                M5.Speaker.tone(t.freq, t.duration);
            }
            _nextActionMs = now + t.duration;
        }
    }

private:
    const ToneDef* _currentSeq = nullptr;
    size_t _seqLen = 0;
    size_t _seqIdx = 0;
    unsigned long _nextActionMs = 0;
};

extern SoundEngine soundEngine;
