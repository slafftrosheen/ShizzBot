// ============================================================
// ShizzBot - IMU Filter + Heading Tracker
// Complementary filter for pitch/roll from BMI270 (via M5Unified)
// Gyro-integrated heading (yaw) for relative compass
// ============================================================
#pragma once
#include <math.h>

class IMUFilter {
public:
    float pitch = 0;   // degrees, + = tilted forward
    float roll  = 0;   // degrees, + = tilted right
    float heading = 0; // degrees 0-360, relative to startup orientation
    float gyroRatePitch = 0; // raw gyro rate (dps) for telemetry

    // Gyro bias calibration (computed at startup)
    float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;
    bool calibrated = false;

    // Calibrate gyro by averaging samples while stationary
    // Call this during setup() when the robot is NOT moving
    void calibrate(float gx, float gy, float gz) {
        _calSamples++;
        _calSumX += gx;
        _calSumY += gy;
        _calSumZ += gz;
        if (_calSamples >= CAL_COUNT) {
            gyroBiasX = _calSumX / _calSamples;
            gyroBiasY = _calSumY / _calSamples;
            gyroBiasZ = _calSumZ / _calSamples;
            calibrated = true;
            Serial.printf("[IMU] Gyro calibrated: bias X=%.3f Y=%.3f Z=%.3f\n",
                          gyroBiasX, gyroBiasY, gyroBiasZ);
        }
    }

    void update(float ax, float ay, float az,
                float gx, float gy, float gz,
                float dt) {

        // Subtract gyro bias if calibrated
        if (calibrated) {
            gx -= gyroBiasX;
            gy -= gyroBiasY;
            gz -= gyroBiasZ;
        }

        gyroRatePitch = gx;

        // Protection against freefall
        float accelMag = sqrtf(ax*ax + ay*ay + az*az);
        if (accelMag < 0.1f) {
            pitch += gx * dt;
            roll  += gz * dt;
            heading += gy * dt; // yaw from gyro Y (vertical mount)
            _wrapHeading();
            return;
        }

        // Accel-based angles for vertical orientation (Y is down)
        float accelPitch = atan2f(az, ay) * RAD2DEG;
        float accelRoll  = atan2f(ax, ay) * RAD2DEG;

        if (dt <= 0 || dt > 0.5f) {
            pitch = accelPitch;
            roll  = accelRoll;
            return;
        }

        // Complementary filter: trust gyro short-term, accel long-term
        pitch = ALPHA * (pitch + gx * dt) + (1.0f - ALPHA) * accelPitch;
        roll  = ALPHA * (roll  + gz * dt) + (1.0f - ALPHA) * accelRoll;

        // Heading: pure gyro integration (no accel correction for yaw)
        // gy is the yaw rate when StickS3 is mounted vertically
        heading += gy * dt;
        _wrapHeading();
    }

    // Zero the heading (called from UI "Calibrate" button)
    void resetHeading() {
        heading = 0;
        Serial.println("[IMU] Heading zeroed");
    }

private:
    static constexpr float ALPHA   = 0.96f;
    static constexpr float RAD2DEG = 57.2957795f;

    // Gyro calibration accumulators
    static const int CAL_COUNT = 100;
    int _calSamples = 0;
    float _calSumX = 0, _calSumY = 0, _calSumZ = 0;

    void _wrapHeading() {
        heading = fmodf(heading, 360.0f);
        if (heading < 0) heading += 360.0f;
    }
};
