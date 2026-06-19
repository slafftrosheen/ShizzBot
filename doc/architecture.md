# ShizzBot Architecture

## System Overview

ShizzBot uses a **distributed multi-node architecture** where specialized M5Stack devices handle specific tasks, orchestrated by a Linux SBC brain.

```
┌─────────────────────────────────────────────────────────────────┐
│                     HOME WiFi NETWORK                          │
│                                                                 │
│  ┌──────────────┐     ┌──────────────┐     ┌───────────────┐  │
│  │ Phone/Tablet │◄───►│  NanoPi Neo3 │◄───►│  CoreS3 SE    │  │
│  │  WebUI 🎮    │     │  Main Brain  │     │  Robot Face   │  │
│  └──────────────┘     │  (ROS2, AI)  │     │  (Expressions)│  │
│                       └──────┬───────┘     └───────────────┘  │
│                              │ Serial/WiFi                     │
│                       ┌──────┴───────┐                        │
│                       │  M5StickS3   │                        │
│                       │  Low-Level   │                        │
│                       │  Controller  │                        │
│                       └──────┬───────┘                        │
│                         I2C (G9/G10)                          │
│                      ┌───────┴───────┐                        │
│               ┌──────┴──┐     ┌──────┴──┐                    │
│               │Roller   │     │Roller   │                    │
│               │CAN 0x64 │     │CAN 0x65 │                    │
│               │Left     │     │Right    │                    │
│               └────┬────┘     └────┬────┘                    │
│                  ⚙️🛞              ⚙️🛞                       │
│                100mm             100mm                        │
└─────────────────────────────────────────────────────────────────┘
```

## Communication Protocols

| Link | Protocol | Purpose |
|---|---|---|
| M5StickS3 ↔ RollerCAN | I2C (400kHz) | Motor speed commands, telemetry readback |
| Phone ↔ M5StickS3 | HTTP/WiFi | WebUI, REST API for motor commands |
| NanoPi ↔ M5StickS3 | Serial UART | High-level commands → low-level execution |
| NanoPi ↔ CoreS3 SE | WiFi/Serial | Face expressions, UI sync |
| C6L Meshtastic | LoRa 868/915MHz | Out-of-band telemetry, mesh control |

## I2C Bus (Port A)

| Address | Device | Function |
|---|---|---|
| `0x64` | RollerCAN #1 | Left motor (Speed Mode) |
| `0x65` | RollerCAN #2 | Right motor (Speed Mode) |
| `0x76`* | BME680 | Temp/Humidity/Air Quality (Phase 2) |
| `0x20`* | 8-Ch Servo | Pan/tilt, manipulators (Phase 4) |

*Future additions — may require I2C hub

## Peripheral Expansion (Planned)

| Device | Interface | Connected To | Phase |
|---|---|---|---|
| BME680 | I2C | M5StickS3 Port A | 2 |
| MR24HPC1 Radar | UART | M5StickS3 or CoreS3 | 3 |
| 8-Ch Servo Controller | I2C | M5StickS3 Port A | 4 |
| AtomS3R-CAM | WiFi Stream | NanoPi Neo3 | 5 |
| CoreS3 SE | WiFi/UART | NanoPi Neo3 | 6 |
| NanoPi Neo3 | Serial/WiFi | M5StickS3 | 7 |
| C6L Meshtastic | LoRa | Standalone | 8 |

## Safety Features

1. **Command Timeout**: Motors auto-stop after 2 seconds without a new command
2. **Physical E-Stop**: BtnA on M5StickS3 immediately halts all motors
3. **Speed Limiter**: WebUI slider caps maximum speed percentage
4. **Stall Protection**: Enabled on both RollerCAN units via firmware
5. **AP Fallback**: If home WiFi fails, creates its own "ShizzBot" AP
