# ⚡ Watt's Up — Smart Energy Monitor

> **Smart Energy. Smarter Dorms.** > A plug-and-play IoT energy monitoring and automation system designed for 4-6 person university dormitories.

**Course:** ENT208TC Industry Readiness  
**Team:** Session 2 - Group 2  

---

## 📖 Overview

**Watt's Up (DormGuard)** is an IoT-based solution designed to eliminate wasted electricity and reduce roommate conflicts over utility bills. The system autonomously monitors room occupancy, ambient light, and power draw to intelligently cut off power to non-essential devices when the room is empty.

The project consists of two main components:
1. **Hardware (M5Stack):** Reads sensor data and executes physical relay controls.
2. **Web Dashboard:** A lightweight, cross-network web interface for real-time monitoring and manual overrides.

---

## ✨ Key Features

* **🏃‍♂️ Occupancy Detection:** Uses a PIR sensor to detect if the room is empty.
* **☀️ Light & Power Monitoring:** Integrates a BH1750 light sensor and an ACS712 current sensor to detect wasted energy (e.g., lights left on during the day).
* **🔌 Auto Power-Off:** Automatically triggers a relay to cut power after a predefined idle timeout, with a grace period.
* **🌐 Universal Web Dashboard:** A vanilla HTML/JS dashboard accessible from any browser anywhere, communicating via MQTT over WebSockets.
* **🎮 Manual Override:** Users can override the automation via the web dashboard or physical buttons on the device.

---

## 🛠️ Hardware Specifications & Wiring

The system is built around the **M5Stack** ecosystem (programmed via M5Unified).

| Component | Pin / Interface | Description |
| :--- | :--- | :--- |
| **PIR Sensor** | `GPIO 33` | Detects human motion (Active HIGH). |
| **Relay Module** | `GPIO 32` | Controls the power outlet (HIGH = ON, LOW = OFF). |
| **Current Sensor (ACS712)** | `GPIO 34` | Analog current measurement. |
| **Light Sensor (BH1750)** | `I2C (SDA: 21, SCL: 22)` | Ambient light measurement (lux). |

---

## 📡 Software Architecture & MQTT

The device and dashboard communicate asynchronously using a public MQTT broker. No local IP configuration or port forwarding is required.

* **Broker:** `broker.emqx.io` (Public)
* **Device Port (TCP):** `1883`
* **Dashboard Port (WebSocket):** `8083`

### MQTT Topics

**1. Status Topic: `wattsup/status`**
The M5Stack publishes a JSON payload every 2 seconds.
```json
{
  "occupied": true,
  "motion": true,
  "power": "on",
  "idle_time": 0,
  "saved_wh": 0.00,
  "lux": 150.5,
  "load_w": 45.2,
  "relay": "on",
  "status": "normal",
  "device": "Watt's Up",
  "manual": false,
  "uptime_ms": 15000
}
