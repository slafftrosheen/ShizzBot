// ============================================================
// ShizzBot Swarm - Main Firmware v4 (Kids World Mega)
// Single motor + steering servo + pan/tilt servos
// WebSocket control, IMU heading, robot identity,
// Achievements, Story Mode, Pet Mode, RGB Effects, Music Box
// ============================================================
#include <M5Unified.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "roller_can.h"
#include "imu_filter.h"
#include "servo_control.h"
#include "webui.h"
#include <ESPmDNS.h>
#include <ElegantOTA.h>
#include <Preferences.h>
#include "sounds.h"
#include "face.h"
#include "ir_control.h"
#include <AsyncUDP.h>
#include "bme_sensor.h"
#include "mic_sensor.h"
#include "achievements.h"
#include "story_mode.h"
#include "pet_mode.h"
#include "rgb_effects.h"

Preferences prefs;
bool configDirty = false;

AsyncUDP udp;
BME680Sensor bme;
MicSensor mic;

bool sentryMode = false;
bool autoPatrol = false;
int tamagotchiEnergy = 100;
int tamagotchiHappiness = 100;
bool isSleeping = false;

struct MacroStep {
    unsigned long timeOffset;
    int throttle;
    int steer;
    int pan, tilt;
};
MacroStep pathMacro[100];
int macroCount = 0;
bool isRecordingMacro = false;
bool isPlayingMacro = false;
unsigned long macroStartMs = 0;
int macroPlayIdx = 0;

// ===== ROBOT IDENTITY =========================================
String robotName = "ShizzBot";
int robotColor = 0;    // 0=cyan(boy), 1=magenta(girl), 2=lime, 3=orange

// Gamification
int32_t score = 0;

// Phase 7 States
float odomX = 0.0f;
float odomY = 0.0f;
bool mirrorBroadcast = false;
bool mirrorMode = false;
bool isZombie = false;
unsigned long lastZombieSnd = 0;
bool hasPotato = false;
long potatoTimerLeft = 0;
unsigned long lastPotatoTick = 0;
unsigned long lastOdomUpdate = 0;

void broadcastSwarm(String event) {
    if(WiFi.status() == WL_CONNECTED) {
        JsonDocument doc;
        doc["from"] = robotName;
        doc["event"] = event;
        String out;
        serializeJson(doc, out);
        udp.broadcastTo(out.c_str(), 8888);
    }
}

void broadcastMirror(int t, int s, int p, int ti) {
    if(WiFi.status() == WL_CONNECTED) {
        JsonDocument doc;
        doc["from"] = robotName;
        doc["event"] = "mirror";
        doc["t"] = t; doc["s"] = s;
        doc["p"] = p; doc["ti"] = ti;
        String out;
        serializeJson(doc, out);
        udp.broadcastTo(out.c_str(), 8888);
    }
}

void broadcastLeaderboard() {
    if(WiFi.status() == WL_CONNECTED) {
        JsonDocument doc;
        doc["from"] = robotName;
        doc["event"] = "leaderboard";
        doc["score"] = score;
        doc["batt"] = M5.Power.getBatteryLevel();
        doc["hap"] = tamagotchiHappiness;
        String out;
        serializeJson(doc, out);
        udp.broadcastTo(out.c_str(), 8888);
    }
}

// ===== MODES ==================================================
bool stealthMode = false;
bool isDancing = false;
unsigned long danceStartTime = 0;
int danceRoutine = 0;  // 0=disco, 1=robot, 2=wiggle, 3=moonwalk
int dancePhase = 0;
bool isMacroAction = false;
unsigned long macroActionStart = 0;
String currentMacro = "";
int macroPhase = 0;

// ===== KIDS WORLD SYSTEMS =====================================
AchievementSystem achievements;
StoryEngine storyEngine;
PetEngine petEngine;
RGBEngine rgbEngine;
int tamagotchiHunger = 100;
int tamagotchiXP = 0;
int tamagotchiLevel = 0;  // 0=baby, 1=teen, 2=adult
int emotionUsageBits = 0; // Bitmask for achievement tracking

// ===== CONFIGURATION =========================================
String wifiSSID;
String wifiPass;

#define I2C_SDA  9
#define I2C_SCL  10
#define MOTOR_ADDR 0x64   // Single rear-axle motor

// ===== GLOBALS ================================================
RollerCAN motor(MOTOR_ADDR);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

SoundEngine soundEngine;
RobotFace robotFace;
IRControl irControl;
ServoController servos;

// Drive state (single motor + steering)
int32_t cmdThrottle = 0;  // -100..100 from joystick Y
int32_t cmdSteer = 0;     // -100..100 from joystick X
int32_t actualOut = 0;    // what we send to motor
int32_t maxRpm = 1500;
unsigned long lastCmdTime = 0;
const unsigned long CMD_TIMEOUT = 1500;

// Telemetry
int32_t telVoltMV  = 0;
int32_t telTempRaw = 0;
int32_t telRpm = 0;
int32_t telCur = 0;

// Heading Lock
bool headingLockActive = false;
float targetHeading = 0.0f;

// IMU
IMUFilter imuFilter;
float imuPitch = 0, imuRoll = 0;
unsigned long lastImuUs = 0;

bool motorOk = false;

