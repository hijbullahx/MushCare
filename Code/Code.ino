// 📘 1. Blynk Cloud Credentials
#define BLYNK_TEMPLATE_ID "TMPL6i1O063MX"
#define BLYNK_TEMPLATE_NAME "MushCare Smart Farm"
#define BLYNK_AUTH_TOKEN "cTnY0xmhE6fBFXVqzf8EkCHM1TgW21IR"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiMulti.h>   
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// 📘 2. Hardware Pin Definitions
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_DIGITAL_PIN 32 
#define LDR_ANALOG_PIN 36  
#define MQ135_PIN 35  

#define PUMP_RELAY 23 
#define FAN_RELAY 25  
#define LED_RELAY 26  
#define BUZZ_RELAY 27 

// 📘 3. Biological Thresholds
const int MOISTURE_DRY = 40;       
const int MOISTURE_OPTIMAL = 65;   
const float TEMP_HOT = 30.0;       
const float TEMP_COOL = 28.5;      
const int CO2_HIGH = 800;          
const int CO2_NORMAL = 600;        
const float ALARM_TEMP = 35.0;  

// 📘 Light Sensor Calibration
const int DARKNESS_SIGNAL = HIGH;       
const int ANALOG_DARK_THRESHOLD = 3700; 

// 📘 4. Objects & Global Variables
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiMulti wifiMulti;   
const int AirValue = 3830;   
const int WaterValue = 2050; 

int isAutoMode = 1; 
float currentHumidity = 0.0;
float currentTempC = 0.0;
unsigned long lastWifiDebugTime = 0; // NEW: Non-blocking debug timer

// ---------------------------------------------------
// ☁️ Phase A: Blynk Manual Override Functions
// ---------------------------------------------------
BLYNK_WRITE(V4) { 
  isAutoMode = param.asInt(); 
  if (isAutoMode == 1) {
    Serial.println("🌐 CLOUD: System set to AUTO.");
  } else {
    Serial.println("🌐 CLOUD: System set to MANUAL.");
    digitalWrite(PUMP_RELAY, HIGH);
    digitalWrite(FAN_RELAY, HIGH);
    digitalWrite(LED_RELAY, HIGH);
  }
}
BLYNK_WRITE(V5) { if (isAutoMode == 0) digitalWrite(PUMP_RELAY, param.asInt() == 1 ? LOW : HIGH); }
BLYNK_WRITE(V6) { if (isAutoMode == 0) digitalWrite(FAN_RELAY, param.asInt() == 1 ? LOW : HIGH); }
BLYNK_WRITE(V7) { if (isAutoMode == 0) digitalWrite(LED_RELAY, param.asInt() == 1 ? LOW : HIGH); }

// ---------------------------------------------------
// 🌡️ Phase B: Slow Sensor Routine
// ---------------------------------------------------
void readSlowSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    currentHumidity = h;
    currentTempC = t;
    if (Blynk.connected()) {
      Blynk.virtualWrite(V0, currentTempC);
      Blynk.virtualWrite(V1, currentHumidity);
    }
  }
}

// ---------------------------------------------------
// 🧠 Phase C: Fast Sensor & AI Routine
// ---------------------------------------------------
void runMushCareRoutine() {
  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = constrain(map(rawMoistureValue, AirValue, WaterValue, 0, 100), 0, 100);
  int rawGasValue = analogRead(MQ135_PIN);
  
  int digitalLightState = digitalRead(LDR_DIGITAL_PIN);
  int analogLightValue = analogRead(LDR_ANALOG_PIN);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, moisturePercent);
    Blynk.virtualWrite(V3, rawGasValue);
  }

  Serial.print("Hum: "); Serial.print(currentHumidity); Serial.print("% | Temp: "); Serial.print(currentTempC); Serial.println("°C");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.print("% | CO2: "); Serial.print(rawGasValue);
  Serial.print(" | Dig. LDR: "); Serial.print(digitalLightState); 
  Serial.print(" | Ana. LDR: "); Serial.println(analogLightValue);

  if (isAutoMode == 1) {
    if (moisturePercent <= MOISTURE_DRY) { digitalWrite(PUMP_RELAY, LOW); } 
    else if (moisturePercent >= MOISTURE_OPTIMAL) { digitalWrite(PUMP_RELAY, HIGH); }

    if (currentTempC >= TEMP_HOT || rawGasValue >= CO2_HIGH) { digitalWrite(FAN_RELAY, LOW); } 
    else if (currentTempC <= TEMP_COOL && rawGasValue < CO2_NORMAL) { digitalWrite(FAN_RELAY, HIGH); }

    if ((digitalLightState == DARKNESS_SIGNAL) || (analogLightValue < ANALOG_DARK_THRESHOLD)) { 
      digitalWrite(LED_RELAY, LOW); 
    } else { 
      digitalWrite(LED_RELAY, HIGH); 
    }
  }

  if (currentTempC >= ALARM_TEMP) { digitalWrite(BUZZ_RELAY, LOW); } 
  else { digitalWrite(BUZZ_RELAY, HIGH); }
  
  Serial.println("---------------------------------------------------");
}

// ---------------------------------------------------
// ⚙️ Phase D: System Setup & POST Diagnostic
// ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Booting IoT Core with Sensor Redundancy...");
  
  WiFi.mode(WIFI_STA); // Ensure ESP32 is explicitly in Station Mode for scanning
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT); 
  pinMode(LDR_DIGITAL_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);
  
  pinMode(PUMP_RELAY, OUTPUT); pinMode(FAN_RELAY, OUTPUT); pinMode(LED_RELAY, OUTPUT); pinMode(BUZZ_RELAY, OUTPUT);

  digitalWrite(PUMP_RELAY, HIGH); digitalWrite(FAN_RELAY, HIGH); 
  digitalWrite(LED_RELAY, HIGH); digitalWrite(BUZZ_RELAY, HIGH);

  Serial.println("Performing Hardware Self-Test...");
  digitalWrite(PUMP_RELAY, LOW); delay(500); digitalWrite(PUMP_RELAY, HIGH); Serial.println("Pump... OK");
  digitalWrite(FAN_RELAY, LOW); delay(500); digitalWrite(FAN_RELAY, HIGH); Serial.println("Fan... OK");
  digitalWrite(LED_RELAY, LOW); delay(500); digitalWrite(LED_RELAY, HIGH); Serial.println("LED... OK");
  digitalWrite(BUZZ_RELAY, LOW); delay(500); digitalWrite(BUZZ_RELAY, HIGH); Serial.println("Buzzer... OK");
  
  // --- WIFI NETWORKS ---
  wifiMulti.addAP("Hijbullah", "01748470965");
  wifiMulti.addAP("Kashem", "rashed123");
  wifiMulti.addAP("IUBATS", "Iubats@1991#");
  wifiMulti.addAP("realme 5i", "11223344");
  
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  timer.setInterval(2000L, readSlowSensors);    
  timer.setInterval(500L, runMushCareRoutine);  
}

// ---------------------------------------------------
// 🔄 Loop with Non-Blocking WiFi Debug
// ---------------------------------------------------
void loop() {
  timer.run(); // Keeps offline logic alive instantly
  
  if (wifiMulti.run() == WL_CONNECTED) {
    Blynk.run();
  }

  // Non-blocking debug print every 5 seconds
  if (millis() - lastWifiDebugTime > 5000) {
    lastWifiDebugTime = millis();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("✅ WiFi Connected: "); Serial.println(WiFi.SSID());
    } else {
      Serial.println("⏳ WiFi Disconnected. Auto-scanning all added networks...");
    }
  }
}
