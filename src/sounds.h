// ============================================================
// ShizzBot - Sound Effects Engine v2
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

    void playChirp() {
        // Generate random cute robot chirps on the fly
        _isDynamic = true;
        _dynamicChirpsLeft = 4 + random(4); // 4 to 7 chirps
        _nextActionMs = 0;
    }

    void playScan() {
        static const ToneDef seq[] = {
            {1500, 50}, {0, 50},
            {1600, 50}, {0, 50},
            {1700, 50}, {0, 50},
            {1800, 50}, {0, 100},
            {2000, 100}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    // Single beep for continuous reverse playing
    void playReverseBeep() {
        // Only trigger if not already playing a sequence
        if (!_currentSeq && !_isDynamic) {
            static const ToneDef seq[] = {
                {800, 150}, {0, 350} // 150ms on, 350ms off (2Hz beep)
            };
            playSequence(seq, sizeof(seq)/sizeof(ToneDef));
        }
    }

    void update() {
        unsigned long now = millis();

        if (_isDynamic) {
            if (now >= _nextActionMs) {
                if (_dynamicChirpsLeft > 0) {
                    uint32_t f = 1000 + random(2000);
                    uint32_t d = 30 + random(60);
                    M5.Speaker.tone(f, d);
                    _nextActionMs = now + d + 20 + random(40);
                    _dynamicChirpsLeft--;
                } else {
                    _isDynamic = false;
                }
            }
            return;
        }

        if (!_currentSeq || _seqIdx >= _seqLen) {
            _currentSeq = nullptr; // Free up channel
            return;
        }
        
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

    bool _isDynamic = false;
    int _dynamicChirpsLeft = 0;
};

extern SoundEngine soundEngine;
