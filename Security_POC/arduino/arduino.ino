#include <Wire.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#define NODE_ID 2
#define SSID "Mario's A55"
#define PASSWORD "password123"
#define MQTT_SERVER_IP "192.168.42.12"

// MQTT Topics
#define TOPIC "sensor/temp_humidity"
#define PUBLIC_KEY_TOPIC "sensor/public_key"
#define SERVER_PUBLIC_KEY_TOPIC "pi/public_key"

// Wi-Fi and MQTT setup
const char* ssid = SSID;
const char* password = PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP; // Pi's IP address

WiFiClient espClient;
PubSubClient client(espClient);

// X25519 Key Generation and Shared Secret Calculation
unsigned char clientPubKey[32] = { 0 };
unsigned char clientPrivKey[32] = { 0 };
unsigned char serverPubKey[32] = { 0 };
unsigned char sharedSecret[32] = { 0 };

// XOR Encryption Key
unsigned char xor_key[16] = { 0 }; // 128-bit XOR key

void printhex(unsigned char* array, int size) {
  for (int i = 0; i < size; i++) {
    char str[3];
    sprintf(str, "%02x", (int)array[i]);
    Serial.print(str);
  }
  Serial.println();
}

void genkeyx25519(unsigned char* pubkey, unsigned char* privkey) {
    mbedtls_ecdh_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);
    mbedtls_ecdh_init(&ctx);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 0, 0);

    mbedtls_ecp_group_load(&ctx.grp, MBEDTLS_ECP_DP_CURVE25519);        
    mbedtls_ecdh_gen_public(&ctx.grp, &ctx.d, &ctx.Q, mbedtls_ctr_drbg_random, &ctr_drbg);

    mbedtls_mpi_write_binary_le(&ctx.Q.X, pubkey, 32);
    mbedtls_mpi_write_binary_le(&ctx.d, privkey, 32);

    mbedtls_ecdh_free(&ctx);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

void calcsecretx25519(unsigned char * privkey, unsigned char * serverpubkey, unsigned char* sharedsecret) {
    mbedtls_ecdh_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);
    mbedtls_ecdh_init(&ctx);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 0, 0);

    mbedtls_ecp_group_load(&ctx.grp, MBEDTLS_ECP_DP_CURVE25519);

    mbedtls_mpi_read_binary_le(&ctx.d, privkey, 32);
    mbedtls_mpi_lset(&ctx.Qp.Z, 1);
    mbedtls_mpi_read_binary_le(&ctx.Qp.X, serverpubkey, 32);

    size_t olen;
    int ret = mbedtls_ecdh_calc_secret(&ctx, &olen, sharedSecret, 32, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        Serial.printf("Error in calculating secret: %d\n", ret);
        return;
    }

    mbedtls_ecdh_free(&ctx);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

// XOR Encryption Function
void xor_encrypt_decrypt(uint8_t *key, uint8_t *data, size_t data_len, uint8_t *output) {
    for (size_t i = 0; i < data_len; i++) {
        output[i] = data[i] ^ key[i % 16]; // XOR each byte of data with the key
    }
}

// MQTT and Wi-Fi Setup (Same as before)

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

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, SERVER_PUBLIC_KEY_TOPIC) == 0) {
        // Handle the received server's public key
        Serial.println("Received server public key:");
        memcpy(serverPubKey, payload, length);
        printhex(serverPubKey, sizeof(serverPubKey));

        // Calculate the shared secret using our private key and the server's public key
        calcsecretx25519(clientPrivKey, serverPubKey, sharedSecret);

        // Derive the XOR key from the shared secret (use first 16 bytes of shared secret)
        memcpy(xor_key, sharedSecret, 16);

        Serial.println("Shared Secret:");
        printhex(sharedSecret, sizeof(sharedSecret));
        Serial.println("XOR Key:");
        printhex(xor_key, sizeof(xor_key));
    }
}

void setup() {
    Serial.begin(115200);
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    // Set up Wi-Fi
    setup_wifi();

    // Set up MQTT
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback);

    // Generate keys
    genkeyx25519(clientPubKey, clientPrivKey);
    Serial.println("Generated public key:");
    printhex(clientPubKey, sizeof(clientPubKey));
    printhex(clientPrivKey, sizeof(clientPrivKey));

    // Publish the public key to the server
    if (client.connect("M5StickCPlus_Client")) {
        Serial.println("Publishing public key to MQTT server...");
    } else {
        Serial.println("Failed to connect to MQTT server!");
        reconnect_mqtt();
    }
    // Subscribe to the server's public key topic
    client.subscribe(SERVER_PUBLIC_KEY_TOPIC);
    client.publish(PUBLIC_KEY_TOPIC, clientPubKey, sizeof(clientPubKey));
}

void loop() {
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop(); // Maintain MQTT connection

    // Generate mock temperature and humidity data
    float temperature = random(15, 35) + random(0, 100) / 100.0;  // 15°C - 35°C
    float humidity = random(30, 90) + random(0, 100) / 100.0;  // 30% - 90% RH

    Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" °C\t");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

    // Create JSON object
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["id"] = NODE_ID;
    jsonDoc["tempe"] = temperature;
    jsonDoc["humidity"] = humidity;

    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);
    Serial.println(jsonBuffer);

    size_t input_len = strlen(jsonBuffer);

    // Encrypt the JSON data using XOR
    unsigned char encrypted_data[input_len];
    xor_encrypt_decrypt(xor_key, (unsigned char*)jsonBuffer, input_len, encrypted_data);

    Serial.println("Encrypted Data (Hex):");
    printhex(encrypted_data, sizeof(encrypted_data));

    // Publish the encrypted JSON to MQTT
    if (client.publish(TOPIC, encrypted_data, sizeof(encrypted_data))) {
        Serial.println("Encrypted MQTT Message Sent.");
    } else {
        Serial.println("Encrypted MQTT Publish Failed");
    }

    delay(1000); // Wait 1 second before sending again
}