// ===== DISPLAY ================================================
void drawStatus(const char* msg) {
    M5.Display.fillRect(0, 50, M5.Display.width(), 20, TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.setCursor(6, 52);
    M5.Display.print(msg);
}

// ===== MOTOR CONTROL ==========================================
void applyMotor(int32_t throttle) {
    actualOut = throttle;
    int32_t rpm = (throttle * maxRpm) / 100;
    if (motorOk) motor.setSpeed(rpm);
}

void emergencyStop() {
    cmdThrottle = 0; cmdSteer = 0;
    actualOut = 0;
    headingLockActive = false;
    servos.stopAll();
    if (motorOk) { motor.stop(); motor.setRGB(255, 0, 0, 60); }
    delay(10);
    if (motorOk) { motor.setMode(ROLLER_MODE_SPEED); motor.setOutput(true); }
}

// ===== IMU ====================================================
void readIMU() {
    M5.Imu.update();
    auto data = M5.Imu.getImuData();
    unsigned long nowUs = micros();
    float dt = (lastImuUs == 0) ? 0 : (nowUs - lastImuUs) / 1000000.0f;
    lastImuUs = nowUs;

    if (!imuFilter.calibrated) {
        imuFilter.calibrate(data.gyro.x, data.gyro.y, data.gyro.z);
        return;
    }

    imuFilter.update(data.accel.x, data.accel.y, data.accel.z,
                     data.gyro.x,  data.gyro.y,  data.gyro.z, dt);
    imuPitch = imuFilter.pitch;
    imuRoll  = imuFilter.roll;
}

// ===== IMU TASK (FreeRTOS Core 1) =============================
void imuTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // 200Hz

    for (;;) {
        readIMU();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ===== WEBSOCKET ==============================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("[WS] Client #%u connected\n", client->id());
    }
    else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("[WS] Client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->opcode == WS_TEXT && info->final && info->index == 0 && info->len == len) {
            data[len] = 0;
            JsonDocument doc;
            if (deserializeJson(doc, (char*)data)) return;

            const char* cmd = doc["c"];
            if (!cmd) return;

            // === DRIVE (throttle + steering) ===
            if (strcmp(cmd, "m") == 0) {
                cmdThrottle = constrain((int)doc["t"], -100, 100);
                cmdSteer = constrain((int)doc["s"], -100, 100);
                if (abs(cmdThrottle) < 8) cmdThrottle = 0; // Deadband
                if (abs(cmdSteer) < 8) cmdSteer = 0;       // Deadband
                lastCmdTime = millis();
                applyMotor(cmdThrottle);
                
                // If manual steering is applied, break heading lock
                if (abs(cmdSteer) > 5) headingLockActive = false;
                
                if (!headingLockActive) {
                    servos.setSteer(cmdSteer);
                }
            }
            // === HEADING LOCK ===
            else if (strcmp(cmd, "hdg") == 0) {
                if (doc["active"] == 1) {
                    targetHeading = doc["h"];
                    headingLockActive = true;
                    Serial.printf("[HDG] Lock engaged: %.1f\n", targetHeading);
                } else {
                    headingLockActive = false;
                    Serial.println("[HDG] Lock disengaged");
                }
            }
            // === EMERGENCY STOP ===
            else if (strcmp(cmd, "stop") == 0) {
                emergencyStop();
                Serial.println("[!] WS Emergency Stop");
            }
            else if (strcmp(cmd, "swarm_estop") == 0) {
                emergencyStop();
                Serial.println("[!] WS Swarm E-Stop Initiated");
                broadcastSwarm("swarm_estop");
            }
            // === CONFIG ===
            else if (strcmp(cmd, "cfg") == 0) {
                bool invMotor = doc["invM"] == 1;
                motor.setInverted(invMotor);
                servos.invertSteer = (doc["invS"] == 1);

                if (doc["maxSp"].is<int>()) {
                    maxRpm = (int)doc["maxSp"] * 30;
                }
                if (doc["servoMax"].is<int>()) {
                    servos.maxServoSpeed = constrain((int)doc["servoMax"], 10, 100);
                }
                configDirty = true;
            }
            // === ROBOT IDENTITY ===
            else if (strcmp(cmd, "name") == 0) {
                const char* n = doc["n"];
                if (n && strlen(n) > 0 && strlen(n) <= 16) {
                    robotName = String(n);
                    robotFace.robotName = robotName;
                    prefs.putString("name", robotName);
                    // Update mDNS
                    MDNS.end();
                    String hostname = robotName;
                    hostname.toLowerCase();
                    hostname.replace(" ", "");
                    MDNS.begin(hostname.c_str());
                    Serial.printf("[ID] Robot renamed to: %s (mDNS: %s.local)\n", 
                                  robotName.c_str(), hostname.c_str());
                    configDirty = true;
                }
            }
            else if (strcmp(cmd, "color") == 0) {
                robotColor = constrain((int)doc["v"], 0, 3);
                robotFace.robotColor = robotColor;
                prefs.putInt("color", robotColor);
                Serial.printf("[ID] Robot color set to: %d\n", robotColor);
                configDirty = true;
            }
            // === FACE CONFIG ===
            else if (strcmp(cmd, "faceCfg") == 0) {
                robotFace.faceType = doc["type"] | 0;
                robotFace.eyeRadius = constrain((int)doc["eye"], 10, 25);
                robotFace.blinkRate = constrain((int)doc["blink"], 0, 100);
                robotFace.bounceFactor = constrain((int)doc["bounce"], 0, 100);
                
                prefs.putInt("fType", robotFace.faceType);
                prefs.putInt("fEye", robotFace.eyeRadius);
                prefs.putInt("fBlink", robotFace.blinkRate);
                prefs.putInt("fBounce", robotFace.bounceFactor);
                Serial.println("[FACE] Config updated & saved");
            }
            // === PAN/TILT SERVOS ===
            else if (strcmp(cmd, "pt") == 0) {
                int pan = doc["p"] | 0;
                int tilt = doc["t"] | 0;
                servos.setPanTilt(pan, tilt);
            }
            else if (strcmp(cmd, "ptPreset") == 0) {
                const char* p = doc["p"];
                if (p) {
                    if (strcmp(p, "center") == 0)  servos.presetCenter();
                    else if (strcmp(p, "scan") == 0) servos.presetScan();
                    else if (strcmp(p, "nod") == 0)  servos.presetNod();
                    else if (strcmp(p, "shake") == 0) servos.presetShake();
                    score += 5;
                }
            }
            // === HEADING CALIBRATE ===
            else if (strcmp(cmd, "zero") == 0) {
                imuFilter.resetHeading();
                odomX = 0;
                odomY = 0;
            }
            // === SOUNDS ===
            else if (strcmp(cmd, "snd") == 0) {
                const char* s = doc["s"];
                if (s) {
                    if (strcmp(s, "horn") == 0) { soundEngine.playHorn(); achievements.check("honk"); }
                    else if (strcmp(s, "chirp") == 0) soundEngine.playChirp();
                    else if (strcmp(s, "scan") == 0) soundEngine.playScan();
                    else if (strcmp(s, "err") == 0) soundEngine.playError();
                    else if (strcmp(s, "fart") == 0) { soundEngine.playFart(); achievements.check("fart"); }
                    else if (strcmp(s, "giggle") == 0) soundEngine.playGiggle();
                    else if (strcmp(s, "magic") == 0) soundEngine.playMagic();
                    else if (strcmp(s, "pewpew") == 0) soundEngine.playPewPew();
                    else if (strcmp(s, "burp") == 0) soundEngine.playBurp();
                    else if (strcmp(s, "whoopee") == 0) soundEngine.playWhoopee();
                    else if (strcmp(s, "alien") == 0) soundEngine.playAlienTalk();
                    else if (strcmp(s, "drumroll") == 0) soundEngine.playDrumRoll();
                    else if (strcmp(s, "tada") == 0) soundEngine.playTaDa();
                    else if (strcmp(s, "boing") == 0) soundEngine.playBoing();
                    score += 1;
                    tamagotchiXP += 1;
                }
            }
            // === EMOTES ===
            else if (strcmp(cmd, "emo") == 0) {
                const char* e = doc["e"];
                if (e) {
                    // Map string to enum
                    struct EmoMap { const char* name; RobotFace::Emotion emo; };
                    static const EmoMap emos[] = {
                        {"idle", RobotFace::IDLE}, {"happy", RobotFace::HAPPY},
                        {"angry", RobotFace::ANGRY}, {"dizzy", RobotFace::DIZZY},
                        {"surprised", RobotFace::SURPRISED}, {"love", RobotFace::LOVE},
                        {"silly", RobotFace::SILLY}, {"cool", RobotFace::COOL},
                        {"crying", RobotFace::CRYING}, {"ninja", RobotFace::NINJA},
                        {"shocked", RobotFace::SHOCKED}, {"wink", RobotFace::WINK},
                        {"bored", RobotFace::BORED}, {"party", RobotFace::PARTY},
                        {"robot", RobotFace::ROBOT_FACE},
                    };
                    for (auto& em : emos) {
                        if (strcmp(e, em.name) == 0) {
                            robotFace.setEmotion(em.emo);
                            emotionUsageBits |= (1 << (int)em.emo);
                            break;
                        }
                    }
                    // Count unique emotions for achievement
                    int emoCount = 0;
                    for (int i = 0; i < RobotFace::EMOTION_COUNT; i++) {
                        if (emotionUsageBits & (1 << i)) emoCount++;
                    }
                    achievements.checkSet("emotions", emoCount);
                    if (strcmp(e, "ninja") == 0) achievements.check("ninja");
                    score += 2;
                    tamagotchiXP += 1;
                }
            }
            // === DANCE ===
            else if (strcmp(cmd, "dance") == 0) {
                isDancing = true;
                danceStartTime = millis();
                dancePhase = 0;
                if (doc["r"].is<int>()) danceRoutine = constrain((int)doc["r"], 0, 3);
                else danceRoutine = (danceRoutine + 1) % 4;
                score += 10;
                achievements.check("dance");
                Serial.printf("[!] DANCE ROUTINE %d\n", danceRoutine);
            }
            // === MACROS ===
            else if (strcmp(cmd, "macro") == 0) {
                const char* m = doc["m"];
                if (m) {
                    isMacroAction = true;
                    macroActionStart = millis();
                    currentMacro = String(m);
                    macroPhase = 0;
                }
            }
            // === MUSICAL NOTES ===
            else if (strcmp(cmd, "note") == 0) {
                int n = constrain((int)doc["n"], 0, 11);
                soundEngine.playNote(n);
                achievements.check("music");
                tamagotchiXP += 1;
                score += 1;
            }
            // === STORY MODE ===
            else if (strcmp(cmd, "story") == 0) {
                int id = constrain((int)doc["id"], 0, 2);
                storyEngine.startStory(id);
                score += 5;
            }
            // === PET MODE ===
            else if (strcmp(cmd, "pet") == 0) {
                const char* a = doc["a"];
                if (a) {
                    if (strcmp(a, "feed") == 0) { petEngine.feed(); tamagotchiXP += 2; }
                    else if (strcmp(a, "pat") == 0) { petEngine.pet(); achievements.check("pet"); tamagotchiXP += 2; }
                    else if (strcmp(a, "toggle") == 0) petEngine.toggle();
                }
            }
            // === RGB EFFECTS ===
            else if (strcmp(cmd, "rgb") == 0) {
                int p = constrain((int)doc["p"], 0, 6);
                rgbEngine.setPattern((RGBPattern)p);
                achievements.checkSet("rgb", rgbEngine.countPatternsUsed());
            }
            // === SPEECH BUBBLE ===
            else if (strcmp(cmd, "speech") == 0) {
                const char* t = doc["t"];
                if (t) robotFace.showSpeech(String(t));
            }
            // === EMOJI OVERLAY ===
            else if (strcmp(cmd, "overlay") == 0) {
                int e = constrain((int)doc["e"], 0, 5);
                robotFace.showOverlay(e);
            }
            // === FACE CUSTOMIZER ===
            else if (strcmp(cmd, "faceCust") == 0) {
                if (doc["eye"].is<int>()) robotFace.eyeShape = constrain((int)doc["eye"], 0, 2);
                if (doc["mouth"].is<int>()) robotFace.mouthStyle = constrain((int)doc["mouth"], 0, 2);
                if (doc["acc"].is<int>()) robotFace.accessory = constrain((int)doc["acc"], 0, 3);
                prefs.putInt("eyeShape", robotFace.eyeShape);
                prefs.putInt("mouthSt", robotFace.mouthStyle);
                prefs.putInt("accessory", robotFace.accessory);
                configDirty = true;
            }
            // === ACHIEVEMENTS QUERY ===
            else if (strcmp(cmd, "getAch") == 0) {
                String json = "{\"ach\":" + achievements.toJson() + "}";
                client->text(json);
            }
            // === STEALTH ===
            else if (strcmp(cmd, "stealth") == 0) {
                stealthMode = (doc["v"] == 1);
                M5.Display.setBrightness(stealthMode ? 0 : 255);
                if (stealthMode) {
                    if (motorOk) motor.setRGB(0, 0, 0, 0);
                } else {
                    if (motorOk) motor.setRGB(0, 60, 0, 20);
                }
                configDirty = true;
            }
            // === SWARM / AUTO ===
            else if (strcmp(cmd, "sentry") == 0) {
                sentryMode = (doc["v"] == 1);
                Serial.printf("[SENTRY] %s\n", sentryMode ? "ARMED" : "DISARMED");
            }
            else if (strcmp(cmd, "patrol") == 0) {
                autoPatrol = (doc["v"] == 1);
                Serial.printf("[PATROL] %s\n", autoPatrol ? "ON" : "OFF");
                if (autoPatrol) {
                    headingLockActive = true;
                    targetHeading = imuFilter.heading;
                }
            }
            else if (strcmp(cmd, "rec") == 0) {
                isRecordingMacro = true;
                isPlayingMacro = false;
                macroCount = 0;
                macroStartMs = millis();
                Serial.println("[MACRO] Recording started");
            }
            else if (strcmp(cmd, "play") == 0) {
                isRecordingMacro = false;
                isPlayingMacro = true;
                macroStartMs = millis();
                macroPlayIdx = 0;
                Serial.println("[MACRO] Playback started");
                broadcastSwarm("swarm_macro_play");
            }
            // === ZOMBIE TAG ===
            else if (strcmp(cmd, "zombie") == 0) {
                isZombie = true;
                robotFace.setEmotion(RobotFace::ANGRY);
                soundEngine.playError();
                Serial.println("[ZOMBIE] Tag started. I am IT.");
            }
            // === HOT POTATO ===
            else if (strcmp(cmd, "potato") == 0) {
                hasPotato = true;
                potatoTimerLeft = 60000;
                lastPotatoTick = millis();
                Serial.println("[POTATO] Spawned. Timer: 60s");
            }
            // === MIRRORING ===
            else if (strcmp(cmd, "mirrorB") == 0) {
                mirrorBroadcast = (doc["v"] == 1);
            }
            else if (strcmp(cmd, "mirrorM") == 0) {
                mirrorMode = (doc["v"] == 1);
                if (!mirrorMode) emergencyStop();
            }
            // === WIFI ===
            else if (strcmp(cmd, "wifi") == 0) {
                const char* s = doc["s"];
                const char* p = doc["p"];
                if (s && p) {
                    prefs.putString("ssid", s);
                    prefs.putString("pass", p);
                    Serial.println("[NVS] WiFi credentials saved. Rebooting...");
                    delay(500);
                    ESP.restart();
                }
            }
        }
    }
}

