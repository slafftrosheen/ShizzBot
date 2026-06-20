// ============================================================
// ShizzBot - Servo Controller (Continuous Rotation)
// All servos are CONTINUOUS ROTATION type:
//   PWM 1500µs (90°) = STOP
//   PWM 1000µs (0°)  = Full speed CW
//   PWM 2000µs (180°) = Full speed CCW
//
// Steering: joystick X → rotation speed (auto-stops at center)
// Arm servos: slider → rotation speed (auto-stops at center)
//
// Safety: all servos auto-stop on command timeout
// ============================================================
#pragma once
#include <ESP32Servo.h>

// Hat2-Bus GPIO assignments
#define SERVO_STEER_PIN    4   // G4 - Steering
#define SERVO_ARM_BASE     5   // G5 - Arm base rotation
#define SERVO_ARM_LIFT     6   // G6 - Arm lift/lower
#define SERVO_GRIPPER      7   // G7 - Gripper open/close

class ServoController {
public:
    // Current speed values: -100..+100 (0 = stopped)
    int steerSpeed   = 0;
    int armBaseSpeed  = 0;
    int armLiftSpeed  = 0;
    int gripSpeed     = 0;

    bool invertSteer = false;

    // Max allowed speed as percentage of full range (safety limiter)
    int maxServoSpeed = 70; // 0-100, caps how fast continuous servos spin

    // Timeout: auto-stop if no command received within this period
    unsigned long lastServoCmd = 0;
    static const unsigned long SERVO_TIMEOUT_MS = 1500;

    void init() {
        // Allocate LEDC timers for all 4 servo PWM channels
        ESP32PWM::allocateTimer(0);
        ESP32PWM::allocateTimer(1);
        ESP32PWM::allocateTimer(2);
        ESP32PWM::allocateTimer(3);

        _steer.setPeriodHertz(50);
        _armBase.setPeriodHertz(50);
        _armLift.setPeriodHertz(50);
        _grip.setPeriodHertz(50);

        _steer.attach(SERVO_STEER_PIN, 500, 2500);
        _armBase.attach(SERVO_ARM_BASE, 500, 2500);
        _armLift.attach(SERVO_ARM_LIFT, 500, 2500);
        _grip.attach(SERVO_GRIPPER, 500, 2500);

        // All servos to STOP
        stopAll();
        lastServoCmd = millis();
        Serial.println("[Servo] 4x continuous rotation servos initialized (G4-G7)");
    }

    // Convert -100..+100 speed to servo PWM microseconds
    // 0 → 1500µs (stop), -100 → 1000µs (full CW), +100 → 2000µs (full CCW)
    static int speedToMicros(int speed, int maxPct) {
        speed = constrain(speed, -100, 100);
        // Apply speed limiter
        int limited = (speed * maxPct) / 100;
        // Map: -100→1000µs, 0→1500µs, +100→2000µs
        return 1500 + (limited * 5); // 500µs range each direction
    }

    // Steering input from joystick X-axis: -100 (full left) to +100 (full right)
    void setSteer(int input) {
        input = constrain(input, -100, 100);
        if (invertSteer) input = -input;
        steerSpeed = input;
        _steer.writeMicroseconds(speedToMicros(input, maxServoSpeed));
        lastServoCmd = millis();
    }

    // Arm servo speeds: -100..+100 each (0 = stop)
    void setArmBase(int speed) {
        armBaseSpeed = constrain(speed, -100, 100);
        _armBase.writeMicroseconds(speedToMicros(armBaseSpeed, maxServoSpeed));
        lastServoCmd = millis();
    }

    void setArmLift(int speed) {
        armLiftSpeed = constrain(speed, -100, 100);
        _armLift.writeMicroseconds(speedToMicros(armLiftSpeed, maxServoSpeed));
        lastServoCmd = millis();
    }

    void setGripper(int speed) {
        gripSpeed = constrain(speed, -100, 100);
        _grip.writeMicroseconds(speedToMicros(gripSpeed, maxServoSpeed));
        lastServoCmd = millis();
    }

    // Set all arm servos at once
    void setArm(int base, int lift, int grip) {
        setArmBase(base);
        setArmLift(lift);
        setGripper(grip);
    }

    // Emergency: stop all servos instantly
    void stopAll() {
        steerSpeed = 0;
        armBaseSpeed = 0;
        armLiftSpeed = 0;
        gripSpeed = 0;
        _steer.writeMicroseconds(1500);
        _armBase.writeMicroseconds(1500);
        _armLift.writeMicroseconds(1500);
        _grip.writeMicroseconds(1500);
    }

    // ===== PRESETS (timed bursts for continuous rotation) =====
    // These briefly spin servos then stop. For kids: tap a button → arm does a thing.

    // Park: stop all arm servos
    void presetPark() {
        setArm(0, 0, 0);
    }

    // Grab: lower arm + close gripper for ~400ms
    void presetGrab() {
        setArmLift(-60);
        setGripper(70);
        // The timeout system will auto-stop after SERVO_TIMEOUT_MS
        // Or the UI can send a stop. For quick action, we use a short burst approach.
    }

    // Wave: rotate base back and forth
    void presetWave() {
        setArmBase(80);
        setArmLift(50);
    }

    // Release: open gripper
    void presetRelease() {
        setGripper(-70);
    }

    // Call from loop() — auto-stops servos if no command received recently
    void update() {
        if (millis() - lastServoCmd > SERVO_TIMEOUT_MS) {
            if (steerSpeed != 0 || armBaseSpeed != 0 ||
                armLiftSpeed != 0 || gripSpeed != 0) {
                Serial.println("[Servo] Timeout -> stopping all servos");
                stopAll();
            }
        }
    }

private:
    Servo _steer;
    Servo _armBase;
    Servo _armLift;
    Servo _grip;
};
