# MushCare: Intelligent Autonomous Farm Ecosystem

MushCare is an advanced, AI-integrated IoT ecosystem designed to optimize mushroom cultivation in the variable climate of Bangladesh. The system utilizes redundant sensor architecture and real-time environmental monitoring to maintain ideal growth conditions.

## 🏗️ Hardware Architecture
The system operates on a robust 5V master rail architecture, powered by a 7.4V Li-ion battery pack with DC-DC buck regulation.

### Pinout Configuration (ESP32)

| Component | ESP32 Pin | Logic/Role |
| :--- | :--- | :--- |
| **DHT22 (Temp/Hum)** | D15 | One-Wire Digital |
| **I2C LCD (SDA)** | D21 | I2C Data |
| **I2C LCD (SCL)** | D22 | I2C Clock |
| **Water Pump Relay** | D23 | Active-Low Relay |
| **Exhaust Fan Relay** | D25 | Active-Low Relay |
| **LED Strip Relay** | D26 | Active-Low Relay |
| **Buzzer Relay** | D27 | Active-Low Relay |
| **Digital LDR** | D32 | Binary Fail-safe |
| **Soil Moisture** | D33 | Analog Input |
| **Analog LDR (10mm)** | VP (D36) | Precision Analog Input |
| **MQ135 Gas Sensor** | D35 | Analog Input |

### Connection Diagrams



## 🧠 System Features
* **Redundant Photoperiod Logic:** Uses dual LDR inputs (Digital + Analog) with OR-gate logic for maximum uptime.
* **Dual-Core Processing:** Wi-Fi scanning occurs on Core 0, ensuring farm automation logic on Core 1 never lags or crashes.
* **Emergency Safety:** Hardware-level buzzer alarm for critical temperature thresholds (≥35°C).
* **Blynk Cloud Integration:** Real-time monitoring and manual override capabilities.

## 🚀 Setup & Installation
1. **Libraries Required:** Install the following via Arduino Library Manager:
   - `Blynk` by Volodymyr Shymanskyy
   - `DHT sensor library` by Adafruit
   - `LiquidCrystal I2C` by Marco Schwartz
2. **Configuration:** Update `BLYNK_AUTH_TOKEN` and add your local Wi-Fi credentials in the `wifiMulti.addAP()` section.
3. **Calibration:** Use the Serial Monitor to determine your environment's baseline `ANALOG_DARK_THRESHOLD`.

## 🛠️ Maintenance
* **Contrast Tuning:** If the LCD backlight glows but text is invisible, adjust the potentiometer on the back of the I2C backpack.
* **Battery:** Ensure the 7.4V battery pack is charged to maintain the 5V Master Rail stability.s