// Push telemetry
void pushTelemetry() {
    if (ws.count() == 0) return;

    char buf[1400];
    snprintf(buf, sizeof(buf),
        "{\"v\":%d,\"t\":%d,\"rpm\":%d,\"cur\":%d,"
        "\"p\":%d,\"r\":%d,\"hdg\":%d,\"hdgLck\":%d,"
        "\"ip\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,\"wc\":%u,\"batt\":%d,"
        "\"maxSp\":%d,\"invM\":%d,\"invS\":%d,\"stealth\":%d,"
        "\"name\":\"%s\",\"color\":%d,\"score\":%d,"
        "\"steer\":%d,\"pan\":%d,\"tilt\":%d,"
        "\"servoMax\":%d,\"fType\":%d,\"fEye\":%d,\"fBlink\":%d,\"fBounce\":%d,"
        "\"bmeT\":%.1f,\"bmeH\":%.1f,\"bmeA\":%d,\"mic\":%d,\"sentry\":%d,\"patrol\":%d,\"eng\":%d,\"hap\":%d,"
        "\"odX\":%.2f,\"odY\":%.2f,"
        "\"hunger\":%d,\"xp\":%d,\"lvl\":%d,\"petOn\":%d,\"petHunger\":%d,"
        "\"achNew\":%d,\"achIdx\":%d,\"rgbP\":%d,\"storyOn\":%d}",
        telVoltMV, telTempRaw, telRpm, telCur,
        (int)(imuPitch * 10), (int)(imuRoll * 10),
        (int)(imuFilter.heading * 10),
        headingLockActive ? (int)(targetHeading * 10) : -1,
        WiFi.localIP().toString().c_str(),
        WiFi.SSID().c_str(), WiFi.RSSI(), ws.count(),
        M5.Power.getBatteryLevel(),
        maxRpm / 30,
        motorOk ? motor.getInverted() : 0,
        servos.invertSteer ? 1 : 0,
        stealthMode ? 1 : 0,
        robotName.c_str(), robotColor, score,
        servos.steerSpeed, servos.panSpeed, servos.tiltSpeed,
        servos.maxServoSpeed,
        robotFace.faceType, robotFace.eyeRadius, robotFace.blinkRate, robotFace.bounceFactor,
        bme.temp, bme.hum, bme.airQuality, mic.noiseLevel,
        sentryMode?1:0, autoPatrol?1:0, tamagotchiEnergy, tamagotchiHappiness,
        odomX, odomY,
        tamagotchiHunger, tamagotchiXP, tamagotchiLevel,
        petEngine.active ? 1 : 0, petEngine.hunger,
        achievements.justUnlocked ? 1 : 0, achievements.lastUnlockedIdx,
        (int)rgbEngine.currentPattern, storyEngine.isPlaying() ? 1 : 0
    );
    ws.textAll(buf);
    // Clear achievement notification after sending
    if (achievements.justUnlocked) achievements.clearNotification();
}

