# Smart Socket
## Table of contents
- [About The Project](#about-the-project)
- [Getting Started](#getting-started)
    - [Hardware Requirements](#hardware-requirements)
    - [Circuit Diagram](#circuit-diagram)
    - [Software Requirements](#software-requirements)
    - [Setup](#setup)
- [Features](#features)
    - [Remote Control via Wi-Fi](#remote-control-via-wi-fi)
    - [Energy Monitoring](#energy-monitoring)
    - [Schedule Configuration](#schedule-configuration)
    - [Night Light Mode](#night-light-mode)
    - [Temperature Mode](#temperature-mode)
    - [Safety Configuration](#safety-configuration)
    - [User Profile](#user-profile)
- [License](#license)
## About The Project
The Smart Socket project aims to create an intelligent socket based on the ESP32 microcontroller. The goal is to design a versatile socket that enables remote control, measurement and data transmission.
## Getting Started 
In this section you will find required hardware components and necessary software to start Smart Socket project.
#### Hardware Requirements
| Component                       | Description                      |
|--------------------------------|----------------------------------|
| Hi-Link HLK-PM01      | 5VDC Power supply unit                |
| ESP32          | Low-power system on a chip with integrated Wi-Fi                 |
| SRD-05VDC                | Relay Module |
| ACS712-20A       | Current Sensor          |
| ZMPT101B               | Voltage Sensor                   |
| DHT11  | Temperature and Humidity Sensor |
| HC-SR501     | PIR Motion Sensor                   |
| DS1307        | Real-Time Clock     |
| 4.7kΩ Resistor        | DHT11 pull-up resistor   |
| 2x100kΩ Resistor        | Divide voltage for measurement  | 
| 2x200kΩ Resistor        | Divide voltage for measurement   |

#### Circuit Diagram

<p align="center" width="100%">
    <img src="images/circuits_diagram.png"> 
</p>

#### Software Requirements
- [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/) or any IDE that can be used to program ESP32.
- [CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)  driver for Computer (This is a driver that allows you to communicate with ESP32 via USB).

#### Setup
After connecting the socket to power, an access point (AP) named ***Smart Socket*** with the password ***automation*** is automatically created, requiring users to log in for further configuration.

<p align="center" width="100%">
    <img src="images/ap.png"> 
</p>

After successfully logging into the access point, users need to fill in the data. Based on the entered SSID and Wi-Fi network password, the ESP32 in station mode connects to the access point integrated with the home router.

<p align="center" width="100%">
    <img src="images/network_config.png"> 
</p>

After correctly entering the Wi-Fi network data, the following message will appear on the webpage:
`ESP will restart, then connect to the router and go to IP address: XXX.XXX.XXX.XXX
If there is no connection, please fill out the form again under IP address: XXX.XXX.XXX.XXX`

At the end, users need to log in to the website using the following credentials:
- Username: ***admin***
- Password: ***admin***
## Features
#### Remote Control via Wi-Fi
Users can control the socket remotely through a web interface.

<p align="center" width="100%">
    <img src="images/main_page.png"> 
</p>

#### Energy Monitoring

The smart socket provides real-time insights into energy consumption, including current, voltage, active power consumption, and total energy usage. This data enables users to analyze their electricity usage patterns, optimize consumption, and ultimately reduce costs.

<p align="center" width="100%">
    <img src="images/electrical_parameters.png"> 
</p>

#### Schedule Configuration
The "Schedule" tab allows users to set up on/off schedules for the smart socket based on specific hours and days of the week, utilizing the precision of the RTC DS1307 clock to maintain accurate timekeeping even in the absence of power supply.

<p align="center" width="100%">
    <img src="images/schedule.png"> 
</p>

#### Night Light Mode
The "Night Light Mode" enables users to set up the socket to activate upon motion detection by the PIR HC-SR501 sensor. This feature, named for its primary function of providing nighttime illumination, allows users to customize the duration for which the smart socket remains activated after motion is detected, thus creating an efficient night light tailored to individual preferences.

<p align="center" width="100%">
    <img src="images/night_light_mode.png"> 
</p>

#### Temperature Mode
The "Temperature Mode" allows users to define the logic for turning the smart socket on and off based on ambient temperature values. This feature serves as a versatile tool suitable for both heating and cooling purposes within a room, utilizing connected devices to regulate temperature efficiently.

<p align="center" width="100%">
    <img src="images/temperature_mode.png"> 
</p>

#### Safety Configuration
The "Safety" page enables users to configure overload and thermal protections. Implementing these safety measures in the smart socket prevents overloads and excessive heating, minimizing the risk of electrical damage and fires.

<p align="center" width="100%">
    <img src="images/safety.png"> 
</p>

The screenshots below depict alarm messages indicating the activation of protections, corresponding to overload and thermal protections. These messages include the date and time of the protection activation, as well as the last recorded measurement data before the protection triggered. This allows users to promptly understand the cause of the alarm and take appropriate steps to resolve the issue. The occurrence of any alarm message results in the immediate shutdown of the smart socket and deactivation of all activated modes.

<p align="center" width="100%">
    <img src="images/overload.png"> 
</p>

<p align="center" width="100%">
    <img src="images/over-temperature.png"> 
</p>

#### User Profile
The "User" tab allows users to modify the login credentials required to access the "Smart Socket" webpage. 

<p align="center" width="100%">
    <img src="images/user.png"> 
</p>

## License
Distributed under the MIT License. See [`LICENSE`](/LICENSE) for more information.