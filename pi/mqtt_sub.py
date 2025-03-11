import paho.mqtt.client as mqtt
import json
import os
import time
import threading
import serial


#------------------------------------------------------------#
#-----MQTT Portion-------------------------------------------#
#------------------------------------------------------------#
# MQTT Broker details
BROKER = "192.168.137.169"  # Replace with your broker IP
PORT = 1883
TOPICS = [("sensor/co2", 0), ("sensor/mock", 0), ("sensor/light", 0), ("sensor/temp_humidity", 0)]  # Topics with QoS
JSON_FILE = "data.json"

file_lock = threading.Lock()

# Function to read JSON data safely
def read_json():
    with file_lock:  # Ensures only one thread accesses the file at a time
        if os.path.exists(JSON_FILE):
            with open(JSON_FILE, "r") as f:
                try:
                    return json.load(f)
                except json.JSONDecodeError:
                    print(f"Error decoding JSON in {JSON_FILE}")
                    return {}
        return {}

# Function to write JSON data safely
def write_json(data):
    with file_lock:  # Ensures exclusive write access
        with open(JSON_FILE, "w") as f:
            json.dump(data, f, indent=4)

# Callback function for MQTT messages
def on_message(client, userdata, message):
    try:
        payload = message.payload.decode("utf-8")
        data = json.loads(payload)  # Parse JSON
        
        # Read existing data safely
        existing_data = read_json()

        # Store data under its respective topic
        topic_key = message.topic.replace("/", "_")
        existing_data[topic_key] = data

        # Write updated JSON safely
        write_json(existing_data)

        print(f"Updated {JSON_FILE}: {data}")

    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

# Function to start MQTT client
def start_mqtt():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(BROKER, PORT)

    # Subscribe to topics
    for topic, qos in TOPICS:
        client.subscribe(topic, qos)

    print(f"Subscribed to: {[t[0] for t in TOPICS]}")

    client.loop_start()  # Start MQTT in a separate thread
    return client

# Function for another task (e.g., logging or monitoring)
""" def background_task():
    while True:
        print("Running background task...")
        time.sleep(5)  # Simulate work """
        
#------------------------------------------------------------#
#-----Lora Portion--------------------------------------------#
#------------------------------------------------------------#
# Initialize serial connection to LoRa (Arduino)
try:
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)  # Ensure correct USB port
    print("Serial connection established.")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit(1)

def read_sensor_data():
    """Read and extract sensor data from the updated data.json format."""
    try:
        data = read_json()

        # Extract CO2 sensor data from NODE_1
        co2_data = data.get("sensor_co2", {})
        TVOC = co2_data.get("TVOC", 0)
        eCO2 = co2_data.get("eCO2", 0)
        H2 = co2_data.get("H2", 0)
        Ethanol = co2_data.get("Ethanol", 0)  # NEW ATTRIBUTE
        
        #Temperature and humidity
        temp_data = data.get("sensor_temp_humidity", {})
        temp = temp_data.get("temp", 0)
        humidity = temp_data.get("humidity", 0)
        
        #Light
        light_data = data.get("sensor_light", {})
        light = light_data.get("light", 0)
        brightness = light_data.get("brightness", 0)

        return {
            "T": temp,
            "H": humidity,
            "V": TVOC,
            "C": eCO2,
            "H2": H2,
            "E": Ethanol ,
            "L": light,
            "B": brightness
        }
    except (json.JSONDecodeError, FileNotFoundError) as e:
        print(f"Error reading {JSON_FILE}: {e}")
        return {}

def send_packet(data):
    # Send JSON data with markers using UTF-8 encoding.
    try:
        json_data = json.dumps(data)  # Convert dict to JSON string
        packet = f"<START>{json_data}<END>\n"  # Ensure markers are included
        
        ser.write(packet.encode('utf-8'))  # Send encoded data
        ser.flush()  # Ensure buffer is cleared
        
        print(f"Sent packet to Arduino: {packet.strip()}")

        # Wait for acknowledgment from Arduino
        ack = ser.readline().decode('utf-8', errors='ignore').strip()
        if ack == "ACK":
            print("ACK received from Arduino.")
        else:
            print(f"Response: {ack}")

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

    time.sleep(1)


# Main script
if __name__ == "__main__":
    # Start MQTT in a separate thread
    mqtt_client = start_mqtt()

    # Start another background task in a separate thread
    """ task_thread = threading.Thread(target=background_task, daemon=True)
    task_thread.start() """
    
    """ while True:
        time.sleep(1) """

    try:
        while True:
            try:
                sensor_data = read_sensor_data()
                if sensor_data:
                    send_packet(sensor_data)
                time.sleep(10)  # Adjust delay to control data frequency
            except KeyboardInterrupt:
                print("Exiting...")
                ser.close()
                break
    except KeyboardInterrupt:
        print("Stopping MQTT client...")
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
