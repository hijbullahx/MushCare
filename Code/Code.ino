#include <Adafruit_Sensor.h>
#include <DHT.h>

// 📘 1. Hardware Pin Definitions
// --- Sensors ---
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_PIN 32    // Digital LDR Pin
#define MQ135_PIN 35  // MQ-135 Analog Pin (Using voltage divider)

// --- Actuators ---
#define PUMP_RELAY_PIN 23 // 5V Relay for the Water Pump

// 📘 2. Objects & Calibration Constants
DHT dht(DHTPIN, DHTTYPE);
const int AirValue = 3830;   
const int WaterValue = 2050; 

void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Full System Initializing...");
  
  // --- Initialize Sensors ---
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);

  // --- Initialize Actuators ---
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  
  // ⚠️ Safety Protocol: Force relay OFF at startup
  // Note: Most relay modules are "Active-Low" (HIGH = OFF, LOW = ON)
  digitalWrite(PUMP_RELAY_PIN, HIGH);
}

void loop() {
  // ---------------------------------------------------
  // 📘 Phase A: Data Acquisition (Sensors)
  // ---------------------------------------------------
  
  // 1. DHT22 Reading (Wait 2 seconds for sensor to stabilize)
  delay(2000); 
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Error: Failed to read from DHT sensor!");
  } else {
    Serial.print("Air Humidity: "); Serial.print(humidity);
    Serial.print("%  |  Air Temp: "); Serial.print(tempC); Serial.println("°C");
  }

  // 2. Substrate Moisture Reading
  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = map(rawMoistureValue, AirValue, WaterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  
  Serial.print("Substrate Moisture: "); Serial.print(moisturePercent); Serial.println("%");

  // 3. Ambient Light Reading
  int lightState = digitalRead(LDR_PIN);
  if (lightState == HIGH) {
    Serial.println("Ambient Light: DARK 🌙");
  } else {
    Serial.println("Ambient Light: BRIGHT ☀️");
  }

  // 4. Air Quality (CO2) Reading
  int rawGasValue = analogRead(MQ135_PIN);
  Serial.print("Raw CO2/Gas Level: "); Serial.println(rawGasValue);
  
  // ---------------------------------------------------
  // 📘 Phase B: Actuation Test (Relay)
  // ---------------------------------------------------
  
  Serial.println("=> Actuation: Triggering Pump Relay ON");
  digitalWrite(PUMP_RELAY_PIN, LOW); // Pulling pin to Ground closes the switch
  delay(2000); // Hold it on for 2 seconds so you hear the click

  Serial.println("=> Actuation: Triggering Pump Relay OFF");
  digitalWrite(PUMP_RELAY_PIN, HIGH); // Pushing 3.3V opens the switch
  
  Serial.println("---------------------------------------------------");
}
