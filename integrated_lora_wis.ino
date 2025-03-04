#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// SoftwareSerial for communication with Raspberry Pi
SoftwareSerial PiSerial(2, 3);  // RX (D2), TX (D3)
String incomingMessage = "";

// LoRaWAN Credentials (Replace with your own TTN credentials)
static const u1_t PROGMEM APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

static const u1_t PROGMEM DEVEUI[8] = {0x9C, 0xE9, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70};
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

static const u1_t PROGMEM APPKEY[16] = {0x89, 0xE2, 0xA9, 0x60, 0x83, 0x72, 0xE2, 0x4D, 0xC5, 0x34, 0xBE, 0xFC, 0x6A, 0xEA, 0x1C, 0x55};
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

// LoRaWAN Transmission Variables
static uint8_t mydata[32];  // Buffer for sensor data transmission
static osjob_t sendjob;
const unsigned TX_INTERVAL = 60;

// LMIC Pin Mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 7,
    .dio = {2, 5, 6},
};

// Function to handle incoming messages from Pi
void processIncomingMessage(String message) {
    int start = message.indexOf("<START>");
    int end = message.indexOf("<END>");

    if (start == -1 || end == -1 || start >= end) {
        Serial.println("Invalid packet format.");
        return;
    }

    String payloadWithChecksum = message.substring(start + 7, end); 
    int separator = payloadWithChecksum.indexOf("|");

    if (separator == -1) {
        Serial.println("Invalid checksum format.");
        return;
    }

    String payload = payloadWithChecksum.substring(0, separator);
    String receivedChecksum = payloadWithChecksum.substring(separator + 1);

    if (calculateChecksum(payload) != receivedChecksum) {
        Serial.println("Checksum mismatch. Data corrupted.");
        return;
    }

    // Parse JSON Data
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract sensor values
    const char* temp = doc["temp"];
    const char* humidity = doc["humidity"];
    int TVOC = doc["TVOC"];
    int eCO2 = doc["eCO2"];
    int H2 = doc["H2"];
    int Ethanol = doc["Ethanol"];

    // Print sensor values
    //Serial.print("Temp: %s, Humidity: %s, TVOC: %d, eCO2: %d, H2: %d, Ethanol: %d\n", temp, humidity, TVOC, eCO2, H2, Ethanol);

    // Prepare LoRa payload
    String loraPayload = String("T:") + temp + ",H:" + humidity + ",TVOC:" + TVOC + ",eCO2:" + eCO2;
    loraPayload.toCharArray((char*)mydata, sizeof(mydata));

    // Send via LoRa
    do_send(&sendjob);

    // Send ACK to Raspberry Pi
    PiSerial.println("ACK");
}

// LoRaWAN Transmission Function
void do_send(osjob_t* j) {
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        LMIC_setTxData2(1, mydata, strlen((char*)mydata), 0);
        Serial.println(F("Packet queued for LoRa transmission."));
    }
}

// LMIC Event Handling
void onEvent(ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev) {
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            LMIC_setLinkCheckMode(0);
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
                Serial.println(F("Received ACK"));
            os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned)ev);
            break;
    }
}

// Calculate checksum
String calculateChecksum(String data) {
    unsigned long hash = 5381;
    for (int i = 0; i < data.length(); i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return String(hash, HEX);
}

// **Setup Function**
void setup() {
    Serial.begin(9600);
    PiSerial.begin(9600);
    Serial.println(F("Starting LoRaWAN and Serial Communication..."));


    // Initialize LoRa
    os_init();
    LMIC_reset();
    do_send(&sendjob); // Initial LoRa transmission
}

// **Main Loop**
void loop() {
    // Read from Raspberry Pi
    while (PiSerial.available() > 0) {
        char incomingChar = PiSerial.read();
        if (incomingChar == '\n') {
            processIncomingMessage(incomingMessage);
            incomingMessage = "";  // Reset buffer
        } else {
            incomingMessage += incomingChar;
        }
    }

    // Run LMIC event loop
    os_runloop_once();
    delay(50);
}
