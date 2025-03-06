#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <M5StickCPlus.h>

#define NODE_ID 2;
#define TOPIC "sensor/mock"
// Wi-Fi and MQTT setup

const char* ssid = "Linksys19988";
const char* password = "531jeffrey";
const char* mqtt_server = "192.168.10.127"; //pi's ip address

const char* mqtt_topic = TOPIC; // MQTT Topic

// Create instances
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  // put your setup code here, to run once:
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

// Function to generate random number between 0-100
int generate_random_number() {
  return random(0, 101);  // Generates a random number between 0 and 100
}

// Function to send mock data to MQTT
void send_mock_data() {
  
  // Generate a random number
  int random_number = generate_random_number();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.printf("Number sent: %d", random_number);

  // Create a JSON document
  StaticJsonDocument<200> jsonDoc;
  //JsonObject mockData = jsonDoc.createNestedObject("Mock_Data");
  
  // Prepare JSON object
  jsonDoc["id"] = NODE_ID;
  jsonDoc["random_number"] = random_number;
  
  // Convert JSON to string
  char jsonBuffer[200];
  serializeJson(jsonDoc, jsonBuffer);

  // Send the message to MQTT
  client.publish(mqtt_topic, jsonBuffer);

  // Print the message for debugging
  Serial.print("Published: ");
  Serial.println(jsonBuffer);
}


void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Ensure that the client is connected to MQTT
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  // Send mock data every 2 seconds
  send_mock_data();
  
  // Wait for 2 seconds before sending the next message
  delay(2000);
}
