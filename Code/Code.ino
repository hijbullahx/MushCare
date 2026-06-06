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
#include <Wire.h>               
#include <LiquidCrystal_I2C.h>  

// 📘 2. Hardware Pin Definitions
#define DHTPIN 15     
#define DHTTYPE DHT22 
#define SOIL_PIN 33 
#define LDR_DIGITAL_PIN 32 
#define LDR_ANALOG_PIN 36  
#define MQ135_PIN 35  

// 📺 Display I2C Pins
#define I2C_SDA 21
#define I2C_SCL 22

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

// ⚠️ LCD ADDRESS FIX: If the screen is blank, change 0x27 to 0x3F below:
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const int AirValue = 3830;   
const int WaterValue = 2050; 

int isAutoMode = 1; 
float currentHumidity = 0.0;
float currentTempC = 0.0;

// FreeRTOS Task Handle for WiFi
TaskHandle_t wifiTask;

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
// 🌡️ Phase B: Slow Sensor Routine (Every 2 Seconds)
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
// 🧠 Phase C: Fast Sensor & AI Routine (Every 0.5 Seconds)
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

  // ⚡ INSTANT AI AUTONOMOUS LOGIC
  if (isAutoMode == 1) {
    if (moisturePercent <= MOISTURE_DRY) { digitalWrite(PUMP_RELAY, LOW); } else if (moisturePercent >= MOISTURE_OPTIMAL) { digitalWrite(PUMP_RELAY, HIGH); }
    if (currentTempC >= TEMP_HOT || rawGasValue >= CO2_HIGH) { digitalWrite(FAN_RELAY, LOW); } else if (currentTempC <= TEMP_COOL && rawGasValue < CO2_NORMAL) { digitalWrite(FAN_RELAY, HIGH); }
    if ((digitalLightState == DARKNESS_SIGNAL) || (analogLightValue < ANALOG_DARK_THRESHOLD)) { digitalWrite(LED_RELAY, LOW); } else { digitalWrite(LED_RELAY, HIGH); }
  }

  if (currentTempC >= ALARM_TEMP) { digitalWrite(BUZZ_RELAY, LOW); } else { digitalWrite(BUZZ_RELAY, HIGH); }

  // ---------------------------------------------------
  // 📺 LCD Screen Formatting
  // ---------------------------------------------------
  String wifiDisplay = "";
  if (WiFi.status() == WL_CONNECTED) { wifiDisplay = WiFi.SSID(); } 
  else { wifiDisplay = "Searching..."; }
  
  String row0 = "WiFi:" + wifiDisplay;
  while(row0.length() < 16) { row0 += " "; } 
  if(row0.length() > 16) { row0 = row0.substring(0, 16); } 

  String pState = (digitalRead(PUMP_RELAY) == LOW) ? "ON " : "OFF";
  String fState = (digitalRead(FAN_RELAY) == LOW) ? "ON " : "OFF";
  String lState = (digitalRead(LED_RELAY) == LOW) ? "ON " : "OFF";

  String row1 = "P:" + pState + "F:" + fState + "L:" + lState;
  while(row1.length() < 16) { row1 += " "; } 
  
  lcd.setCursor(0, 0); lcd.print(row0);
  lcd.setCursor(0, 1); lcd.print(row1);

  // Serial Debug Print
  Serial.print("Hum: "); Serial.print(currentHumidity); Serial.print("% | Temp: "); Serial.print(currentTempC); Serial.println("°C");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.print("% | CO2: "); Serial.print(rawGasValue);
  Serial.print(" | Dig. LDR: "); Serial.print(digitalLightState); Serial.print(" | Ana. LDR: "); Serial.println(analogLightValue);
  
  Serial.println("\n+--- LCD SCREEN ---+");
  Serial.println("|" + row0 + "|");
  Serial.println("|" + row1 + "|");
  Serial.println("+------------------+\n");
}

// ---------------------------------------------------
// 🌐 Phase D: Core 0 Background WiFi Task
// ---------------------------------------------------
void keepWiFiAlive(void * parameter) {
  for(;;) {
    if (WiFi.status() != WL_CONNECTED) {
      wifiMulti.run(); // This blocks Core 0, but Core 1 keeps farming!
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait 5 seconds before checking again
  }
}

// ---------------------------------------------------
// ⚙️ Phase E: System Setup (Runs Instantly)
// ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("MushCare: Booting IoT Core with Sensor Redundancy...");
  
  Wire.begin(I2C_SDA, I2C_SCL); 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("MushCare OS v2.0");
  lcd.setCursor(0, 1); lcd.print("Booting System..");
  delay(1000);
  
  pinMode(SOIL_PIN, INPUT); pinMode(LDR_DIGITAL_PIN, INPUT); pinMode(MQ135_PIN, INPUT);
  pinMode(PUMP_RELAY, OUTPUT); pinMode(FAN_RELAY, OUTPUT); pinMode(LED_RELAY, OUTPUT); pinMode(BUZZ_RELAY, OUTPUT);

  digitalWrite(PUMP_RELAY, HIGH); digitalWrite(FAN_RELAY, HIGH); 
  digitalWrite(LED_RELAY, HIGH); digitalWrite(BUZZ_RELAY, HIGH);

  // INSTANT HARDWARE SELF TEST
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Hardware Test:");
  
  lcd.setCursor(0, 1); lcd.print("Testing Pump... ");
  digitalWrite(PUMP_RELAY, LOW); delay(500); digitalWrite(PUMP_RELAY, HIGH); Serial.println("Pump... OK");
  
  lcd.setCursor(0, 1); lcd.print("Testing Fan...  ");
  digitalWrite(FAN_RELAY, LOW); delay(500); digitalWrite(FAN_RELAY, HIGH); Serial.println("Fan... OK");
  
  lcd.setCursor(0, 1); lcd.print("Testing LED...  ");
  digitalWrite(LED_RELAY, LOW); delay(500); digitalWrite(LED_RELAY, HIGH); Serial.println("LED... OK");
  
  lcd.setCursor(0, 1); lcd.print("Testing Buzzer..");
  digitalWrite(BUZZ_RELAY, LOW); delay(500); digitalWrite(BUZZ_RELAY, HIGH); Serial.println("Buzzer... OK");
  
  lcd.clear(); lcd.setCursor(0,0); lcd.print("Test Complete!");
  delay(500);

  // START SENSORS
  dht.begin();
  WiFi.mode(WIFI_STA); 
  
  // ADD WIFI NETWORKS
  wifiMulti.addAP("******", "******");
  wifiMulti.addAP("******", "******");
  wifiMulti.addAP("******", "******");
  wifiMulti.addAP("******", "******");
  
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  timer.setInterval(2000L, readSlowSensors);    
  timer.setInterval(500L, runMushCareRoutine);  

  // 🔥 THESIS UPGRADE: Assign WiFi to Core 0 so it never freezes the system!
  xTaskCreatePinnedToCore(
    keepWiFiAlive,   // Function to run
    "wifiTask",      // Task name
    4096,            // Stack size
    NULL,            // Parameters
    1,               // Priority
    &wifiTask,       // Task handle
    0                // Pin to Core 0 (Background)
  );
}

// ---------------------------------------------------
// 🔄 Phase F: High-Speed Loop Core (Runs on Core 1)
// ---------------------------------------------------
void loop() {
  timer.run(); // Keeps sensors and relays running instantly
  
  // Only process cloud data if WiFi is currently healthy
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
}
