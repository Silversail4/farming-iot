#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

Adafruit_SGP30 sgp;

// Wi-Fi and MQTT setup
const char* ssid = "dhjle";
const char* password = "6M9489/d";
const char* mqtt_server = "192.168.137.1";  

const char* mqtt_topic = "co2_reading"; // MQTT Topic

// Create instances
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
    Serial.print("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi connected. IP: " + WiFi.localIP().toString());
}

void reconnect_mqtt() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("M5StickCPlus_Client")) {
            Serial.println("Connected!");
        } else {
            Serial.print("Failed. State=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5s...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("SGP30 test");

    if (!sgp.begin()) {
        Serial.println("Sensor not found :(");
        while (1);
    }
    
    Serial.print("Found SGP30 serial #");
    Serial.print(sgp.serialnumber[0], HEX);
    Serial.print(sgp.serialnumber[1], HEX);
    Serial.println(sgp.serialnumber[2], HEX);

    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void loop() {
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop(); // Maintain MQTT connection

    if (!sgp.IAQmeasure()) {
        Serial.println("Measurement failed");
        return;
    }

    Serial.print("TVOC: "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
    Serial.print("eCO2: "); Serial.print(sgp.eCO2); Serial.println(" ppm");

    if (!sgp.IAQmeasureRaw()) {
        Serial.println("Raw Measurement failed");
        return;
    }

    Serial.print("Raw H2: "); Serial.print(sgp.rawH2); Serial.print(" \t");
    Serial.print("Raw Ethanol: "); Serial.print(sgp.rawEthanol); Serial.println("");

    // Display data on M5StickC Plus
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("TVOC: %d ppb", sgp.TVOC);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("eCO2: %d ppm", sgp.eCO2);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.printf("H2: %d", sgp.rawH2);
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.printf("Ethanol: %d", sgp.rawEthanol);

    // Create JSON object
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["TVOC"] = sgp.TVOC;
    jsonDoc["eCO2"] = sgp.eCO2;
    jsonDoc["H2"] = sgp.rawH2;
    jsonDoc["Ethanol"] = sgp.rawEthanol;

    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);

    // Publish JSON to MQTT
    if (client.publish(mqtt_topic, jsonBuffer)) {
        Serial.println("MQTT Message Sent: " + String(jsonBuffer));
    } else {
        Serial.println("MQTT Publish Failed");
    }

    delay(1000); // Wait 1 second before sending again
}
