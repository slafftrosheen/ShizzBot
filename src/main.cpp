// ============================================================
// ShizzBot - Main Firmware v2
// WebSocket control, IMU self-balancing, telemetry push
// ============================================================
#include <M5Unified.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "roller_can.h"
#include "imu_filter.h"
#include "webui.h"
#include <ESPmDNS.h>
#include <ElegantOTA.h>
#include <Preferences.h>
#include "sounds.h"
#include "face.h"
#include "ir_control.h"

Preferences prefs;
bool configDirty = false;

// ===== CONFIGURATION =========================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASS";

#define I2C_SDA  9
#define I2C_SCL  10
#define MOTOR_L_ADDR 0x64
#define MOTOR_R_ADDR 0x65

// ===== GLOBALS ================================================
RollerCAN motorL(MOTOR_L_ADDR);
RollerCAN motorR(MOTOR_R_ADDR);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

SoundEngine soundEngine;
RobotFace robotFace;
IRControl irControl;

// Drive state
enum DriveMode { DRIVE_SKID, DRIVE_BALANCE };
DriveMode driveMode = DRIVE_SKID;

int32_t cmdL = 0, cmdR = 0;         // -100..100 from UI
int32_t actualOutL = 0, actualOutR = 0;  // what we send to motors
int32_t maxRpm = 3000;
unsigned long lastCmdTime = 0;
const unsigned long CMD_TIMEOUT = 1500;

// Telemetry (raw from registers)
int32_t telVoltMV  = 0;    // millivolts
int32_t telTempRaw = 0;    // °C * 10
int32_t telRpmL = 0, telRpmR = 0;
int32_t telCurL = 0, telCurR = 0;  // milliamps

// IMU + Balance
IMUFilter imuFilter;
PIDController balancePID;
float imuPitch = 0, imuRoll = 0;
unsigned long lastImuUs = 0;
int32_t balanceSteer = 0;  // differential from joystick X during balance

bool motorLOk = false, motorROk = false;

// ===== DISPLAY ================================================
void drawBoot() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(6, 8);
    M5.Display.print("ShizzBot");
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0x7BEF); // grey
    M5.Display.setCursor(6, 32);
    M5.Display.print("v2.0 booting...");
}

void drawStatus(const char* msg) {
    M5.Display.fillRect(0, 50, M5.Display.width(), 20, TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.setCursor(6, 52);
    M5.Display.print(msg);
}

void drawRunningScreen(const char* ip) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.setTextSize(1.5);
    M5.Display.setCursor(4, 2);
    M5.Display.print("ShizzBot");
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.setCursor(4, 22);
    M5.Display.printf("http://%s", ip);
}

