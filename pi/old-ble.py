from bluepy.btle import Scanner, Peripheral, BTLEException, BTLEDisconnectError
import time

# If using venv use sudo venv/bin/python ble.py
# Device name you're searching for
device_name = "SmartFarm-M5-CO2"

# Function to scan for the device by name
def find_device():
    scanner = Scanner()
    devices = scanner.scan(5.0)  # 5 seconds scan

    device_mac = None

    for dev in devices:
        name = dev.getValueText(9)  # Get the device name (field 9)
        if name:
            print(f"Found device: {name} (Address: {dev.addr})")
            if name == device_name:
                device_mac = dev.addr
                print(f"Device found with name '{device_name}' - Address: {device_mac}")
                break

    return device_mac


# Function to connect to the device
def connect_to_device(device_mac):
    try:
        print(f"Attempting to connect to {device_mac}...")

        # Connect to the M5StickC Plus device
        peripheral = Peripheral(device_mac, addrType="public", timeout=30)  # Increased timeout to 60 seconds
        print("Connected to the device!")

        # Define UUIDs for service and characteristics
        SERVICE_UUID = "01234567-0123-4567-89ab-0123456789ab"
        CO2_CHAR_UUID = "01234567-0123-4567-89ab-0123456789cd"

        # Get the service and characteristics
        service = peripheral.getServiceByUUID(SERVICE_UUID)

        # Temperature and Battery Characteristics
        CO2_char = service.getCharacteristics(CO2_CHAR_UUID)[0]

        return peripheral, CO2_char

    except BTLEDisconnectError as e:
        print(f"Device disconnected: {e}")
        return None, None
    except BTLEException as e:
        print(f"Failed to connect to device: {e}")
        return None, None


# Function to continuously get data from the device
def get_data_from_device(peripheral, co2_char):
    try:
        # Read the temperature and battery values
        co2_data = co2_char.read().decode('utf-8')
        

        # Print the received data
        print(f"eCO2: {co2_data} ppm")
        

    except BTLEDisconnectError:
        print("Device disconnected. Attempting to reconnect...")
        return False  # Indicate that the device is disconnected
    except BTLEException as e:
        print(f"Error reading data: {e}")
        return False

    return True


def main():
    device_mac = None
    peripheral = None
    co2_char = None

    while True:
        # Scan for the device
        try:
            if device_mac is None:
                print("Scanning for devices...")
                device_mac = find_device()

            if device_mac:
                # Connect to the device if not connected
                if peripheral is None:
                    peripheral, co2_char= connect_to_device(device_mac)

                if peripheral:
                    # Retrieve data continuously every 5 seconds
                    success = get_data_from_device(peripheral, co2_char)

                    if not success:
                        # If the device was disconnected, try reconnecting
                        peripheral, co2_char = connect_to_device(device_mac)
                    else:
                        # Wait 5 seconds before retrieving data again
                        time.sleep(1)

                else:
                    print("Failed to connect or lost connection. Retrying...")
                    time.sleep(1)

            else:
                print(f"Device with name '{device_name}' not found. Retrying...")
                time.sleep(1)
        except BTLEDisconnectError as e:
            print("Device disconnected. Attempting to reconnect...")
            pass


if __name__ == "__main__":
    main()