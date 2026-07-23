#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <esp_task_wdt.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define RELAY_PIN 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BME280 bme; 

const char* serverName = "http://192.168.0.101:8080/api/sensor"; 

unsigned long previousMillisScreen = 0;
unsigned long previousMillisServer = 0;
const long screenInterval = 1000;  
const long serverInterval = 60000; 

float currentTemp = 0.0;
float currentHum = 0.0;
float currentPres = 0.0;

float targetTemp = 24.0;
float targetHum = 40.0;
float targetDev = 2.0;
String systemState = "IDLE";

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.println("Checking Wi-Fi...");
  display.display();

  WiFiManager wifiManager;

  wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
    display.clearDisplay();
    display.setTextSize(1); 
    display.setCursor(0, 0);
    display.println("Wi-Fi not found!");
    display.println("-----------------");
    display.println("Connect phone to:");
    display.println("Microclimate_Setup");
    display.println("IP: 192.168.4.1");
    display.display();
  });

  if (!wifiManager.autoConnect("Microclimate_Setup")) {
    delay(3000);
    ESP.restart(); 
  }
  
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Wi-Fi Connected!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  if (!bme.begin(0x76)) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("BME280 Error!");
    display.display();
    while (1);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisScreen >= screenInterval) {
    previousMillisScreen = currentMillis; 

    currentTemp = bme.readTemperature();
    currentHum = bme.readHumidity();
    currentPres = bme.readPressure() / 100.0F;

    if (currentTemp <= targetTemp - targetDev) {
        systemState = "HEATING";
        digitalWrite(RELAY_PIN, HIGH);
    } 
    else if (currentTemp >= targetTemp + targetDev) {
        systemState = "COOLING";
        digitalWrite(RELAY_PIN, HIGH);
    } 
    else if (systemState == "HEATING" && currentTemp >= targetTemp) {
        systemState = "IDLE";
        digitalWrite(RELAY_PIN, LOW);
    } 
    else if (systemState == "COOLING" && currentTemp <= targetTemp) {
        systemState = "IDLE";
        digitalWrite(RELAY_PIN, LOW);
    }

    display.clearDisplay();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Trgt:"); display.print(targetTemp, 1);
    display.print(" +-"); display.println(targetDev, 1);
    display.drawLine(0, 9, 128, 9, WHITE);

    display.setTextSize(2);
    display.setCursor(0, 12);
    display.print("T:"); display.print(currentTemp, 1);
    
    display.setCursor(0, 28);
    display.print("H:"); display.print(currentHum, 1);

    display.setTextSize(1);
    display.setCursor(0, 46);
    display.print("P: "); display.print(currentPres, 1); display.print(" hPa");

    display.setCursor(0, 56);
    if (systemState == "HEATING") {
        display.print("> HEAT to "); display.print(targetTemp, 1);
    } else if (systemState == "COOLING") {
        display.print("> COOL to "); display.print(targetTemp, 1);
    } else {
        display.print("> IDLE (Temp OK)");
    }
    
    display.display();
  }

  if (currentMillis - previousMillisServer >= serverInterval) {
    previousMillisServer = currentMillis; 

    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");
      
      String jsonPayload = "{\"temperature\":" + String(currentTemp) + 
                           ",\"humidity\":" + String(currentHum) + 
                           ",\"pressure\":" + String(currentPres) + "}";
                           
      int httpResponseCode = http.POST(jsonPayload);
      
      if (httpResponseCode == 200) {
        String response = http.getString(); 
        
        int firstComma = response.indexOf(',');
        int secondComma = response.indexOf(',', firstComma + 1);
        
        if (firstComma > 0 && secondComma > 0) {
          targetTemp = response.substring(0, firstComma).toFloat();
          targetHum = response.substring(firstComma + 1, secondComma).toFloat();
          targetDev = response.substring(secondComma + 1).toFloat();
        }
      }
      http.end(); 
    }
  }
}