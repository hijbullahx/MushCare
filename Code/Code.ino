#include <Adafruit_Sensor.h>
#include <DHT.h>

// 📘 1. Hardware Pin Definitions
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_PIN 32    
#define MQ135_PIN 35  
#define PUMP_RELAY_PIN 23 

// 📘 2. Objects & Calibration Constants
DHT dht(DHTPIN, DHTTYPE);
const int AirValue = 3830;   
const int WaterValue = 2050; 

void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Smart Logic Initializing...");
  
  // --- Initialize Sensors ---
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);

  // --- Initialize Actuators ---
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, HIGH); // Force OFF at startup
}

void loop() {
  // ---------------------------------------------------
  // 📘 Phase A: Data Acquisition
  // ---------------------------------------------------
  delay(2000); 
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Error: Failed to read from DHT sensor!");
  } else {
    Serial.print("Air Humidity: "); Serial.print(humidity);
    Serial.print("%  |  Air Temp: "); Serial.print(tempC); Serial.println("°C");
  }

  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = map(rawMoistureValue, AirValue, WaterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  Serial.print("Substrate Moisture: "); Serial.print(moisturePercent); Serial.println("%");

  int lightState = digitalRead(LDR_PIN);
  if (lightState == HIGH) {
    Serial.println("Ambient Light: DARK 🌙");
  } else {
    Serial.println("Ambient Light: BRIGHT ☀️");
  }

  int rawGasValue = analogRead(MQ135_PIN);
  Serial.print("Raw CO2/Gas Level: "); Serial.println(rawGasValue);
  
  // ---------------------------------------------------
  // 📘 Phase B: Smart Actuation (Water Pump Logic)
  // ---------------------------------------------------
  
  // Apply Grey Oyster biological thresholds
  if (moisturePercent <= 40) {
    Serial.println("=> ALERT: Substrate DRY (<=40%). Misting Pump ON 💦");
    digitalWrite(PUMP_RELAY_PIN, LOW); // Relay ON
  } 
  else if (moisturePercent >= 65) {
    Serial.println("=> ALERT: Substrate OPTIMAL (>=65%). Misting Pump OFF 🛑");
    digitalWrite(PUMP_RELAY_PIN, HIGH); // Relay OFF
  } 
  else {
    Serial.println("=> STATUS: Moisture in acceptable range. Maintaining state.");
  }
  
  Serial.println("---------------------------------------------------");
}
