from m5stack import *
from m5ui import *
from uiflow import *
import hat
import json
import wifiCfg
from umqtt.simple import MQTTClient

#UPLOAD THIS USING UIFLOW

# Wi-Fi Credentials
SSID = "vqqqq"
PASSWORD = "qwerty123"

# MQTT Broker Details
MQTT_BROKER = "192.168.137.169"  # You can change this to your broker
MQTT_TOPIC = "sensor/temp_humidity"
CLIENT_ID = "5"

# Set up Screen
setScreenColor(0x111111)
lcd.setRotation(1)  # Rotate screen 90 degrees (USB on the right)

# Connect to Wi-Fi
wifiCfg.doConnect(SSID, PASSWORD)
while not wifiCfg.is_connected():
    wait(1)
print("Connected to Wi-Fi")

# Connect to MQTT
mqtt_client = MQTTClient(CLIENT_ID, MQTT_BROKER, port=1883)
mqtt_client.connect()
print("Connected to MQTT Broker")

# Initialize Sensor
hat_env_0 = hat.get(hat.ENV)

while True:
    lcd.clear()  # Clears screen to prevent overlapping text

    temp = hat_env_0.temperature  # Get temperature
    humid = hat_env_0.humidity  # Get humidity
    device_id = CLIENT_ID  # Unique ID for the device

    # Create JSON payload
    #data = {"id": device_id, "temp": temp, "humidity": humid}
    #json_data = json.dumps(data)
    
    json_data = '{"id":"%s","temp":%.2f,"humidity":%.2f}' % (device_id, temp, humid)

    # Publish to MQTT
    mqtt_client.publish(MQTT_TOPIC, json_data)

    # Print on screen
    lcd.print("Temp: {:.2f}C".format(temp), 10, 40, 0xffffff)
    lcd.print("Humidity: {:.2f}%".format(humid), 10, 60, 0xffffff)

    # Print to Serial (debugging)
    print("Published:", json_data)

    wait(5)  # Send every 5 seconds
