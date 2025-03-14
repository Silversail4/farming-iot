from bluepy.btle import Scanner, Peripheral, BTLEException, BTLEDisconnectError
import time
import threading

# Device prefix you're searching for
device_name_prefix = "SmartFarm-M5"

# A set to store MAC addresses of devices that are currently being handled
current_devices = set()

# Function to scan for devices with the specified prefix in their name
def find_devices():
    scanner = Scanner()
    devices = scanner.scan(5.0)  # 5 seconds scan

    device_macs = []

    for dev in devices:
        name = dev.getValueText(9)  # Get the device name (field 9)
        if name and name.startswith(device_name_prefix):
            print(f"Found device: {name} (Address: {dev.addr})")
            device_macs.append(dev.addr)  # Add device MAC to the list

    return device_macs


# Function to connect to the device
def connect_to_device(device_mac):
    try:
        #print(f"Attempting to connect to {device_mac}...")

        # Connect to the M5StickC Plus device
        peripheral = Peripheral(device_mac, addrType="public", timeout=60)  # Increased timeout to 30 seconds
        print(f"Connected to device {device_mac}!")

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
        print(f"Device {device_mac} disconnected: {e}")
        return None, None, None
    except BTLEException as e:
        print(f"Failed to connect to device {device_mac}: {e}")
        return None, None, None


# Function to continuously get data from the device
def get_data_from_device(peripheral, temp_char, voltage_char, device_mac):
    try:
        # Read the temperature and battery values
        temp_data = temp_char.read().decode('utf-8')
        voltage_data = voltage_char.read().decode('utf-8')

        # Print the received data
        print(f"Device {device_mac} - Temperature: {temp_data} °C")
        print(f"Device {device_mac} - Battery Voltage: {voltage_data} V")

    except BTLEDisconnectError:
        print(f"Device {device_mac} disconnected. Attempting to reconnect...")
        return False  # Indicate that the device is disconnected
    except BTLEException as e:
        print(f"Error reading data from {device_mac}: {e}")
        return False

    return True


# Function to handle the device connection, data retrieval, and retry logic in a separate thread
def handle_device(device_mac):
    peripheral, temp_char, voltage_char = None, None, None

    while True:
        if peripheral is None:
            # Try to connect to the device
            peripheral, temp_char, voltage_char = connect_to_device(device_mac)

        if peripheral:
            # Retrieve data from the device
            success = get_data_from_device(peripheral, temp_char, voltage_char, device_mac)
            if not success:
                # If the device was disconnected, try reconnecting
                print(f"Device {device_mac} is disconnected. Retrying connection...")
                peripheral, temp_char, voltage_char = connect_to_device(device_mac)
        else:
            print(f"Failed to connect to {device_mac}. Retrying...")

        # Wait before retrying connection or fetching data
        time.sleep(3)


# Function to continuously scan for new devices in a separate thread
def scan_for_devices():
    while True:
        try:
            print("Scanning for devices...")
            device_macs = find_devices()

            print("Current Devices:")
            print(current_devices)
            # For each device found, start a new thread to handle the device
            for device_mac in device_macs:
                if device_mac not in current_devices:  # Check if device is not already being handled
                    print(f"Starting thread for device {device_mac}")
                    current_devices.add(device_mac)
                    thread = threading.Thread(target=handle_device, args=(device_mac,))
                    thread.daemon = True  # Set the thread to be a daemon thread
                    thread.start()

            # Wait before scanning again for new devices
            time.sleep(10)
        except BTLEDisconnectError as e:
            pass
        except BTLEException as e:
            pass



def main():
    # Start the scanning thread
    scan_thread = threading.Thread(target=scan_for_devices)
    scan_thread.daemon = True  # Set the thread to be a daemon thread
    scan_thread.start()

    # Keep the main program running so that the scanning thread and device threads can continue running
    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()