#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "DHT.h"

const char* AP_SSID = "ESP32"; 
const char* AP_PASS = "12345678"; 

IPAddress localIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// --- SENSOR PINS ---
const int FLAME_SENSOR_DO_PIN = 34; 
const int MQ2_DO_PIN = 35;
const int MQ2_AO_PIN = 32;

// --- LED & BUZZER PINS ---
const int LED_FLAME_PIN = 2;   // Red LED 1 (Blinks for Fire)
const int LED_TEMP_PIN  = 4;   // Red LED 2 (High for Heat) .
const int LED_GAS_PIN   = 16;  // Yellow LED (High for Gas) 
const int LED_SAFE_PIN  = 17;  // Green LED (High if Safe).
const int BUZZER_PIN    = 5;   // Buzzer (High if ANY danger detected).

// --- CONFIGURATION ---
const float TEMP_THRESHOLD = 33.0; // Temperature to trigger Red LED
const long BLINK_INTERVAL = 200;   // Speed of blinking

// Define DHT
#define DHTPIN 27     
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// Global Variables
int flameStatus = 0;
int mq2DigitalStatus = 0;
int mq2AnalogValue = 0;
float currentTemp = 0.0;

// Timer Variables
unsigned long previousMillis = 0;
int flameLedState = LOW;
unsigned long lastDHTRead = 0;

void handleRoot() {
    File file = LittleFS.open("/index.html", "r"); 
    if (!file) {
        server.send(404, "text/plain", "404: /index.html Not Found!");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

void handleSensorData() {
    // Send data to the web
    String json = "{";
    json += "\"flame\": " + String(flameStatus) + ",";
    json += "\"mq2_digital\": " + String(mq2DigitalStatus) + ",";
    json += "\"mq2_analog\": " + String(mq2AnalogValue) + ",";
    json += "\"temp\": " + String(currentTemp); 
    json += "}";
    server.send(200, "application/json", json);
}

void updateSensorsAndLEDs() {
    unsigned long currentMillis = millis();

    // --- 1. READ SENSORS ---
    
    // Flame (LOW usually means Fire detected)
    int flameRaw = digitalRead(FLAME_SENSOR_DO_PIN);
    flameStatus = (flameRaw == LOW) ? 1 : 0; 

    // Gas (LOW usually means Gas detected)
    int mq2Raw = digitalRead(MQ2_DO_PIN);
    mq2DigitalStatus = (mq2Raw == LOW) ? 1 : 0; 
    mq2AnalogValue = analogRead(MQ2_AO_PIN); 

    // Temperature (Read every 2 seconds)
    if (currentMillis - lastDHTRead > 10) {
        float t = dht.readTemperature();
        if (!isnan(t)) {
            currentTemp = t;
        }
        lastDHTRead = currentMillis;
    }

    // --- 2. LED & BUZZER LOGIC ---
    bool isUnsafe = false; // Tracks if ANY danger exists

    // A. Fire Logic (Blink if detected)
    if (flameStatus == 1) {
        isUnsafe = true;
        if (currentMillis - previousMillis >= BLINK_INTERVAL) {
            previousMillis = currentMillis;
            flameLedState = (flameLedState == LOW) ? HIGH : LOW;
            digitalWrite(LED_FLAME_PIN, flameLedState);
        }
    } else {
        digitalWrite(LED_FLAME_PIN, LOW); // Turn Off if Safe
    }

    // B. Gas Logic
    if (mq2DigitalStatus == 1) {
        isUnsafe = true;
        digitalWrite(LED_GAS_PIN, HIGH); 
    } else {
        digitalWrite(LED_GAS_PIN, LOW);  
    }

    // C. Temperature Logic
    if (currentTemp >= TEMP_THRESHOLD) {
        isUnsafe = true;
        digitalWrite(LED_TEMP_PIN, HIGH);
    } else {
        digitalWrite(LED_TEMP_PIN, LOW);
    }

    // D. Green "Safe" LED Logic
    // Only turns ON if all other dangers are FALSE
    if (!isUnsafe) {
        digitalWrite(LED_SAFE_PIN, HIGH);
    } else {
        digitalWrite(LED_SAFE_PIN, LOW);
    }

    // E. BUZZER LOGIC (New)
    // If Unsafe (Fire OR Gas OR Temp), Buzzer goes HIGH
    if (isUnsafe) {
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    delay(10);

    // Sensor Pins
    pinMode(FLAME_SENSOR_DO_PIN, INPUT);
    pinMode(MQ2_DO_PIN, INPUT);
    pinMode(MQ2_AO_PIN, INPUT); 

    // LED Pins
    pinMode(LED_FLAME_PIN, OUTPUT);
    pinMode(LED_TEMP_PIN, OUTPUT);
    pinMode(LED_GAS_PIN, OUTPUT);
    pinMode(LED_SAFE_PIN, OUTPUT);

    // Buzzer Pin
    pinMode(BUZZER_PIN, OUTPUT);

    // Start DHT
    dht.begin();

    // WiFi Setup
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(localIP, gateway, subnet);
    if (!WiFi.softAP(AP_SSID, AP_PASS)) {
        Serial.println("Soft AP creation failed.");
        return;
    }
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // File System Setup
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleSensorData); 
    server.onNotFound([]() { server.send(404, "text/plain", "404: File Not Found"); });

    server.begin();
    Serial.println("HTTP Web Server started.");
}

void loop() {
    server.handleClient();      
    updateSensorsAndLEDs();     
}