<img width="1235" height="448" alt="Poolconnect logo" src="https://github.com/user-attachments/assets/75dec9b7-f1db-4642-aca4-e8e317624f90" />

PoolConnect is an open-source system that makes your pool smart and compatible with Home Assistant.

---

## Overview

**PoolConnect** is an innovative open-source solution designed to make your pool management **smarter, safer, and more energy-efficient**.  
Forget traditional mechanical timers: PoolConnect offers you **complete and customizable control** of your filtration cycle.  

You can create your own filtration equation, from the simplest (e.g., water temperature ÷ 2) to advanced formulas integrating sunlight, daily weather forecasts, or pool cover status.

### Intelligent Equipment Control

The system manages multiple pool devices **independently and securely**.  
For example, the electrolyzer cannot activate if the pump is not running:

- Heat pump (dry contact)  
- Electrolyzer (dry contact)  
- Pool lighting (dry contact)  
- 12V solenoid valve for water refill  
- Filtration pump  

### Monitoring and Alerts

PoolConnect also provides:

- Filter pressure measurement and alerts for abnormal values  
- Water leak detection in the technical room  
- Pool cover status verification (open/closed)  

### Technology

The system's core is based on an **ESP32-S3-WROOM-1 (N16R8)**, ensuring power, flexibility, and connectivity for all your needs.

---

## Main Features (Hardware + Software)

### Filtration and Water Quality

- 6 pre-recorded profiles  
- Ability to create your own equation to determine filtration time  
  *(available variables: water temperature, current outdoor temperature, predicted MAX outdoor temperature, predicted MIN outdoor temperature, sunlight percentage)*  
- Complete filtration cycle history  
- Ability to create fully custom profiles from A to Z  

### Safety and Monitoring

- 4–20 mA pressure sensor input for real-time filter clogging detection, with configurable alert threshold  
  *(sensor calibration available)*  
- Leak detector in technical room → instant alert in case of flooding  
- Integrated audible alarm (buzzer) for alarms (leak, filter clogging, fault)  
- Each input can be used to trigger automation (pump shutdown, heat pump shutdown, etc.)  

### Device Control

- Filtration pump  
- Electrolyzer  
- Pool lighting  
- Heat pump (on/off control)  
- 12V solenoid valve for automatic refill  

### Connectivity and Integration

- Integrated web interface to configure your entire PoolConnect system  
- Home Assistant compatibility via MQTT integration  
- Fully local system: no cloud required  
- Integrated weather API  
- Data logging  
- Pressure and temperature sensor calibration  
- Manual control via web interface  
- Language selection (FR or EN)  
- User configuration  

---

## Hardware Overview

### Main Microcontroller

- ESP32-S3-WROOM-1 (N16R8) → 16 MB Flash, 8 MB PSRAM  

### Sensors and Inputs

- DS18B20 digital temperature probe  
- 4–20 mA pressure sensor (filtration)  
- Leak detector  
- Dry contact input for cover (open / closed)  

### Outputs and Relays

- Relay for filtration pump  
- Relay for heat pump  
- Relay for lighting  
- Relay for electrolyzer  
- 12V solenoid valve control  

### Alerts and Indicators

- Integrated buzzer  
- Input protection and filtering  
- External mode selector AUTO / OFF / MANUAL for each relay (rotary switch)  
- External status LEDs for each relay (5V)  

---

## Safety

⚠️ **WARNING**: This project controls electrical equipment (230V).  
Any installation must be performed by a qualified person.  
PoolConnect does not replace mandatory electrical protections (circuit breaker, GFCI, etc.).

---

## Build with Commercial Modules

It is possible to use independent modules to build this project, or to manufacture/purchase the official PCB (coming soon).

### Bill of Materials

