// ============================================================
// ShizzBot - Robot Face Engine v3 (Swarm)
// Animated face for the M5StickS3 LCD using M5Canvas (Sprite)
// Features bouncy movement, mouths, expanded emotions,
// robot identity (name + color tinting), and heading indicator.
// ============================================================
#pragma once
#include <M5Unified.h>
#include <math.h>

class RobotFace {
public:
    enum Emotion { IDLE, MOVING, ERROR, SLEEPY, HAPPY, ANGRY, DIZZY, SURPRISED };

    // Robot identity
    // 0=cyan, 1=magenta, 2=lime, 3=orange
    int robotColor = 0;
    String robotName = "ShizzBot";
    float heading = 0; // 0-360 from IMU, displayed as mini compass

    uint32_t getAccentColor() {
        switch(robotColor) {
            case 1: return M5.Display.color565(255, 0, 170);  // magenta
            case 2: return M5.Display.color565(0, 255, 100);  // lime
            case 3: return M5.Display.color565(255, 170, 0);  // orange
            default: return TFT_CYAN;
        }
    }

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
            if (e == SURPRISED) _emotionTimer = millis() + 2000;
        }
    }

    Emotion getEmotion() const {
        return _currentEmotion;
    }

    // Pass in the actual rpm commands to determine face wobble
    void update(int32_t speedL, int32_t speedR) {
        unsigned long now = millis();

        // Speed logic
        int32_t avgSpeed = (speedL + speedR) / 2;
        int32_t turnSpeed = speedL - speedR;

        // Auto-change emotion based on speed, unless in a sticky emotion
        bool sticky = (_currentEmotion == ERROR || _currentEmotion == SLEEPY || 
                       _currentEmotion == HAPPY || _currentEmotion == ANGRY || 
                       _currentEmotion == SURPRISED || _currentEmotion == DIZZY);

        if (now > _emotionTimer && sticky && _currentEmotion == SURPRISED) {
            sticky = false; // surprised expires after 2s
        }

        // Extreme spinning = DIZZY
        if (abs(turnSpeed) > 100 && !sticky) {
            setEmotion(DIZZY);
            _emotionTimer = now + 3000; // stay dizzy for 3s
            sticky = true;
        }
        else if (now > _emotionTimer && _currentEmotion == DIZZY) {
            sticky = false;
        }

        if (!sticky) {
            if (abs(avgSpeed) > 10 || abs(turnSpeed) > 10) {
                setEmotion(MOVING);
            } else {
                setEmotion(IDLE);
            }
        }

        // Calculate dynamic eye offsets based on movement
        // Multiply by higher factor to make it visible!
        int targetX = turnSpeed / 3; 
        int targetY = -avgSpeed / 3; 

        // Smoothly interpolate eye position (low pass filter)
        _xOffset += (targetX - _xOffset) * 0.2f;
        _yOffset += (targetY - _yOffset) * 0.2f;

        // Bounce effect when moving (sine wave based on time and speed)
        if (abs(avgSpeed) > 10) {
            float bounceFreq = abs(avgSpeed) * 0.02f;
            _bounceY = sin(now * 0.005f * bounceFreq) * (abs(avgSpeed) / 10.0f);
        } else {
            _bounceY = 0;
        }

        // Draw at 30fps max
        if (now - _lastUpdate > 33) {
            _lastUpdate = now;
            drawFace(now);
        }
    }

