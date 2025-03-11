# Connection from Raspberry Pi to LoRa Device
## Step 1: Configuring Raspberry Pi
On your Raspberry Pi, type "sudo nano /boot/firmware/config.txt" and add/uncomment these lines:
```
enable_uart=1

dtoverlay=disable-bt
```
**Note:** 'dtoverlay' may have been called before so you may want to comment it out. 

On your Raspberry Pi, type "sudo raspi-config" and

```Disable the shell access over serial (login shell)``` 

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
From my code, this is my configurations
Raspberry Pi -> Arduino Uno
```
GP14 (TXD) -> d2
GP15 (RXD) -> d3
GND -> GND
```
In reference to the picture:

GP14 -> d2 will be the purple cable

GP15 -> d3 will be the green cable next to the purple cable

GND -> GND will be the farthest and longest green cable

**Picture Reference:**
![image](https://github.com/user-attachments/assets/a284df25-5d9d-4b1f-809c-d636d6a7e4c8)

---------------------------------------------
## Step 3: Testing Serial Connection on Raspberry Pi
Download the 'serial_pi.py' and transfer the file to your raspberry pi by typing this:
```
scp [source files] [user]@[host]:[path]
```
e.g.
```
scp serial_pi.py limku@192.168.1.161:./Projects
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
python serial_pi.py
```
**Note:** If python serial_pi.py results in an error that says 'Permission Denied', you can try this command<br>for temporary solution. For a permanent solution, refer to the Common Trouble Shooting section:
```
sudo python3 serial_pi.py
```
------------------------------------------------
## Step 4: Testing on LoRa Device
Download the ```lora_iot_proj.ino``` file and burn it on the Arduino Uno

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
python3 serial_pi.py
```

------------------------------------------------
#Set up with Gateway WIS

Turn on your Gateway WIS Edge Lite 2 Device and connect to the Wifi name: RAK7268V2_A545

On your web browser, enter:
```
https://192.168.230.1/
```

Enter the credentials:
User:
```
root
```
Password:
```
admin12345678!
```


##Then set up your network under WAN and connect to a Wifi or mobile hotspot to access internet. 
##Ensure Configuration network is set to Packet Forwarder and server: au1.cloud.thethings.network

------------------------------------------------
#Establish connection with TTN
Go to:
```
https://www.thethingsnetwork.org
```

Login to the credentials underneath your Gateway WIS Edge Lite 2

Select AU1 as region

Go to Gateways and you will see your registered WIS gateway 

Go to your registered WIS gateway > general setting, make sure your frequency plan is:
```
AU_915_928_FSB_2
```

Go to Applications and create a new application, the application ID






