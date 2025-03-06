#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32  // 32px height reduces RAM usage
#define OLED_RESET    -1   // No reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String incomingMessage = "";

// Function to initialize OLED display
void setupDisplay() {
    delay(100);  // Short delay for power stabilization

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
        while (1);  // Halt if display fails
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.display();
}

void setup() {
    Serial.begin(9600);  // USB Serial Communication
    setupDisplay();  // Initialize OLED Display
}

void loop() {
    while (Serial.available() > 0) {
        char incomingChar = Serial.read();

        if (incomingChar == '\n') {  // Detect end of message
            processIncomingMessage(incomingMessage);
            incomingMessage = "";  // Clear buffer for next message
        } else {
            incomingMessage += incomingChar;
        }
    }
    delay(50);
}

void processIncomingMessage(String message) {
    int start = message.indexOf("<START>");
    int end = message.indexOf("<END>");

    if (start == -1 || end == -1 || start >= end) {
        return;  // Ignore invalid packets
    }

    // Extract JSON payload
    String jsonPayload = message.substring(start + 7, end);

    // Parse JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonPayload);

    if (error) {
        return;  // Ignore if parsing fails
    }

    // Extract sensor data
    const char* temp = doc["temp"];
    const char* humidity = doc["humidity"];
    int TVOC = doc["TVOC"];
    int eCO2 = doc["eCO2"];
    int H2 = doc["H2"];

    // Print only the JSON payload
    Serial.println(jsonPayload);

    // Display on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("Temp: ")); display.println(temp);
    display.print(F("Humidity: ")); display.println(humidity);
    display.print(F("TVOC: ")); display.println(TVOC);
    display.print(F("eCO2: ")); display.println(eCO2);
    display.print(F("H2: ")); display.println(H2);
    display.display();
}