void updateOLED() {
    static unsigned long lastUp = 0;
    if (millis() - lastUp < 500) return;
    lastUp = millis();

    int y = 42;
    M5.Display.fillRect(0, y, M5.Display.width(), M5.Display.height() - y, TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(4, y);

    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.printf("L:%4d R:%4d\n", actualOutL, actualOutR);
    M5.Display.setCursor(4, y + 14);
    M5.Display.printf("RPM %4d | %4d\n", telRpmL, telRpmR);
    M5.Display.setCursor(4, y + 28);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.printf("%.1fV  %.1fC", telVoltMV / 1000.0, telTempRaw / 10.0);
    M5.Display.setCursor(4, y + 42);
    M5.Display.setTextColor(driveMode == DRIVE_BALANCE ? TFT_MAGENTA : TFT_GREEN);
    M5.Display.print(driveMode == DRIVE_BALANCE ? "BALANCE" : "SKID");
}

// ===== MOTOR CONTROL ==========================================
void applyMotors(int32_t outL, int32_t outR) {
    actualOutL = outL;
    actualOutR = outR;
    int32_t rpmL = (outL * maxRpm) / 100;
    int32_t rpmR = (outR * maxRpm) / 100;
    if (motorLOk) motorL.setSpeed(rpmL);
    if (motorROk) motorR.setSpeed(rpmR);
}

void emergencyStop() {
    cmdL = 0; cmdR = 0;
    actualOutL = 0; actualOutR = 0;
    balancePID.reset();
    if (motorLOk) { motorL.stop(); motorL.setRGB(255, 0, 0, 60); }
    if (motorROk) { motorR.stop(); motorR.setRGB(255, 0, 0, 60); }
    // Re-enable output after stop so next command works
    delay(10);
    if (motorLOk) { motorL.setMode(ROLLER_MODE_SPEED); motorL.setOutput(true); }
    if (motorROk) { motorR.setMode(ROLLER_MODE_SPEED); motorR.setOutput(true); }
}

// ===== IMU ====================================================
void readIMU() {
    M5.Imu.update(); // Fetch fresh data!
    auto data = M5.Imu.getImuData();
    unsigned long nowUs = micros();
    float dt = (lastImuUs == 0) ? 0 : (nowUs - lastImuUs) / 1000000.0f;
    lastImuUs = nowUs;

    imuFilter.update(data.accel.x, data.accel.y, data.accel.z,
                     data.gyro.x,  data.gyro.y,  data.gyro.z, dt);
    imuPitch = imuFilter.pitch;
    imuRoll  = imuFilter.roll;
}

// ===== SELF-BALANCE LOOP ======================================
void balanceLoop(float dt) {
    // Add joystick steering as differential
    int32_t steer = cmdL;  // in balance mode, UI sends steer in cmdL, throttle in cmdR
    int32_t lean  = cmdR;  // forward lean offset

    // Apply lean offset as setpoint shift (forward lean = robot moves forward)
    float leanAngle = lean * 0.15f;  // max ±15° lean from joystick
    
    // PID computes motor power to maintain upright (pitch ≈ leanAngle)
    // We pass imuFilter.gyroY as the derivative term for a much smoother response
    float pidOut = balancePID.compute(leanAngle, imuPitch, imuFilter.gyroY, dt);

    int32_t outL = (int32_t)pidOut + steer;
    int32_t outR = (int32_t)pidOut - steer;
    outL = constrain(outL, -100, 100);
    outR = constrain(outR, -100, 100);

    // Tip-over protection: if pitch > 45°, give up
    if (fabsf(imuPitch) > 45.0f) {
        outL = 0; outR = 0;
    }

    applyMotors(outL, outR);
}

// ===== FREERTOS BALANCE TASK ==================================
void balanceTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // exactly 5ms (200Hz)

    for (;;) {
        // Always read IMU
        readIMU();

        if (driveMode == DRIVE_BALANCE) {
            balanceLoop(0.005f); // exactly 5ms
        }

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
            data[len] = 0; // null-terminate
            JsonDocument doc;
            if (deserializeJson(doc, (char*)data)) return;

            const char* cmd = doc["c"];
            if (!cmd) return;

            if (strcmp(cmd, "m") == 0) {
                cmdL = constrain((int)doc["l"], -100, 100);
                cmdR = constrain((int)doc["r"], -100, 100);
                lastCmdTime = millis();
                if (driveMode == DRIVE_SKID) {
                    applyMotors(cmdL, cmdR);
                }
                // In balance mode, cmdL/cmdR are used as steer/lean inputs
            }
            else if (strcmp(cmd, "stop") == 0) {
                emergencyStop();
                Serial.println("[!] WS Emergency Stop");
            }
            else if (strcmp(cmd, "cfg") == 0) {
                bool bal = doc["bal"] == 1;
                DriveMode newMode = bal ? DRIVE_BALANCE : DRIVE_SKID;
                if (newMode != driveMode) {
                    driveMode = newMode;
                    balancePID.reset();
                    cmdL = 0; cmdR = 0;
                    if (driveMode == DRIVE_SKID) applyMotors(0, 0);
                    Serial.printf("[Mode] %s\n", bal ? "BALANCE" : "SKID");
                }
                bool invL = doc["invL"] == 1;
                bool invR = doc["invR"] == 1;
                motorL.setInverted(invL);
                motorR.setInverted(invR);
                
                if (doc.containsKey("maxSp")) {
                    maxRpm = (int)doc["maxSp"] * 30; // maxRpm max is 3000
                }
                
                configDirty = true;
            }
            else if (strcmp(cmd, "pid") == 0) {
                balancePID.kp = doc["kp"] | 12.0f;
                balancePID.ki = doc["ki"] | 0.4f;
                balancePID.kd = doc["kd"] | 0.6f;
                Serial.printf("[PID] Kp=%.1f Ki=%.1f Kd=%.1f\n",
                    balancePID.kp, balancePID.ki, balancePID.kd);
                configDirty = true;
            }
            else if (strcmp(cmd, "snd") == 0) {
                const char* s = doc["s"];
                if (s) {
                    if (strcmp(s, "horn") == 0) soundEngine.playHorn();
                    else if (strcmp(s, "chirp") == 0) soundEngine.playChirp();
                    else if (strcmp(s, "scan") == 0) soundEngine.playScan();
                    else if (strcmp(s, "err") == 0) soundEngine.playError();
                }
            }
            else if (strcmp(cmd, "emo") == 0) {
                const char* e = doc["e"];
                if (e) {
                    if (strcmp(e, "happy") == 0) robotFace.setEmotion(RobotFace::HAPPY);
                    else if (strcmp(e, "angry") == 0) robotFace.setEmotion(RobotFace::ANGRY);
                    else if (strcmp(e, "dizzy") == 0) robotFace.setEmotion(RobotFace::DIZZY);
                    else if (strcmp(e, "surprised") == 0) robotFace.setEmotion(RobotFace::SURPRISED);
                    else if (strcmp(e, "idle") == 0) robotFace.setEmotion(RobotFace::IDLE);
                }
            }
        }
    }
}

