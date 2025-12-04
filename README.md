🌊 PoolConnect
Smart, adaptive and open-source pool automation controller
🔥 Overview

PoolConnect is an open-source hardware and software project designed to make pool management smarter, safer, and more energy-efficient.
Instead of relying on outdated mechanical timers, PoolConnect dynamically adjusts filtration and controls multiple pool devices based on real conditions — not guesswork.

The hardware schematic and PCB prototype are already designed using an ESP32-S3-WROOM-1 (N16R8), and a first production run in China is planned. Community support helps accelerate this step.

🎯 What Problem Does It Solve?

Traditional pool installations often rely on:

⛔ Fixed timers that ignore water temperature

⛔ Manual monitoring of water level

⛔ No alert system for leaks in the pool house

⛔ No monitoring of filter pressure or clogging

⛔ No automation based on weather

PoolConnect brings intelligence, connectivity, and safety — all in a fully open-source solution.

✨ Key Features (hardware + software)
✔ Filtration & Water Quality

Automatic filtration time based on water temperature

Temperature measured via DS18B20 digital probe

Weather-aware filtration adjustments

Custom timer mode (behavior similar to a programmable clock)

✔ Safety & Monitoring

4–20 mA pressure sensor input for real-time filter clogging detection
→ Configurable alert thresholds
→ Predictive maintenance (“time to clean the filter”)

Water leak sensor in the pool house
→ Instant alert if the technical room floods

On-board buzzer for audible alarms (leak, high pressure, fault)

✔ Device Control

PoolConnect controls multiple devices through relay outputs:

Pump (filtration)

Electrolyser

Pool Lighting

Heat Pump (ON/OFF control)

12 V solenoid valve for automatic pool refilling

✔ Connectivity & Integration

Embedded web interface (ESP32-S3)

Home Assistant compatibility (MQTT / native integrations planned)

Local-first system: no cloud required

Optional external API for remote access

🔧 Hardware Overview
Main MCU

ESP32-S3-WROOM-1 (N16R8)
→ 16 MB Flash, 8 MB PSRAM
→ Wi-Fi + USB native support

Sensors & Inputs

DS18B20 digital temperature probe

4–20 mA pressure sensor input (filter monitoring)

Water leak detector (simple dry-contact)

Outputs & Relays

Relay for filtration pump

Relay for heat pump

Relay for lighting

Relay for electrolyser

12 V solenoid valve driver (automatic filling)

Alerts & Indicators

On-board buzzer

Status LED indicators (optional)

Power + Protections

Isolated low-voltage domain

12 V driver stage isolation

Input protection and filtering

📸 Insert your PCB + schematic screenshots here when ready.

📡 System Architecture
flowchart TD
    TEMP[DS18B20 Temp Sensor] --> LOGIC[Filtration Algorithm]
    PRESS[Pressure Sensor 4-20mA] --> LOGIC
    LEAK[Leak Sensor] --> ALERT
    WEATHER[Weather Forecast] --> LOGIC
    LOGIC --> RELAY1[Pump Control]
    LOGIC --> RELAY2[Electrolyser]
    LOGIC --> RELAY3[Lighting]
    LOGIC --> RELAY4[Heat Pump ON/OFF]
    LOGIC --> SOLV[12V Solenoid Valve]
    ALERT --> BUZZER[Buzzer Alarm]
    ESP[ESP32-S3 Web UI / MQTT] --> LOGIC

🧪 Software Status

The firmware is currently in a prototype/testing stage:

Temperature monitoring ✔

First version of filtration logic ✔

Basic web interface ✔

Hardware abstraction layer WIP

MQTT + Home Assistant integration WIP

Pressure sensor, leak detection & alarms planned next

➡ When you send me your current prototype code, je peux :

Décrire précisément les modules

Améliorer la section architecture

Proposer une structure firmware propre et scalable

Donner un aperçu clair du code pour les futurs contributeurs

🚀 Roadmap
🟢 Hardware

 Schematic V1

 PCB prototype

 PCB V2 (production-ready)

 First manufacturing batch (China)

 Enclosure design

🟠 Software

 Complete firmware rewrite

 Full pressure monitoring logic

 Leak detection alarms

 Dynamic dashboard (web)

 Mobile responsive UI

 Home Assistant auto-discovery

 OTA updates

 Calibration wizard

🤝 Contributing

PoolConnect is a community-driven project:
You can help by providing feedback, ideas, testing prototypes, or contributing to code and electronics.
Every skill matters: software, electronics, 3D printing, UI, documentation…

❤️ Support the Project

The next milestone is manufacturing the PCB in China.
If PoolConnect inspires you or helps make pool automation accessible, you can support development:

👉 Add here your PayPal / Ko-Fi / BuyMeACoffee / GitHub Sponsors links

Your contribution directly funds hardware production.

📄 License

MIT License — free to use, modify and redistribute.

📬 Contact

Feel free to reach out or open an issue to follow development or contribute.

Your support accelerates hardware production and helps keep the project fully open-source.

📄 License

MIT License — free to use, modify, and improve.

📬 Contact

Feel free to reach out if you want to follow development or contribute.
