# FurFeeder 🐾 — Smart IoT Pet Treat Dispenser

> *"Your pet can't text you. FurFeeder can."*

FurFeeder is a motion-activated smart pet treat dispenser built with an ESP32-C6 and AWS IoT Core. When your pet approaches the device, you receive an alert in the cloud and can remotely trigger a treat to be dispensed — giving you full control over your pet's treat intake from anywhere.

Built for the SUTD Product Design Studio (Electronics & IoT) module.

---

## How It Works

1. Pet approaches FurFeeder → PIR sensor detects motion
2. ESP32 sends an alert to AWS IoT Core with a timestamp and treat level status
3. Owner sees the alert in AWS MQTT Test Client and publishes a dispense command
4. Servo motor opens the sliding gate → one treat drops into the bowl
5. Gate closes automatically, ready for the next dispense

---

## Hardware Required

- ESP32-C6-WROOM-1
- PIR Sensor (HC-SR501)
- IR Sensor Module
- SG90 Micro Servo
- Red LED + 220Ω Resistor
- Breadboard + Jumper Wires
- USB-C Cable

## Software Required

- Arduino IDE
- AWS IoT Core (free tier)
- Arduino Libraries:
  - MQTT by Joel Gaehwiler
  - ArduinoJson by Benoit Blanchon
  - ESP32Servo by Kevin Harrington

---

## Getting Started

1. Clone this repository
2. Open `arduino/pet_dispenser.ino` in Arduino IDE
3. Fill in your credentials in `arduino/secrets.h`:
   - WiFi SSID and password
   - AWS IoT Thing name
   - AWS IoT endpoint
   - AWS certificates (Root CA, Device Certificate, Private Key)
4. Upload to your ESP32-C6
5. Open AWS IoT → Test → MQTT Test Client and subscribe to `esp32/esp32-to-aws`

To dispense a treat, publish the following to `esp32/aws-to-esp32`:
```json
{"command": "dispense"}
```

---

## Demo Video

Watch FurFeeder in action: https://youtu.be/EMwhCCRMcPY

---

## Full Instructions

For the complete step-by-step guide including wiring, build instructions and AWS setup, refer to the full instructional manual: [Furfeeder Instructional Manual.pdf](https://github.com/user-attachments/files/26096408/Furfeeder.Instructional.Manual.pdf)
