import serial

# Open serial port (ttyAMA0 or ttyS0 depending on setup)
ser = serial.Serial('/dev/serial0', 9600, timeout=1)

while True:
    ser.write(b'Hello from Pi!\n')
    data = ser.readline().decode('utf-8').strip()
    if data:
        print(f"Received from Maker Uno: {data}")
