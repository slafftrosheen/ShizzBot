// ============================================================
// ShizzBot - RollerCAN Motor Driver v2
// I2C register-level driver for M5Stack Unit-RollerCAN
// Fixed: unsigned bit-shift, I2C timing, error handling
// ============================================================
#pragma once
#include <Wire.h>

// --- RollerCAN I2C Register Map ---
#define ROLLER_REG_MODE         0x00
#define ROLLER_REG_OUTPUT       0x01
#define ROLLER_REG_STALL_PROT   0x02

#define ROLLER_REG_SPEED_SET    0x40   // int32, actual = val/100
#define ROLLER_REG_SPEED_MAX    0x44
#define ROLLER_REG_SPEED_READ   0x60   // R/O
#define ROLLER_REG_POS_READ     0x64   // R/O
#define ROLLER_REG_CURRENT_READ 0x68   // int32, actual = val/100 (mA)
#define ROLLER_REG_VIN_READ     0x6C   // int32, raw mV
#define ROLLER_REG_TEMP_READ    0x70   // int32, actual = val/10 (°C)

#define ROLLER_REG_RGB          0x10   // 4 bytes: R, G, B, Brightness

enum RollerMode : uint8_t {
    ROLLER_MODE_SPEED    = 1,
    ROLLER_MODE_POSITION = 2,
    ROLLER_MODE_CURRENT  = 3,
    ROLLER_MODE_ENCODER  = 4,
};

class RollerCAN {
public:
    RollerCAN(uint8_t addr, TwoWire &wire = Wire)
        : _addr(addr), _wire(&wire), _connected(false), _inverted(false) {}

    bool begin() {
        _wire->beginTransmission(_addr);
        _connected = (_wire->endTransmission() == 0);
        return _connected;
    }

    bool isConnected() const { return _connected; }

    // ---- Configuration ----
    void setMode(RollerMode mode) {
        writeReg8(ROLLER_REG_MODE, (uint8_t)mode);
        delay(2);
    }

    void setOutput(bool enable) {
        writeReg8(ROLLER_REG_OUTPUT, enable ? 1 : 0);
        delay(2);
    }

    void setStallProtection(bool enable) {
        writeReg8(ROLLER_REG_STALL_PROT, enable ? 1 : 0);
        delay(2);
    }

    void setInverted(bool inv) { _inverted = inv; }
    bool getInverted() const { return _inverted; }

    // ---- Speed Control ----
    // speed: RPM. Applies inversion if set.
    void setSpeed(int32_t rpm) {
        int32_t actual = _inverted ? -rpm : rpm;
        writeReg32(ROLLER_REG_SPEED_SET, actual * 100);
    }

    void setMaxSpeed(int32_t maxRpm) {
        writeReg32(ROLLER_REG_SPEED_MAX, maxRpm * 100);
    }

    // ---- Telemetry (all return human-readable values) ----
    int32_t getSpeedRPM() {
        int32_t raw = readReg32(ROLLER_REG_SPEED_READ);
        int32_t rpm = raw / 100;
        return _inverted ? -rpm : rpm;
    }

    int32_t getCurrentMA() {
        return readReg32(ROLLER_REG_CURRENT_READ) / 100;
    }

    // Returns voltage in millivolts (e.g. 12400 = 12.4V)
    int32_t getVoltageMV() {
        return readReg32(ROLLER_REG_VIN_READ);
    }

    // Returns temperature in °C * 10 (e.g. 352 = 35.2°C)
    int32_t getTempRaw() {
        return readReg32(ROLLER_REG_TEMP_READ);
    }

    // ---- RGB LED ----
    void setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness = 50) {
        uint8_t buf[4] = { r, g, b, brightness };
        _wire->beginTransmission(_addr);
        _wire->write(ROLLER_REG_RGB);
        _wire->write(buf, 4);
        _wire->endTransmission();
        delay(1);
    }

    // ---- Emergency ----
    void stop() {
        writeReg32(ROLLER_REG_SPEED_SET, 0);
        delay(2);
        writeReg8(ROLLER_REG_OUTPUT, 0);
    }

    uint8_t getAddress() const { return _addr; }

private:
    uint8_t  _addr;
    TwoWire *_wire;
    bool     _connected;
    bool     _inverted;

    void writeReg8(uint8_t reg, uint8_t val) {
        _wire->beginTransmission(_addr);
        _wire->write(reg);
        _wire->write(val);
        _wire->endTransmission();
    }

    void writeReg32(uint8_t reg, int32_t val) {
        uint8_t data[4];
        data[0] = (uint8_t)((val >>  0) & 0xFF);
        data[1] = (uint8_t)((val >>  8) & 0xFF);
        data[2] = (uint8_t)((val >> 16) & 0xFF);
        data[3] = (uint8_t)((val >> 24) & 0xFF);
        _wire->beginTransmission(_addr);
        _wire->write(reg);
        _wire->write(data, 4);
        _wire->endTransmission();
    }

    // FIXED: use uint32_t intermediates to prevent sign-extension bugs on <<24
    int32_t readReg32(uint8_t reg) {
        _wire->beginTransmission(_addr);
        _wire->write(reg);
        if (_wire->endTransmission(false) != 0) return 0;
        _wire->requestFrom(_addr, (uint8_t)4);
        if (_wire->available() < 4) return 0;
        uint32_t val = 0;
        val  = (uint32_t)_wire->read();
        val |= (uint32_t)_wire->read() << 8;
        val |= (uint32_t)_wire->read() << 16;
        val |= (uint32_t)_wire->read() << 24;
        return (int32_t)val;
    }
};