private:
    Emotion _currentEmotion = IDLE;
    unsigned long _lastUpdate = 0;
    unsigned long _emotionTimer = 0;
    
    float _xOffset = 0;
    float _yOffset = 0;
    float _bounceY = 0;
    
    bool _isBlinking = false;
    unsigned long _nextBlink = 0;
    M5Canvas _canvas{&M5.Display};

    void drawFace(unsigned long now) {
        int w = _canvas.width();
        int h = _canvas.height();
        int eyeSpacing = w / 3;
        int cx = w / 2;
        int cy = h / 2 - 10; // Eyes slightly above center to leave room for mouth
        
        int eyeLeftX = cx - eyeSpacing / 2;
        int eyeRightX = cx + eyeSpacing / 2;

        _canvas.fillScreen(TFT_BLACK);

        // Blinking logic
        if (_currentEmotion == IDLE || _currentEmotion == HAPPY) {
            if (now > _nextBlink) {
                _isBlinking = !_isBlinking;
                if (_isBlinking) {
                    _nextBlink = now + 150; 
                } else {
                    _nextBlink = now + 2000 + random(3000); 
                }
            }
        } else {
            _isBlinking = false;
        }

        uint32_t accent = getAccentColor();
        uint32_t eyeColor = accent;
        int xo = constrain(_xOffset, -25, 25);
        int yo = constrain(_yOffset + _bounceY, -25, 25);

        // Draw Eyes
        if (_currentEmotion == ERROR) {
            eyeColor = TFT_RED;
            _canvas.drawLine(eyeLeftX - 12, cy - 12, eyeLeftX + 12, cy + 12, eyeColor);
            _canvas.drawLine(eyeLeftX - 12, cy + 12, eyeLeftX + 12, cy - 12, eyeColor);
            _canvas.drawLine(eyeRightX - 12, cy - 12, eyeRightX + 12, cy + 12, eyeColor);
            _canvas.drawLine(eyeRightX - 12, cy + 12, eyeRightX + 12, cy - 12, eyeColor);
        } 
        else if (_currentEmotion == SLEEPY) {
            _canvas.fillRect(eyeLeftX - 15, cy, 30, 10, eyeColor);
            _canvas.fillRect(eyeRightX - 15, cy, 30, 10, eyeColor);
        }
        else if (_currentEmotion == ANGRY) {
            eyeColor = TFT_RED;
            _canvas.fillCircle(eyeLeftX + xo, cy + yo, 15, eyeColor);
            _canvas.fillCircle(eyeRightX + xo, cy + yo, 15, eyeColor);
            // Angry brows (angled lines cutting into the circle)
            _canvas.fillTriangle(eyeLeftX-20, cy-20, eyeLeftX+20, cy-5, eyeLeftX-20, cy-5, TFT_BLACK);
            _canvas.fillTriangle(eyeRightX-20, cy-5, eyeRightX+20, cy-20, eyeRightX+20, cy-5, TFT_BLACK);
            _canvas.fillCircle(eyeLeftX + xo + xo/2, cy + yo + yo/2, 5, TFT_BLACK);
            _canvas.fillCircle(eyeRightX + xo + xo/2, cy + yo + yo/2, 5, TFT_BLACK);
        }
        else if (_currentEmotion == HAPPY) {
            eyeColor = TFT_GREEN;
            if (_isBlinking) {
                _canvas.fillRect(eyeLeftX - 15, cy - 2, 30, 4, eyeColor);
                _canvas.fillRect(eyeRightX - 15, cy - 2, 30, 4, eyeColor);
            } else {
                // Happy arc eyes (half circles)
                _canvas.fillCircle(eyeLeftX + xo, cy + yo, 16, eyeColor);
                _canvas.fillCircle(eyeRightX + xo, cy + yo, 16, eyeColor);
                _canvas.fillCircle(eyeLeftX + xo, cy + yo + 6, 16, TFT_BLACK); // cutout bottom
                _canvas.fillCircle(eyeRightX + xo, cy + yo + 6, 16, TFT_BLACK); 
            }
        }
        else if (_currentEmotion == SURPRISED) {
            eyeColor = TFT_YELLOW;
            _canvas.fillCircle(eyeLeftX, cy - 10, 20, eyeColor); // big eyes!
            _canvas.fillCircle(eyeRightX, cy - 10, 20, eyeColor);
            _canvas.fillCircle(eyeLeftX, cy - 10, 6, TFT_BLACK); // small pupils
            _canvas.fillCircle(eyeRightX, cy - 10, 6, TFT_BLACK);
        }
        else if (_currentEmotion == DIZZY) {
            eyeColor = TFT_ORANGE;
            float spinAngle = now * 0.01f;
            int dx = cos(spinAngle) * 10;
            int dy = sin(spinAngle) * 10;
            _canvas.drawCircle(eyeLeftX, cy, 15, eyeColor);
            _canvas.drawCircle(eyeRightX, cy, 15, eyeColor);
            _canvas.fillCircle(eyeLeftX + dx, cy + dy, 6, eyeColor);
            _canvas.fillCircle(eyeRightX + dx, cy + dy, 6, eyeColor);
        }
        else if (_isBlinking) {
            _canvas.fillRect(eyeLeftX - 15, cy - 2 + yo, 30, 4, eyeColor);
            _canvas.fillRect(eyeRightX - 15, cy - 2 + yo, 30, 4, eyeColor);
        }
        else {
            // Normal IDLE / MOVING
            _canvas.fillCircle(eyeLeftX + xo, cy + yo, 15, eyeColor);
            _canvas.fillCircle(eyeRightX + xo, cy + yo, 15, eyeColor);
            _canvas.fillCircle(eyeLeftX + xo + xo/2, cy + yo + yo/2, 5, TFT_BLACK);
            _canvas.fillCircle(eyeRightX + xo + xo/2, cy + yo + yo/2, 5, TFT_BLACK);
        }

        // Draw Mouth
        int mouthY = cy + 40;
        if (_currentEmotion == IDLE) {
            _canvas.drawLine(cx - 10, mouthY, cx + 10, mouthY, accent);
        }
        else if (_currentEmotion == HAPPY || _currentEmotion == MOVING) {
            _canvas.fillCircle(cx + xo/2, mouthY + yo/2, 12, accent);
            _canvas.fillCircle(cx + xo/2, mouthY + yo/2 - 4, 14, TFT_BLACK);
        }
        else if (_currentEmotion == ANGRY) {
            _canvas.fillCircle(cx, mouthY + 10, 12, TFT_RED);
            _canvas.fillCircle(cx, mouthY + 14, 14, TFT_BLACK);
        }
        else if (_currentEmotion == SURPRISED) {
            _canvas.drawCircle(cx, mouthY + 5, 8, TFT_YELLOW);
        }
        else if (_currentEmotion == DIZZY) {
            _canvas.drawLine(cx - 15, mouthY, cx - 5, mouthY + 5, TFT_ORANGE);
            _canvas.drawLine(cx - 5, mouthY + 5, cx + 5, mouthY - 5, TFT_ORANGE);
            _canvas.drawLine(cx + 5, mouthY - 5, cx + 15, mouthY, TFT_ORANGE);
        }
        else if (_currentEmotion == ERROR) {
            _canvas.drawLine(cx - 10, mouthY, cx + 10, mouthY, TFT_RED);
        }

        // Draw robot name at bottom
        _canvas.setTextSize(1);
        _canvas.setTextColor(accent);
        int nameW = robotName.length() * 6; // approx 6px per char at size 1
        _canvas.setCursor((w - nameW) / 2, h - 12);
        _canvas.print(robotName);

        // Mini heading indicator (top-right corner)
        int compassX = w - 14;
        int compassY = 14;
        _canvas.drawCircle(compassX, compassY, 10, accent);
        float headRad = heading * 0.01745f; // deg to rad
        int nx = compassX + sin(headRad) * 8;
        int ny = compassY - cos(headRad) * 8;
        _canvas.drawLine(compassX, compassY, nx, ny, accent);

        _canvas.pushSprite(0, 0);
    }
};

extern RobotFace robotFace;
