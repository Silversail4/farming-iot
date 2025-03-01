#include <SoftwareSerial.h>

// Define RX (D2) and TX (D3) pins
SoftwareSerial PiSerial(2, 3); 

void setup() {
  Serial.begin(9600);   // For debugging via USB
  PiSerial.begin(9600); // UART to Raspberry Pi
}

void loop() {
  if (PiSerial.available()) {
    String msg = PiSerial.readStringUntil('\n');
    Serial.println("From Pi: " + msg);  // Debug message to Serial Monitor
  }
  
  PiSerial.println("Hello from Maker Uno!");
  delay(1000);
}
