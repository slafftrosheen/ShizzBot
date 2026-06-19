# ShizzBot 🤖💨

> **ShizzBot** (from Russian «шиз» — "crazy active") is a dad+son Lego Technics + M5Stack robotics project: an autonomous, remote-controlled robot with WiFi WebUI, AI chatbot, and Meshtastic mesh networking.

---

## 🏗️ Current Phase: Phase 1 — Drive & Control

| Component | Role |
|---|---|
| **M5StickS3** | Low-level controller, WiFi host, WebUI server |
| **2× Unit-RollerCAN** | Brushless FOC motors at I2C `0x64` / `0x65` via Port A (G9, G10) |
| **100mm wheels** | Lego Technics chassis |

### What Works
- ✅ Tank-style differential drive via virtual joystick (touch + mouse)
- ✅ Real-time telemetry (voltage, temperature, RPM)
- ✅ Quick actions: Forward, Spin, Emergency Stop
- ✅ Max speed limiter (safety slider for kids!)
- ✅ Auto-stop timeout if connection lost
- ✅ Physical emergency stop via M5Stick BtnA
- ✅ WiFi STA mode with AP fallback

---

## 🗺️ Roadmap

| Phase | Components | Features |
|---|---|---|
| **1** ✅ | M5StickS3, 2× RollerCAN | WiFi WebUI, drive control |
| **2** | BME680 | Environment sensing (temp/hum/air quality) |
| **3** | MR24HPC1 radar | Human/breath detection |
| **4** | 8-ch Servo Controller | Pan/tilt camera, manipulators |
| **5** | AtomS3R-CAM | Camera streaming, computer vision |
| **6** | CoreS3 SE | Robot "face", expressions, touch UI |
| **7** | NanoPi Neo3 2GB | Main brain, ROS2, AI chatbot |
| **8** | C6L Meshtastic | LoRa mesh telemetry & control |

---

## 📁 Project Structure

```
ShizzBot/
├── src/
│   ├── main.cpp          # Main firmware (WiFi, WebServer, control loop)
│   ├── roller_can.h      # I2C driver for Unit-RollerCAN
│   └── webui.h           # Embedded HTML/CSS/JS WebUI
├── doc/
│   └── architecture.md   # System architecture
├── deps/                 # Custom local libraries
├── platformio.ini        # Build configuration
└── README.md
```

## 🚀 Quick Start

### Prerequisites
- [PlatformIO](https://platformio.org/) (VSCode extension recommended)
- M5StickS3 connected via USB-C

### Build & Flash
1. Open this folder in VSCode with PlatformIO
2. Edit `src/main.cpp` and set your WiFi credentials:
   ```cpp
   const char* WIFI_SSID = "YOUR_WIFI_SSID";
   const char* WIFI_PASS = "YOUR_WIFI_PASS";
   ```
3. Build & Upload (PlatformIO toolbar or `pio run -t upload`)
4. Open Serial Monitor at 115200 baud to see the assigned IP
5. Navigate to `http://<IP>` on your phone/tablet — drive ShizzBot! 🎮

### WebUI Features
- 🕹️ **Virtual joystick** — drag to drive (tank-style differential mixing)
- 📡 **Live telemetry** — voltage, motor temp, actual RPM
- 🎚️ **Speed limiter** — safety slider to cap max speed
- 🛑 **Emergency stop** — big red button or press BtnA on the M5Stick
- 🌀 **Quick actions** — one-tap forward, spin moves

---

## 🔌 Hardware Connections

```
M5StickS3 Port A
├── G9  (SDA) ──► RollerCAN #1 (0x64) ──► Left Motor
└── G10 (SCL) ──► RollerCAN #2 (0x65) ──► Right Motor

Power: 6-16V via XT30 to RollerCAN units
```

## 📝 License

This is a personal hobby project. Have fun and build robots! 🤖
