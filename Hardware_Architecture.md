# MushCare Hardware Architecture

## Power Distribution (v2.0 - Low Voltage Optimization)
To ensure system stability during load-shedding in Bangladesh, the system relies on a highly efficient, single-rail 5V power architecture:
* **7.4V Core:** Two 3.7V Lithium-ion batteries wired in series to provide a stable 7.4V input.
* **5V Master Rail:** Regulated by a DC-DC Buck Converter (tuned to exactly 5.0V). This single rail powers the entire ecosystem, including the ESP32 (via VIN), the 4-channel relay module, the 1602A I2C LCD display, and all 5V actuators (Water Pump, CPU Fan, LED Strip, and Buzzer).
* **Future Expansion:** The system is pre-calculated to accept a mini solar panel integrated via a TP4056 charge controller for true off-grid sustainability.

## Actuator Control Matrix (v1.0)
The system utilizes an active-low 4-Channel 5V Relay module to physically isolate the ESP32 logic pins from the mechanical load of the actuators. All actuators draw from the unified 5V Master Rail.
* **CH1 (Pin 23):** 5V DC Submersible Water Pump (Misting / Moisture Control)
* **CH2 (Pin 25):** 5V CPU Exhaust Fan (Cooling / CO2 Evacuation)
* **CH3 (Pin 26):** 5V White LED Strip (Photoperiod Simulation)
* **CH4 (Pin 27):** 5V Active Buzzer (Emergency Temperature Alarm)
* **Switching Logic:** Power routed through Common (COM) to Normally Open (NO).

## Sensor Redundancy Protocol (v1.0)
To ensure system resilience, the photoperiod logic utilizes a hardware redundancy model combining both binary and analog inputs via an `OR` gate logic structure.
* **Primary Sensor (Analog):** A 10mm raw LDR integrated via a 10kΩ voltage divider into the ESP32's `VP` (Pin 36 / ADC1). This provides high-resolution 12-bit data (0-4095) for precision threshold triggering.
* **Backup Sensor (Digital):** An LM393-based LDR module connected to digital Pin 32. Acts as a strict binary fail-safe in the event of voltage divider failure.