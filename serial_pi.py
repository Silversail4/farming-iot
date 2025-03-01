import serial
import json
import time

# Open serial port
ser = serial.Serial('/dev/serial0', 9600, timeout=1)

# Path to data.json file
json_file = "data.json"

def read_sensor_data():
    """Reads temperature and humidity data from data.json."""
    try:
        with open(json_file, "r") as f:
            data = json.load(f)
        
        # Extract and clean temp and humidity values
        temp = data.get("Temperature_Sensor", {}).get("temp", "N/A").replace(" degree", "")
        humidity = data.get("Humidity Sensor", {}).get("humidity", "N/A").replace("%", "")
        
        return temp, humidity
    
    except (json.JSONDecodeError, FileNotFoundError) as e:
        print(f"Error reading {json_file}: {e}")
        return "N/A", "N/A"

def send_data(label, value):
    """Sends a labeled value to Arduino and waits for acknowledgment."""
    message = f"{label}:{value}\n"
    ser.write(message.encode('utf-8'))
    print(f"Sent to Arduino: {message.strip()}")

    # Wait for acknowledgment from Arduino
    ack = ser.readline().decode('utf-8').strip()
    if ack:
        print(f"Received from Arduino: {ack}")
    time.sleep(1)  # Ensure time to process before next send

while True:
    # Read sensor data
    temp, humidity = read_sensor_data()

    # Ensure data is valid before sending
    if temp != "N/A" and humidity != "N/A":
        send_data("temp", temp)
        send_data("humidity", humidity)

    # Poll every 30 seconds
    time.sleep(3)
