//Used for generating mock data for testing
#include <Wire.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define NODE_ID "M2"
#define TOPIC "sensor/light"
#define SSID "DESKTOP-H7EA0ES 2580"
#define pw "496P29y)"
#define mqtt_server_ip "192.168.137.93"

// Wi-Fi and MQTT setup
const char* ssid = SSID;
const char* password = pw;
const char* mqtt_server = mqtt_server_ip; // Pi's IP address
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
    randomSeed(analogRead(0)); // Initialize random generator
}

void loop() {
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop(); // Maintain MQTT connection

    // Generate random values
    int rawValue = random(0, 4096);  // Simulate light sensor value (0 - 4095)
    float voltage = rawValue * (3.3 / 4095.0);    // Convert ADC value to voltage
    int darkness = map(rawValue, 0, 4095, 0, 100);  // Map to 0 - 100% brightness
    int brightness = 100 - darkness;

    const char* light_level;
    if (rawValue < 600) {
        light_level = "Over exposed";
    } else if (rawValue > 3000) {
        light_level = "Under exposed";
    } else {
        light_level = "Normal";
    }
    
    // Display on Serial Monitor
    Serial.print("Raw ADC Value: ");
    Serial.print(rawValue);
    Serial.print(" | Light Level: ");
    Serial.print(light_level);
    Serial.print(" | Voltage: ");
    Serial.print(voltage, 2);
    Serial.print("V | Brightness: ");
    Serial.println(brightness);

    // Display on M5StickC LCD
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("Raw value: %d", rawValue);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("Light level: %s", light_level);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.printf("Brightness: %d%%", brightness);

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
