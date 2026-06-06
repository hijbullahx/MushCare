// 📘 1. Blynk Cloud Credentials (Must be at the very top!)
#define BLYNK_TEMPLATE_ID "TMPL6i1O063MX"
#define BLYNK_TEMPLATE_NAME "MushCare Smart Farm"
#define BLYNK_AUTH_TOKEN "cTnY0xmhE6fBFXVqzf8EkCHM1TgW21IR"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiMulti.h>   // NEW: Multiple Wi-Fi networks
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// 📘 2. Hardware Pin Definitions
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_PIN 32    
#define MQ135_PIN 35  

#define PUMP_RELAY 23 
#define FAN_RELAY 25  
#define LED_RELAY 26  
#define BUZZ_RELAY 27 

// 📘 3. Biological Thresholds (Optimized for Bangladesh Climate)
const int MOISTURE_DRY = 40;       
const int MOISTURE_OPTIMAL = 65;   
const float TEMP_HOT = 30.0;       
const float TEMP_COOL = 28.5;      
const int CO2_HIGH = 800;          
const int CO2_NORMAL = 600;        
const float ALARM_TEMP = 35.0;     

// 📘 4. Objects & Global Variables
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiMulti wifiMulti;   // Multi-WiFi object
const int AirValue = 3830;   
const int WaterValue = 2050; 

int isAutoMode = 1; 
float currentHumidity = 0.0;
float currentTempC = 0.0;

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
  int lightState = digitalRead(LDR_PIN);
  int rawGasValue = analogRead(MQ135_PIN);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, moisturePercent);
    Blynk.virtualWrite(V3, rawGasValue);
  }

  Serial.print("Hum: "); Serial.print(currentHumidity); Serial.print("% | Temp: "); Serial.print(currentTempC); Serial.println("°C");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.print("% | CO2: "); Serial.println(rawGasValue);

  if (isAutoMode == 1) {
    if (moisturePercent <= MOISTURE_DRY) { digitalWrite(PUMP_RELAY, LOW); } 
    else if (moisturePercent >= MOISTURE_OPTIMAL) { digitalWrite(PUMP_RELAY, HIGH); }

    if (currentTempC >= TEMP_HOT || rawGasValue >= CO2_HIGH) { digitalWrite(FAN_RELAY, LOW); } 
    else if (currentTempC <= TEMP_COOL && rawGasValue < CO2_NORMAL) { digitalWrite(FAN_RELAY, HIGH); }

    if (lightState == HIGH) { digitalWrite(LED_RELAY, LOW); } 
    else { digitalWrite(LED_RELAY, HIGH); }
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
  Serial.println("MushCare: Booting IoT Core...");
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT); pinMode(LDR_PIN, INPUT); pinMode(MQ135_PIN, INPUT);
  pinMode(PUMP_RELAY, OUTPUT); pinMode(FAN_RELAY, OUTPUT); pinMode(LED_RELAY, OUTPUT); pinMode(BUZZ_RELAY, OUTPUT);

  digitalWrite(PUMP_RELAY, HIGH); digitalWrite(FAN_RELAY, HIGH); 
  digitalWrite(LED_RELAY, HIGH); digitalWrite(BUZZ_RELAY, HIGH);

  Serial.println("Performing Hardware Self-Test...");
  digitalWrite(PUMP_RELAY, LOW); delay(500); digitalWrite(PUMP_RELAY, HIGH); Serial.println("Pump... OK");
  digitalWrite(FAN_RELAY, LOW); delay(500); digitalWrite(FAN_RELAY, HIGH); Serial.println("Fan... OK");
  digitalWrite(LED_RELAY, LOW); delay(500); digitalWrite(LED_RELAY, HIGH); Serial.println("LED... OK");
  digitalWrite(BUZZ_RELAY, LOW); delay(500); digitalWrite(BUZZ_RELAY, HIGH); Serial.println("Buzzer... OK");
  
  // --- ADD YOUR WIFI NETWORKS HERE ---
  wifiMulti.addAP("Hijbullah", "****");
  wifiMulti.addAP("Hijbullah", "*****");
  wifiMulti.addAP("Friends_WiFi", "Friends_Password");
  wifiMulti.addAP("Backup_Hotspot", "Backup_Password");
  
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  timer.setInterval(2000L, readSlowSensors);    
  timer.setInterval(500L, runMushCareRoutine);  
}

void loop() {
  timer.run();
  
  if (wifiMulti.run() == WL_CONNECTED) {
    // Show WiFi details once connected
    static bool shown = false;
    if (!shown) {
      Serial.println("✅ WiFi Connected!");
      Serial.print("SSID: "); Serial.println(WiFi.SSID());
      Serial.print("IP Address: "); Serial.println(WiFi.localIP());
      shown = true;
    }
    Blynk.run();
  }
}
