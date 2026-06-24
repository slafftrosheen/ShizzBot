// ============================================================
// ShizzBot - Robot Face Engine v4 (Kids World Mega)
// Animated face with 18 emotions, emoji overlays, speech bubbles,
// face customizer (eye shape, mouth style, accessory).
// ============================================================
#pragma once
#include <M5Unified.h>
#include <math.h>

class RobotFace {
public:
    enum Emotion {
        IDLE, MOVING, ERROR, SLEEPY, HAPPY, ANGRY, DIZZY, SURPRISED,
        LOVE, SILLY, COOL, CRYING, NINJA,
        SHOCKED, WINK, BORED, PARTY, ROBOT_FACE
    };
    static const int EMOTION_COUNT = 18;

    // Robot identity
    int robotColor = 0;  // 0=cyan, 1=magenta, 2=lime, 3=orange
    String robotName = "ShizzBot";
    float heading = 0;   // 0-360 from IMU

    // Customization
    int faceType = 0;      // 0=Boy, 1=Girl
    int eyeRadius = 15;    // 10 to 25
    int blinkRate = 50;    // 0 to 100
    int bounceFactor = 50; // 0 to 100
    int eyeShape = 0;      // 0=round, 1=square, 2=star
    int mouthStyle = 0;    // 0=default, 1=zigzag, 2=fangs
    int accessory = 0;     // 0=none, 1=hat, 2=bow, 3=antenna

    // Overlay system
    int overlayEmoji = 0;   // 0=none, 1=star, 2=lightning, 3=fire, 4=poop, 5=rainbow

    // Speech bubble
    String speechText = "";

    // HUD Telemetry
    String ipAddress = "0.0.0.0";
    String wifiName = "Offline";
    int batteryPct = 100;
    int scoreDisplay = 0;
    int tamagotchiLvl = 0;

    uint32_t getAccentColor() {
        switch(robotColor) {
            case 1: return M5.Display.color565(255, 0, 170);
            case 2: return M5.Display.color565(0, 255, 100);
            case 3: return M5.Display.color565(255, 170, 0);
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
            _lastUpdate = 0;
            // Temporary emotions auto-expire
            if (e == SURPRISED || e == LOVE || e == SILLY || e == COOL ||
                e == CRYING || e == NINJA || e == SHOCKED || e == WINK ||
                e == BORED || e == PARTY || e == ROBOT_FACE) {
                _emotionTimer = millis() + 3000;
            }
        }
    }

    Emotion getEmotion() const { return _currentEmotion; }

    void showOverlay(int emoji) {
        overlayEmoji = emoji;
        _overlayTimer = millis() + 2000;
    }

    void showSpeech(const String& text) {
        speechText = text;
        _speechTimer = millis() + 4000;
    }

