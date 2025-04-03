import paho.mqtt.client as mqtt
import json
import time
import threading
import serial

# This is the code currently running on pi
# MQTT Broker details
BROKER = "192.168.137.253"
PORT = 1883
TOPICS = ["sensor/co2", "sensor/mock", "sensor/light", "sensor/temp_humidity", "raspberrypi/threshold"]

# eCO2 threshold for controlling the plug
ECO2_THRESHOLD = 0  

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
    global sensor_data, ECO2_THRESHOLD
    try:
        payload = message.payload.decode("utf-8")
        data = json.loads(payload)
        topic_key = message.topic.replace("/", "_")
        
        if topic_key == "raspberrypi_threshold":
            ECO2_THRESHOLD = int(data)
            print(f"[MQTT] Updated eCO2 threshold: {ECO2_THRESHOLD}")
        else:
            sensor_data[topic_key] = data
            print(f"[MQTT] Received data: {topic_key} -> {data}")
        
        # Check if eCO2 value is available and compare it to threshold
        eCO2_value = sensor_data["sensor_co2"].get("eCO2", 0)
        if eCO2_value < ECO2_THRESHOLD:
            print(f"[eCO2] Value {eCO2_value} is below threshold. Turning ON the plug.")
            publish_mqtt("ON")  # Publish to turn the plug ON
        else:
            print(f"[eCO2] Value {eCO2_value} is above threshold. Turning OFF the plug.")
            publish_mqtt("OFF")  # Publish to turn the plug OFF
            
    except json.JSONDecodeError as e:
        print(f"[MQTT] Error decoding JSON: {e}")
        

# Function to initialize and start MQTT client
def start_mqtt():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(BROKER, PORT)
    
    for topic in TOPICS:
        client.subscribe(topic)
    
    print(f"[MQTT] Subscribed to: {TOPICS}")
    client.loop_start()
    return client

# Function to publish MQTT message to control the plug
def publish_mqtt(command):
    try:
        topic = "cmnd/athom_plug_1/POWER"
        payload = command
        # Publish the MQTT message
        mqtt_client.publish(topic, payload)
        print(f"[MQTT] Sent {command} to {topic}")
    except Exception as e:
        print(f"[MQTT] Error publishing message: {e}")
        
# Function to format and send data to LoRa
def send_to_lora():
    while True:
        if sensor_data:
            try:
                formatted_data = {
                    "T": int(sensor_data.get("sensor_temp_humidity", {}).get("temp", 0)),
                    "H": int(sensor_data.get("sensor_temp_humidity", {}).get("humidity", 0)),
                    "V": sensor_data.get("sensor_co2", {}).get("TVOC", 0),
                    "C": sensor_data.get("sensor_co2", {}).get("eCO2", 0),
                    "L": sensor_data.get("sensor_light", {}).get("light", 0),
                    "B": sensor_data.get("sensor_light", {}).get("brightness", 0)
                }
                packet = f"<START>{json.dumps(formatted_data, separators=(',', ':'))}<END>\n"
                ser.write(packet.encode('utf-8'))
                ser.flush()
                print(f"[LORA] Sent packet to Arduino: {packet.strip()}")
                ack = ser.readline().decode('utf-8', errors='ignore').strip()
                print("[LORA] ACK received from Arduino." if ack == "ACK" else f"[LORA] Unexpected response: {ack}")
            except serial.SerialException as e:
                print(f"[LORA] Serial error: {e}")
            except Exception as e:
                print(f"[LORA] Unexpected error: {e}")
        time.sleep(3)

if __name__ == "__main__":
    mqtt_client = start_mqtt()
    lora_thread = threading.Thread(target=send_to_lora, daemon=True)
    lora_thread.start()
    while True:
        time.sleep(1)