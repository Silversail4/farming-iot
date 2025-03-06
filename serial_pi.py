import serial
import json
import time

# Initialize serial connection to LoRa (Arduino)
try:
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)  # Ensure correct USB port
    print("Serial connection established.")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit(1)

# Path to data.json
json_file = "data.json"

def read_sensor_data():
    """Read and clean sensor data from data.json."""
    try:
        with open(json_file, "r") as f:
            data = json.load(f)

        # Extract sensor values
        temp = data.get("Temperature_Sensor", {}).get("temp", "N/A").replace(" degree", "")
        humidity = data.get("Humidity Sensor", {}).get("humidity", "N/A").replace("%", "")
        co2_data = data.get("Co2 Sensor", {})

        TVOC = co2_data.get("TVOC", "0")
        eCO2 = co2_data.get("eCO2", "0")
        H2 = co2_data.get("H2", "0")

        return {
            "temp": temp,
            "humidity": humidity,
            "TVOC": TVOC,
            "eCO2": eCO2,
            "H2": H2
        }
    except (json.JSONDecodeError, FileNotFoundError) as e:
        print(f"Error reading {json_file}: {e}")
        return {}

def send_packet(data):
    """Send JSON data with markers using UTF-8 encoding."""
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
            print(f"Unexpected response: {ack}")

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

    time.sleep(1)

# Main loop
while True:
    try:
        sensor_data = read_sensor_data()
        if sensor_data:
            send_packet(sensor_data)
        time.sleep(3)  # Adjust delay to control data frequency
    except KeyboardInterrupt:
        print("Exiting...")
        ser.close()
        break
