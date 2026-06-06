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
char ssid[] = "****";     
char pass[] = "****"; 

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

// 📘 4. Biological Thresholds (Optimized for Bangladesh Climate)
const int MOISTURE_DRY = 40;       // % - Turn Pump ON
const int MOISTURE_OPTIMAL = 65;   // % - Turn Pump OFF
const float TEMP_HOT = 30.0;       // °C - Turn Fan ON 
const float TEMP_COOL = 28.5;      // °C - Turn Fan OFF (Realistic for BD)
const int CO2_HIGH = 800;          // PPM Raw - Turn Fan ON
const int CO2_NORMAL = 600;        // PPM Raw - Turn Fan OFF
const float ALARM_TEMP = 35.0;     // °C - Trigger Emergency Buzzer

// 📘 5. Objects & Global Variables
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
const int AirValue = 3830;   
const int WaterValue = 2050; 

int isAutoMode = 1; // 1 = Auto Mode ON, 0 = Manual Mode ON

// ---------------------------------------------------
// ☁️ Phase A: Blynk Manual Override Functions
// ---------------------------------------------------

// V4: Auto/Manual Toggle Button on App
BLYNK_WRITE(V4) { 
  isAutoMode = param.asInt(); 
  if (isAutoMode == 1) {
    Serial.println("🌐 CLOUD: System set to AUTO. AI taking over.");
  } else {
    Serial.println("🌐 CLOUD: System set to MANUAL. Waiting for user commands.");
    // Safety feature: Turn all relays OFF when switching to manual
    digitalWrite(PUMP_RELAY, HIGH);
    digitalWrite(FAN_RELAY, HIGH);
    digitalWrite(LED_RELAY, HIGH);
  }
}

// V5: Manual Pump Switch
BLYNK_WRITE(V5) {
  if (isAutoMode == 0) { 
    digitalWrite(PUMP_RELAY, param.asInt() == 1 ? LOW : HIGH);
  }
}

// V6: Manual Fan Switch
BLYNK_WRITE(V6) {
  if (isAutoMode == 0) {
    digitalWrite(FAN_RELAY, param.asInt() == 1 ? LOW : HIGH);
  }
}

// V7: Manual Grow Light Switch
BLYNK_WRITE(V7) {
  if (isAutoMode == 0) {
    digitalWrite(LED_RELAY, param.asInt() == 1 ? LOW : HIGH);
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
    return; 
  }

  // 2. Upload Telemetry to Blynk Cloud
  Blynk.virtualWrite(V0, tempC);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, moisturePercent);
  Blynk.virtualWrite(V3, rawGasValue);

  // 3. Print Local Telemetry
  Serial.print("Hum: "); Serial.print(humidity); Serial.print("% | Temp: "); Serial.print(tempC); Serial.println("°C");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.print("% | CO2: "); Serial.println(rawGasValue);

  // 4. Autonomous Logic Execution (ONLY runs if Auto Mode is Active)
  if (isAutoMode == 1) {
    
    // --- MISTING PUMP LOGIC ---
    if (moisturePercent <= MOISTURE_DRY) {
      digitalWrite(PUMP_RELAY, LOW); 
      Serial.println("   [AUTO] Substrate dry. Pump ON.");
    } else if (moisturePercent >= MOISTURE_OPTIMAL) {
      digitalWrite(PUMP_RELAY, HIGH); 
    }

    // --- VENTILATION FAN LOGIC ---
    if (tempC >= TEMP_HOT || rawGasValue >= CO2_HIGH) {
      digitalWrite(FAN_RELAY, LOW);
      Serial.println("   [AUTO] Heat/CO2 high. Fan ON.");
    } else if (tempC <= TEMP_COOL && rawGasValue < CO2_NORMAL) {
      digitalWrite(FAN_RELAY, HIGH);
    }

    // --- PHOTOPERIOD LIGHT LOGIC ---
    if (lightState == HIGH) { // It is dark
      digitalWrite(LED_RELAY, LOW);
    } else { // It is bright
      digitalWrite(LED_RELAY, HIGH);
    }
  }

  // 5. Emergency Buzzer (Always runs for safety, even in Manual)
  if (tempC >= ALARM_TEMP) {
    digitalWrite(BUZZ_RELAY, LOW);
    Serial.println("   🚨 ALARM: CRITICAL TEMPERATURE!");
  } else {
    digitalWrite(BUZZ_RELAY, HIGH);
  }
  
  Serial.println("---------------------------------------------------");
}

// ---------------------------------------------------
// ⚙️ Phase C: System Setup
// ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Booting up IoT Core...");
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);

  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);
  pinMode(BUZZ_RELAY, OUTPUT);

  // Force Relays OFF at startup
  digitalWrite(PUMP_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(LED_RELAY, HIGH);
  digitalWrite(BUZZ_RELAY, HIGH);

  // Connect to WiFi and Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Run the routine every 2 seconds without freezing the ESP32
  timer.setInterval(2000L, runMushCareRoutine);
}

void loop() {
  Blynk.run();
  timer.run();
}
