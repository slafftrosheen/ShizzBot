// ============================================================
// ShizzBot - RGB Effects Engine v1
// Non-blocking motor LED pattern engine using RollerCAN
// ============================================================
#pragma once
#include <Arduino.h>
#include "roller_can.h"

enum RGBPattern {
    RGB_OFF = 0,
    RGB_RAINBOW,
    RGB_POLICE,
    RGB_HEARTBEAT,
    RGB_DISCO,
    RGB_BREATHING,
    RGB_FIRE
};

class RGBEngine {
public:
    RGBPattern currentPattern = RGB_OFF;
    bool autoSync = true;  // Auto-sync to emotions when no manual pattern
    int patternsUsed = 0;  // Bitmask for achievement tracking

    void setPattern(RGBPattern p) {
        currentPattern = p;
        patternsUsed |= (1 << (int)p);
        _lastTick = 0;
        _phase = 0;
        if (p == RGB_OFF) autoSync = true;
        else autoSync = false;
        Serial.printf("[RGB] Pattern: %d\n", (int)p);
    }

    // Count unique patterns used (for achievement)
    int countPatternsUsed() {
        int count = 0;
        for (int i = 0; i < 7; i++) {
            if (patternsUsed & (1 << i)) count++;
        }
        return count;
    }

    void update(RollerCAN& motor, bool motorOk) {
        if (!motorOk) return;
        unsigned long now = millis();

        if (currentPattern == RGB_OFF) return;

        // Update at 30Hz
        if (now - _lastTick < 33) return;
        _lastTick = now;
        _phase++;

        uint8_t r = 0, g = 0, b = 0, bri = 40;

        switch (currentPattern) {
            case RGB_RAINBOW:
                hsvToRgb((_phase * 3) % 360, 255, 200, r, g, b);
                bri = 50;
                break;

            case RGB_POLICE: {
                int cycle = (_phase / 5) % 4;
                if (cycle == 0)      { r = 255; g = 0;   b = 0;   bri = 80; }
                else if (cycle == 1) { r = 0;   g = 0;   b = 0;   bri = 0;  }
                else if (cycle == 2) { r = 0;   g = 0;   b = 255; bri = 80; }
                else                 { r = 0;   g = 0;   b = 0;   bri = 0;  }
                break;
            }

            case RGB_HEARTBEAT: {
                // Double-pulse heartbeat
                int t = _phase % 60;
                if (t < 5)       bri = t * 16;
                else if (t < 10) bri = (10 - t) * 16;
                else if (t < 15) bri = (t - 10) * 16;
                else if (t < 20) bri = (20 - t) * 16;
                else             bri = 0;
                r = 255; g = 30; b = 60;
                break;
            }

            case RGB_DISCO: {
                // Random color flashes
                if (_phase % 6 == 0) {
                    _discR = random(256);
                    _discG = random(256);
                    _discB = random(256);
                }
                r = _discR; g = _discG; b = _discB;
                bri = (_phase % 6 < 3) ? 80 : 20;
                break;
            }

            case RGB_BREATHING: {
                // Smooth sine wave breathing
                float breath = (sin(_phase * 0.05f) + 1.0f) * 0.5f;
                bri = (uint8_t)(breath * 60);
                r = 0; g = 240; b = 255; // cyan breathing
                break;
            }

            case RGB_FIRE: {
                // Flickering warm tones
                r = 200 + random(55);
                g = 50 + random(80);
                b = 0;
                bri = 30 + random(40);
                break;
            }

            default:
                break;
        }

        motor.setRGB(r, g, b, bri);
    }

    int _lastEmotion = -1;
    unsigned long _lastSyncTick = 0;

    // Auto-sync: map an emotion to a solid color
    void syncToEmotion(RollerCAN& motor, bool motorOk, int emotion) {
        if (!motorOk || !autoSync) return;
        unsigned long now = millis();
        // Update only if emotion changed, or every 50ms for animations (like PARTY)
        if (emotion == _lastEmotion && (now - _lastSyncTick < 50)) return;
        _lastSyncTick = now;
        _lastEmotion = emotion;

        uint8_t r = 0, g = 60, b = 20; // default green
        uint8_t bri = 20;

        // Emotion enum: IDLE=0, MOVING=1, ERROR=2, SLEEPY=3, HAPPY=4,
        // ANGRY=5, DIZZY=6, SURPRISED=7, LOVE=8, SILLY=9, COOL=10,
        // CRYING=11, NINJA=12, SHOCKED=13, WINK=14, BORED=15, PARTY=16, ROBOT_FACE=17
        switch (emotion) {
            case 0:  r=0;   g=60;  b=20;  bri=15; break; // IDLE: dim green
            case 1:  r=0;   g=200; b=255; bri=30; break; // MOVING: cyan
            case 2:  r=255; g=0;   b=0;   bri=60; break; // ERROR: red
            case 3:  r=40;  g=20;  b=80;  bri=10; break; // SLEEPY: dim purple
            case 4:  r=0;   g=255; b=100; bri=40; break; // HAPPY: green
            case 5:  r=255; g=20;  b=0;   bri=50; break; // ANGRY: red
            case 6:  r=255; g=170; b=0;   bri=40; break; // DIZZY: orange
            case 7:  r=255; g=255; b=0;   bri=50; break; // SURPRISED: yellow
            case 8:  r=255; g=50;  b=100; bri=40; break; // LOVE: pink
            case 9:  r=255; g=200; b=0;   bri=40; break; // SILLY: yellow
            case 10: r=100; g=100; b=255; bri=30; break; // COOL: blue
            case 11: r=0;   g=150; b=255; bri=30; break; // CRYING: blue
            case 12: r=50;  g=50;  b=50;  bri=10; break; // NINJA: dim grey
            case 16: // PARTY: rainbow cycle
                hsvToRgb((millis() / 5) % 360, 255, 200, r, g, b);
                bri = 60;
                break;
        }

        motor.setRGB(r, g, b, bri);
    }

private:
    unsigned long _lastTick = 0;
    uint16_t _phase = 0;
    uint8_t _discR = 0, _discG = 0, _discB = 0;

    // Simple HSV to RGB conversion
    static void hsvToRgb(int h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
        h = h % 360;
        int region = h / 60;
        int remainder = (h - (region * 60)) * 255 / 60;
        uint8_t p = (v * (255 - s)) >> 8;
        uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
        uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

        switch (region) {
            case 0: r=v; g=t; b=p; break;
            case 1: r=q; g=v; b=p; break;
            case 2: r=p; g=v; b=t; break;
            case 3: r=p; g=q; b=v; break;
            case 4: r=t; g=p; b=v; break;
            default: r=v; g=p; b=q; break;
        }
    }
};