- [ESP32-S3-WROOM-1 (N16R8)](https://fr.aliexpress.com/item/1005007319706057.html)  
- [DS18B20 Digital Temperature Probe](https://fr.aliexpress.com/item/1005008024174225.html)  
- [4–20 mA Pressure Sensor](https://fr.aliexpress.com/item/1005010536806042.html)  
- [Leak Sensor (use relay as digital output)](https://fr.aliexpress.com/item/1005001462767498.html)  
- [INA226 + 1 ohm resistor to install](https://fr.aliexpress.com/item/1005009774355109.html)  
- [5-relay board minimum](https://fr.aliexpress.com/item/1005008269018032.html)  
- [Buzzer](https://fr.aliexpress.com/item/1005007798521103.html)  

The items mentioned above can be replaced by equivalent equipment.

---

## Pin Configuration (PIN OUT)

| Pin 	 | Function						 | Type 			|
|--------|-------------------------------|------------------|
| IO4 	 | Filtration pump relay 		 | Digital output 	|
| IO5 	 | Electrolyzer relay 			 | Digital output	|
| IO6	 | Light relay 					 | Digital output	|
| IO7	 | Solenoid valve relay 		 | Digital output   |
| IO17	 | SCL INA226 (4–20 mA pressure) | I2C 				|
| IO18	 | SDA INA226 (4–20 mA pressure) | I2C 				|
| IO1 	 | Leak sensor 					 | Digital input 	|
| IO2	 | Cover open contact 			 | Digital input 	|
| IO38 	 | SK6812 addressable LED		 | Digital output 	|
| IO46	 | Heat pump relay 				 | Digital output 	|
| IO14	 | DS18B20 temperature probe 	 | OneWire			|
| IO21	 | Buzzer 						 | Digital output 	|

### Wiring Diagram

Follow this wiring:

<img width="1263" height="802" alt="PoolConnect wiring diagram" src="https://github.com/user-attachments/assets/73d831a6-8316-4375-83a4-593155cd4892" />

---

## Project PCB

A first version of the PCB is validated and functional. You will find the files to allow you to order the PCB online (especially from JLCPCB who supported me for this project).

A second version is in progress. This new version aims to make the system integrable on DIN rail of electrical cabinets. It will also integrate the addition of memory for history and some minor improvements.

### First Version

<img width="1263" height="802" alt="PoolConnect PCB V1" src="https://github.com/user-attachments/assets/73d831a6-8316-4375-83a4-593155cd2701" />

---

## Firmware Installation

To download the firmware, you can use Arduino IDE or the **"Poolconnect_installer"** tool.

### With Arduino IDE

1. Download the **"Poolconnect_installer"** tool
2. Launch the tool, click on **"Install All Libraries"** then on **"Install ESP32 Board"**  
   *(the tool will automatically install the necessary libraries for the Arduino project)*
3. You will also need the [LittleFS Upload Plugin](https://github.com/earlephilhower/arduino-littlefs-upload)

### With Poolconnect_installer Tool

1. Download the **"Poolconnect_installer"** tool
2. Launch the tool, click on the **"Flash ESP32"** tab
3. Select the 4 necessary binaries available in the **"build"** folder:
   - `Poolconnect.ino.bin`
   - `Poolconnect.ino.bootloader.bin`
   - `Poolconnect.ino.partitions.bin`
   - `littlefs_web_V1.0.2.bin`
4. Click on **"Flash ESP32"**

---

## 🤝 Contributing

PoolConnect is a community project: you can contribute by giving your feedback, testing, or participating in code and electronics development.  

All skills are welcome:
- Software development
- Electronics
- 3D printing
- User interface
- Documentation
- ...

### PCB Partner

I chose **JLCPCB** for their expertise and support throughout the prototype production process.

They also offer:
- Excellent manufacturing quality
- Remarkable responsiveness
- Very attractive pricing
- Many available options to verify or measure different aspects of PCBs

---

## ❤️ Support the Project

If PoolConnect inspires you or makes pool automation accessible, you can support its development:  

- [💸 Support me on PayPal](https://www.paypal.com/donate/?hosted_button_id=K8RZ7FPXE39XA)  
- [☕ Support me on Buy Me a Coffee](https://buymeacoffee.com/poolconnect)  

---

## 📄 License

**MIT License** – free use, modification, and redistribution.

See the [LICENSE](LICENSE) file for more details.

---

## 📬 Contact

Feel free to contact us or open an issue to follow the development or contribute to it.  

Your support accelerates hardware production and keeps the project entirely open source.

---

**⭐ If you like this project, feel free to give it a star on GitHub!**