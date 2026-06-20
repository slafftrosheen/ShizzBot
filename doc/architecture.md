# ShizzBot Swarm Architecture

## System Overview

ShizzBot Swarm is a **2-robot mini-swarm** where each robot is independently controllable via a phone WebUI, and coordinated by a future Command Center.

```
┌──────────────────────────────────────────────────────────────┐
│                   HOME WiFi (Sinstro_HomeLab)                │
│                                                              │
│  ┌──────────┐   ┌──────────┐   ┌──────────────────────┐     │
│  │ Phone A  │   │ Phone B  │   │  Command Center      │     │
│  │ WebUI    │   │ WebUI    │   │  (CoreS3+CAM)        │     │
│  │→buzz.local│  │→rex.local│   │  Phase 2 (future)    │     │
│  └────┬─────┘   └────┬─────┘   └──────────┬───────────┘     │
│       │WS            │WS                   │HTTP/WS          │
│  ┌────▼─────┐   ┌────▼─────┐               │                │
│  │ ROBOT A  │   │ ROBOT B  │◄──────────────┘                │
│  │ StickS3  │   │ StickS3  │                                 │
│  └──┬──┬────┘   └──┬──┬────┘                                │
│  I2C│  │PWM     I2C│  │PWM                                  │
│  ┌──▼┐ │G4-G7  ┌──▼┐ │G4-G7                                │
│  │CAN│ ├─Steer │CAN│ ├─Steer                               │
│  │0x64│├─Arm1  │0x64│├─Arm1                                │
│  └──┬┘ ├─Arm2  └──┬┘ ├─Arm2                                │
│   🛞🛞 └─Grip   🛞🛞 └─Grip                                │
└──────────────────────────────────────────────────────────────┘
```

## Per-Robot Hardware

| Component | Interface | Pin | Notes |
|---|---|---|---|
| M5StickS3 (ESP32-S3) | — | — | Brain, LCD, IMU (BMI270), WiFi |
| RollerCAN Motor | I2C | G9 (SDA), G10 (SCL) | Rear axle drive, addr 0x64 |
| Steering Servo | PWM (LEDC) | G4 | Continuous rotation |
| Arm Base Servo | PWM (LEDC) | G5 | Continuous rotation |
| Arm Lift Servo | PWM (LEDC) | G6 | Continuous rotation |
| Gripper Servo | PWM (LEDC) | G7 | Continuous rotation |

> All servos are continuous rotation type. Power comes from external 5V powerbank. Only signal wires connect to StickS3 GPIOs (Hat2-Bus header).

## Communication Protocols

| Link | Protocol | Purpose |
|---|---|---|
| StickS3 ↔ RollerCAN | I2C (100kHz) | Motor speed commands, telemetry |
| Phone ↔ StickS3 | WebSocket/WiFi | 1:1 drive control |
| StickS3 ↔ StickS3 | mDNS discovery | Swarm awareness |
| Command Center ↔ Robots | HTTP/WebSocket | Coordination (Phase 2) |
| C6L Meshtastic | LoRa | Out-of-band I/O (Phase 3) |

## Robot Identity

Each robot has a unique:
- **Name**: Stored in NVS, editable from WebUI (e.g., "Buzz", "Rex")
- **Color Theme**: Cyber Blue, Neon Pink, Hacker Green, or Blaze Orange
- **mDNS Hostname**: Auto-derived from name (e.g., `buzz.local`)

## Orientation System

- **Pitch/Roll**: Complementary filter on BMI270 accel+gyro (gravity-corrected)
- **Heading (Yaw)**: Pure gyro Z integration. Drifts over time, but sufficient for relative commands ("turn 90° left"). Reset via "Zero Heading" button in WebUI.
- **Gyro Bias Calibration**: Auto-calibrated at startup (100 samples while stationary)

## Safety Features

1. **Motor Command Timeout**: Motor auto-stops after 1.5 seconds without a new command
2. **Servo Auto-Stop Timeout**: All servos stop after 1.5 seconds without commands
3. **Servo Speed Limiter**: Adjustable cap on maximum continuous rotation speed (default 70%)
4. **Physical E-Stop**: BtnA on StickS3 immediately halts motor + all servos
5. **Speed Limiter**: WebUI slider caps maximum motor speed
6. **Stall Protection**: Enabled on RollerCAN via firmware
7. **AP Fallback**: If home WiFi fails, creates its own "ShizzBot" AP

## Future Phases

| Phase | Feature | Hardware |
|---|---|---|
| 2 | Command Center UI | CoreS3 + CAM |
| 3 | Mesh control channel | C6L Meshtastic |
| 4 | Vision / obstacle avoidance | Pi Zero + Camera per robot |
| 5 | Additional sensors | Pi Pico co-processor per robot |
