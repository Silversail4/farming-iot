import serial
import json
import time

# Initialize serial connection
try:
    ser = serial.Serial('/dev/serial0', 9600, timeout=1)
    print("Serial connection established.")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit(1)

# Path to data.json
json_file = "data.json"

def read_sensor_data():
    """Read and clean data from data.json."""
    try:
        with open(json_file, "r") as f:
            data = json.load(f)

        # Extract and clean sensor values
        temp = data.get("Temperature_Sensor", {}).get("temp", "N/A").replace(" degree", "")
        humidity = data.get("Humidity Sensor", {}).get("humidity", "N/A").replace("%", "")
        return {"temp": temp, "humidity": humidity}
    except (json.JSONDecodeError, FileNotFoundError) as e:
        print(f"Error reading {json_file}: {e}")
        return {}

def calculate_checksum(data):
    """Calculate DJB2 hash for checksum."""
    hash = 5381
    for char in data:
        hash = ((hash << 5) + hash) + ord(char)
    return format(hash & 0xFFFFFFFF, '08x')  # Ensure it's 8 hex digits

def send_packet(data):
    """Send JSON data with start/end markers and checksum."""
    try:
        # Convert data to JSON string
        json_data = json.dumps(data)
        checksum = calculate_checksum(json_data)
        
        # Format the packet with markers and checksum
        packet = f"<START>{json_data}|{checksum}<END>\n"
        ser.write(packet.encode('utf-8'))
        print(f"Sent packet to Arduino: {packet.strip()}")

        # Wait for ACK
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
        time.sleep(3)
    except KeyboardInterrupt:
        print("Exiting...")
        ser.close()
        break
