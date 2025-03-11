#include <Wire.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define NODE_ID 4;
#define TOPIC "sensor/light"
#define SSID "vqqqq"
#define pw "qwerty123"
#define mqtt_server_ip "192.168.137.169"

#define LIGHT_SENSOR_PIN 32  // ADC1 Channel 0 (GPIO36)

// Wi-Fi and MQTT setup

const char* ssid = SSID;
const char* password = pw;
const char* mqtt_server = mqtt_server_ip; //pi's ip address

const char* mqtt_topic = TOPIC; // MQTT Topic

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
    M5.begin();             // Initialize M5StickC Plus
    M5.Lcd.setRotation(1);  // Rotate display for better readability
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    Serial.begin(115200);   // Start serial communication
    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void loop() {
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop(); // Maintain MQTT connection

    int rawValue = analogRead(LIGHT_SENSOR_PIN);  // Read light sensor value (0 - 4095)
    float voltage = rawValue * (3.3 / 4095.0);    // Convert ADC value to voltage
    float darkness = map(rawValue, 0, 4095, 0, 100);  // Map to 0 - 100% brightness
    float brightness = 100 - darkness;
    // Display on Serial Monitor
    Serial.print("Raw ADC Value: ");
    Serial.print(rawValue);
    Serial.print(" | Voltage: ");
    Serial.print(voltage, 2);
    Serial.print("V | Brightness: ");
    Serial.print(brightness);
    Serial.println("%");

    // Display on M5StickC LCD
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("Light: %d", rawValue);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("Voltage: %.2fV", voltage);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.printf("Brightness: %.0f%%", brightness);

    // Create JSON object
    StaticJsonDocument<200> jsonDoc;

    jsonDoc["id"] = NODE_ID;
    jsonDoc["light"] = rawValue;
    jsonDoc["brightness"] = brightness;

    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);


    // Publish JSON to MQTT
    if (client.publish(mqtt_topic, jsonBuffer)) {
        Serial.println("MQTT Message Sent: " + String(jsonBuffer));
    } else {
        Serial.println("MQTT Publish Failed");
    }

    delay(1000); // Small delay to reduce flickering
}
