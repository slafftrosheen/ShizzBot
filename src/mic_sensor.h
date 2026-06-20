#pragma once
#include <M5Unified.h>

class MicSensor {
public:
    bool loudNoiseDetected = false;
    int noiseLevel = 0; // 0-100

    void init() {
        auto cfg = M5.Mic.config();
        cfg.sample_rate = 16000;
        M5.Mic.config(cfg);
        M5.Mic.begin();
        Serial.println("[MIC] Initialized");
    }

    void update() {
        loudNoiseDetected = false;
        
        // Only sample occasionally to save CPU and not block speaker
        static unsigned long lastSample = 0;
        if (millis() - lastSample < 100) return;
        lastSample = millis();

        if (M5.Mic.isEnabled()) {
            int16_t micData[256];
            size_t bytes_read = M5.Mic.record(micData, 256, 16000);
            
            if (bytes_read > 0) {
                // Calculate RMS
                int64_t sumSq = 0;
                for (size_t i = 0; i < bytes_read; i++) {
                    sumSq += micData[i] * micData[i];
                }
                int rms = sqrt(sumSq / bytes_read);
                
                // Map to 0-100% roughly
                noiseLevel = constrain(rms / 10, 0, 100);

                if (noiseLevel > 70) {
                    loudNoiseDetected = true;
                }
            }
        }
    }
};
