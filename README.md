# Connection from Raspberry Pi to LoRa Device
## Step 1: Configuring Raspberry Pi
On your Raspberry Pi, type "sudo nano /boot/firmware/config.txt" and add/uncomment these lines:
```
enable_uart=1

dtoverlay=disable-bt
```
**Note:** 'dtoverlay' may have been called before so you may want to comment it out. 

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
## Step 2: Testing Serial Connection on Raspberry Pi
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
## Step 3: Testing on LoRa Device
Download the ```lora_iot_proj.ino``` file and burn it on the Arduino Uno

I connected the Serial cable to my computer. Not sure if it works otherwise

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












