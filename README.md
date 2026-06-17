# Workspace Fortress: Privacy-First Edge IoT \& Ergonomic Console

An embedded workspace monitor designed around an asynchronous C++ state machine running locally on an ESP32 microcontroller. The system actively reduces digital eye strain, static muscle fatigue, and cognitive environmental decline using research-backed thresholds from the American Optometric Association (AOA).

To ensure 100% video and data privacy, the entire ecosystem relies strictly on non-optical radar tracking arrays and edge processing. It operates completely independently of external cloud services or camera-based tracking networks.

\---

## 📸 System Demo



\### Circuit \& Hardware Setup

!\[Circuit Photo](schematics/circuit\_photo.jpg)

### Monitoring Dashboard

!\[Monitoring Dashboard](docs/screenshots/dashboard\_monitoring.png)
*Real-time telemetry — posture distance, thermal readings, lux analytics, pomodoro sync, wellness score, and live session audit logs.*

### Presence Detection Alert

!\[Presence Detect Failure](docs/screenshots/presence\_detect\_alert.png)
*Automatic focus sprint pause triggered when operator absence exceeds 10 seconds. Browser push notification issued simultaneously.*

### Settings \& Log Audits

!\[Settings and Logs](docs/screenshots/settings\_logs.png)
*Hardware link configuration, OTA calibration sliders, timestamped system security log stream, and 7-day local privacy analytics summary.*

\---

## 📊 Core Architecture \& Pin Map

The system separates its physical components into three functional layers, isolating low-current CMOS logic lines from high-current inductive mechanical loads.

|Hardware Layer|Component Model|ESP32 GPIO Pin|Interface Type|Electrical Isolation Routing|
|-|-|-|-|-|
|**Perception (Inputs)**|HC-SR04 Proximity Radar|Trigger: `G18` / Echo: `G19`|Digital I/O|Direct Logic Rail|
||DHT11 Microclimate Unit|Data: `G4`|Single-Wire Bus|10kΩ Pull-Up Resistor|
||LDR Photoresistor Array|Analog: `G34`|ADC Register|Voltage Divider Network|
|**Actuation (Outputs)**|SG90 Status Needle Servo|PWM: `G26`|Hardware Timer|Isolated External 5V Rail|
||Coin Haptic Vibration Motor|Control: `G27`|Digital Out|**ULN2003 Darlington Driver**|
||Active Alarm Siren|Trigger: `G25`|Digital Out|**ULN2003 Darlington Driver**|
||Passive Piezo Chime Buzzer|Tone: `G32`|LEDC PWM Channel|Direct Driver Channel|

\---

## ⚙️ Implemented Rule Engine \& System Parameters

The background `loop()` coordinates four automated safety modules without running blocking code delays:

### ⏱️ 1. Smart Pomodoro Sync

Manages an isolated 25-minute work sprint and 5-minute break sequence. The frontend telemetry loop queries the on-chip HTTP server at `/readSensors` every 1000ms. If a user stands up or completely leaves the radar sweet spot for more than 10 continuous seconds during a focus sprint, it triggers an automation breakdown, records a distraction event, drops the mode to idle, and sounds a warning tone.

### 🧘 2. Biomechanical \& Static Fatigue Tracking

* **AOA Proximity Guard:** Detects if a user slouches closer than 50 cm. It grants a 3-second buffer window before pulsing a tactile vibration reminder.
* **Static Stiff Monitor:** Prolonged fixed sitting creates deep fatigue. If body movement shifts by less than 3 cm over 20 continuous minutes, the mechanical status needle swings to its 45° break zone and delivers a firm 1-second physical shake.

### 🌡️ 3. Layered Thermal Safety Matrix

Productivity declines rapidly above 25°C, with severe cognitive accuracy degradation at 28°C+.

* **25°C to 29°C:** Sounds a polite double-chirp tone via the piezo channel once every 5 minutes as a microclimate warning.
* **≥ 30°C:** Triggers an emergency system override, swinging the status servo to 180° and latching the high-output active siren.

### 💡 4. Asymmetric Optical Contrast Guard

To avoid high-contrast computer vision strain (working on a bright screen in a pitch-black room), the logic checks the LDR sensor. If focus mode is running and ambient illuminance drops below a raw ADC value of 1250, a non-blocking timer triggers a three-tone descending alert chime once every 5 minutes.

\---

## 🛠️ Installation \& Verification

1. Clone this repository to your local directory.
2. Open `firmware/workspace\_fortress.ino` within the Arduino IDE.
3. Update the global placeholder network credential strings (`ssid` and `password`) with your local access point details.
4. Install the required libraries via Arduino Library Manager:

   * `DHT sensor library` by Adafruit
   * `ESP32Servo` by Kevin Harrington
5. Compile and flash the sketch to your ESP32 board node.
6. Launch your browser and navigate to the local IP printed on your IDE Serial Monitor at `115200` baud.

\---

## 📦 Bill of Materials

|Component|Model|Qty|
|-|-|:-:|
|Microcontroller|ESP32 DevKit V1|1|
|Proximity Radar|HC-SR04 Ultrasonic|1|
|Microclimate Sensor|DHT11|1|
|Light Sensor|LDR Photoresistor|1|
|Status Actuator|SG90 Micro Servo|1|
|Haptic Feedback|Coin Vibration Motor|1|
|Alarm Output|Active Buzzer 5V|1|
|Chime Output|Passive Piezo Buzzer|1|
|Motor Driver|ULN2003 Darlington Array|1|
|Display|1602A LCD (16-pin parallel)|1|
|Power Regulation|LM2596 Buck Converter|1|
|Relay|5V Single Channel Relay|1|

\---

## 🔒 Privacy Architecture

All processing occurs entirely on-device. No camera, no cloud API, no data exfiltration. The 7-day analytics summary is stored in browser `localStorage` — encrypted and confined to the local machine. The ESP32 HTTP server is LAN-bound and never exposed to the public internet.

\---

## 🏆 Project Status

|Milestone|Status|
|-|-|
|Firmware state machine|✅ Complete|
|Flask backend integration|✅ Complete|
|Web dashboard (HTML/JS)|✅ Complete|
|OTA calibration via sliders|✅ Complete|
|Session log audit stream|✅ Complete|
|Hardware assembly (breadboard)|🔧 In Progress|

\---

## 👤 Author

**Aswin Menon** — ECE, VIT Vellore  
Embedded Systems | Edge AI | IoT  
[GitHub](https://github.com/aswinmenon525)

