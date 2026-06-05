#include <Adafruit_Sensor.h>
#include <DHT.h>

// --- Sensor Pins ---
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_PIN 32    // Digital LDR Pin
#define MQ135_PIN 35  // MQ-135 Analog Pin (Must use voltage divider!)

// --- Objects & Constants ---
DHT dht(DHTPIN, DHTTYPE);
const int AirValue = 3830;   
const int WaterValue = 2050; 

void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Full Sensor Array Initializing...");
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  
  // Initialize LDR pin as a digital input
  pinMode(LDR_PIN, INPUT); 

  // Initialize MQ-135 pin as an analog input
  pinMode(MQ135_PIN, INPUT);
}

void loop() {
  // 1. DHT22 Reading
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

  // 2. Soil Moisture Reading
  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = map(rawMoistureValue, AirValue, WaterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Substrate Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  // 3. Digital LDR Light Reading
  int lightState = digitalRead(LDR_PIN);
  
  // Note: Most digital LDR modules output HIGH (1) when it is DARK 
  // and LOW (0) when it is BRIGHT. 
  if (lightState == HIGH) {
    Serial.println("Ambient Light: DARK 🌙 (Grow Lights will trigger here)");
  } else {
    Serial.println("Ambient Light: BRIGHT ☀️ (Grow Lights OFF)");
  }

  // 4. MQ-135 Gas Reading
  // Reads the analog voltage (0-4095) stepped down by the voltage divider
  int rawGasValue = analogRead(MQ135_PIN);
  Serial.print("Raw CO2/Gas Level: ");
  Serial.println(rawGasValue);
  
  Serial.println("---------------------------------------------------");

  delay(1000);
}
