import paho.mqtt.client as mqtt
import json

# MQTT Broker details
BROKER = "192.168.10.127"  # PI's ip address
PORT = 1883  # Default MQTT port
TOPIC = "sensor/co2"  # Replace with your actual topic
JSON_FILE = "data.json"

# Callback when a message is received
def on_message(client, userdata, message):
    try:
        payload = message.payload.decode("utf-8")  # Decode message
        data = json.loads(payload)  # Parse JSON

        # Write to JSON file
        with open(JSON_FILE, "w") as f:
            json.dump(data, f, indent=4)

        print(f"Data written to {JSON_FILE}: {data}")

    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

# MQTT client setup
client = mqtt.Client()
client.on_message = on_message

client.connect(BROKER, PORT)
client.subscribe(TOPIC)

print(f"Subscribed to {TOPIC}, waiting for messages...")
client.loop_forever()  # Keep the script running
