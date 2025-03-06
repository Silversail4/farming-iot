import paho.mqtt.client as mqtt
import json
import os

# MQTT Broker details
BROKER = "192.168.10.127"  # PI's ip address
PORT = 1883  # Default MQTT port
#TOPIC = "sensor/co2" 
TOPICS = [("sensor/co2", 0), ("sensor/mock", 0)] # 0 is the QoS level, at most once
JSON_FILE = "data.json"

# Function to read the current data from the JSON file if it exists
def read_json():
    if os.path.exists(JSON_FILE):
        with open(JSON_FILE, "r") as f:
            return json.load(f)
    return {}

# Function to write or append data to the JSON file
def write_json(data):
    with open(JSON_FILE, "w") as f:  # Open the file in write mode to overwrite with updated data
        json.dump(data, f, indent=4)

# Callback when a message is received
def on_message(client, userdata, message):
    try:
        payload = message.payload.decode("utf-8")  # Decode message
        data = json.loads(payload)  # Parse JSON

        # Read the existing data from the JSON file
        existing_data = read_json()

        # Add or update the topic-specific data
        topic_name = message.topic.replace("/", "_")  # Sanitizing topic to be a valid key
        existing_data[topic_name] = data

        # Write the updated data back to the JSON file
        write_json(existing_data)

        print(f"Data written to {JSON_FILE}: {data}")

    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

# MQTT client setup
client = mqtt.Client()
client.on_message = on_message

client.connect(BROKER, PORT)
# Subscribe to multiple topics
for topic, qos in TOPICS:
    client.subscribe(topic, qos)

print(f"Subscribed to: {[t[0] for t in TOPICS]}, waiting for messages...")
client.loop_forever()  # Keep the script running