// Push to websocket AND update WebUI config on connection
void pushTelemetry() {
    if (ws.count() == 0) return;

    char buf[400];
    snprintf(buf, sizeof(buf),
        "{\"v\":%d,\"t\":%d,\"sl\":%d,\"sr\":%d,\"cl\":%d,\"cr\":%d,"
        "\"p\":%d,\"r\":%d,\"m\":\"%s\","
        "\"ip\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,\"wc\":%u,\"batt\":%d,"
        "\"kp\":%.1f,\"ki\":%.1f,\"kd\":%.1f,\"maxSp\":%d,\"invL\":%d,\"invR\":%d}",
        telVoltMV,
        telTempRaw,
        telRpmL, telRpmR,
        telCurL, telCurR,
        (int)(imuPitch * 10),   // send as °*10 int
        (int)(imuRoll * 10),
        driveMode == DRIVE_BALANCE ? "balance" : "skid",
        WiFi.localIP().toString().c_str(),
        WiFi.SSID().c_str(),
        WiFi.RSSI(),
        ws.count(),
        M5.Power.getBatteryLevel(),
        balancePID.kp, balancePID.ki, balancePID.kd,
        maxRpm / 30, // send back as percentage (0-100)
        motorLOk ? motorL.getInverted() : 0,
        motorROk ? motorR.getInverted() : 0
    );
    ws.textAll(buf);
}

