// ============================================================
// ShizzBot - Pet Mode Engine v1
// Virtual pet with idle behaviors, hunger, and interaction
// ============================================================
#pragma once
#include <Arduino.h>

class PetEngine {
public:
    bool active = false;
    int hunger = 100;          // 0-100, decays over time
    int petCount = 0;          // lifetime pet interactions

    void init() {
        _lastDecay = millis();
        _nextIdleAction = millis() + 5000 + random(5000);
    }

    void toggle() {
        active = !active;
        if (active) {
            _nextIdleAction = millis() + 3000;
            Serial.println("[PET] Pet Mode ON");
        } else {
            Serial.println("[PET] Pet Mode OFF");
        }
    }

    void feed() {
        hunger = min(hunger + 30, 100);
        _feedAnim = true;
        _feedAnimTriggered = false;
        _feedAnimStart = millis();
        Serial.printf("[PET] Fed! Hunger: %d\n", hunger);
    }

    void pet() {
        petCount++;
        _petAnim = true;
        _petAnimTriggered = false;
        _petAnimStart = millis();
        Serial.printf("[PET] Petted! Count: %d\n", petCount);
    }

    // Call from loop(). Returns action codes:
    // 0 = nothing
    // 1 = yawn (show SLEEPY)
    // 2 = look around (show curious eye drift)
    // 3 = happy chirp (show HAPPY + chirp)
    // 4 = hungry complaint (show ANGRY)
    // 5 = starving cry (show CRYING)
    // 6 = feed reaction (show HAPPY + giggle)
    // 7 = pet reaction (show LOVE + giggle)
    // 8 = shake detected (show SURPRISED)
    int update() {
        if (!active) return 0;
        unsigned long now = millis();

        // Feed animation takes priority
        if (_feedAnim) {
            if (now - _feedAnimStart > 2000) {
                _feedAnim = false;
            } else if (!_feedAnimTriggered) {
                _feedAnimTriggered = true;
                return 6;
            }
            return 0; // block other actions
        }

        // Pet animation
        if (_petAnim) {
            if (now - _petAnimStart > 2000) {
                _petAnim = false;
            } else if (!_petAnimTriggered) {
                _petAnimTriggered = true;
                return 7;
            }
            return 0; // block other actions
        }

        // Hunger decay: -1 every 60 seconds
        if (now - _lastDecay > 60000) {
            _lastDecay = now;
            hunger = max(hunger - 1, 0);
        }

        // Starving overrides idle
        if (hunger < 15) return 5;
        if (hunger < 30) return 4;

        // Idle behavior timer
        if (now > _nextIdleAction) {
            int delayBase = (hunger > 50) ? 8000 : 4000; // more restless when hungry
            _nextIdleAction = now + delayBase + random(delayBase);

            int roll = random(100);
            if (roll < 25) return 1;       // yawn
            else if (roll < 55) return 2;  // look around
            else if (roll < 80) return 3;  // happy chirp
            else return 0;                 // do nothing
        }

        return 0;
    }

    // Check accelerometer for shake detection
    // Call with current accel magnitude delta
    bool checkShake(float accelDelta) {
        if (!active) return false;
        if (accelDelta > 0.8f) {
            _shakeDetected = true;
            return true;
        }
        return false;
    }

    bool wasShaken() {
        bool s = _shakeDetected;
        _shakeDetected = false;
        return s;
    }

private:
    unsigned long _lastDecay = 0;
    unsigned long _nextIdleAction = 0;
    bool _feedAnim = false;
    bool _feedAnimTriggered = false;
    unsigned long _feedAnimStart = 0;
    bool _petAnim = false;
    bool _petAnimTriggered = false;
    unsigned long _petAnimStart = 0;
    bool _shakeDetected = false;
};