// ===== SETUP ==================================================
void setup() {
    auto cfg = M5.config();
    cfg.internal_imu = true;
    M5.begin(cfg);

    soundEngine.init();
    robotFace.init();
    irControl.init();

    soundEngine.playStartup();

    Serial.begin(115200);
    Serial.println("\n[ShizzBot Swarm v3] Starting...");

    // Load Config
    prefs.begin("shizzbot", false);
    maxRpm = prefs.getInt("maxRpm", 1500);
    stealthMode = prefs.getBool("stealth", false);
    if (stealthMode) M5.Display.setBrightness(0);

    // Robot Identity
    robotName = prefs.getString("name", "ShizzBot");
    robotColor = prefs.getInt("color", 0);
    robotFace.robotName = robotName;
    robotFace.robotColor = robotColor;
    
    robotFace.faceType = prefs.getInt("fType", 0);
    robotFace.eyeRadius = prefs.getInt("fEye", 15);
    robotFace.blinkRate = prefs.getInt("fBlink", 50);
    robotFace.bounceFactor = prefs.getInt("fBounce", 50);
    robotFace.eyeShape = prefs.getInt("eyeShape", 0);
    robotFace.mouthStyle = prefs.getInt("mouthSt", 0);
    robotFace.accessory = prefs.getInt("accessory", 0);

    // Kids World systems
    achievements.init(prefs);
    petEngine.init();
    
    Serial.printf("[ID] Robot: %s (color=%d, faceType=%d)\n", robotName.c_str(), robotColor, robotFace.faceType);

    String currentSsid = prefs.getString("ssid", "");
    if (currentSsid == "Sinstro_HomeLab" || currentSsid == "") {
        prefs.putString("ssid", "STARLINK.TAK");
        prefs.putString("pass", "Slaff181188");
    }
    wifiSSID = prefs.getString("ssid", "STARLINK.TAK");
    wifiPass = prefs.getString("pass", "Slaff181188");

    // I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    delay(100);
    
    bme.init();
    // mic.init(); // Disabled to fix I2S hardware conflict with Speaker

    // Motor (single rear-axle)
    drawStatus("Motor...");
    motorOk = motor.begin();
    Serial.printf("[Motor] @0x%02X: %s\n", MOTOR_ADDR, motorOk ? "OK" : "FAIL");

    if (motorOk) {
        motor.setMode(ROLLER_MODE_SPEED); delay(5);
        motor.setOutput(true); delay(5);
        motor.setStallProtection(true); delay(5);
        motor.setMaxSpeed(3000); delay(5);
        bool defaultInv = prefs.getBool("invM", false);
        motor.setInverted(defaultInv);
        if (!stealthMode) motor.setRGB(0, 60, 0, 20);
        else motor.setRGB(0,0,0,0);
    }

    // Servos
    drawStatus("Servos...");
    servos.invertSteer = prefs.getBool("invS", false);
    servos.maxServoSpeed = prefs.getInt("servoMax", 70);
    servos.init();

    // IMU Task on Core 1 (200Hz)
    xTaskCreatePinnedToCore(imuTask, "IMU", 4096, NULL, 5, NULL, 1);

    // WiFi
    drawStatus("WiFi...");
    WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
        delay(500); Serial.print("."); tries++;
    }

    String ip;
    if (WiFi.status() == WL_CONNECTED) {
        ip = WiFi.localIP().toString();
        Serial.printf("\n[WiFi] STA IP: %s\n", ip.c_str());
        String hostname = robotName;
        hostname.toLowerCase();
        hostname.replace(" ", "");
        if (MDNS.begin(hostname.c_str())) {
            Serial.printf("[mDNS] %s.local started\n", hostname.c_str());
        }
    } else {
        Serial.println("\n[WiFi] STA fail -> AP mode");
        WiFi.softAP("ShizzBot", "shizzbot123");
        ip = WiFi.softAPIP().toString();
        Serial.printf("[WiFi] AP IP: %s\n", ip.c_str());
    }

    if (udp.listen(8888)) {
        udp.onPacket([](AsyncUDPPacket packet) {
            String msg = packet.readString();
            JsonDocument doc;
            if (!deserializeJson(doc, msg)) {
                String from = doc["from"] | "unknown";
                String event = doc["event"] | "none";
                if (from != robotName) {
                    Serial.printf("[SWARM] RX from %s: %s\n", from.c_str(), event.c_str());
                    if (event == "swarm_estop") {
                        emergencyStop();
                        robotFace.setEmotion(RobotFace::SURPRISED);
                        soundEngine.playError();
                        Serial.println("[SWARM] E-STOP RECEIVED!");
                    } else if (event == "crash" && !isDancing && !autoPatrol) {
                        emergencyStop();
                        robotFace.setEmotion(RobotFace::SURPRISED);
                        soundEngine.playError();
                    } else if (event == "sentry_breach") {
                        if (sentryMode) {
                            robotFace.setEmotion(RobotFace::ANGRY);
                            soundEngine.playScan(); // Chain reaction alarm
                        } else {
                            robotFace.setEmotion(RobotFace::SURPRISED);
                        }
                    } else if (event == "swarm_macro_play" && macroCount > 0 && !isRecordingMacro) {
                        isPlayingMacro = true;
                        macroStartMs = millis();
                        macroPlayIdx = 0;
                        Serial.println("[SWARM] Synced macro playback started");
                    } else if (event == "zombie_tag" && !isZombie) {
                        isZombie = true;
                        hasPotato = false; // Mutually exclusive
                        robotFace.setEmotion(RobotFace::ANGRY);
                        soundEngine.playError();
                        Serial.println("[ZOMBIE] I was tagged! I am IT.");
                    } else if (event == "pass_potato" && !hasPotato) {
                        hasPotato = true;
                        isZombie = false; // Mutually exclusive
                        potatoTimerLeft = doc["time_left"] | 60000;
                        if (potatoTimerLeft < 0) potatoTimerLeft = 0;
                        lastPotatoTick = millis();
                        Serial.println("[POTATO] Caught the potato!");
                    } else if (event == "mirror" && mirrorMode) {
                        applyMotor(doc["t"] | 0);
                        servos.setSteer(doc["s"] | 0);
                        servos.setPanTilt(doc["p"] | 0, doc["ti"] | 0);
                    }
                }
            }
        });
    }

    // WebSocket
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // HTTP: serve WebUI
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        AsyncWebServerResponse *response = req->beginResponse(200, "text/html", WEBUI_HTML);
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        req->send(response);
    });

    ElegantOTA.begin(&server);

    server.begin();
    Serial.println("[ShizzBot Swarm v3] Ready!");
}

