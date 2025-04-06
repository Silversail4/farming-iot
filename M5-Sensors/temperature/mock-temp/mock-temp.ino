//Generates mock temperature data
#include <Wire.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define NODE_ID "M3"
#define TOPIC "sensor/temperature"
#define SSID "vqqqq"
#define pw "qwerty123"
#define mqtt_server_ip "192.168.137.253"

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

    Serial.println("Temperature & Humidity Simulation");

    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    randomSeed(analogRead(0)); // Initialize random generator
}

void loop() {
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop(); // Maintain MQTT connection

    // Generate random values for temperature and humidity
    float temperature = random(15, 35) + random(0, 100) / 100.0;  // 15°C - 35°C
    float humidity = random(30, 90) + random(0, 100) / 100.0;  // 30% - 90% RH

    Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" °C\t");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

    // Display data on M5StickC Plus
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.printf("Temp: %.2f C", temperature);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("Humidity: %.2f %%", humidity);

    // Create JSON object
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["id"] = NODE_ID;
    jsonDoc["tempe"] = temperature;
    jsonDoc["humidity"] = humidity;

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
