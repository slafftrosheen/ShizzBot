// ============================================================
// ShizzBot - IMU Filter + Self-Balancing PID
// Complementary filter for pitch from BMI270 (via M5Unified)
// ============================================================
#pragma once
#include <math.h>

// ---- Complementary Filter for pitch estimation ----
class IMUFilter {
public:
    float pitch = 0;   // degrees, + = tilted forward
    float roll  = 0;   // degrees, + = tilted right
    float gyroRatePitch = 0;   // raw gyro rate (dps) around X (pitch)

    void update(float ax, float ay, float az,
                float gx, float gy, float gz,
                float dt) {
        
        // Save raw gyro for PID derivative (X is pitch in vertical orientation)
        gyroRatePitch = gx;
        // Protection against freefall (accel magnitude near 0)
        float accelMag = sqrtf(ax*ax + ay*ay + az*az);
        if (accelMag < 0.1f) {
            // In freefall or bump, just trust gyro
            pitch = pitch + gx * dt;
            roll  = roll  + gz * dt;
            return;
        }

        // Accel-based angles for vertical orientation (Y is down)
        float accelPitch = atan2f(az, ay) * RAD2DEG;
        float accelRoll  = atan2f(ax, ay) * RAD2DEG;

        if (dt <= 0 || dt > 0.5f) {
            // First run or bogus dt — just trust accel
            pitch = accelPitch;
            roll  = accelRoll;
            return;
        }

        // Complementary filter: trust gyro short-term, accel long-term
        pitch = ALPHA * (pitch + gx * dt) + (1.0f - ALPHA) * accelPitch;
        roll  = ALPHA * (roll  + gz * dt) + (1.0f - ALPHA) * accelRoll;
    }

private:
    static constexpr float ALPHA   = 0.96f;
    static constexpr float RAD2DEG = 57.2957795f;
};

// ---- Simple PID controller ----
class PIDController {
public:
    float kp = 12.0f;
    float ki = 0.4f;
    float kd = 0.6f;

    // currentRate is the raw gyroscope value (e.g. gyroY for pitch)
    // We use it directly as the derivative to avoid amplifying noise from the error signal.
    float compute(float setpoint, float current, float currentRate, float dt) {
        float error = setpoint - current;

        // Anti-windup: clamp integral
        _integral += error * dt;
        _integral = clampf(_integral, -MAX_INTEGRAL, MAX_INTEGRAL);

        // Instead of derivative = d(error)/dt, we use -currentRate (-gyro).
        // Since error = setpoint - current, d(error)/dt = - d(current)/dt (if setpoint is constant)
        // d(current)/dt is exactly the gyro rate.
        float derivative = -currentRate;

        float output = kp * error + ki * _integral + kd * derivative;
        return clampf(output, -100.0f, 100.0f);
    }

    void reset() {
        _integral = 0;
        _prevError = 0;
    }

private:
    float _integral  = 0;
    float _prevError = 0;
    static constexpr float MAX_INTEGRAL = 200.0f;

    float clampf(float v, float lo, float hi) {
        return (v < lo) ? lo : (v > hi) ? hi : v;
    }
};