    void update(int32_t speedL, int32_t speedR) {
        unsigned long now = millis();

        // Clear expired overlays/speech
        if (overlayEmoji && now > _overlayTimer) overlayEmoji = 0;
        if (speechText.length() > 0 && now > _speechTimer) speechText = "";

        int32_t avgSpeed = (speedL + speedR) / 2;
        int32_t turnSpeed = speedL - speedR;

        // Sticky emotion logic
        bool sticky = (_currentEmotion >= ERROR); // ERROR and above are all sticky
        if (now > _emotionTimer && sticky &&
            _currentEmotion != ERROR && _currentEmotion != SLEEPY) {
            sticky = false;
        }

        // Extreme spinning = DIZZY
        if (abs(turnSpeed) > 100 && !sticky) {
            setEmotion(DIZZY);
            _emotionTimer = now + 3000;
            sticky = true;
        } else if (now > _emotionTimer && _currentEmotion == DIZZY) {
            sticky = false;
        }

        if (!sticky) {
            if (abs(avgSpeed) > 10 || abs(turnSpeed) > 10) {
                setEmotion(MOVING);
            } else {
                setEmotion(IDLE);
            }
        }

        int targetX = turnSpeed / 3;
        int targetY = -avgSpeed / 3;
        _xOffset += (targetX - _xOffset) * 0.2f;
        _yOffset += (targetY - _yOffset) * 0.2f;

        if (abs(avgSpeed) > 10) {
            float bounceFreq = abs(avgSpeed) * 0.02f;
            float bounceAmp = (bounceFactor / 50.0f);
            _bounceY = sin(now * 0.005f * bounceFreq) * (abs(avgSpeed) / 10.0f) * bounceAmp;
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
    unsigned long _overlayTimer = 0;
    unsigned long _speechTimer = 0;
    float _xOffset = 0, _yOffset = 0, _bounceY = 0;
    bool _isBlinking = false;
    unsigned long _nextBlink = 0;
    M5Canvas _canvas{&M5.Display};

    // ===== DRAWING HELPERS =====
    void drawStar(int cx, int cy, int r, uint32_t color) {
        // 5-pointed star using triangles
        for (int i = 0; i < 5; i++) {
            float a1 = (i * 72 - 90) * DEG_TO_RAD;
            float a2 = ((i + 2) * 72 - 90) * DEG_TO_RAD;
            int x1 = cx + cos(a1) * r;
            int y1 = cy + sin(a1) * r;
            int x2 = cx + cos(a2) * r;
            int y2 = cy + sin(a2) * r;
            _canvas.drawLine(x1, y1, x2, y2, color);
        }
    }

    void drawEye(int x, int y, int radius, uint32_t color) {
        switch (eyeShape) {
            case 1: // Square
                _canvas.fillRect(x - radius, y - radius, radius * 2, radius * 2, color);
                break;
            case 2: // Star
                drawStar(x, y, radius + 3, color);
                _canvas.fillCircle(x, y, radius / 2, color);
                break;
            default: // Round
                _canvas.fillCircle(x, y, radius, color);
                break;
        }
    }

    void drawMouth(int cx, int y, uint32_t color, bool isSmile) {
        switch (mouthStyle) {
            case 1: // Zigzag
                _canvas.drawLine(cx-15, y, cx-8, y+5, color);
                _canvas.drawLine(cx-8, y+5, cx, y-3, color);
                _canvas.drawLine(cx, y-3, cx+8, y+5, color);
                _canvas.drawLine(cx+8, y+5, cx+15, y, color);
                break;
            case 2: // Fangs
                if (isSmile) {
                    _canvas.fillCircle(cx, y, 12, color);
                    _canvas.fillCircle(cx, y - 4, 14, TFT_BLACK);
                }
                // Fangs
                _canvas.fillTriangle(cx-8, y-2, cx-4, y-2, cx-6, y+8, TFT_WHITE);
                _canvas.fillTriangle(cx+4, y-2, cx+8, y-2, cx+6, y+8, TFT_WHITE);
                break;
            default: // Normal
                if (isSmile) {
                    _canvas.fillCircle(cx, y, 12, color);
                    _canvas.fillCircle(cx, y - 4, 14, TFT_BLACK);
                } else {
                    _canvas.drawLine(cx - 10, y, cx + 10, y, color);
                }
                break;
        }
    }

    void drawAccessory(int cx, int w, uint32_t accent) {
        switch (accessory) {
            case 1: { // Top Hat
                _canvas.fillRect(cx - 15, 4, 30, 4, accent);
                _canvas.fillRect(cx - 10, 4 - 16, 20, 16, accent);
                break;
            }
            case 2: { // Bow
                uint32_t bowColor = M5.Display.color565(255, 100, 180);
                int bowY = 16;
                _canvas.fillTriangle(cx, bowY, cx - 12, bowY - 8, cx - 12, bowY + 8, bowColor);
                _canvas.fillTriangle(cx, bowY, cx + 12, bowY - 8, cx + 12, bowY + 8, bowColor);
                _canvas.fillCircle(cx, bowY, 3, TFT_WHITE);
                break;
            }
            case 3: { // Antenna
                _canvas.drawLine(cx, 0, cx, 18, accent);
                _canvas.fillCircle(cx, 0, 4, accent);
                _canvas.fillCircle(cx, 0, 2, TFT_WHITE);
                break;
            }
            default: {
                // Boy/Girl default accessories from faceType
                if (faceType == 1) { // Girl: bow
                    uint32_t bowColor = M5.Display.color565(255, 100, 180);
                    int bowY = 18;
                    _canvas.fillTriangle(cx, bowY, cx-12, bowY-8, cx-12, bowY+8, bowColor);
                    _canvas.fillTriangle(cx, bowY, cx+12, bowY-8, cx+12, bowY+8, bowColor);
                    _canvas.fillCircle(cx, bowY, 3, TFT_WHITE);
                } else { // Boy: cap
                    _canvas.fillRoundRect(cx - 30, 8, 60, 6, 3, accent);
                    _canvas.fillRoundRect(cx + 10, 10, 25, 10, 3, accent);
                }
                break;
            }
        }
    }

    void drawOverlay(int w, int h) {
        if (overlayEmoji == 0) return;
        int ox = w - 30, oy = h / 2 - 20;

        switch (overlayEmoji) {
            case 1: // Star ⭐
                drawStar(ox, oy, 12, TFT_YELLOW);
                break;
            case 2: // Lightning ⚡
                _canvas.fillTriangle(ox-2, oy-15, ox+8, oy, ox-2, oy, TFT_YELLOW);
                _canvas.fillTriangle(ox+2, oy, ox-8, oy+15, ox+2, oy, TFT_YELLOW);
                break;
            case 3: // Fire 🔥
                _canvas.fillCircle(ox, oy+5, 8, TFT_RED);
                _canvas.fillCircle(ox, oy, 6, TFT_ORANGE);
                _canvas.fillCircle(ox, oy-4, 4, TFT_YELLOW);
                break;
            case 4: // Poop 💩
                _canvas.fillCircle(ox, oy+4, 8, M5.Display.color565(139, 90, 43));
                _canvas.fillCircle(ox, oy-2, 6, M5.Display.color565(139, 90, 43));
                _canvas.fillCircle(ox, oy-7, 4, M5.Display.color565(139, 90, 43));
                _canvas.fillCircle(ox-3, oy+2, 2, TFT_WHITE); // eyes
                _canvas.fillCircle(ox+3, oy+2, 2, TFT_WHITE);
                break;
            case 5: // Rainbow 🌈
                for (int i = 0; i < 5; i++) {
                    uint32_t colors[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN};
                    _canvas.drawCircle(ox, oy + 10, 15 - i*2, colors[i]);
                }
                break;
        }
    }

    void drawSpeechBubble(int w) {
        if (speechText.length() == 0) return;
        int textW = speechText.length() * 6;
        int bubbleW = min(textW + 16, w - 10);
        int bubbleX = (w - bubbleW) / 2;
        int bubbleY = 2;

        _canvas.fillRoundRect(bubbleX, bubbleY, bubbleW, 18, 4, TFT_WHITE);
        // Speech pointer triangle
        _canvas.fillTriangle(w/2 - 4, 20, w/2 + 4, 20, w/2, 26, TFT_WHITE);
        _canvas.setTextColor(TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.setCursor(bubbleX + 8, bubbleY + 5);
        _canvas.print(speechText.substring(0, (bubbleW - 16) / 6)); // clip to fit
    }

    // ===== MAIN DRAW =====
    void drawFace(unsigned long now) {
        int w = _canvas.width();
        int h = _canvas.height();
        int eyeSpacing = w / 3;
        int cx = w / 2;
        int cy = h / 2 - 10;
        int eyeLeftX = cx - eyeSpacing / 2;
        int eyeRightX = cx + eyeSpacing / 2;

        _canvas.fillScreen(TFT_BLACK);

        // Blink logic
        if (_currentEmotion == IDLE || _currentEmotion == HAPPY || _currentEmotion == BORED) {
            if (now > _nextBlink) {
                _isBlinking = !_isBlinking;
                if (_isBlinking) {
                    _nextBlink = now + 150;
                } else {
                    int baseDelay = map(blinkRate, 0, 100, 5000, 500);
                    _nextBlink = now + baseDelay + random(baseDelay);
                }
            }
        } else {
            _isBlinking = false;
        }

        uint32_t accent = getAccentColor();
        uint32_t eyeColor = accent;
        int xo = constrain(_xOffset, -25, 25);
        int yo = constrain(_yOffset + _bounceY, -25, 25);

        // ===== DRAW EYES =====
        if (_currentEmotion == ERROR) {
            eyeColor = TFT_RED;
            _canvas.drawLine(eyeLeftX-12, cy-12, eyeLeftX+12, cy+12, eyeColor);
            _canvas.drawLine(eyeLeftX-12, cy+12, eyeLeftX+12, cy-12, eyeColor);
            _canvas.drawLine(eyeRightX-12, cy-12, eyeRightX+12, cy+12, eyeColor);
            _canvas.drawLine(eyeRightX-12, cy+12, eyeRightX+12, cy-12, eyeColor);
        }
        else if (_currentEmotion == SLEEPY) {
            _canvas.fillRect(eyeLeftX - eyeRadius, cy, eyeRadius*2, 10, eyeColor);
            _canvas.fillRect(eyeRightX - eyeRadius, cy, eyeRadius*2, 10, eyeColor);
        }
        else if (_currentEmotion == ANGRY) {
            eyeColor = TFT_RED;
            drawEye(eyeLeftX+xo, cy+yo, eyeRadius, eyeColor);
            drawEye(eyeRightX+xo, cy+yo, eyeRadius, eyeColor);
            _canvas.fillTriangle(eyeLeftX-20, cy-20, eyeLeftX+20, cy-5, eyeLeftX-20, cy-5, TFT_BLACK);
            _canvas.fillTriangle(eyeRightX-20, cy-5, eyeRightX+20, cy-20, eyeRightX+20, cy-5, TFT_BLACK);
            _canvas.fillCircle(eyeLeftX+xo+xo/2, cy+yo+yo/2, eyeRadius/3, TFT_BLACK);
            _canvas.fillCircle(eyeRightX+xo+xo/2, cy+yo+yo/2, eyeRadius/3, TFT_BLACK);
        }
        else if (_currentEmotion == HAPPY) {
            eyeColor = TFT_GREEN;
            if (_isBlinking) {
                _canvas.fillRect(eyeLeftX-eyeRadius, cy-2, eyeRadius*2, 4, eyeColor);
                _canvas.fillRect(eyeRightX-eyeRadius, cy-2, eyeRadius*2, 4, eyeColor);
            } else {
                _canvas.fillCircle(eyeLeftX+xo, cy+yo, eyeRadius+1, eyeColor);
                _canvas.fillCircle(eyeRightX+xo, cy+yo, eyeRadius+1, eyeColor);
                _canvas.fillCircle(eyeLeftX+xo, cy+yo+6, eyeRadius+1, TFT_BLACK);
                _canvas.fillCircle(eyeRightX+xo, cy+yo+6, eyeRadius+1, TFT_BLACK);
            }
        }
        else if (_currentEmotion == SURPRISED) {
            eyeColor = TFT_YELLOW;
            _canvas.fillCircle(eyeLeftX, cy-10, eyeRadius+5, eyeColor);
            _canvas.fillCircle(eyeRightX, cy-10, eyeRadius+5, eyeColor);
            _canvas.fillCircle(eyeLeftX, cy-10, 6, TFT_BLACK);
            _canvas.fillCircle(eyeRightX, cy-10, 6, TFT_BLACK);
        }
        else if (_currentEmotion == DIZZY) {
            eyeColor = TFT_ORANGE;
            float spinAngle = now * 0.01f;
            int dx = cos(spinAngle) * 10;
            int dy = sin(spinAngle) * 10;
            _canvas.drawCircle(eyeLeftX, cy, eyeRadius, eyeColor);
            _canvas.drawCircle(eyeRightX, cy, eyeRadius, eyeColor);
            _canvas.fillCircle(eyeLeftX+dx, cy+dy, 6, eyeColor);
            _canvas.fillCircle(eyeRightX+dx, cy+dy, 6, eyeColor);
        }
        else if (_currentEmotion == LOVE) {
            eyeColor = M5.Display.color565(255, 50, 100);
            _canvas.fillCircle(eyeLeftX-5+xo, cy-5+yo, eyeRadius/2+2, eyeColor);
            _canvas.fillCircle(eyeLeftX+5+xo, cy-5+yo, eyeRadius/2+2, eyeColor);
            _canvas.fillTriangle(eyeLeftX-12+xo, cy-2+yo, eyeLeftX+12+xo, cy-2+yo, eyeLeftX+xo, cy+12+yo, eyeColor);
            _canvas.fillCircle(eyeRightX-5+xo, cy-5+yo, eyeRadius/2+2, eyeColor);
            _canvas.fillCircle(eyeRightX+5+xo, cy-5+yo, eyeRadius/2+2, eyeColor);
            _canvas.fillTriangle(eyeRightX-12+xo, cy-2+yo, eyeRightX+12+xo, cy-2+yo, eyeRightX+xo, cy+12+yo, eyeColor);
        }
        else if (_currentEmotion == SILLY) {
            eyeColor = TFT_WHITE;
            _canvas.fillCircle(eyeLeftX+xo, cy+yo, eyeRadius+8, eyeColor);
            _canvas.fillCircle(eyeLeftX+xo, cy+yo, 5, TFT_BLACK);
            _canvas.fillCircle(eyeRightX+xo, cy+yo, eyeRadius-5, eyeColor);
            _canvas.fillCircle(eyeRightX+xo-4, cy+yo, 3, TFT_BLACK);
        }
        else if (_currentEmotion == COOL) {
            eyeColor = TFT_DARKGREY;
            _canvas.fillRect(eyeLeftX-25, cy-10, 50, 20, eyeColor);
            _canvas.fillRect(eyeRightX-25, cy-10, 50, 20, eyeColor);
            _canvas.fillRect(cx-15, cy-5, 30, 5, eyeColor);
            _canvas.drawLine(eyeLeftX-15, cy+5, eyeLeftX+5, cy-5, TFT_WHITE);
            _canvas.drawLine(eyeRightX-15, cy+5, eyeRightX+5, cy-5, TFT_WHITE);
        }
        else if (_currentEmotion == CRYING) {
            eyeColor = TFT_CYAN;
            _canvas.fillCircle(eyeLeftX+xo, cy+yo, eyeRadius, accent);
            _canvas.fillRect(eyeLeftX-20, cy+yo, 40, 20, TFT_BLACK);
            _canvas.drawLine(eyeLeftX-10, cy+yo, eyeLeftX+10, cy+yo-5, TFT_BLACK);
            _canvas.fillCircle(eyeRightX+xo, cy+yo, eyeRadius, accent);
            _canvas.fillRect(eyeRightX-20, cy+yo, 40, 20, TFT_BLACK);
            _canvas.drawLine(eyeRightX-10, cy+yo-5, eyeRightX+10, cy+yo, TFT_BLACK);
            int tearY = (millis()/20) % 30;
            _canvas.fillCircle(eyeLeftX, cy+10+tearY, 5, eyeColor);
            _canvas.fillTriangle(eyeLeftX-5, cy+10+tearY, eyeLeftX+5, cy+10+tearY, eyeLeftX, cy+5+tearY, eyeColor);
            _canvas.fillCircle(eyeRightX, cy+10+tearY, 5, eyeColor);
            _canvas.fillTriangle(eyeRightX-5, cy+10+tearY, eyeRightX+5, cy+10+tearY, eyeRightX, cy+5+tearY, eyeColor);
        }
        else if (_currentEmotion == NINJA) {
            _canvas.fillRect(0, cy-20, w, 40, TFT_DARKGREY);
            _canvas.fillRoundRect(eyeLeftX-eyeRadius-5, cy-eyeRadius+5, eyeRadius*2+10, 15, 5, TFT_BLACK);
            _canvas.fillRoundRect(eyeRightX-eyeRadius-5, cy-eyeRadius+5, eyeRadius*2+10, 15, 5, TFT_BLACK);
            _canvas.fillTriangle(eyeLeftX-10, cy, eyeLeftX+10, cy+5, eyeLeftX+10, cy-5, TFT_WHITE);
            _canvas.fillTriangle(eyeRightX+10, cy, eyeRightX-10, cy+5, eyeRightX-10, cy-5, TFT_WHITE);
        }
        else if (_currentEmotion == SHOCKED) {
            eyeColor = TFT_YELLOW;
            // Huge wide eyes with zigzag pupils
            _canvas.fillCircle(eyeLeftX, cy-8, eyeRadius+8, eyeColor);
            _canvas.fillCircle(eyeRightX, cy-8, eyeRadius+8, eyeColor);
            // Zigzag pupils
            _canvas.drawLine(eyeLeftX-4, cy-12, eyeLeftX, cy-8, TFT_BLACK);
            _canvas.drawLine(eyeLeftX, cy-8, eyeLeftX+4, cy-12, TFT_BLACK);
            _canvas.drawLine(eyeLeftX+4, cy-12, eyeLeftX+8, cy-8, TFT_BLACK);
            _canvas.drawLine(eyeRightX-4, cy-12, eyeRightX, cy-8, TFT_BLACK);
            _canvas.drawLine(eyeRightX, cy-8, eyeRightX+4, cy-12, TFT_BLACK);
            _canvas.drawLine(eyeRightX+4, cy-12, eyeRightX+8, cy-8, TFT_BLACK);
        }
        else if (_currentEmotion == WINK) {
            eyeColor = TFT_GREEN;
            // Left eye: normal open
            drawEye(eyeLeftX+xo, cy+yo, eyeRadius, eyeColor);
            _canvas.fillCircle(eyeLeftX+xo+xo/2, cy+yo+yo/2, eyeRadius/3, TFT_BLACK);
            // Right eye: closed (wink line)
            _canvas.drawLine(eyeRightX-eyeRadius, cy, eyeRightX+eyeRadius, cy, eyeColor);
            _canvas.drawLine(eyeRightX+eyeRadius-4, cy, eyeRightX+eyeRadius, cy-4, eyeColor);
        }
        else if (_currentEmotion == BORED) {
            eyeColor = accent;
            // Half-closed droopy eyes
            _canvas.fillCircle(eyeLeftX+xo, cy+yo+5, eyeRadius, eyeColor);
            _canvas.fillRect(eyeLeftX-eyeRadius-2, cy-eyeRadius+yo, eyeRadius*2+4, eyeRadius, TFT_BLACK);
            _canvas.fillCircle(eyeRightX+xo, cy+yo+5, eyeRadius, eyeColor);
            _canvas.fillRect(eyeRightX-eyeRadius-2, cy-eyeRadius+yo, eyeRadius*2+4, eyeRadius, TFT_BLACK);
            // Slow drift
            float drift = sin(now * 0.001f) * 5;
            _canvas.fillCircle(eyeLeftX+(int)drift, cy+yo+5, eyeRadius/3, TFT_BLACK);
            _canvas.fillCircle(eyeRightX+(int)drift, cy+yo+5, eyeRadius/3, TFT_BLACK);
        }
        else if (_currentEmotion == PARTY) {
            // Animated sparkle eyes — alternating colors
            uint32_t colors[] = {TFT_RED, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA};
            int ci = (now / 150) % 5;
            eyeColor = colors[ci];
            drawStar(eyeLeftX+xo, cy+yo, eyeRadius+4, eyeColor);
            _canvas.fillCircle(eyeLeftX+xo, cy+yo, eyeRadius/2, eyeColor);
            drawStar(eyeRightX+xo, cy+yo, eyeRadius+4, colors[(ci+2)%5]);
            _canvas.fillCircle(eyeRightX+xo, cy+yo, eyeRadius/2, colors[(ci+2)%5]);
        }
        else if (_currentEmotion == ROBOT_FACE) {
            eyeColor = TFT_GREEN;
            // Classic square pixel grid eyes
            int sz = eyeRadius - 2;
            _canvas.fillRect(eyeLeftX-sz, cy-sz, sz*2, sz*2, eyeColor);
            _canvas.fillRect(eyeRightX-sz, cy-sz, sz*2, sz*2, eyeColor);
            // Pixel grid lines
            for (int i = -sz; i <= sz; i += 6) {
                _canvas.drawLine(eyeLeftX-sz, cy+i, eyeLeftX+sz, cy+i, TFT_BLACK);
                _canvas.drawLine(eyeLeftX+i, cy-sz, eyeLeftX+i, cy+sz, TFT_BLACK);
                _canvas.drawLine(eyeRightX-sz, cy+i, eyeRightX+sz, cy+i, TFT_BLACK);
                _canvas.drawLine(eyeRightX+i, cy-sz, eyeRightX+i, cy+sz, TFT_BLACK);
            }
        }
        else if (_isBlinking) {
            _canvas.fillRect(eyeLeftX-eyeRadius, cy-2+yo, eyeRadius*2, 4, eyeColor);
            _canvas.fillRect(eyeRightX-eyeRadius, cy-2+yo, eyeRadius*2, 4, eyeColor);
        }
        else {
            // Normal IDLE / MOVING — use custom eye shape
            drawEye(eyeLeftX+xo, cy+yo, eyeRadius, eyeColor);
            drawEye(eyeRightX+xo, cy+yo, eyeRadius, eyeColor);
            _canvas.fillCircle(eyeLeftX+xo+xo/2, cy+yo+yo/2, eyeRadius/3, TFT_BLACK);
            _canvas.fillCircle(eyeRightX+xo+xo/2, cy+yo+yo/2, eyeRadius/3, TFT_BLACK);
        }

        // Eyelashes for Girl mode
        if (faceType == 1 && accessory == 0 && _currentEmotion != ERROR && !_isBlinking) {
            _canvas.drawLine(eyeLeftX+xo-eyeRadius+3, cy+yo-eyeRadius+5, eyeLeftX+xo-eyeRadius-6, cy+yo-eyeRadius-4, accent);
            _canvas.drawLine(eyeLeftX+xo-eyeRadius+8, cy+yo-eyeRadius+2, eyeLeftX+xo-eyeRadius-1, cy+yo-eyeRadius-7, accent);
            _canvas.drawLine(eyeRightX+xo+eyeRadius-3, cy+yo-eyeRadius+5, eyeRightX+xo+eyeRadius+6, cy+yo-eyeRadius-4, accent);
            _canvas.drawLine(eyeRightX+xo+eyeRadius-8, cy+yo-eyeRadius+2, eyeRightX+xo+eyeRadius+1, cy+yo-eyeRadius-7, accent);
        }

        // ===== DRAW ACCESSORY =====
        if (_currentEmotion != ERROR) {
            drawAccessory(cx, w, accent);
        }

        // ===== DRAW MOUTH =====
        int mouthY = cy + 40;
        if (_currentEmotion == IDLE || _currentEmotion == BORED) {
            drawMouth(cx, mouthY, accent, false);
        }
        else if (_currentEmotion == HAPPY || _currentEmotion == MOVING || _currentEmotion == WINK) {
            drawMouth(cx + xo/2, mouthY + yo/2, accent, true);
        }
        else if (_currentEmotion == ANGRY) {
            _canvas.fillCircle(cx, mouthY+10, 12, TFT_RED);
            _canvas.fillCircle(cx, mouthY+14, 14, TFT_BLACK);
        }
        else if (_currentEmotion == SURPRISED || _currentEmotion == SHOCKED) {
            _canvas.drawCircle(cx, mouthY+5, 8, TFT_YELLOW);
        }
        else if (_currentEmotion == SILLY) {
            _canvas.fillCircle(cx, mouthY, 12, accent);
            _canvas.fillCircle(cx, mouthY-4, 14, TFT_BLACK);
            _canvas.fillRoundRect(cx+2, mouthY+5, 12, 18, 6, M5.Display.color565(255,100,150));
            _canvas.drawLine(cx+8, mouthY+5, cx+8, mouthY+18, M5.Display.color565(200,50,100));
        }
        else if (_currentEmotion == COOL) {
            _canvas.drawLine(cx-15, mouthY, cx+15, mouthY, accent);
            _canvas.drawLine(cx+10, mouthY, cx+15, mouthY-5, accent);
        }
        else if (_currentEmotion == CRYING) {
            _canvas.fillCircle(cx, mouthY+10, 12, accent);
            _canvas.fillCircle(cx, mouthY+14, 14, TFT_BLACK);
        }
        else if (_currentEmotion == DIZZY) {
            _canvas.drawLine(cx-15, mouthY, cx-5, mouthY+5, TFT_ORANGE);
            _canvas.drawLine(cx-5, mouthY+5, cx+5, mouthY-5, TFT_ORANGE);
            _canvas.drawLine(cx+5, mouthY-5, cx+15, mouthY, TFT_ORANGE);
        }
        else if (_currentEmotion == ERROR) {
            _canvas.drawLine(cx-10, mouthY, cx+10, mouthY, TFT_RED);
        }
        else if (_currentEmotion == PARTY) {
            // Big excited smile
            _canvas.fillCircle(cx, mouthY, 14, accent);
            _canvas.fillCircle(cx, mouthY-5, 16, TFT_BLACK);
        }
        else if (_currentEmotion == ROBOT_FACE) {
            // Rectangular pixel mouth
            _canvas.fillRect(cx-12, mouthY-2, 24, 6, TFT_GREEN);
            for (int i = -12; i < 12; i += 6) {
                _canvas.drawLine(cx+i, mouthY-2, cx+i, mouthY+4, TFT_BLACK);
            }
        }

        // ===== OVERLAYS =====
        drawOverlay(w, h);
        drawSpeechBubble(w);

        // ===== HUD GRAPHICS =====
        _canvas.setTextSize(1);
        _canvas.setTextColor(accent);

        // Top-Left: WiFi Network
        _canvas.setCursor(4, 4);
        _canvas.print("WiFi: ");
        _canvas.print(wifiName);

        // Top-Right: Battery & Compass
        int compassX = w - 16, compassY = 16;
        _canvas.drawCircle(compassX, compassY, 10, accent);
        float headRad = heading * 0.01745f;
        int nx = compassX + sin(headRad) * 8;
        int ny = compassY - cos(headRad) * 8;
        _canvas.drawLine(compassX, compassY, nx, ny, accent);

        String battStr = String(batteryPct) + "%";
        int battW = battStr.length() * 6;
        // Draw Battery Icon
        int bx = compassX - 22 - battW;
        int by = 6;
        _canvas.drawRect(bx, by, 16, 8, accent);
        _canvas.fillRect(bx+16, by+2, 2, 4, accent);
        int fillW = (batteryPct * 14) / 100;
        if (fillW > 0) _canvas.fillRect(bx+1, by+1, fillW, 6, batteryPct > 20 ? TFT_GREEN : TFT_RED);
        _canvas.setCursor(bx + 20, by);
        _canvas.print(battStr);

        // Bottom-Left: IP Address
        _canvas.setCursor(4, h - 12);
        _canvas.print(ipAddress);

        // Bottom-Right: Level / Score
        String lvlStr = "Lvl:" + String(tamagotchiLvl) + " XP:" + String(scoreDisplay);
        int lvlW = lvlStr.length() * 6;
        _canvas.setCursor(w - lvlW - 4, h - 12);
        _canvas.print(lvlStr);

        // Center-Bottom: Robot Name (Prominent)
        _canvas.setTextSize(2);
        int nameW = robotName.length() * 12;
        _canvas.setCursor((w - nameW) / 2, h - 22);
        _canvas.print(robotName);

        _canvas.pushSprite(0, 0);
    }
};

extern RobotFace robotFace;
