# CSC2106: Container Farming Project (Group 23) 🌱
---
- This project focuses on container farming using an IoT-based monitoring system. The system collects environmental data such as temperature, humidity, CO₂ levels, and light intensity from M5StickC Plus devices and transmits it to a Raspberry Pi aggregator via MQTT for real-time monitoring and analysis.
- **Set up Guides:**
  - 🟠 M5StickCPlus set up
  - 🍓 Raspberry Pi set up
  - 📡 LoRa set up
  - 🏰 Wisgate set up
  - ☁️ TTN set up
  - 🔴 Node-Red set up
---
# 📂 Project Structure
- `\M5-Sensors\` Code for M5StickC Plus devices, which collect and publish sensor data over MQTT.
- `\Raspberry-pi\` Code for the Raspberry Pi aggregator, responsible for receiving, processing, and storing sensor data.
- `\lora-wi\` Code for LoRa devices, enabling long-range communication between container units.
---
# 🟠 M5StickCPlus Sensors Set-Up
- All code for M5StickCPlus sensor can be found in the `/M5-Sensors/` folder.
- `co2-mqtt.ino` and `light-mqtt.ino` are flashed onto the M5StickCPlus in Arduino IDE
- `temp-humidity.py` is flashed using UI Flow IDE

**Hardware needed:**
- Sensors:
  - SGP30
  - ENV HAT
  - TSL2561 I2C
- M5StickCPlus
---
# 🍓 Raspberry Pi Aggregator Set-Up
- Code can be found in the `/Raspberry-pi/` folder.
- Mosquitto MQTT is needed on the Raspberry Pi.
  - Update `mosquitto.conf` as shown in below.
```bash
listener 1883
allow_anonymous true
```

#### Install the dependancies
```bash
pip install paho-mqtt
```
#### Run the program
```bash
python3 mqtt-lora-actuator.py
```
---
# 📡 Lora Set-Up
## Step 1: Configuring Raspberry Pi
On your Raspberry Pi, type "sudo nano /boot/firmware/config.txt" and add/uncomment these lines:
```
enable_uart=1

dtoverlay=disable-bt
```
**Note:** 'dtoverlay' may have been called before so you may want to comment it out. 

On your Raspberry Pi, type "sudo raspi-config" and navigate to Interface options

```Select Serial Port then disable login shell over serial``` 

```enable the serial port hardware```

Thereafter, reboot your device

---------------------------------------------

Thereafter, on your Raspberry Pi, type the following to disable serial console:
```
sudo systemctl disable serial-getty@ttyS0.service
```
---------------------------------------------

Reboot the Pi by typing: ```sudo reboot```

---------------------------------------------

Check if UART is enabled:

```
ls /dev/serial*
```
You should see something like ```/dev/serial0 -> ttyAMA0```

---------------------------------------------
## Step 2: Connecting the wirings from Raspberry Pi to LoRa Device
Connect the LoRa Device to Raspberry PI via USB connection

---------------------------------------------
## Step 3: Testing Serial Connection on Raspberry Pi
Download the 'serial_pi.py' and transfer the file to your raspberry pi by typing this:
```
scp [source files] [user]@[host]:[path]
```
e.g.
```
scp mqtt-lora-actuator.py limku@192.168.1.161:./Projects
```
**Note:** For me, I needed the '.' before passing to a directory that is connected to my home page, i.e. (home/Projects)

---------------------------------------------
Thereafter, go to the directory where your serial_pi.py is located and set up a virtual environment.
Install ```venv``` by typing:
```
sudo apt install python3-venv
```
Then, create a virtual environment:
```
python3 -m venv venv
```
Activate it by:
```
source venv/bin/activate
```
Install pyserial in your virtual environment:
```
pip install pyserial
```
And run the script in our virtual environment:
```
python mqtt-lora-actuator.py
```
**Note:** If python serial_pi.py results in an error that says 'Permission Denied', you can try this command<br>for temporary solution. For a permanent solution, refer to the Common Trouble Shooting section:
```
sudo python3 mqtt-lora-actuator.py
```
------------------------------------------------
## Step 4: Testing on LoRa Device
Download the ```wislora.ino``` file and burn it on the Arduino Uno

**Note:** Need to connect the Micro-USB cable for LoRa to work 

------------------------------------------------
# Common Troubleshooting:
## Permission Denied on Raspberry Pi Permanent Fix:
Add your user to the ```dialout``` group
```
sudo usermod -a -G dialout $USER
```
Logout and reboot:
```
sudo reboot
```
Thereafter, check if you are part of the group:
```
groups
```
Try running the script again:
```
python3 mqtt-lora-actuator.py
```

------------------------------------------------
# 🏰 Set up with Gateway WIS

## Turn on your Gateway WIS Edge Lite 2 Device and connect to the Wifi name: RAK7268V2_A545

## On your web browser, enter:
```
https://192.168.230.1/
```

## Enter the credentials:
User:
```
root
```
Password:
```
admin12345678!
```


## Then set up your network under WAN and connect to a Wifi or mobile hotspot to access internet. 
## Ensure Configuration network is set to Packet Forwarder and server: au1.cloud.thethings.network

------------------------------------------------
# ☁️ Establish connection with TTN
Go to:
```
https://www.thethingsnetwork.org
```

## Login to the credentials underneath your Gateway WIS Edge Lite 2

Select AU1 as region

Go to Gateways and you will see your registered WIS gateway 

Go to your registered WIS gateway > general setting, make sure your frequency plan is:
```
AU_915_928_FSB_2
```

## Go to Applications and create a new application, the application ID
Make sure you have the following configurations:
- Frequency Plan: “Australia 915-928 MHz, FSB 2”
- LoRaWan Version: “LoRaWan Specification 1.0.2”
- Regional Parameters Version: “RP001 Regional Parameters 1.0.2 revision B”
- Activation mode: Over The Air Activation (OTAA)
- Additional LoRaWAN class capabilities: None (Class A only)

Go to End Devices and register a new end device
- Choose and enter End Device ID.
- Join EUI: 00 00 00 00 00 00 00 00
- Application EUI: Auto-generate.
- App Key: Auto-generate.

------------------------------------------------

# Complete setup with Arduino

On your Arduino IDE: 
- Install the MCCI LoRaWAN LMIC library.
- In the Arduino IDE, select menu Sketch | Include Library | Manage Libraries
- In the search box enter: MCCI
- Click the MCCI LoRaWAN LMIC library by Terry Moore.
- Select the latest version and press the Install button.

Configure the MCCI LoRaWAN LMIC Library:
- Edit file lmic_project_config.h.
- This file can be found at: ".../libraries/MCCI_LoRaWAN_LMIC_library/project_config"
- Comment "#define CFG_us915 1", uncomment "#define CFG_au915 1"

To configure your end device to your Lora transceiver:
Go to your end device and look for Dev EUI, App EUI and App Key

Go to:
```
https://www.mobilefish.com/download/lora/eui_key_converter.html
```
Enter the 3 keys inside this website to convert to byte array.

Return to your arduino code and ensure these values are according to the keys generated from the website:
```
static const u1_t PROGMEM APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

