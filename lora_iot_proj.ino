#include <SoftwareSerial.h>

SoftwareSerial PiSerial(2, 3);  // RX, TX

void setup() {
  Serial.begin(9600);   // USB serial for debugging
  PiSerial.begin(9600); // UART for Raspberry Pi
}

void loop() {
  if (PiSerial.available()) {
    String msg = PiSerial.readStringUntil('\n');
    msg.trim();  // Remove any stray newlines or spaces
    Serial.println(msg);  // Display on Serial Monitor
    
    // Send acknowledgment to Pi
    PiSerial.println("ACK");
  }
  
  delay(100);  // Small delay to prevent buffer overflow
}
