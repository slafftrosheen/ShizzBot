// ============================================================
// ShizzBot - Servo Controller (Continuous Rotation)
// Native ESP32 LEDC implementation (Bypasses ESP32Servo bugs)
// All servos are CONTINUOUS ROTATION type:
//   PWM 1500µs (90°) = STOP
//   PWM 1000µs (0°)  = Full speed CW
//   PWM 2000µs (180°) = Full speed CCW
// ============================================================
#pragma once
#include <Arduino.h>

// Hat2-Bus GPIO assignments
#define SERVO_STEER_PIN    4   // G4 - Steering
#define SERVO_PAN_PIN      5   // G5 - Pan (Left/Right)
#define SERVO_TILT_PIN     6   // G6 - Tilt (Up/Down)

// LEDC Channels (using high channels to avoid M5Unified conflicts)
#define LEDC_CH_STEER 5
#define LEDC_CH_PAN   6
#define LEDC_CH_TILT  7

class ServoController {
public:
    int steerSpeed   = 0;
    int panSpeed     = 0;
    int tiltSpeed    = 0;

    bool invertSteer = false;
    int maxServoSpeed = 70; // 0-100%

    unsigned long lastServoCmd = 0;
    static const unsigned long SERVO_TIMEOUT_MS = 1500;

    void init() {
        // Use native ESP32 LEDC API instead of ESP32Servo to fix timer hangs
        // 50Hz frequency, 14-bit resolution (0-16383)
        ledcSetup(LEDC_CH_STEER, 50, 14);
        ledcAttachPin(SERVO_STEER_PIN, LEDC_CH_STEER);

        ledcSetup(LEDC_CH_PAN, 50, 14);
        ledcAttachPin(SERVO_PAN_PIN, LEDC_CH_PAN);

        ledcSetup(LEDC_CH_TILT, 50, 14);
        ledcAttachPin(SERVO_TILT_PIN, LEDC_CH_TILT);

        stopAll();
        lastServoCmd = millis();
        Serial.println("[Servo] 3x native LEDC servos initialized (G4-G6)");
    }

    // Convert -100..+100 speed to 14-bit duty cycle for 50Hz (20000us period)
    static uint32_t speedToDuty(int speed, int maxPct) {
        speed = constrain(speed, -100, 100);
        int limited = (speed * maxPct) / 100;
        int us = 1500 + (limited * 5); // 1000 to 2000 us
        // Duty cycle: (us / 20000) * 16384
        return (us * 16384) / 20000;
    }

    void setSteer(int input) {
        input = constrain(input, -100, 100);
        if (invertSteer) input = -input;
        steerSpeed = input;
        ledcWrite(LEDC_CH_STEER, speedToDuty(input, maxServoSpeed));
        lastServoCmd = millis();
    }

    void setPan(int speed) {
        panSpeed = constrain(speed, -100, 100);
        ledcWrite(LEDC_CH_PAN, speedToDuty(panSpeed, maxServoSpeed));
        lastServoCmd = millis();
    }

    void setTilt(int speed) {
        tiltSpeed = constrain(speed, -100, 100);
        ledcWrite(LEDC_CH_TILT, speedToDuty(tiltSpeed, maxServoSpeed));
        lastServoCmd = millis();
    }

    void setPanTilt(int pan, int tilt) {
        setPan(pan);
        setTilt(tilt);
    }

    void stopAll() {
        steerSpeed = 0;
        panSpeed = 0;
        tiltSpeed = 0;
        ledcWrite(LEDC_CH_STEER, speedToDuty(0, 100));
        ledcWrite(LEDC_CH_PAN, speedToDuty(0, 100));
        ledcWrite(LEDC_CH_TILT, speedToDuty(0, 100));
    }

    void presetCenter() { setPanTilt(0, 0); }
    void presetScan() { setPan(60); }
    void presetNod() { setTilt(50); }
    void presetShake() { setPan(80); }

    void update() {
        if (millis() - lastServoCmd > SERVO_TIMEOUT_MS) {
            if (steerSpeed != 0 || panSpeed != 0 || tiltSpeed != 0) {
                Serial.println("[Servo] Timeout -> stopping all servos");
                stopAll();
            }
        }
    }
};