static const u1_t PROGMEM DEVEUI[8] = {0x9C, 0xE9, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70};
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

static const u1_t PROGMEM APPKEY[16] = {0x89, 0xE2, 0xA9, 0x60, 0x83, 0x72, 0xE2, 0x4D, 0xC5, 0x34, 0xBE, 0xFC, 0x6A, 0xEA, 0x1C, 0x55};
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }
```

Check your lora device if the pin mappings are correct:
```
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 7,
    .dio = {2, 5, 6},
};
```


To compile and upload the code:
- Choose Arduino Uno as your board
- Upload the code into your connected Lora device

------------------------------------------------

# Once uploaded the arduino code, connect the lora device to the raspberry pi via USB connection

Then run:
```
python3 mqtt-lora-actuator.py
```

You will see the pi is sending messages to the lora device and on the terminal, it will display the messages Lora is sending.

# The following messages to understand the flow of the transmission from Pi -> Lora -> WIS gateway device

The packet containing the formatted data from Pi it sent to the Lora device
```
Sent packet to Arduino: <START>{"T":24,"H":42,"V":14,"C":400,"L":2000,"B":72}<END>
```

The Lora device has received the packet and is parsing the object to send to WIS gateway
```
Processing JSON message (Manual Parsing)...
DEBUG: Extracted JSON -> '{"T":24,"H":42,"V":14,"C":400,"L":2000,"B":72}'
DEBUG: Cleaned JSON -> '{"T":24,"H":42,"V":14,"C":400,"L":2000,"B":72}'
JSON Parsed Successfully!
```

Lora has successfully transmitted the data to WIS gateway
```
Parsed values -> Temp: 24, Humidity: 42, TVOC: 14, eCO2: 400, Light: 2000, Brightness: 72
Sending LoRa payload: 24 42 14 1 144 72 7 208
Transmission successful!
```

------------------------------------------------

# Data Retrieval and intepretation on TTN console

Go to Gateway > Live Data:
You will see:
```
Received uplink message
```
It means that the Gateway receives the message from Lora and it will proceed to display it on Application side of TTN


Go to Application > End Devices > Payload formatter

To intepret the data that is sent from Lora Device, select Custom Javascript and enter this decoder format:
```
function decodeUplink(input) {
    return {
        data: {
            T: input.bytes[0],             
            H: input.bytes[1],             
            V: input.bytes[2],                    
            C: (input.bytes[3] << 8) | input.bytes[4],  // eCO2 (2 bytes)
            B: input.bytes[5],                    
            L: (input.bytes[6] << 8) | input.bytes[7]   // Light (2 bytes)
        },
        warnings: [],
        errors: []
    };
}
```

Save changes.

Go to your registered application end device and look out for the decoded payload, you will see the extracted message that is sent from Lora Device!

------------------------------------------------

# 🔴 Data Visualization: Node-RED 

To visualize the data, install Node-RED on Windows

Pre-requisites:
- Download Node.js
- Verify installation: run
```
node -v
npm -v
```

Install Node-RED:
- run 
```
npm install -g --unsafe-perm node-red
```

Once installed, enter this in your terminal to start node-RED:
```
node-red
```

Go to your web browser and enter:
```
http://127.0.0.1:1880
```

On the node-RED interface, you can use nodes and link accordingly:
![image](https://github.com/user-attachments/assets/da2b2bcf-a87d-407d-8b69-b774b4489f7f)


------------------------------------------------

# Setting up MQTT with TTN to node-RED

Go back to your TTN console > Application > Integrations > MQTT

You will see the MQTT server host and server credentials for TTN to establish MQTT connection.

Go back to node-RED interface, search for MQTT nodes:
- MQTT in
- MQTT out

To receive the uplink message via MQTT from TTN to node-RED:
- Configure the MQTT in Node
- Topic name, follow this format: v3/csc2106container@ttn/devices/au-70b3d57ed006e99c/up
- Edit the topic name accordingly to the setting from your MQTT settings on the TTN console

In the node setting:
- Create a new server and click on the pencil icon
- Enter the server: au1.cloud.thethings.network
- Port number use 8883
- Click tick on TLS
- Then go to Security and fill in the credentials for MQTT from the TTN MQTT setting
- Click update and you are ready
- When doing the saving the settings, you may be prompted to upload a CAA cert, you can upload the isrgrootx1.pem and save it

Since we are sending a JSON object, take a JSON node and link it to the MQTT in node
- To decode the JSON, take a function node and link it on the other side of JSON node
- Function 1 for retrieving the attributes of each payload:
```
// Extract the payload
var payload = msg.payload;

