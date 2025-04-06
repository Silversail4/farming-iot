import paho.mqtt.client as mqtt
import json
import time
from cryptography.hazmat.primitives.asymmetric import x25519
from cryptography.hazmat.primitives import serialization

# MQTT Broker details
BROKER = "192.168.42.12"
PORT = 1883
TOPICS = ["sensor/co2", "sensor/mock", "sensor/light", "sensor/temp_humidity", "raspberrypi/threshold", "sensor/public_key"]

# eCO2 threshold for controlling the plug
ECO2_THRESHOLD = 0

# Shared secret and keys
shared_secret = None
iv = bytes(16)  # 16-byte IV, assuming same IV is used for encryption (you may need to send it)

def generate_keys():
    private_key = x25519.X25519PrivateKey.generate()
    public_key = private_key.public_key()
    return private_key, public_key

private_key, public_key = generate_keys()

def compute_shared_secret(private_key, peer_public_key):
    shared_secret = private_key.exchange(peer_public_key)
    return shared_secret

# Function to handle the message when the public key is received
def on_message(client, userdata, message):
    global shared_secret, private_key, public_key
    try:
        payload = message.payload
        topic = message.topic

        if topic == "sensor/public_key":
            print("[MQTT] Received Arduino's public key")

            # Load the Arduino's public key from the raw payload (32 bytes for Curve25519)
            arduino_public_key = x25519.X25519PublicKey.from_public_bytes(bytes(payload))

            # Compute the shared secret
            shared_secret = compute_shared_secret(private_key, arduino_public_key)
            print("[MQTT] Shared secret computed:")
            print(shared_secret.hex())

            # Send the server's public key to the Arduino (send in raw format)
            server_public_key_bytes = public_key.public_bytes(
                encoding=serialization.Encoding.Raw,
                format=serialization.PublicFormat.Raw
            )
            mqtt_client.publish("pi/public_key", server_public_key_bytes)
            print("[MQTT] Sent server's public key to Arduino.")

        else:
            # Decrypt the message received
            decrypted_data = decrypt_data(shared_secret, payload)
            print("[MQTT] Decrypted data:")
            print(decrypted_data)
            data = json.loads(decrypted_data)
            print(f"[MQTT] Decrypted data: {data}")
    except Exception as e:
        print(f"[MQTT] Error: {e}")

# MQTT setup function
def start_mqtt():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(BROKER, PORT)

    for topic in TOPICS:
        client.subscribe(topic)

    client.loop_start()
    return client

def publish_mqtt(topic, message):
    # Publish an encrypted message
    encrypted_data = encrypt_data(shared_secret, message)
    mqtt_client.publish(topic, encrypted_data)
    print(f"[MQTT] Sent encrypted message to {topic}")

def xor_encrypt(key, input_data):
    # Ensure the input is bytes
    if isinstance(input_data, str):
        input_data = input_data.encode('utf-8')  # Convert string to bytes

    encrypted_data = bytearray(len(input_data))

    # XOR encryption in byte-by-byte manner
    for i in range(len(input_data)):
        encrypted_data[i] = input_data[i] ^ key[i % len(key)]

    return bytes(encrypted_data)

def xor_decrypt(key, encrypted_data):
    # XOR decryption is the same as encryption
    return xor_encrypt(key, encrypted_data)

def decrypt_data(shared_secret, payload):
    # Decrypt the payload using XOR with the shared secret
    return xor_decrypt(shared_secret[:16], payload)

def encrypt_data(shared_secret, data):
    # Encrypt the data using XOR with the shared secret
    return xor_encrypt(shared_secret[:16], data)

# Main execution
if __name__ == "__main__":
    mqtt_client = start_mqtt()
    while True:
        time.sleep(1)
