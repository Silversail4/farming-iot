import paho.mqtt.client as mqtt
import json
import time
import threading
import serial

# This code is to make it take the previous values instead of reseting
# MQTT Broker details
BROKER = "192.168.137.253"
PORT = 1883
TOPICS = ["sensor/co2", "sensor/mock", "sensor/light", "sensor/temp_humidity"]

# Initialize serial connection to LoRa (Arduino)
try:
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    print("Serial connection established.")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit(1)

# Dictionary to store sensor data with default values
sensor_data = {
    "sensor_temp_humidity": {"temp": 0, "humidity": 0},
    "sensor_co2": {"TVOC": 0, "eCO2": 0},
    "sensor_light": {"light": 0, "brightness": 0}
}

# Callback function for received MQTT messages
def on_message(client, userdata, message):
    global sensor_data
    try:
        payload = message.payload.decode("utf-8")
        data = json.loads(payload)
        topic_key = message.topic.replace("/", "_")
        if topic_key in sensor_data:
            for key in data:
                sensor_data[topic_key][key] = data.get(key, sensor_data[topic_key].get(key, 0))
        else:
            sensor_data[topic_key] = data
        print(f"Received data: {topic_key} -> {data}")
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

# Function to initialize and start MQTT client
def start_mqtt():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(BROKER, PORT)
    
    for topic in TOPICS:
        client.subscribe(topic)
    
    print(f"Subscribed to: {TOPICS}")
    client.loop_start()
    return client

# Function to format and send data to LoRa
def send_to_lora():
    while True:
        if sensor_data:
            try:
                formatted_data = {
                    "T": sensor_data["sensor_temp_humidity"].get("temp", 0),
                    "H": sensor_data["sensor_temp_humidity"].get("humidity", 0),
                    "V": sensor_data["sensor_co2"].get("TVOC", 0),
                    "C": sensor_data["sensor_co2"].get("eCO2", 0),
                    "L": sensor_data["sensor_light"].get("light", 0),
                    "B": sensor_data["sensor_light"].get("brightness", 0)
                }
                packet = f"<START>{json.dumps(formatted_data, separators=(',', ':'))}<END>\n"
                ser.write(packet.encode('utf-8'))
                ser.flush()
                print(f"📤 Sent packet to Arduino: {packet.strip()}")
                ack = ser.readline().decode('utf-8', errors='ignore').strip()
                print("✅ ACK received from Arduino." if ack == "ACK" else f"⚠️ Unexpected response: {ack}")
            except serial.SerialException as e:
                print(f"❌ Serial error: {e}")
            except Exception as e:
                print(f"❌ Unexpected error: {e}")
        time.sleep(3)

if __name__ == "__main__":
    mqtt_client = start_mqtt()
    lora_thread = threading.Thread(target=send_to_lora, daemon=True)
    lora_thread.start()
    while True:
        time.sleep(1)
