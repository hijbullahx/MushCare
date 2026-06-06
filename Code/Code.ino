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
char ssid[] = "Hijbullah";     
char pass[] = "01748470965"; 

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
const float TEMP_COOL = 28.5;      // °C - Turn Fan OFF 
const int CO2_HIGH = 800;          // PPM Raw - Turn Fan ON
const int CO2_NORMAL = 600;        // PPM Raw - Turn Fan OFF
const float ALARM_TEMP = 35.0;     // °C - Trigger Emergency Buzzer

// 📘 5. Objects & Global Variables
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
const int AirValue = 3830;   
const int WaterValue = 2050; 

int isAutoMode = 1; // 1 = Auto, 0 = Manual

// Global variables for the slow DHT sensor
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
// 🌡️ Phase B: Slow Sensor Routine (Every 2.0 Seconds)
// ---------------------------------------------------
void readSlowSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    currentHumidity = h;
    currentTempC = t;
    Blynk.virtualWrite(V0, currentTempC);
    Blynk.virtualWrite(V1, currentHumidity);
  }
}

// ---------------------------------------------------
// 🧠 Phase C: Fast Sensor & AI Routine (Every 0.5 Seconds)
// ---------------------------------------------------
void runMushCareRoutine() {
  // 1. Fast Sensor Reads
  int rawMoistureValue = analogRead(SOIL_PIN);
  int moisturePercent = constrain(map(rawMoistureValue, AirValue, WaterValue, 0, 100), 0, 100);
  int lightState = digitalRead(LDR_PIN);
  int rawGasValue = analogRead(MQ135_PIN);

  // 2. Upload Fast Telemetry
  Blynk.virtualWrite(V2, moisturePercent);
  Blynk.virtualWrite(V3, rawGasValue);

  // 3. Print Local Telemetry (using cached Temp/Hum)
  Serial.print("Hum: "); Serial.print(currentHumidity); Serial.print("% | Temp: "); Serial.print(currentTempC); Serial.println("°C");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.print("% | CO2: "); Serial.println(rawGasValue);

  // 4. Autonomous Logic (Instant Execution)
  if (isAutoMode == 1) {
    
    if (moisturePercent <= MOISTURE_DRY) { digitalWrite(PUMP_RELAY, LOW); } 
    else if (moisturePercent >= MOISTURE_OPTIMAL) { digitalWrite(PUMP_RELAY, HIGH); }

    if (currentTempC >= TEMP_HOT || rawGasValue >= CO2_HIGH) { digitalWrite(FAN_RELAY, LOW); } 
    else if (currentTempC <= TEMP_COOL && rawGasValue < CO2_NORMAL) { digitalWrite(FAN_RELAY, HIGH); }

    if (lightState == HIGH) { digitalWrite(LED_RELAY, LOW); } 
    else { digitalWrite(LED_RELAY, HIGH); }
  }

  // Emergency Buzzer (Always active)
  if (currentTempC >= ALARM_TEMP) { digitalWrite(BUZZ_RELAY, LOW); } 
  else { digitalWrite(BUZZ_RELAY, HIGH); }
  
  Serial.println("---------------------------------------------------");
}

// ---------------------------------------------------
// ⚙️ Phase D: System Setup & POST Diagnostic
// ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Initializing Hardware...");
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); 
  pinMode(MQ135_PIN, INPUT);

  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);
  pinMode(BUZZ_RELAY, OUTPUT);

  // Force all OFF initially
  digitalWrite(PUMP_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(LED_RELAY, HIGH);
  digitalWrite(BUZZ_RELAY, HIGH);

  // --- HARDWARE POST DIAGNOSTIC ---
  Serial.println("Performing Hardware Self-Test...");
  
  digitalWrite(PUMP_RELAY, LOW); delay(500); digitalWrite(PUMP_RELAY, HIGH);
  Serial.println("Pump... OK");
  
  digitalWrite(FAN_RELAY, LOW); delay(500); digitalWrite(FAN_RELAY, HIGH);
  Serial.println("Fan... OK");
  
  digitalWrite(LED_RELAY, LOW); delay(500); digitalWrite(LED_RELAY, HIGH);
  Serial.println("LED... OK");
  
  digitalWrite(BUZZ_RELAY, LOW); delay(500); digitalWrite(BUZZ_RELAY, HIGH);
  Serial.println("Buzzer... OK");
  
  Serial.println("Diagnostic Complete. Connecting to WiFi & Blynk...");
  // --------------------------------

  // Connect to Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Initialize Dual Timers
  timer.setInterval(2000L, readSlowSensors);    // Protects the DHT22
  timer.setInterval(500L, runMushCareRoutine);  // High-speed responsiveness
}

void loop() {
  Blynk.run();
  timer.run();
}
