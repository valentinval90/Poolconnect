<img width="1235" height="448" alt="Poolconnect logo" src="https://github.com/user-attachments/assets/75dec9b7-f1db-4642-aca4-e8e317624f90" />

PoolConnect is a system that makes your swimming pool connected, smart, and fully open source

---

## Overview

PoolConnect is an open-source hardware and software project designed to simplify pool management by making it smarter, safer, and more energy-efficient.
Instead of relying on traditional mechanical timers, PoolConnect dynamically adjusts filtration time based on water temperature and weather forecasts for the day and more.

It also allows control of multiple pool devices: 
- Heat pump (dry contact)
- Electrolyzer (dry contact) 
- Pool lighting (dry contact)
- 12 V solenoid valve for automatic pool water refill.
- Monitors the filter pressure using a 12V 4–20 mA pressure sensor.
- Continuously reads the water temperature.

The hardware schematic and the first PCB prototype are already designed around an **ESP32-S3-WROOM-1 (N16R8)**.

---

## 🎯 What problem does it solve ?

Traditional pool installations often rely on:  
⛔ Filtration timers that ignore water temperature  
⛔ No alert system for leaks in the technical room  
⛔ No automatic monitoring of filter pressure or clogging  
⛔ No weather-based automation  
⛔ No customizable  
⛔ No MQTT compatible  

**PoolConnect** combines intelligence, connectivity, and safety in a fully open-source solution.

---

## ✨ Key Features (hardware + software)

### ✔ Filtration & Water Quality
- Filtration duration configurable as you wish based on water temperature, weather forecasts, sunlight exposure, and other parameters.
- Water temperature measured using a DS18B20 digital sensor
- Preconfigured scenarios available

### ✔ Safety & Monitoring
- 4–20 mA pressure sensor input for real-time filter clogging detection → configurable alert thresholds
- Leak detector in the technical room → instant flood alert  
- Integrated buzzer for alarms (leak, filter clogging, fault)

### ✔ Device Control
- Filtration pump
- Electrolyzer  
- Pool lighting
- Heat pump (on/off control)
- 12 V solenoid valve for automatic water filling

### ✔ Connectivity & Integration
- Embedded web interface 
- Home Assistant compatibility (MQTT / native integration planned)  
- Local-first system: no cloud required
- Weather API integration

---

## 🔧 Hardware Overview

**Main microcontroller :**  
- ESP32-S3-WROOM-1 (N16R8) → 16 Mo Flash, 8 Mo PSRAM  

**Sensors & Inputs :**  
- DS18B20 digital temperature sensor 
- 4–20 mA pressure sensor (filtration)  
- Leak detector

**Outputs & Relays :**  
- Filtration pump relay 
- Heat pump relay
- Lighting relay
- Electrolyzer relay
- 12 V solenoid valve control 

**Alerts & Indicators :**  
- Integrated buzzer 
- Input protection and filtering
- External PILOT / OFF / MANUAL mode selector for each relay (rotary switch)
- External status LEDs for each relay (5 V) 

---
<img width="1263" height="802" alt="image" src="https://github.com/user-attachments/assets/73d831a6-8316-4375-83a4-593155cd2701" />


## 🧪 Software Status

The firmware is currently in prototype/testing phase: 
- Temperature monitoring ✔  
- First version of filtration logic ✔  
- Web interface ✔  
- Hardware layer ✔  
- MQTT + Home Assistant integration ✔  
- Pressure sensor, leak detection, and alarms ✔  
- OTA firmware updates ✔
- More features coming soon... 

---
<img width="446" height="538" alt="image" src="https://github.com/user-attachments/assets/f8c1eac2-8a08-45c3-abbe-f5ee3647511e" />
<img width="1892" height="902" alt="image" src="https://github.com/user-attachments/assets/56ebca87-8b25-4ff3-93ae-a713702825d7" />
- For more visuals, see the img folder in the GitHub repository.
  
## 🚀 Roadmap

### 🟢 Hardware
- Schematic V1  
- PCB prototype 
- PCB V2  
- First prototype batch from [JLCPCB](https://cart.jlcpcb.com/fr/quote?from=valentin.)  
- Integration into a pool electrical enclosure ([exemple Amazon](https://amzn.eu/d/afS3UlJ))  

### 🟠 Software
- Complete pressure monitoring logic and customizable 
- Leak detection alarms  
- Dynamic web dashboard 
- Mobile-friendly adaptive UI
- Automatic discovery in Home Assistant 
- OTA firmware updates 

---

## 🤝 Contributing

PoolConnect is a community-driven project. You can contribute by sharing feedback, testing prototypes, or helping with software and hardware development.
All skills are welcome: software development, electronics, 3D printing, UI/UX, documentation, and more.

I chose JLCPCB for their expertise and support throughout the prototype production process.
They provide excellent manufacturing quality, fast turnaround, and very competitive pricing.
They also offer many advanced PCB inspection and measurement options — a real advantage!

---

## ❤️ Support the Project
 
If PoolConnect inspires you or helps make pool automation more accessible, you can support its development: 

[💸 Support me on PayPal](https://www.paypal.com/donate/?hosted_button_id=K8RZ7FPXE39XA)  
[☕ Support me on Buy Me a Coffee](https://buymeacoffee.com/poolconnect)  

Your contribution directly funds hardware production and software development.

---

## 📄 Licence

MIT License — free to use, modify, and redistribute.

---

## 📬 Contact

Feel free to reach out or open an issue to follow the project or contribute.
Your support helps accelerate hardware production and keeps the project fully open source.
