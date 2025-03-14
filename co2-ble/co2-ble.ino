#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <M5StickCPlus.h>
#include <BLEDevice.h>
#include <BLEServer.h>

#define NODE_ID 3

Adafruit_SGP30 sgp;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;  // Update every 15 sec

bool deviceConnected = false;

// Unique BLE server name
#define bleServerName "SmartFarm-M5-CO2"

// BLE Service UUID
#define SERVICE_UUID "01234567-0123-4567-89ab-0123456789ab"
#define CO2CHARACTERISTIC_UUID "01234567-0123-4567-89ab-0123456789cd"

BLECharacteristic CO2Characteristics(CO2CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor CO2Descriptor(BLEUUID((uint16_t)0x2902));

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected to client");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected from client");
  }
};

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    //Serial.println("SGP30 test");

    // if (!sgp.begin()) {
    //     Serial.println("Sensor not found :(");
    //     while (1);
    // }
    
    Serial.print("Found SGP30 serial #");
    Serial.print(sgp.serialnumber[0], HEX);
    Serial.print(sgp.serialnumber[1], HEX);
    Serial.println(sgp.serialnumber[2], HEX);

    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    BLEDevice::init(bleServerName);

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *bleService = pServer->createService(SERVICE_UUID);

    bleService->addCharacteristic(&CO2Characteristics);
    CO2Descriptor.setValue("eCO2");
    CO2Characteristics.addDescriptor(&CO2Descriptor);

    bleService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pServer->getAdvertising()->start();
    
    Serial.println("Waiting for client connection...");
}

void loop() {
    // if (!sgp.IAQmeasure()) {
    //     Serial.println("Measurement failed");
    //     return;
    // }
    // Generate a random number between 400 and 450
    int randomValue = random(400, 451);
    //int eco2Value = sgp.eCO2;  // Get eCO2 reading

    Serial.print("eCO2: ");
    Serial.print(randomValue);
    Serial.println(" ppm");

    // Display on M5StickC Plus
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 20);
    M5.lcd.printf("BLE version");
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.printf("eCO2: %d ppm", randomValue);
    // M5.Lcd.setCursor(10, 80);
    // M5.Lcd.printf("H2: %d", sgp.rawH2);
    // M5.Lcd.setCursor(10, 110);
    // M5.Lcd.printf("Ethanol: %d", sgp.rawEthanol);

    // Send eCO2 value over BLE if connected
    if (deviceConnected) {
        char valueStr[10];
        sprintf(valueStr, "%d ppm", randomValue);
        CO2Characteristics.setValue(valueStr);
        CO2Characteristics.notify();
        Serial.println("Sent eCO2 via BLE");
    }

    delay(1000);
}
