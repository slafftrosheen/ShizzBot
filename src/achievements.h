// ============================================================
// ShizzBot - Achievement System v1
// NVS-persisted badge tracker for Kids World gamification
// ============================================================
#pragma once
#include <Preferences.h>

struct Achievement {
    const char* id;
    const char* name;
    const char* desc;
    bool unlocked;
    int threshold;    // how many events needed
    int progress;     // current count
};

class AchievementSystem {
public:
    static const int COUNT = 12;
    bool justUnlocked = false;
    int lastUnlockedIdx = -1;

    Achievement list[COUNT] = {
        {"dance",     "First Dance",      "Triggered your first dance",    false, 1,   0},
        {"honk",      "100 Honks",        "Honked the horn 100 times",     false, 100, 0},
        {"speed",     "Speed Demon",      "Set max speed to 100%",         false, 1,   0},
        {"distance",  "Explorer",         "Drove more than 10 meters",     false, 1,   0},
        {"rename",    "Identity Crisis",  "Changed your name 3 times",     false, 3,   0},
        {"emotions",  "Emotion Master",   "Used all emotion faces",        false, 18,  0},
        {"fart",      "Fart Champion",    "Played 50 farts",              false, 50,  0},
        {"ninja",     "Ninja Mode",       "Activated ninja face",          false, 1,   0},
        {"pet",       "Pet Lover",        "Petted 10 times",              false, 10,  0},
        {"music",     "Musician",         "Played 20 musical notes",       false, 20,  0},
        {"story",     "Storyteller",      "Completed a story",            false, 1,   0},
        {"rgb",       "Rainbow",          "Used all RGB patterns",         false, 7,   0},
    };

    void init(Preferences& prefs) {
        _prefs = &prefs;
        load();
    }

    // Check and increment progress for an achievement
    // Returns true if a NEW unlock just happened
    bool check(const char* id, int increment = 1) {
        for (int i = 0; i < COUNT; i++) {
            if (strcmp(list[i].id, id) == 0) {
                if (list[i].unlocked) return false;
                list[i].progress += increment;
                if (list[i].progress >= list[i].threshold) {
                    list[i].unlocked = true;
                    justUnlocked = true;
                    lastUnlockedIdx = i;
                    save();
                    Serial.printf("[ACH] UNLOCKED: %s\n", list[i].name);
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    // Set progress to exact value (for things like "speed set to 100")
    bool checkSet(const char* id, int value) {
        for (int i = 0; i < COUNT; i++) {
            if (strcmp(list[i].id, id) == 0) {
                if (list[i].unlocked) return false;
                list[i].progress = value;
                if (list[i].progress >= list[i].threshold) {
                    list[i].unlocked = true;
                    justUnlocked = true;
                    lastUnlockedIdx = i;
                    save();
                    Serial.printf("[ACH] UNLOCKED: %s\n", list[i].name);
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    void clearNotification() {
        justUnlocked = false;
        lastUnlockedIdx = -1;
    }

    // Build JSON array string for WebUI
    String toJson() {
        String out = "[";
        for (int i = 0; i < COUNT; i++) {
            if (i > 0) out += ",";
            out += "{\"id\":\"";
            out += list[i].id;
            out += "\",\"n\":\"";
            out += list[i].name;
            out += "\",\"u\":";
            out += list[i].unlocked ? "1" : "0";
            out += ",\"p\":";
            out += list[i].progress;
            out += ",\"t\":";
            out += list[i].threshold;
            out += "}";
        }
        out += "]";
        return out;
    }

private:
    Preferences* _prefs = nullptr;

    void save() {
        if (!_prefs) return;
        uint32_t bits = 0;
        for (int i = 0; i < COUNT; i++) {
            if (list[i].unlocked) bits |= (1 << i);
        }
        _prefs->putUInt("ach_bits", bits);
        // Save progress as packed bytes (each clamped to 255)
        uint8_t prog[COUNT];
        for (int i = 0; i < COUNT; i++) {
            prog[i] = min(list[i].progress, 255);
        }
        _prefs->putBytes("ach_prog", prog, COUNT);
    }

    void load() {
        if (!_prefs) return;
        uint32_t bits = _prefs->getUInt("ach_bits", 0);
        for (int i = 0; i < COUNT; i++) {
            list[i].unlocked = (bits >> i) & 1;
        }
        uint8_t prog[COUNT] = {0};
        _prefs->getBytes("ach_prog", prog, COUNT);
        for (int i = 0; i < COUNT; i++) {
            list[i].progress = prog[i];
        }
    }
};
