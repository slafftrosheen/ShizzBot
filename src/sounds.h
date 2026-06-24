// ============================================================
// ShizzBot - Sound Effects Engine v3 (Kids World Mega)
// Non-Blocking State Machine + Musical Note System
// ============================================================
#pragma once
#include <M5Unified.h>

struct ToneDef {
    uint32_t freq;
    uint32_t duration;
};

class SoundEngine {
public:
    // Musical note frequencies (C4 to B4 chromatic scale)
    static constexpr uint16_t NOTES[12] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};

    void init() {
        M5.Speaker.begin();
        M5.Speaker.setVolume(128);
    }

    void playSequence(const ToneDef* seq, size_t len) {
        _currentSeq = seq;
        _seqLen = len;
        _seqIdx = 0;
        _nextActionMs = 0;
        _isDynamic = false;
    }

    // ===== ORIGINAL SOUNDS =====
    void playStartup() {
        static const ToneDef seq[] = {
            {440, 100}, {0, 50}, {554, 100}, {0, 50},
            {659, 100}, {0, 50}, {880, 200}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playHorn() {
        static const ToneDef seq[] = {
            {1000, 300}, {0, 100}, {1000, 300}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playLowBattery() {
        static const ToneDef seq[] = {
            {400, 200}, {0, 100}, {300, 200}, {0, 100}, {200, 400}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playError() {
        static const ToneDef seq[] = {{150, 500}, {0, 100}};
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playFart() {
        static const ToneDef seq[] = {
            {120, 100}, {100, 100}, {80, 100}, {60, 200}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playGiggle() {
        static const ToneDef seq[] = {
            {2000, 40}, {0, 30}, {2200, 40}, {0, 30},
            {2400, 40}, {0, 30}, {2000, 40}, {0, 30},
            {2500, 40}, {0, 30}, {2600, 80}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playMagic() {
        static const ToneDef seq[] = {
            {800, 40}, {1000, 40}, {1200, 40}, {1400, 40},
            {1600, 40}, {1800, 40}, {2000, 40}, {2400, 100}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playPewPew() {
        static const ToneDef seq[] = {
            {3000, 20}, {2000, 20}, {1000, 20}, {500, 20}, {0, 50},
            {3000, 20}, {2000, 20}, {1000, 20}, {500, 20}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    // ===== NEW KIDS WORLD SOUNDS =====
    void playBurp() {
        static const ToneDef seq[] = {
            {200, 80}, {150, 80}, {100, 120}, {80, 150},
            {60, 200}, {40, 100}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playWhoopee() {
        static const ToneDef seq[] = {
            {300, 60}, {250, 60}, {200, 60}, {150, 60},
            {120, 100}, {100, 100}, {80, 150}, {60, 200},
            {40, 300}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playAlienTalk() {
        static const ToneDef seq[] = {
            {1800, 30}, {600, 30}, {2200, 30}, {400, 30},
            {1500, 30}, {800, 30}, {2500, 30}, {500, 30},
            {1900, 40}, {700, 40}, {2100, 60}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playDrumRoll() {
        static const ToneDef seq[] = {
            {200, 30}, {0, 20}, {200, 30}, {0, 20},
            {200, 30}, {0, 15}, {200, 25}, {0, 15},
            {200, 25}, {0, 10}, {200, 20}, {0, 10},
            {200, 20}, {0, 8}, {200, 15}, {0, 8},
            {200, 15}, {0, 5}, {200, 10}, {0, 5},
            {300, 200}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playTaDa() {
        static const ToneDef seq[] = {
            {523, 150}, {0, 50}, {659, 150}, {0, 50},
            {784, 150}, {0, 50}, {1047, 400}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playBoing() {
        static const ToneDef seq[] = {
            {200, 30}, {400, 30}, {600, 30}, {800, 30},
            {1200, 40}, {1000, 30}, {800, 30}, {600, 30},
            {800, 30}, {1000, 40}, {0, 50}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    // ===== MUSICAL NOTE SYSTEM =====
    void playNote(uint8_t noteIdx) {
        if (noteIdx >= 12) return;
        M5.Speaker.tone(NOTES[noteIdx], 200);
    }

    // ===== DYNAMIC SOUNDS =====
    void playChirp() {
        _isDynamic = true;
        _dynamicChirpsLeft = 4 + random(4);
        _nextActionMs = 0;
    }

    void playScan() {
        static const ToneDef seq[] = {
            {1500, 50}, {0, 50}, {1600, 50}, {0, 50},
            {1700, 50}, {0, 50}, {1800, 50}, {0, 100},
            {2000, 100}, {0, 100}
        };
        playSequence(seq, sizeof(seq)/sizeof(ToneDef));
    }

    void playReverseBeep() {
        if (!_currentSeq && !_isDynamic) {
            static const ToneDef seq[] = {{800, 150}, {0, 350}};
            playSequence(seq, sizeof(seq)/sizeof(ToneDef));
        }
    }

    // ===== SOUND DISPATCH BY INDEX =====
    // Used by story_mode.h: 0=chirp, 1=horn, 2=magic, 3=pewpew, 4=giggle,
    // 5=fart, 6=tada, 7=scan, 8=error, 9=boing, 10=drumroll, 11=alien
    void playSoundByIndex(int idx) {
        switch (idx) {
            case 0:  playChirp(); break;
            case 1:  playHorn(); break;
            case 2:  playMagic(); break;
            case 3:  playPewPew(); break;
            case 4:  playGiggle(); break;
            case 5:  playFart(); break;
            case 6:  playTaDa(); break;
            case 7:  playScan(); break;
            case 8:  playError(); break;
            case 9:  playBoing(); break;
            case 10: playDrumRoll(); break;
            case 11: playAlienTalk(); break;
        }
    }

    // ===== STATE MACHINE UPDATE =====
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
            _currentSeq = nullptr;
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

// Static member definition
constexpr uint16_t SoundEngine::NOTES[12];

extern SoundEngine soundEngine;
