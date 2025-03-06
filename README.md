# farming-iot
# Mosquitto Installation on Raspberry Pi

This guide will walk you through the process of installing and setting up Mosquitto MQTT Broker on a Raspberry Pi.

## Prerequisites

- Raspberry Pi with Raspbian OS installed and running.
- Internet connection for downloading packages.
- Access to the Raspberry Pi terminal (either directly or through SSH).

# Installing mosquitto on Pi
## Step 1: Update System Packages

Before installing any new software, make sure your system is up-to-date. Open the terminal and run:

```bash
sudo apt-get update
sudo apt-get upgrade -y
```
## Step 2: Install mosquitto
```bash
sudo apt-get install mosquitto mosquitto-clients -y
```

## Step 3: Enable and start mosquitto
```bash
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```
## Step 4: Change config file
```bash
sudo nano /etc/mosquitto/mosquitto.conf
```
Add these lines to allow remote MQTT clients
```yaml
listener 1883
allow_anonymous true
```

# Running python file

## Creating venv
```bash
python -m venv venv
```
## Activating venv
```bash
source venv/bin/activate
```

## Install dependancies
```bash
pip install paho-mqtt
```
## Running the file
```bash
python mqtt_sub.py
```



