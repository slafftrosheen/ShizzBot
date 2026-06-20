// ============================================================
// ShizzBot Swarm - Main Firmware v3
// Single motor + steering servo + 3 arm servos
// WebSocket control, IMU heading, robot identity
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

Preferences prefs;
bool configDirty = false;

// ===== ROBOT IDENTITY =========================================
String robotName = "ShizzBot";
int robotColor = 0;    // 0=cyan(boy), 1=magenta(girl), 2=lime, 3=orange

// ===== MODES ==================================================
bool stealthMode = false;
bool isDancing = false;
unsigned long danceStartTime = 0;

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

// Gamification
int32_t score = 0;

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
            // === ARM SERVOS ===
            else if (strcmp(cmd, "arm") == 0) {
                int base = doc["b"] | 0;
                int lift = doc["l"] | 0;
                int grip = doc["g"] | 0;
                servos.setArm(base, lift, grip);
            }
            else if (strcmp(cmd, "armPreset") == 0) {
                const char* p = doc["p"];
                if (p) {
                    if (strcmp(p, "park") == 0)    servos.presetPark();
                    else if (strcmp(p, "grab") == 0)    servos.presetGrab();
                    else if (strcmp(p, "wave") == 0)    servos.presetWave();
                    else if (strcmp(p, "release") == 0) servos.presetRelease();
                    score += 5;
                }
            }
            // === HEADING CALIBRATE ===
            else if (strcmp(cmd, "zero") == 0) {
                imuFilter.resetHeading();
            }
            // === SOUNDS ===
            else if (strcmp(cmd, "snd") == 0) {
                const char* s = doc["s"];
                if (s) {
                    if (strcmp(s, "horn") == 0) soundEngine.playHorn();
                    else if (strcmp(s, "chirp") == 0) soundEngine.playChirp();
                    else if (strcmp(s, "scan") == 0) soundEngine.playScan();
                    else if (strcmp(s, "err") == 0) soundEngine.playError();
                    score += 1;
                }
            }
            // === EMOTES ===
            else if (strcmp(cmd, "emo") == 0) {
                const char* e = doc["e"];
                if (e) {
                    if (strcmp(e, "happy") == 0) robotFace.setEmotion(RobotFace::HAPPY);
                    else if (strcmp(e, "angry") == 0) robotFace.setEmotion(RobotFace::ANGRY);
                    else if (strcmp(e, "dizzy") == 0) robotFace.setEmotion(RobotFace::DIZZY);
                    else if (strcmp(e, "surprised") == 0) robotFace.setEmotion(RobotFace::SURPRISED);
                    else if (strcmp(e, "idle") == 0) robotFace.setEmotion(RobotFace::IDLE);
                    score += 2;
                }
            }
            // === DANCE ===
            else if (strcmp(cmd, "dance") == 0) {
                isDancing = true;
                danceStartTime = millis();
                score += 10;
                Serial.println("[!] DANCE MODE ACTIVATED");
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

    char buf[512];
    snprintf(buf, sizeof(buf),
        "{\"v\":%d,\"t\":%d,\"rpm\":%d,\"cur\":%d,"
        "\"p\":%d,\"r\":%d,\"hdg\":%d,\"hdgLck\":%d,"
        "\"ip\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,\"wc\":%u,\"batt\":%d,"
        "\"maxSp\":%d,\"invM\":%d,\"invS\":%d,\"stealth\":%d,"
        "\"name\":\"%s\",\"color\":%d,\"score\":%d,"
        "\"steer\":%d,\"armB\":%d,\"armL\":%d,\"grip\":%d,"
        "\"servoMax\":%d,\"fType\":%d,\"fEye\":%d,\"fBlink\":%d,\"fBounce\":%d}",
        telVoltMV,
        telTempRaw,
        telRpm,
        telCur,
        (int)(imuPitch * 10),
        (int)(imuRoll * 10),
        (int)(imuFilter.heading * 10),
        headingLockActive ? (int)(targetHeading * 10) : -1,
        WiFi.localIP().toString().c_str(),
        WiFi.SSID().c_str(),
        WiFi.RSSI(),
        ws.count(),
        M5.Power.getBatteryLevel(),
        maxRpm / 30,
        motorOk ? motor.getInverted() : 0,
        servos.invertSteer ? 1 : 0,
        stealthMode ? 1 : 0,
        robotName.c_str(),
        robotColor,
        score,
        servos.steerSpeed,
        servos.armBaseSpeed,
        servos.armLiftSpeed,
        servos.gripSpeed,
        servos.maxServoSpeed,
        robotFace.faceType,
        robotFace.eyeRadius,
        robotFace.blinkRate,
        robotFace.bounceFactor
    );
    ws.textAll(buf);
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
    
    Serial.printf("[ID] Robot: %s (color=%d, faceType=%d)\n", robotName.c_str(), robotColor, robotFace.faceType);

    wifiSSID = prefs.getString("ssid", "Sinstro_HomeLab");
    wifiPass = prefs.getString("pass", "Dev18118810208");

    // I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    delay(100);

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
        if (batt < 20 && actualOut == 0 && !isDancing) {
            robotFace.setEmotion(RobotFace::SLEEPY);
            if (random(100) < 10 && !stealthMode) soundEngine.playError();
        } else if (batt >= 20 && robotFace.getEmotion() == RobotFace::SLEEPY) {
            robotFace.setEmotion(RobotFace::IDLE);
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

    // Dance Macro (single motor + steering)
    if (isDancing) {
        unsigned long t = millis() - danceStartTime;
        if (t < 800) {
            cmdThrottle = 50; servos.setSteer(100);
            robotFace.setEmotion(RobotFace::SURPRISED);
            if (t == 0 && !stealthMode) soundEngine.playChirp();
        } else if (t < 1600) {
            cmdThrottle = 50; servos.setSteer(-100);
            robotFace.setEmotion(RobotFace::HAPPY);
        } else if (t < 2200) {
            cmdThrottle = 100; servos.setSteer(0);
            robotFace.setEmotion(RobotFace::ANGRY);
            if (t >= 1600 && t < 1610 && !stealthMode) soundEngine.playHorn();
        } else if (t < 2800) {
            cmdThrottle = -80; servos.setSteer(0);
            robotFace.setEmotion(RobotFace::DIZZY);
        } else {
            cmdThrottle = 0; cmdSteer = 0;
            servos.setSteer(0);
            isDancing = false;
            robotFace.setEmotion(RobotFace::IDLE);
        }
        applyMotor(cmdThrottle);
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
        // Update face heading from IMU
        robotFace.heading = imuFilter.heading;
        pushTelemetry();
    }

    // Face update
    if (!stealthMode) {
        robotFace.update(actualOut, 0);
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