// Log raw payload for debugging
node.warn("Raw Payload: " + JSON.stringify(payload, null, 2));

// Ensure payload is an object and not a string
if (typeof payload === "string") {
    try {
        payload = JSON.parse(payload);
    } catch (error) {
        node.warn("Error parsing payload as JSON!");
        return null;
    }
}

// Validate expected structure
if (!payload || !payload.uplink_message || !payload.uplink_message.decoded_payload) {
    node.warn("Invalid or missing data - Skipping processing.");
    return null;
}

// Extract decoded sensor data
var data = payload.uplink_message.decoded_payload;

// Log extracted data for debugging
node.warn("Extracted Data: " + JSON.stringify(data, null, 2));
var lightMode;
if (data.L >= 3000) {
    lightMode = "Dark";
} else if (data.L >= 1500) {
    lightMode = "Normal";
} else {
    lightMode = "Bright";
}

// Validate and create messages
var messages = [];
var keys = Object.keys(data);

keys.forEach(function (key) {
    if (data[key] !== undefined) {
        messages.push({ topic: key, payload: data[key] });
    }
});

messages.push({ topic: "lightMode", payload: lightMode });

// Log messages being sent
node.warn("Messages Sent: " + JSON.stringify(messages, null, 2));

// Return array of messages
return [messages];

```
- Function 2 for publishing the MQTT message responsible for changing the actuator threshold
```
// Extract slider value from payload
var sliderValue = msg.payload;

// Log received value for debugging
node.warn("🔧 Slider Adjusted - Value: " + sliderValue);

// Define MQTT topic for Raspberry Pi
var mqtt_topic = "raspberrypi/threshold";  // Adjust topic based on your MQTT setup

// Format MQTT message
var mqtt_message = {
    topic: mqtt_topic,
    payload: sliderValue,
    qos: 0,
    retain: false
};

// Log the outgoing MQTT message
node.warn("📡 Sending to MQTT: " + JSON.stringify(mqtt_message));

// Return the formatted message to MQTT out
return mqtt_message;
```

To debug this, add a debug node to the other side of the function node.
Click Deploy and you will see if the MQTT can receive the data from TTN.

------------------------------------------------

# To visualize the data into graphs and charts

Take a Switch node:
- Inside establish the topic names of each attribute to visualize
- Since we are visualizing the data, take out Gauge and Charts nodes
- Link the specific nodes to the each point of the Switch e.g. Temperature nodes will connect to the point of the Switch that is sending Temperature Topic
- Label your node and enter {{ msg.payload }} as value to visualize the data.

# Install to visualize on the dashboard
```
npm install node-red-dashboard

```

Click Deploy and enter:
```
http://127.0.0.1:1880/ui
```

You should see the data visualization now!
