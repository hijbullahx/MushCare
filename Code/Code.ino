#include <Adafruit_Sensor.h>
#include <DHT.h>

// --- DHT22 Sensor ---
#define DHTPIN 15     
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

// --- Soil Moisture Sensor ---
#define SOIL_PIN 33 

// 📘 Calibration Constants
const int AirValue = 3830;   // Raw value when completely dry
const int WaterValue = 2050; // Raw value when submerged in water

void setup() {
  Serial.begin(115200);

  Serial.println("MushCare: Sensors Initializing...");
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
}

void loop() {
  // --- DHT22 Reading ---
  delay(2000); 
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Error: Failed to read from DHT sensor!");
  } else {
    Serial.print("Air Humidity: ");
    Serial.print(humidity);
    Serial.print("%  |  Air Temp: ");
    Serial.print(tempC);
    Serial.println("°C");
  }

  // --- Soil Moisture Reading & Mapping ---
  int rawMoistureValue = analogRead(SOIL_PIN);
  
  // Convert raw value to percentage (0-100)
  int moisturePercent = map(rawMoistureValue, AirValue, WaterValue, 0, 100);
  
  // Prevent percentage from going out of bounds (e.g., -5% or 105%)
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Raw Soil Data: ");
  Serial.print(rawMoistureValue);
  Serial.print("  |  Substrate Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");
  Serial.println("---------------------------------------------------");

  delay(1000);
}
