#include <SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial PiSerial(2, 3);  // RX (D2), TX (D3)
String incomingMessage = "";

void setup() {
  Serial.begin(9600);   // For debugging
  PiSerial.begin(9600); // For Pi communication
  Serial.println("Arduino ready to receive data...");
}

void loop() {
  while (PiSerial.available() > 0) {
    char incomingChar = PiSerial.read();

    // Build message until end marker
    if (incomingChar == '\n') {
      processIncomingMessage(incomingMessage);
      incomingMessage = "";  // Clear buffer for next packet
    } else {
      incomingMessage += incomingChar;
    }
  }

  delay(50); // To prevent buffer overflow
}

void processIncomingMessage(String message) {
  // Check start and end markers
  int start = message.indexOf("<START>");
  int end = message.indexOf("<END>");

  if (start == -1 || end == -1 || start >= end) {
    Serial.println("Invalid packet format.");
    return;
  }

  // Extract payload and checksum
  String payloadWithChecksum = message.substring(start + 7, end); // between <START> and <END>
  int separator = payloadWithChecksum.indexOf("|");

  if (separator == -1) {
    Serial.println("Invalid checksum format.");
    return;
  }

  String payload = payloadWithChecksum.substring(0, separator);
  String receivedChecksum = payloadWithChecksum.substring(separator + 1);

  // Verify checksum
  String calculatedChecksum = calculateChecksum(payload);

  if (calculatedChecksum != receivedChecksum) {
    Serial.println("Checksum mismatch. Data corrupted.");
    return;
  }

  // Parse JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract sensor data
  const char* temp = doc["temp"];
  const char* humidity = doc["humidity"];
  int TVOC = doc["TVOC"];
  int eCO2 = doc["eCO2"];
  int H2 = doc["H2"];
  int Ethanol = doc["Ethanol"];

  // Display data
  Serial.print("Received temp: ");
  Serial.println(temp);
  Serial.print("Received humidity: ");
  Serial.println(humidity);
  Serial.print("Received TVOC: ");
  Serial.println(TVOC);
  Serial.print("Received eCO2: ");
  Serial.println(eCO2);
  Serial.print("Received H2: ");
  Serial.println(H2);
  Serial.print("Received Ethanol: ");
  Serial.println(Ethanol);

  // Send ACK
  PiSerial.println("ACK");
}

String calculateChecksum(String data) {
  unsigned long hash = 5381;
  for (int i = 0; i < data.length(); i++) {
    hash = ((hash << 5) + hash) + data[i];
  }
  return String(hash, HEX);
}
