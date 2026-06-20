#pragma once
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

class BME680Sensor {
public:
    Adafruit_BME680 bme;
    bool ok = false;
    float temp = 0.0f;
    float hum = 0.0f;
    float pres = 0.0f;
    float gas = 0.0f; // kOhms
    
    // Gas resistance baseline for air quality calc
    float gasBaseline = 50.0f; 
    int airQuality = 100; // 0-100% (100 is best)

    void init() {
        // Try 0x76 first (common default), then 0x77
        if (!bme.begin(0x76)) {
            if (!bme.begin(0x77)) {
                Serial.println("[BME680] Could not find a valid BME680 sensor, check wiring!");
                ok = false;
                return;
            }
        }
        ok = true;
        
        // Setup oversampling and filter initialization
        bme.setTemperatureOversampling(BME680_OS_8X);
        bme.setHumidityOversampling(BME680_OS_2X);
        bme.setPressureOversampling(BME680_OS_4X);
        bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme.setGasHeater(320, 150); // 320*C for 150 ms
        
        Serial.println("[BME680] Initialized successfully");
    }
    
    void update() {
        if (!ok) return;
        
        // performReading is blocking for ~150ms. 
        // In a real application, we should read async, but since this runs in a separate task
        // we can block safely here.
        if (!bme.performReading()) {
            return;
        }
        
        temp = bme.temperature;
        hum = bme.humidity;
        pres = bme.pressure / 100.0; // hPa
        gas = bme.gas_resistance / 1000.0; // kOhms

        // Simple AQI heuristic based on gas resistance
        if (gas > gasBaseline) gasBaseline = gas; // Calibrate upward
        
        float ratio = gas / gasBaseline; // 0.0 to 1.0
        airQuality = constrain((int)(ratio * 100.0f), 0, 100);
    }
};
