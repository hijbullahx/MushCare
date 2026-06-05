// 📘 1. Blynk Cloud Credentials (Must be at the very top!)
#define BLYNK_TEMPLATE_ID "TMPL6i1O063MX"
#define BLYNK_TEMPLATE_NAME "MushCare Smart Farm"
#define BLYNK_AUTH_TOKEN "cTnY0xmhE6fBFXVqzf8EkCHM1TgW21IR"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// 📘 2. Wi-Fi Credentials
char ssid[] = "Hijbullah";     // Enter your Wi-Fi SSID
char pass[] = "01748470965"; // Enter your Wi-Fi Password

// 📘 3. Hardware Pin Definitions
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_PIN 32    
#define MQ135_PIN 35  

#define PUMP_RELAY 23 
#define FAN_RELAY 25  
#define LED_RELAY 26  
#define BUZZ_RELAY 27 

// 📘 4. Objects & Global Variables
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
const int AirValue = 3830;   
const int WaterValue = 2050; 

// System State Variables
int isAutoMode = 1; // Default to Auto Mode ON (1)

// ---------------------------------------------------
// ☁️ Phase A: Blynk Manual Override Functions
// ---------------------------------------------------

// V4: Auto/Manual Toggle
BLYNK_WRITE(V4) { 
  isAutoMode = param.asInt(); 
  if (isAutoMode == 1) {
    Serial.println("🌐 CLOUD COMMAND: Switched to AUTO Mode");
  } else {
    Serial.println("🌐 CLOUD COMMAND: Switched to MANUAL Mode");
  }
}

// V5: Manual Pump Switch
BLYNK_WRITE(V5) {
  if (isAutoMode == 0) { // Only allow manual control if Auto is OFF
    int pinValue = param.asInt();
    digitalWrite(PUMP_RELAY, pinValue == 1 ? LOW : HIGH);
    Serial.print("🌐 MANUAL Override -> Pump: "); Serial.println(pinValue == 1 ? "ON" : "OFF");
  }
}

// V6: Manual Fan Switch
BLYNK_WRITE(V6) {
  if (isAutoMode == 0) {
    int pinValue = param.asInt();
    digitalWrite(FAN_RELAY, pinValue == 1 ? LOW : HIGH);
    Serial.print("🌐 MANUAL Override -> Fan: "); Serial.println(pinValue == 1 ? "ON" : "OFF");
  }
}

// V7: Manual Grow Light Switch
BLYNK_WRITE(V7) {
  if (isAutoMode == 0) {
    int pinValue = param.asInt();
    digitalWrite(LED_RELAY, pinValue == 1 ? LOW : HIGH);
    Serial.print("🌐 MANUAL Override -> LED: "); Serial.println(pinValue == 1 ? "ON" : "OFF");
  }
}

// ---------------------------------------------------
// 🧠 Phase B: The Main Sensor & Automation Loop
// ---------------------------------------------------
void runMushCareRoutine() {
  // 1. Read Sensors
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = constrain(map(rawMoistureValue, AirValue, WaterValue, 0, 100), 0, 100);
  int lightState = digitalRead(LDR_PIN);
  int rawGasValue = analogRead(MQ135_PIN);

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Error: Failed to read DHT!");
    return; // Skip this cycle if sensor fails
  }

  // 2. Upload Telemetry to Blynk Cloud
  Blynk.virtualWrite(V0, tempC);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, moisturePercent);
  Blynk.virtualWrite(V3, rawGasValue);

  // 3. Print Local Telemetry
  Serial.print("Hum: "); Serial.print(humidity); Serial.print("% | Temp: "); Serial.print(tempC); Serial.println("°C");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.print("% | CO2: "); Serial.println(rawGasValue);

  // 4. Autonomous Logic Execution (Only if Auto Mode is ON)
  if (isAutoMode == 1) {
    // Pump Logic
    if (moisturePercent <= 40) {
      digitalWrite(PUMP_RELAY, LOW); 
    } else if (moisturePercent >= 65) {
      digitalWrite(PUMP_RELAY, HIGH); 
    }

    // Fan Logic 
    if (tempC >= 29.0 || rawGasValue >= 800) {
      digitalWrite(FAN_RELAY, LOW);
    } else if (tempC <= 25.0 && rawGasValue < 600) {
      digitalWrite(FAN_RELAY, HIGH);
    }

    // Grow Light Logic
    if (lightState == HIGH) { 
      digitalWrite(LED_RELAY, LOW);
    } else {
      digitalWrite(LED_RELAY, HIGH);
    }
  }

  // Emergency Buzzer (Always runs, even in manual mode, for safety)
  if (tempC >= 35.0) {
    digitalWrite(BUZZ_RELAY, LOW);
  } else {
    digitalWrite(BUZZ_RELAY, HIGH);
  }
  
  Serial.println("---------------------------------------------------");
}

// ---------------------------------------------------
// ⚙️ Phase C: System Setup & Loop
// ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Connecting to IoT Cloud...");
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);

  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);
  pinMode(BUZZ_RELAY, OUTPUT);

  // Force Relays OFF
  digitalWrite(PUMP_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(LED_RELAY, HIGH);
  digitalWrite(BUZZ_RELAY, HIGH);

  // Connect to WiFi and Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Set the routine to run every 2000 milliseconds (2 seconds)
  timer.setInterval(2000L, runMushCareRoutine);
}

void loop() {
  // The loop is completely empty except for these two core commands
  Blynk.run();
  timer.run();
}
