import json
import time
import threading
import random
import serial

# Initialize serial connection to LoRa (Arduino)
try:
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    print("Serial connection established.")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit(1)

packets_sent = 0  # Counter for packets sent to Arduino

# Function to format and send mock data to LoRa
def send_to_lora():
    global packets_sent
    while True:
        try:
            # Generate mock data for stress testing
            mock_data = {
                "T": random.randint(20, 30),  # Mock temperature
                "H": random.randint(40, 60),  # Mock humidity
                "V": random.randint(0, 500),  # Mock TVOC
                "C": random.randint(400, 600),  # Mock eCO2
                "L": random.randint(100, 1000),  # Mock light
                "B": random.randint(0, 255)  # Mock brightness
            }
            packet = f"<START>{json.dumps(mock_data, separators=(',', ':'))}<END>\n"
            packet_size = len(packet.encode('utf-8'))
            print(f"[LORA] Packet size: {packet_size} bytes")
            ser.write(packet.encode('utf-8'))
            ser.flush()
            print(f"[LORA] Sent packet to Arduino: {packet.strip()}")
            packets_sent += 1
            print(f"[LORA] Packets sent: {packets_sent}")
            ack = ser.readline().decode('utf-8', errors='ignore').strip()
            print("[LORA] ACK received from Arduino." if ack == "ACK" else f"[LORA] Unexpected response: {ack}")
        except serial.SerialException as e:
            print(f"[LORA] Serial error: {e}")
        except Exception as e:
            print(f"[LORA] Unexpected error: {e}")
        # Reduce sleep time to increase stress
        time.sleep(0.1)  # Send packets every 100ms

if __name__ == "__main__":
    lora_thread = threading.Thread(target=send_to_lora, daemon=True)
    lora_thread.start()
    while True:
        time.sleep(1)