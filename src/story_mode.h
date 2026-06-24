// ============================================================
// ShizzBot - Story Mode Engine v1
// Non-blocking pre-scripted adventure sequences
// ============================================================
#pragma once
#include <Arduino.h>

// Forward declarations — resolved in main.cpp
class RobotFace;
class SoundEngine;
class ServoController;

struct StoryStep {
    uint16_t durationMs;
    int8_t   emotion;     // -1 = don't change, maps to RobotFace::Emotion
    int8_t   soundIdx;    // -1 = none, index into sound dispatch
    int8_t   panSpeed;    // servo pan command (-100..100)
    int8_t   tiltSpeed;   // servo tilt command (-100..100)
    const char* speech;   // speech bubble text (nullptr = none)
};

// Sound index mapping (dispatched in main.cpp):
// 0=chirp, 1=horn, 2=magic, 3=pewpew, 4=giggle, 5=fart,
// 6=tada, 7=scan, 8=error, 9=boing, 10=drumroll, 11=alien

// ===== STORY 1: The Brave Robot =====
static const StoryStep STORY_BRAVE[] PROGMEM = {
    { 2000,  0, 0,   0,   0, "Once upon a time..."},     // IDLE + chirp
    { 2000, -1, -1,  40,   0, "I looked around..."},      // pan left
    { 2000, -1, -1, -40,   0, "What was that?!"},         // pan right
    { 1500,  7, 7,   0, -30, "Something scary!"},         // SURPRISED + scan
    { 2000,  5, 8,   0,   0, "AHHH!"},                    // ANGRY + error
    { 1500, -1, -1,  80,   0, nullptr},                   // dodge left
    { 1500, -1, -1, -80,   0, nullptr},                   // dodge right
    { 2000,  4, 1,   0,   0, "I am BRAVE!"},              // HAPPY + horn
    { 2000, 12, 2,   0, -40, "NINJA MODE!"},              // NINJA + magic
    { 1500,  4, 6,   0,   0, "I won!"},                   // HAPPY + tada
    { 2000,  0, 0,   0,   0, "The End!"},                 // IDLE + chirp
    { 0,    -1, -1,  0,   0, nullptr}                     // sentinel
};

// ===== STORY 2: Space Mission =====
static const StoryStep STORY_SPACE[] PROGMEM = {
    { 2500,  0, 10,  0,   0, "Mission Control..."},       // IDLE + drumroll
    { 2000, -1, -1,  0,   0, "3... 2... 1..."},           // countdown text
    { 1500,  7, 1,   0, -80, "LIFTOFF!"},                 // SURPRISED + horn, tilt up
    { 2000,  1, 3,   0,   0, "Zooming through space!"},   // MOVING + pewpew
    { 2000, -1, 7,  60,   0, "Scanning for aliens..."},   // scan + pan
    { 2000, 11, 11, -60,   0, "ALIEN DETECTED!"},         // ALIEN(=SILLY mapped) + alien sound
    { 1500,  9, 3,   0,   0, "PEW PEW PEW!"},             // LOVE + pewpew
    { 2000, 10, 2,   0,   0, "Using magic shield!"},      // COOL + magic
    { 2000,  4, 6,   0,   0, "Mission Complete!"},        // HAPPY + tada
    { 2000,  0, 4,   0,   0, "Back to Earth!"},           // IDLE + giggle
    { 0,    -1, -1,  0,   0, nullptr}                     // sentinel
};

// ===== STORY 3: The Silly Dance =====
static const StoryStep STORY_SILLY[] PROGMEM = {
    { 2000,  0, 0,   0,   0, "Let's get silly!"},         // IDLE + chirp
    { 1500,  8, 5,   0,   0, "Oops..."},                  // SILLY(=LOVE mapped) + fart
    { 1200, -1, 4,  80,   0, "Hehehehe!"},                // giggle + pan
    { 1200, -1, -1, -80,   0, nullptr},                   // pan other way
    { 1500,  6, 9,   0,  40, "BOING!"},                   // DIZZY + boing
    { 1200,  7, 5,   0,   0, "Pffft!"},                   // SILLY + fart
    { 1500, -1, 4,  60,   0, "Can't stop!"},              // giggle
    { 1500, 10, 2,   0, -40, "Magic time!"},              // COOL + magic
    { 2000,  4, 6,   0,   0, "TA-DA!"},                   // HAPPY + tada
    { 2000,  0, 0,   0,   0, "That was fun!"},            // IDLE + chirp
    { 0,    -1, -1,  0,   0, nullptr}                     // sentinel
};

class StoryEngine {
public:
    bool isPlaying() const { return _playing; }
    int currentStoryId() const { return _storyId; }

    void startStory(int id) {
        switch (id) {
            case 0: _steps = STORY_BRAVE; break;
            case 1: _steps = STORY_SPACE; break;
            case 2: _steps = STORY_SILLY; break;
            default: return;
        }
        _storyId = id;
        _stepIdx = 0;
        _playing = true;
        _stepStart = millis();
        _applied = false;
        Serial.printf("[STORY] Starting story %d\n", id);
    }

    void stop() {
        _playing = false;
        _steps = nullptr;
        Serial.println("[STORY] Stopped");
    }

    // Returns current step data for main.cpp to dispatch
    // Call from loop(). Returns true if a new step just started.
    bool update(StoryStep& out) {
        if (!_playing || !_steps) return false;

        // Read current step from PROGMEM
        StoryStep current;
        memcpy_P(&current, &_steps[_stepIdx], sizeof(StoryStep));

        // Check for sentinel (end of story)
        if (current.durationMs == 0) {
            _playing = false;
            Serial.println("[STORY] Finished!");
            return false;
        }

        // If we haven't applied this step yet, do it now
        if (!_applied) {
            _applied = true;
            out = current;
            return true;
        }

        // Check if step duration elapsed
        if (millis() - _stepStart >= current.durationMs) {
            _stepIdx++;
            _stepStart = millis();
            _applied = false;

            // Read next step
            memcpy_P(&current, &_steps[_stepIdx], sizeof(StoryStep));
            if (current.durationMs == 0) {
                _playing = false;
                Serial.println("[STORY] Finished!");
                return false;
            }

            _applied = true;
            out = current;
            return true;
        }

        return false;
    }

private:
    const StoryStep* _steps = nullptr;
    int _storyId = -1;
    int _stepIdx = 0;
    bool _playing = false;
    bool _applied = false;
    unsigned long _stepStart = 0;
};
