#include <Adafruit_Sensor.h>
#include <DHT.h>


// Define the corrected pin and sensor type
#define DHTPIN 15     
#define DHTTYPE DHT22 

// Initialize the sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: DHT22 Test Initializing...");
  dht.begin();
}

void loop() {
  // DHT22 requires about 2 seconds between readings
  delay(2000);

  // Read data
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  // Error handling
  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Error: Failed to read from DHT sensor!");
    return;
  }

  // Print results
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  |  Temperature: ");
  Serial.print(tempC);
  Serial.println("°C");
}