// ===== LOOP ===================================================
void loop() {
    M5.update();

    // Physical e-stop
    if (M5.BtnA.wasPressed()) {
        emergencyStop();
        soundEngine.playError();
        Serial.println("[!] BtnA Emergency Stop");
    }

    // IR Control
    char irCmd = irControl.update();
    if (irCmd) {
        if (irCmd == 'F') { cmdThrottle = 100; cmdSteer = 0; }
        else if (irCmd == 'B') { cmdThrottle = -100; cmdSteer = 0; }
        else if (irCmd == 'L') { cmdThrottle = 0; cmdSteer = -80; }
        else if (irCmd == 'R') { cmdThrottle = 0; cmdSteer = 80; }
        else if (irCmd == 'S') { cmdThrottle = 0; cmdSteer = 0; }
        lastCmdTime = millis();
        applyMotor(cmdThrottle);
        servos.setSteer(cmdSteer);
        robotFace.setEmotion(RobotFace::MOVING);
    }

    // Sound engine
    soundEngine.update();

    // Servo safety timeout
    servos.update();

    // Motor command timeout
    if (millis() - lastCmdTime > CMD_TIMEOUT && (cmdThrottle != 0 || cmdSteer != 0) && !isDancing) {
        Serial.println("[!] Timeout -> stop");
        emergencyStop();
    }

    // Battery Fatigue System
    static unsigned long lastBattCheck = 0;
    if (millis() - lastBattCheck > 5000) {
        lastBattCheck = millis();
        int batt = M5.Power.getBatteryLevel();
        if (batt < 20 && actualOut == 0 && !isDancing && !autoPatrol) {
            isSleeping = true;
            robotFace.setEmotion(RobotFace::SLEEPY);
            if (random(100) < 10 && !stealthMode) soundEngine.playError();
        } else if (batt >= 20 && isSleeping) {
            isSleeping = false;
            robotFace.setEmotion(RobotFace::IDLE);
        }
    }

    // Sensors Update
    bme.update();
    // mic.update(); // Disabled

    // IMU Crash & Sentry
    static float lastAccelMag = 1.0f;
    auto data = M5.Imu.getImuData();
    float accelMag = sqrt(data.accel.x*data.accel.x + data.accel.y*data.accel.y + data.accel.z*data.accel.z);
    if (abs(accelMag - lastAccelMag) > 0.6f) { // roughly 1.6G spike
        if (sentryMode) {
            soundEngine.playScan();
            robotFace.setEmotion(RobotFace::ANGRY);
            broadcastSwarm("sentry_breach");
        } else if (abs(actualOut) > 20) {
            // Collision!
            emergencyStop();
            
            if (isZombie) {
                isZombie = false;
                broadcastSwarm("zombie_tag");
                robotFace.setEmotion(RobotFace::HAPPY);
                Serial.println("[ZOMBIE] Tagged someone else!");
            } else if (hasPotato) {
                hasPotato = false;
                JsonDocument doc;
                doc["from"] = robotName;
                doc["event"] = "pass_potato";
                doc["time_left"] = potatoTimerLeft;
                String out; serializeJson(doc, out);
                udp.broadcastTo(out.c_str(), 8888);
                robotFace.setEmotion(RobotFace::HAPPY);
                Serial.println("[POTATO] Passed the potato!");
            } else {
                robotFace.setEmotion(RobotFace::ANGRY);
                soundEngine.playError();
                broadcastSwarm("crash");
                tamagotchiHappiness -= 20;
            }
            
            cmdThrottle = -50; // auto reverse
            applyMotor(cmdThrottle);
            delay(300);
            emergencyStop();
        }
    }
    lastAccelMag = accelMag;

    // Tamagotchi update (every 1s)
    static unsigned long lastTama = 0;
    if (millis() - lastTama > 1000) {
        lastTama = millis();
        if (actualOut != 0 || isDancing || autoPatrol) {
            tamagotchiHappiness++;
            tamagotchiEnergy--;
            isSleeping = false;
        } else {
            tamagotchiHappiness--;
            tamagotchiEnergy++;
        }
        
        // Air quality penalty
        if (bme.airQuality < 30) {
            tamagotchiHappiness -= 2; // Extra decay for bad air
        }
        
        tamagotchiHappiness = constrain(tamagotchiHappiness, 0, 100);
        tamagotchiEnergy = constrain(tamagotchiEnergy, 0, 100);

        // Hunger decay (every 60 tamagotchi ticks = 1 min)
        static int hungerTick = 0;
        if (++hungerTick >= 60) {
            hungerTick = 0;
            tamagotchiHunger = max(tamagotchiHunger - 1, 0);
        }

        // XP leveling
        if (tamagotchiLevel == 0 && tamagotchiXP >= 100) { tamagotchiLevel = 1; Serial.println("[TAMA] Level up: TEEN!"); }
        if (tamagotchiLevel == 1 && tamagotchiXP >= 500) { tamagotchiLevel = 2; Serial.println("[TAMA] Level up: ADULT!"); }

        // Distance achievement
        float dist = sqrt(odomX*odomX + odomY*odomY);
        if (dist > 10.0f) achievements.check("distance");

        if (bme.airQuality < 40 && !isSleeping && !stealthMode) {
            robotFace.setEmotion(RobotFace::DIZZY);
        } else if (tamagotchiHappiness < 20 && !isSleeping && !stealthMode) {
            robotFace.setEmotion(RobotFace::ANGRY);
        }
        
        // 1Hz Leaderboard Broadcast
        broadcastLeaderboard();
    }
    
    // 10Hz Update Loop (Odometry, Mirroring, Potato)
    if (millis() - lastOdomUpdate > 100) {
        float dt = (millis() - lastOdomUpdate) / 1000.0f;
        lastOdomUpdate = millis();
        
        // Odometry
        float hdgRad = imuFilter.heading * (PI / 180.0f);
        float speed = (float)telRpm * 0.01f; // Arbitrary units for radar visual
        odomX += speed * cos(hdgRad) * dt;
        odomY += speed * sin(hdgRad) * dt;
        
        // Mirror Broadcast
        if (mirrorBroadcast) {
            broadcastMirror(cmdThrottle, cmdSteer, servos.panSpeed, servos.tiltSpeed);
        }
        
        // Zombie Grumble
        if (isZombie && millis() - lastZombieSnd > 4000) {
            lastZombieSnd = millis();
            if(!stealthMode) soundEngine.playReverseBeep(); // Substitute for grumble
        }
        
        // Hot Potato Logic
        if (hasPotato) {
            long passed = millis() - lastPotatoTick;
            lastPotatoTick = millis();
            potatoTimerLeft -= passed;
            if (potatoTimerLeft < 0) potatoTimerLeft = 0; // Clamp
            
            if (potatoTimerLeft <= 0) {
                hasPotato = false;
                emergencyStop();
                soundEngine.playError();
                robotFace.setEmotion(RobotFace::DIZZY);
                tamagotchiHappiness -= 50;
                Serial.println("[POTATO] BOOM! You held the potato too long.");
            } else {
                // Ticking sound increases frequency
                int tickRate = map(potatoTimerLeft, 0, 60000, 200, 1500);
                static unsigned long lastTickSound = 0;
                if (millis() - lastTickSound > tickRate) {
                    lastTickSound = millis();
                    if(!stealthMode) soundEngine.playChirp(); // Tick
                }
            }
        }
    }

    // Path Recording
    if (isRecordingMacro && macroCount < 100) {
        static unsigned long lastRec = 0;
        if (millis() - lastRec > 100) {
            lastRec = millis();
            pathMacro[macroCount].timeOffset = millis() - macroStartMs;
            pathMacro[macroCount].throttle = actualOut;
            pathMacro[macroCount].steer = servos.steerSpeed;
            pathMacro[macroCount].pan = servos.panSpeed;
            pathMacro[macroCount].tilt = servos.tiltSpeed;
            macroCount++;
            if (macroCount >= 100) isRecordingMacro = false;
        }
    }

    // Path Playback
    if (isPlayingMacro) {
        unsigned long t = millis() - macroStartMs;
        while (macroPlayIdx < macroCount && pathMacro[macroPlayIdx].timeOffset <= t) {
            applyMotor(pathMacro[macroPlayIdx].throttle);
            servos.setSteer(pathMacro[macroPlayIdx].steer);
            servos.setPanTilt(pathMacro[macroPlayIdx].pan, pathMacro[macroPlayIdx].tilt);
            macroPlayIdx++;
        }
        if (macroPlayIdx >= macroCount) {
            isPlayingMacro = false;
            emergencyStop();
        }
    }

    // Auto Patrol
    if (autoPatrol) {
        static unsigned long patrolStateTime = 0;
        static int patrolState = 0; // 0=drive, 1=turn
        unsigned long pt = millis() - patrolStateTime;
        
        if (patrolState == 0) { // Drive straight
            headingLockActive = true;
            if (actualOut == 0) applyMotor(maxRpm > 0 ? 50 : 0);
            if (pt > 2000) {
                patrolState = 1;
                patrolStateTime = millis();
                targetHeading += 90;
                while(targetHeading >= 360) targetHeading -= 360;
                emergencyStop();
                headingLockActive = true;
            }
        } else if (patrolState == 1) { // Turn until heading matched
            float err = targetHeading - imuFilter.heading;
            while(err <= -180.0f) err += 360.0f;
            while(err > 180.0f) err -= 360.0f;
            if (abs(err) < 5.0f) {
                patrolState = 0;
                patrolStateTime = millis();
                emergencyStop();
            } else {
                applyMotor(0); // keep stopped
                servos.setSteer(err > 0 ? 100 : -100);
            }
        }
    }

    // Heading Lock Auto-Steering
    if (headingLockActive && !isDancing && actualOut != 0) {
        // Calculate shortest angular distance (-180 to +180)
        float currentHdg = imuFilter.heading;
        float error = targetHeading - currentHdg;
        
        // Normalize error to -180..+180
        while (error <= -180.0f) error += 360.0f;
        while (error > 180.0f) error -= 360.0f;
        
        // Simple P-controller
        float pGain = 1.5f; 
        int steerCorr = (int)(error * pGain);
        steerCorr = constrain(steerCorr, -100, 100);
        
        // If moving backwards, invert steering correction
        if (actualOut < 0) steerCorr = -steerCorr;
        
        servos.setSteer(steerCorr);
    }

    // Dance Routines (4 varieties)
    if (isDancing) {
        unsigned long t = millis() - danceStartTime;
        bool done = false;
        if (danceRoutine == 0) { // Disco
            if (dancePhase == 0) { cmdThrottle = 50; servos.setSteer(100); robotFace.setEmotion(RobotFace::PARTY); if (!stealthMode) soundEngine.playChirp(); dancePhase++; }
            else if (dancePhase == 1 && t >= 800) { cmdThrottle = 50; servos.setSteer(-100); robotFace.setEmotion(RobotFace::HAPPY); dancePhase++; }
            else if (dancePhase == 2 && t >= 1600) { cmdThrottle = 100; servos.setSteer(0); robotFace.setEmotion(RobotFace::COOL); if (!stealthMode) soundEngine.playHorn(); dancePhase++; }
            else if (dancePhase == 3 && t >= 2200) { cmdThrottle = -80; servos.setSteer(0); robotFace.setEmotion(RobotFace::DIZZY); dancePhase++; }
            else if (t >= 2800) done = true;
        } else if (danceRoutine == 1) { // Robot Dance
            if (dancePhase == 0) { cmdThrottle = 0; servos.setSteer(100); robotFace.setEmotion(RobotFace::ROBOT_FACE); if (!stealthMode) soundEngine.playDrumRoll(); dancePhase++; }
            else if (dancePhase == 1 && t >= 500) { cmdThrottle = 0; servos.setSteer(-100); dancePhase++; }
            else if (dancePhase == 2 && t >= 1000) { cmdThrottle = 60; servos.setSteer(0); servos.setPan(80); dancePhase++; }
            else if (dancePhase == 3 && t >= 1500) { cmdThrottle = -60; servos.setSteer(0); servos.setPan(-80); dancePhase++; }
            else if (dancePhase == 4 && t >= 2000) { cmdThrottle = 0; servos.setPan(0); servos.setTilt(60); dancePhase++; }
            else if (dancePhase == 5 && t >= 2500) { servos.setTilt(-60); dancePhase++; }
            else if (t >= 3000) done = true;
        } else if (danceRoutine == 2) { // Wiggle
            if (dancePhase == 0) { robotFace.setEmotion(RobotFace::SILLY); if (!stealthMode) soundEngine.playGiggle(); dancePhase++; }
            if (t < 2500) {
                int wiggle = ((t / 150) % 2 == 0) ? 80 : -80;
                servos.setPan(wiggle);
                cmdThrottle = 30;
            } else done = true;
        } else if (danceRoutine == 3) { // Moonwalk
            if (dancePhase == 0) { robotFace.setEmotion(RobotFace::COOL); if (!stealthMode) soundEngine.playMagic(); dancePhase++; }
            if (t < 3000) {
                cmdThrottle = -40;
                servos.setSteer(((t / 500) % 2 == 0) ? 30 : -30);
            } else done = true;
        }
        if (done) {
            cmdThrottle = 0; cmdSteer = 0;
            servos.setSteer(0); servos.setPan(0); servos.setTilt(0);
            isDancing = false;
            robotFace.setEmotion(RobotFace::IDLE);
        }
        applyMotor(cmdThrottle);
    }

    // Action Macros (Non-blocking)
    if (isMacroAction) {
        unsigned long t = millis() - macroActionStart;
        if (currentMacro == "fart_shake") {
            if (macroPhase == 0) { soundEngine.playFart(); robotFace.setEmotion(RobotFace::SILLY); servos.setPan(80); macroPhase++; }
            else if (macroPhase == 1 && t >= 200) { servos.setPan(-80); macroPhase++; }
            else if (macroPhase == 2 && t >= 400) { servos.setPan(0); isMacroAction = false; }
        } else if (currentMacro == "magic_reveal") {
            if (macroPhase == 0) { soundEngine.playMagic(); robotFace.setEmotion(RobotFace::LOVE); servos.setTilt(-60); macroPhase++; }
            else if (macroPhase == 1 && t >= 500) { servos.setTilt(60); macroPhase++; }
            else if (macroPhase == 2 && t >= 1000) { servos.setTilt(0); isMacroAction = false; }
        } else if (currentMacro == "victory_dance") {
            if (macroPhase == 0) { soundEngine.playTaDa(); robotFace.setEmotion(RobotFace::HAPPY); servos.setSteer(100); macroPhase++; }
            else if (macroPhase == 1 && t >= 600) { servos.setSteer(-100); soundEngine.playHorn(); macroPhase++; }
            else if (macroPhase == 2 && t >= 1200) { servos.setSteer(0); isMacroAction = false; }
        } else if (currentMacro == "tantrum") {
            if (macroPhase == 0) { soundEngine.playFart(); robotFace.setEmotion(RobotFace::CRYING); servos.setPan(80); macroPhase++; }
            else if (macroPhase == 1 && t >= 300) { servos.setPan(-80); macroPhase++; }
            else if (macroPhase == 2 && t >= 600) { soundEngine.playBurp(); servos.setPan(60); macroPhase++; }
            else if (macroPhase == 3 && t >= 900) { servos.setPan(0); isMacroAction = false; }
        } else if (currentMacro == "sneeze") {
            if (macroPhase == 0) { robotFace.setEmotion(RobotFace::BORED); servos.setTilt(-50); macroPhase++; }
            else if (macroPhase == 1 && t >= 600) { servos.setTilt(80); soundEngine.playChirp(); robotFace.setEmotion(RobotFace::SURPRISED); macroPhase++; }
            else if (macroPhase == 2 && t >= 900) { servos.setTilt(0); isMacroAction = false; }
        } else if (currentMacro == "belly_laugh") {
            if (macroPhase == 0) { soundEngine.playGiggle(); robotFace.setEmotion(RobotFace::SILLY); servos.setPan(60); macroPhase++; }
            else if (macroPhase == 1 && t >= 200) { servos.setPan(-60); macroPhase++; }
            else if (macroPhase == 2 && t >= 400) { servos.setPan(60); macroPhase++; }
            else if (macroPhase == 3 && t >= 600) { servos.setPan(-60); macroPhase++; }
            else if (macroPhase == 4 && t >= 800) { servos.setPan(0); isMacroAction = false; }
        } else {
            isMacroAction = false; // Unknown macro, stop
        }
    }

    // Story Mode update
    StoryStep storyStep;
    if (storyEngine.update(storyStep)) {
        if (storyStep.emotion >= 0) robotFace.setEmotion((RobotFace::Emotion)storyStep.emotion);
        if (storyStep.soundIdx >= 0) soundEngine.playSoundByIndex(storyStep.soundIdx);
        if (storyStep.panSpeed != 0) servos.setPan(storyStep.panSpeed);
        if (storyStep.tiltSpeed != 0) servos.setTilt(storyStep.tiltSpeed);
        if (storyStep.speech) robotFace.showSpeech(String(storyStep.speech));
    }
    if (!storyEngine.isPlaying() && storyEngine.currentStoryId() >= 0) {
        achievements.check("story");
    }

    // Pet Mode update
    int petAction = petEngine.update();
    if (petAction > 0 && !storyEngine.isPlaying() && !isDancing && !isMacroAction) {
        switch (petAction) {
            case 1: robotFace.setEmotion(RobotFace::SLEEPY); break;
            case 2: robotFace.setEmotion(RobotFace::IDLE); break; // look around via eye drift
            case 3: robotFace.setEmotion(RobotFace::HAPPY); if(!stealthMode) soundEngine.playChirp(); break;
            case 4: robotFace.setEmotion(RobotFace::ANGRY); break;
            case 5: robotFace.setEmotion(RobotFace::CRYING); break;
            case 6: robotFace.setEmotion(RobotFace::HAPPY); if(!stealthMode) soundEngine.playGiggle(); break;
            case 7: robotFace.setEmotion(RobotFace::LOVE); if(!stealthMode) soundEngine.playGiggle(); break;
        }
    }

    // RGB Effects update
    rgbEngine.update(motor, motorOk);
    if (rgbEngine.autoSync && motorOk) {
        rgbEngine.syncToEmotion(motor, motorOk, (int)robotFace.getEmotion());
    }

    // Reverse beep
    if (cmdThrottle < -10 && !stealthMode) {
        soundEngine.playReverseBeep();
    }

    // Read motor telemetry (throttled)
    static unsigned long lastTel = 0;
    if (millis() - lastTel > 200) {
        lastTel = millis();
        if (motorOk) {
            telRpm = motor.getSpeedRPM();
            telCur = motor.getCurrentMA();
            telVoltMV = motor.getVoltageMV();
            telTempRaw = motor.getTempRaw();
        }
    }

    // Push telemetry over WebSocket (throttled)
    static unsigned long lastPush = 0;
    if (millis() - lastPush > 150) {
        lastPush = millis();
        pushTelemetry();
    }

    // ===== HUD TELEMETRY UPDATE =====
    static unsigned long lastHudUpdate = 0;
    if (millis() - lastHudUpdate > 1000) {
        lastHudUpdate = millis();
        robotFace.ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "OFFLINE";
        robotFace.wifiName = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Disconnected";
        robotFace.batteryPct = M5.Power.getBatteryLevel();
        robotFace.scoreDisplay = score;
        robotFace.tamagotchiLvl = tamagotchiLevel;
    }
    robotFace.heading = imuFilter.heading;

    // Face update
    if (!stealthMode) {
        robotFace.update(actualOut, servos.steerSpeed);
    }

    // WebSocket cleanup
    static unsigned long lastClean = 0;
    if (millis() - lastClean > 1000) {
        lastClean = millis();
        ws.cleanupClients();
    }

    // Save Config to Flash
    if (configDirty) {
        configDirty = false;
        prefs.putInt("maxRpm", maxRpm);
        if (motorOk) prefs.putBool("invM", motor.getInverted());
        prefs.putBool("invS", servos.invertSteer);
        prefs.putInt("servoMax", servos.maxServoSpeed);
        prefs.putBool("stealth", stealthMode);
        Serial.println("[NVS] Config saved to Flash");
    }

    ElegantOTA.loop();

    delay(10);
}
