---
title: Getting Started
weight: -20
---

This page tells you how to get started with this project, including installation and basic configuration.

<!--more-->

{{< toc >}}

# Arduino IDE
## Install requirements

Head over to the official website of Arduino - [arduino.cc](https://www.arduino.cc/en/software) and download the installer as per the Operating System (Windows/Linux)

## Windows Setup
Install the executable file mentioned as WIN7 and newer. After the installation connect your board and see if you can see any `COM Ports` under the `Tools > `Ports` it `may be `labeled` as `COM3, COM4, COM*` in such a naming fashion, if there are no COM Ports visible you probably need the **CP210x USB to UART Bridge VCP Drivers** 

## Linux Install
Download the LINUX `32bits/64bits` zip file as per your device OS.\
Download the `arduino-*.*.*-linux64.tar.`xz` to the Downloads folder and extract its contents.

```Shell
# Make a copy of the folder on Desktop
cp -r arduino-*.*.* ~/Desktop/
cd ~/Desktop/arduino-*.*.*/

# run the script with SuperUser privileges
sudo ./install.sh
```

This should install the IDE to our system, and confirm your working of the board by running a simple program, if you get any Serial Port errors check the next section


## Adding ESP32 Add-on in the IDE
Go to `preferences` and search for `Additional Boards Manager URLs:` add the following URL in the text box `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` if you have any other URLs, you can combine them by separating them by a `,` like this `URL1, URL2`\
Go to `Tools > Board > Boards Manager` and search for `ESP32` by `Espressiff Systems` and install the files.\
Now you should be able to see `ESP32 Arduino` under `Tools > Boards`

## Other Settings
Under `File > Preferences` check the following boxes\
- Display Line Numbers
- Enable Code Folding
- Save when verifying or uploading
- Level of Compiler Warnings
- Display of Verbose Output during `compilation and upload`

# Platform IO
## Install Visual Studio Code
Download Visual Studio Code, for your platform from the [Official Website](https://code.visualstudio.com/download)
## Setup
* Install the [PlatformIO](https://platformio.org/) extension from the Extensions tab
* Create *New Project*
* Choose platform - **espressif32**, board - **esp32dev**, framework - **arduino**

# Additional Setup 
## Installing the CP210x Drivers [Windows]
Head over to Silicon Labs website - [silabs.com](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) and install the drivers
## User privileges error [Linux]
Sometimes you might not be having `User Privileges` to access the `Serial Ports` on your system.\
Thankfully the devs at Arduino have written a simple script for us to run to eliminate this issue
```bash
# get your username
echo $USER

# Run the script
./arduino-linux-setup.sh $USER

# Reboot System for changes to take effect
sudo reboot
```