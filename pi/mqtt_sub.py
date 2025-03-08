import paho.mqtt.client as mqtt
import json
import os
import time
import threading

# MQTT Broker details
BROKER = "192.168.10.127"  # Replace with your broker IP
PORT = 1883
TOPICS = [("sensor/co2", 0), ("sensor/mock", 0)]  # Topics with QoS
JSON_FILE = "data.json"

# Function to read existing JSON data
def read_json():
    if os.path.exists(JSON_FILE):
        with open(JSON_FILE, "r") as f:
            return json.load(f)
    return {}

# Function to write updated data to JSON
def write_json(data):
    with open(JSON_FILE, "w") as f:
        json.dump(data, f, indent=4)

# Callback function for received messages
def on_message(client, userdata, message):
    try:
        payload = message.payload.decode("utf-8")  # Decode message
        data = json.loads(payload)  # Parse JSON

        # Read existing data
        existing_data = read_json()

        # Store message data under its respective topic
        topic_key = message.topic.replace("/", "_") + "_NODE_" + str(data["id"])  # makes the key using topic and id
        existing_data[topic_key] = data

        # Save updated JSON
        write_json(existing_data)

        print(f"Updated {JSON_FILE}: {data}")

    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

# Function to initialize MQTT client
def start_mqtt():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(BROKER, PORT)

    # Subscribe to all topics
    for topic, qos in TOPICS:
        client.subscribe(topic, qos)

    print(f"Subscribed to: {[t[0] for t in TOPICS]}")
    
    # Start MQTT loop in a separate thread
    client.loop_start()

    return client

# Function for another task (e.g., logging or monitoring)
def background_task():
    while True:
        print("Running background task...")
        time.sleep(5)  # Simulate work

# Main script
if __name__ == "__main__":
    # Start MQTT in a separate thread
    mqtt_client = start_mqtt()

    # Start another background task in a separate thread
    task_thread = threading.Thread(target=background_task, daemon=True)
    task_thread.start()

    try:
        while True:
            time.sleep(1)  # Keep the main thread alive
    except KeyboardInterrupt:
        print("Stopping MQTT client...")
        mqtt_client.loop_stop()
        mqtt_client.disconnect()