// ===== SETUP ==================================================
void setup() {
    auto cfg = M5.config();
    cfg.internal_imu = true;  // enable BMI270
    M5.begin(cfg);
    
    soundEngine.init();
    robotFace.init();
    irControl.init();
    
    soundEngine.playStartup();
    
    // Disable text boot screen, show face instead
    // drawBoot(); 

    Serial.begin(115200);
    Serial.println("\n[ShizzBot v2] Starting...");

    // Load Config
    prefs.begin("shizzbot", false);
    driveMode = (DriveMode)prefs.getInt("driveMode", DRIVE_SKID);
    balancePID.kp = prefs.getFloat("kp", 12.0f);
    balancePID.ki = prefs.getFloat("ki", 0.4f);
    balancePID.kd = prefs.getFloat("kd", 0.6f);
    maxRpm = prefs.getInt("maxRpm", 1500);

    // Default right motor to inverted if not set in prefs (for opposite facing wheels)
    bool defaultInvR = !prefs.isKey("invR") ? true : prefs.getBool("invR");
    bool defaultInvL = prefs.getBool("invL", false);

    // I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);  // 100kHz for stability
    delay(100);

    // Motors
    drawStatus("Motors...");
    motorLOk = motorL.begin();
    motorROk = motorR.begin();
    Serial.printf("[Motors] L@0x%02X:%s  R@0x%02X:%s\n",
        MOTOR_L_ADDR, motorLOk ? "OK" : "FAIL",
        MOTOR_R_ADDR, motorROk ? "OK" : "FAIL");

    if (motorLOk) {
        motorL.setMode(ROLLER_MODE_SPEED); delay(5);
        motorL.setOutput(true); delay(5);
        motorL.setStallProtection(true); delay(5);
        motorL.setMaxSpeed(3000); delay(5);
        motorL.setInverted(defaultInvL);
        motorL.setRGB(0, 60, 0, 20);
    }
    if (motorROk) {
        motorR.setMode(ROLLER_MODE_SPEED); delay(5);
        motorR.setOutput(true); delay(5);
        motorR.setStallProtection(true); delay(5);
        motorR.setMaxSpeed(3000); delay(5);
        motorR.setInverted(defaultInvR);
        motorR.setRGB(0, 60, 0, 20);
    }

    // Start FreeRTOS Balance Task on Core 1 (App Core)
    xTaskCreatePinnedToCore(balanceTask, "Balance", 4096, NULL, 5, NULL, 1);

    // WiFi
    drawStatus("WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
        delay(500); Serial.print("."); tries++;
    }

    String ip;
    if (WiFi.status() == WL_CONNECTED) {
        ip = WiFi.localIP().toString();
        Serial.printf("\n[WiFi] STA IP: %s\n", ip.c_str());
        if (MDNS.begin("shizzbot")) {
            Serial.println("[mDNS] shizzbot.local started");
        }
    } else {
        Serial.println("\n[WiFi] STA fail -> AP mode");
        WiFi.softAP("ShizzBot", "shizzbot123");
        ip = WiFi.softAPIP().toString();
        Serial.printf("[WiFi] AP IP: %s\n", ip.c_str());
    }
    // We use the robot face now instead of text screens
    // drawRunningScreen(ip.c_str());

    // WebSocket
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // HTTP: serve WebUI
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        AsyncWebServerResponse *response = req->beginResponse(200, "text/html", WEBUI_HTML);
        // Prevent aggressive mobile browser caching of the UI
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        req->send(response);
    });

    ElegantOTA.begin(&server);

    server.begin();
    Serial.println("[ShizzBot v2] Ready!");
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
        if (irCmd == 'F') { cmdL = 100; cmdR = 100; }
        else if (irCmd == 'B') { cmdL = -100; cmdR = -100; }
        else if (irCmd == 'L') { cmdL = -100; cmdR = 100; }
        else if (irCmd == 'R') { cmdL = 100; cmdR = -100; }
        else if (irCmd == 'S') { cmdL = 0; cmdR = 0; }
        lastCmdTime = millis();
        if (driveMode == DRIVE_SKID) applyMotors(cmdL, cmdR);
        robotFace.setEmotion(RobotFace::MOVING);
    }

    // FreeRTOS task handles IMU and Balancing now
    // Sound engine
    soundEngine.update();

    // Command timeout safety
    if (millis() - lastCmdTime > CMD_TIMEOUT && (cmdL != 0 || cmdR != 0)) {
        Serial.println("[!] Timeout -> stop");
        emergencyStop();
    }

    // Automatic reverse beep
    if (cmdL < -10 && cmdR < -10) {
        soundEngine.playReverseBeep();
    }

    // Read motor telemetry (throttled to avoid I2C congestion)
    static unsigned long lastTel = 0;
    if (millis() - lastTel > 200) {
        lastTel = millis();
        if (motorLOk) {
            telRpmL = motorL.getSpeedRPM();
            telCurL = motorL.getCurrentMA();
            telVoltMV = motorL.getVoltageMV();
            telTempRaw = motorL.getTempRaw();
        }
        if (motorROk) {
            telRpmR = motorR.getSpeedRPM();
            telCurR = motorR.getCurrentMA();
        }
    }

    // Push telemetry over WebSocket (throttled)
    static unsigned long lastPush = 0;
    if (millis() - lastPush > 150) {
        lastPush = millis();
        pushTelemetry();
    }

    // OLED Face Update
    robotFace.update(actualOutL, actualOutR);
    // updateOLED();

    // WebSocket cleanup
    static unsigned long lastClean = 0;
    if (millis() - lastClean > 1000) {
        lastClean = millis();
        ws.cleanupClients();
    }

    // Save Config to Flash (non-blocking in main loop)
    if (configDirty) {
        configDirty = false;
        prefs.putInt("driveMode", driveMode);
        prefs.putFloat("kp", balancePID.kp);
        prefs.putFloat("ki", balancePID.ki);
        prefs.putFloat("kd", balancePID.kd);
        prefs.putInt("maxRpm", maxRpm);
        if (motorLOk) prefs.putBool("invL", motorL.getInverted());
        if (motorROk) prefs.putBool("invR", motorR.getInverted());
        Serial.println("[NVS] Config saved to Flash");
    }

    ElegantOTA.loop();

    delay(10); // Telemetry/WiFi loop can be relaxed now that balancing is in FreeRTOS
}
