#include <ArduinoJson.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#define MAX_MESSAGE_LENGTH 128  // Adjusted buffer to prevent overflow
char messageBuffer[MAX_MESSAGE_LENGTH];
bool receiving = false;
int bufferIndex = 0;

// LoRaWAN Configuration
static osjob_t sendjob;
const unsigned TX_INTERVAL = 60;  // Send every 60 seconds

static const u1_t PROGMEM APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

static const u1_t PROGMEM DEVEUI[8] = {0x9C, 0xE9, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70};
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

static const u1_t PROGMEM APPKEY[16] = {0x89, 0xE2, 0xA9, 0x60, 0x83, 0x72, 0xE2, 0x4D, 0xC5, 0x34, 0xBE, 0xFC, 0x6A, 0xEA, 0x1C, 0x55};
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 7,
    .dio = {2, 5, 6},
};

void setup() {
    Serial.begin(9600);
    delay(2000);  // Give some time for Serial to stabilize

    Serial.println(F("System Initializing..."));

    os_init();
    LMIC_reset();
    Serial.println(F("Joining TTN network..."));
    LMIC_startJoining();
}

void loop() {
    os_runloop_once();  // Keep LoRaWAN running

    if (Serial.available()) {
        memset(messageBuffer, 0, MAX_MESSAGE_LENGTH);  // Clear buffer before reading
        int bytesRead = Serial.readBytesUntil('\n', messageBuffer, MAX_MESSAGE_LENGTH - 1);
        messageBuffer[bytesRead] = '\0';  // Null-terminate

        Serial.println(F("Full message received:"));
        Serial.println(messageBuffer);

        processIncomingMessage(messageBuffer);
    }
}

void processIncomingMessage(char *message) {
    Serial.println(F("Processing JSON message (Manual Parsing)..."));

    // Extract JSON payload between { ... }
    char *start = strchr(message, '{');
    char *end = strrchr(message, '}');

    if (!start || !end || start >= end) {
        Serial.println(F("Error: JSON markers not found!"));
        return;
    }

    // Make sure the extracted JSON is null-terminated
    int jsonLength = end - start + 1;
    char jsonBuffer[128];  // Small buffer
    strncpy(jsonBuffer, start, jsonLength);
    jsonBuffer[jsonLength] = '\0';

    Serial.print(F("Extracted JSON: "));
    Serial.println(jsonBuffer);

    // Extract values manually using sscanf() (low memory)
    int temp, humidity, TVOC, eCO2, H2;
    int matched = sscanf(jsonBuffer, "{\"temp\": %d, \"humidity\": %d, \"TVOC\": %d, \"eCO2\": %d, \"H2\": %d}",
                         &temp, &humidity, &TVOC, &eCO2, &H2);

    if (matched < 5) {  // Check if all 5 values were extracted correctly
        Serial.println(F("Error: Failed to parse JSON manually!"));
        return;
    }

    Serial.println(F("✅ JSON parsed successfully (Manual)!"));

    // 🟢 Prepare LoRa payload (5 bytes)
    uint8_t lora_payload[5] = { (uint8_t)temp, (uint8_t)humidity, (uint8_t)TVOC, (uint8_t)eCO2, (uint8_t)H2 };

    Serial.print(F("Sending LoRa payload: "));
    for (int i = 0; i < 5; i++) {
        Serial.print(lora_payload[i]);
        Serial.print(" ");
    }
    Serial.println();

    sendLoRaMessage(lora_payload, 5);
}

void sendLoRaMessage(uint8_t *data, uint8_t len) {
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("⚠ LoRa transmission pending, skipping send."));
        return;
    }

    Serial.println(F("Transmitting LoRa data..."));
    LMIC_setTxData2(1, data, len, 0);
}

void onEvent(ev_t ev) {
    switch (ev) {
        case EV_JOINING:
            Serial.println(F("Joining LoRaWAN network..."));
            break;
        case EV_JOINED:
            Serial.println(F("Successfully joined TTN!"));
            LMIC_setLinkCheckMode(0);
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("Transmission successful!"));
            os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), sendLoRaMessage);
            break;
        case EV_TXSTART:
            Serial.println(F("Transmission started..."));
            break;
        case EV_TXCANCELED:
            Serial.println(F("Transmission cancelled."));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("LoRaWAN link lost. Reconnecting..."));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("LoRaWAN link restored."));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("Joining failed. Retrying..."));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("Rejoin failed. Restarting join process..."));
            LMIC_startJoining();
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}
