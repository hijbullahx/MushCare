#include <Adafruit_Sensor.h>
#include <DHT.h>

// 📘 1. Pin Definitions
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_PIN 32    
#define MQ135_PIN 35  

// --- Actuator Pins ---
#define PUMP_RELAY 23 
#define FAN_RELAY 25  // Updated to safe GPIO pin
#define LED_RELAY 26  // Updated to safe GPIO pin
#define BUZZ_RELAY 27 

// 📘 2. Calibration Constants
DHT dht(DHTPIN, DHTTYPE);
const int AirValue = 3830;   
const int WaterValue = 2050; 

void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Full System Autonomous Mode Initializing...");
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);

  // Initialize all Actuators
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);
  pinMode(BUZZ_RELAY, OUTPUT);

  // Force all relays OFF at startup (Active-Low)
  digitalWrite(PUMP_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(LED_RELAY, HIGH);
  digitalWrite(BUZZ_RELAY, HIGH);
}

void loop() {
  // ---------------------------------------------------
  // 📘 Phase A: Data Acquisition
  // ---------------------------------------------------
  delay(2000); 
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = constrain(map(rawMoistureValue, AirValue, WaterValue, 0, 100), 0, 100);
  int lightState = digitalRead(LDR_PIN);
  int rawGasValue = analogRead(MQ135_PIN);

  // Print Telemetry
  Serial.print("Hum: "); Serial.print(humidity); Serial.print("% | Temp: "); Serial.print(tempC); Serial.println("°C");
  Serial.print("Moisture: "); Serial.print(moisturePercent); Serial.print("% | CO2 Level: "); Serial.println(rawGasValue);
  
  // ---------------------------------------------------
  // 📘 Phase B: Smart Actuation Logic
  // ---------------------------------------------------
  
  // 1. Misting Pump Logic (Moisture)
  if (moisturePercent <= 40) {
    digitalWrite(PUMP_RELAY, LOW); 
    Serial.println("-> Pump: ON 💦");
  } else if (moisturePercent >= 65) {
    digitalWrite(PUMP_RELAY, HIGH); 
  }

  // 2. Fan Logic (Temperature & CO2 Priority)
  // Triggers if Temp >= 29C OR if CO2 goes above baseline (approx 800+ raw analog)
  if (tempC >= 29.0 || rawGasValue >= 800) {
    digitalWrite(FAN_RELAY, LOW);
    Serial.println("-> Fan: ON 💨 (Cooling/Exhaust active)");
  } else if (tempC <= 25.0 && rawGasValue < 600) {
    digitalWrite(FAN_RELAY, HIGH);
  }

  // 3. Grow Light Logic (Photoperiod)
  if (lightState == HIGH) { // Dark environment detected
    digitalWrite(LED_RELAY, LOW);
    Serial.println("-> LED: ON 💡 (Providing Light)");
  } else {
    digitalWrite(LED_RELAY, HIGH);
  }

  // 4. Emergency Overrun Buzzer Logic
  // Simple safety check: If temp gets dangerously high (>35C), sound alarm
  if (tempC >= 35.0) {
    digitalWrite(BUZZ_RELAY, LOW);
    Serial.println("-> BUZZER: ON 🚨 (CRITICAL TEMP ALERT!)");
  } else {
    digitalWrite(BUZZ_RELAY, HIGH);
  }
  
  Serial.println("---------------------------------------------------");
}
