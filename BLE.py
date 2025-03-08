from bluepy.btle import Scanner, Peripheral, BTLEException, BTLEDisconnectError
import time

# Device name you're searching for
device_name = "SmartFarm-M5#1"

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
        TEMP_CHAR_UUID = "01234567-0123-4567-89ab-0123456789cd"
        VOLTAGE_CHAR_UUID = "01234567-0123-4567-89ab-0123456789ef"

        # Get the service and characteristics
        service = peripheral.getServiceByUUID(SERVICE_UUID)

        # Temperature and Battery Characteristics
        temp_char = service.getCharacteristics(TEMP_CHAR_UUID)[0]
        voltage_char = service.getCharacteristics(VOLTAGE_CHAR_UUID)[0]

        return peripheral, temp_char, voltage_char

    except BTLEDisconnectError as e:
        print(f"Device disconnected: {e}")
        return None, None, None
    except BTLEException as e:
        print(f"Failed to connect to device: {e}")
        return None, None, None


# Function to continuously get data from the device
def get_data_from_device(peripheral, temp_char, voltage_char):
    try:
        # Read the temperature and battery values
        temp_data = temp_char.read().decode('utf-8')
        voltage_data = voltage_char.read().decode('utf-8')

        # Print the received data
        print(f"Temperature: {temp_data} °C")
        print(f"Battery Voltage: {voltage_data} V")

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
    temp_char = None
    voltage_char = None

    while True:
        # Scan for the device
        try:
            if device_mac is None:
                print("Scanning for devices...")
                device_mac = find_device()

            if device_mac:
                # Connect to the device if not connected
                if peripheral is None:
                    peripheral, temp_char, voltage_char = connect_to_device(device_mac)

                if peripheral:
                    # Retrieve data continuously every 5 seconds
                    success = get_data_from_device(peripheral, temp_char, voltage_char)

                    if not success:
                        # If the device was disconnected, try reconnecting
                        peripheral, temp_char, voltage_char = connect_to_device(device_mac)
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