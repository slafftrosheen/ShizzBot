// ============================================================
// ShizzBot - Robot Face Engine
// Animated face for the M5StickS3 LCD using M5Canvas (Sprite)
// ============================================================
#pragma once
#include <M5Unified.h>

class RobotFace {
public:
    enum Emotion { IDLE, MOVING, ERROR, SLEEPY };

    void init() {
        _canvas.createSprite(M5.Display.width(), M5.Display.height());
        _canvas.fillScreen(TFT_BLACK);
        _canvas.pushSprite(0, 0);
        setEmotion(IDLE);
    }

    void setEmotion(Emotion e) {
        if (_currentEmotion != e) {
            _currentEmotion = e;
            _lastUpdate = 0; // force redraw
        }
    }

    void update(int32_t speedL, int32_t speedR) {
        unsigned long now = millis();

        // Auto-change emotion based on speed
        if (speedL != 0 || speedR != 0) {
            setEmotion(MOVING);
            _xOffset = (speedL - speedR) / 10; // look left/right based on turn
            _yOffset = -(speedL + speedR) / 10; // look up/down based on fwd/rev
        } else {
            if (_currentEmotion == MOVING) {
                setEmotion(IDLE);
            }
            _xOffset = 0;
            _yOffset = 0;
        }


        if (now - _lastUpdate > 50) {
            _lastUpdate = now;
            drawFace();
        }
    }

private:
    Emotion _currentEmotion = IDLE;
    unsigned long _lastUpdate = 0;
    int _xOffset = 0;
    int _yOffset = 0;
    bool _isBlinking = false;
    unsigned long _nextBlink = 0;
    M5Canvas _canvas{&M5.Display};

    void drawFace() {
        int w = _canvas.width();
        int h = _canvas.height();
        int eyeSpacing = w / 3;
        int cx = w / 2;
        int cy = h / 2;
        
        int eyeLeftX = cx - eyeSpacing / 2;
        int eyeRightX = cx + eyeSpacing / 2;

        _canvas.fillScreen(TFT_BLACK);

        unsigned long now = millis();

        // Handle blinking logic
        if (_currentEmotion == IDLE) {
            if (now > _nextBlink) {
                _isBlinking = !_isBlinking;
                if (_isBlinking) {
                    _nextBlink = now + 150; // blink duration
                } else {
                    _nextBlink = now + 2000 + random(3000); // time until next blink
                }
            }
        } else {
            _isBlinking = false;
        }

        uint32_t eyeColor = TFT_CYAN;

        if (_currentEmotion == ERROR) {
            eyeColor = TFT_RED;
            // Draw X eyes
            _canvas.drawLine(eyeLeftX - 10, cy - 10, eyeLeftX + 10, cy + 10, eyeColor);
            _canvas.drawLine(eyeLeftX - 10, cy + 10, eyeLeftX + 10, cy - 10, eyeColor);
            _canvas.drawLine(eyeRightX - 10, cy - 10, eyeRightX + 10, cy + 10, eyeColor);
            _canvas.drawLine(eyeRightX - 10, cy + 10, eyeRightX + 10, cy - 10, eyeColor);
        } 
        else if (_currentEmotion == SLEEPY) {
            // Half-closed eyes
            _canvas.fillRect(eyeLeftX - 15, cy, 30, 10, eyeColor);
            _canvas.fillRect(eyeRightX - 15, cy, 30, 10, eyeColor);
        }
        else if (_isBlinking) {
            // Closed eyes (lines)
            _canvas.fillRect(eyeLeftX - 15, cy - 2, 30, 4, eyeColor);
            _canvas.fillRect(eyeRightX - 15, cy - 2, 30, 4, eyeColor);
        }
        else {
            // Normal open eyes (circles)
            // Limit offsets so pupils don't leave the screen completely
            int xo = constrain(_xOffset, -20, 20);
            int yo = constrain(_yOffset, -20, 20);
            
            _canvas.fillCircle(eyeLeftX + xo, cy + yo, 15, eyeColor);
            _canvas.fillCircle(eyeRightX + xo, cy + yo, 15, eyeColor);
            // Draw pupil
            _canvas.fillCircle(eyeLeftX + xo + xo/2, cy + yo + yo/2, 5, TFT_BLACK);
            _canvas.fillCircle(eyeRightX + xo + xo/2, cy + yo + yo/2, 5, TFT_BLACK);
        }

        _canvas.pushSprite(0, 0);
    }
};

extern RobotFace robotFace;
