🌊 PoolConnect
Smart, adaptive and open-source pool filtration controller
🔥 Overview

PoolConnect is an open-source project designed to modernize pool filtration.
Instead of relying on a classic timer that ignores water temperature and real usage conditions, PoolConnect adjusts filtration intelligently — reducing energy costs and improving water quality.

This project is currently in early prototype stage, with the electronic schematic and PCB already designed. A first hardware batch is planned for production in China, and community support will help accelerate development.

🎯 Why PoolConnect?

Traditional pool timers run on fixed schedules:
⛔ They don’t consider water temperature
⛔ They waste energy when filtration runs 7/7
⛔ They require manual adjustments

PoolConnect solves this by turning any pool into a smart, connected system.

✨ Key Features (current & planned)
✔ Already working in prototype code

Automatic filtration time based on water temperature

Weather-aware filtration (planned conditions adjust runtime)

Custom timer feature (classic timer replacement)

Pool cover detection & integration

Web interface prototype

Early Home Assistant integration (WIP)

🚧 Under active development

Complete firmware rewrite (clean architecture)

Secure API and web dashboard

OTA firmware updates

Energy-optimized algorithms

Local & offline mode

Plug-and-play installation guide

📡 Hardware Status

The following elements are already completed:

✔ Electronic schematic

✔ PCB V1 (prototype)

✔ Complete design for manufacturing

✔ ESP32 architecture

✔ Relay/driver system for pump + electrolyser

✔ Temperature sensor interface

📸 You can include your screenshots of PCB + schematic here.

Next step: funding the first 20–50 units for production in China.

🧠 How It Works (simplified)

PoolConnect measures water temperature, checks weather forecasts, and applies a filtration algorithm to decide how long and when the pump should run.

flowchart LR
    TEMP[Water Temperature Sensor] --> LOGIC[Filtration Algorithm]
    WEATHER[Weather Forecast] --> LOGIC
    COVER[Pool Cover Status] --> LOGIC
    LOGIC --> RELAY[Pump & Electrolyser Control]
    ESP[ESP32 Web/API] --> LOGIC

🛠️ Current Prototype Code

The firmware is currently in a proof-of-concept phase.
Nothing is final, and the code structure will evolve heavily.

➡ If you want, you can send me your code and I’ll rewrite this section to match your current logic, files, and modules.

🚀 Roadmap

 Clean, modular firmware architecture

 Fully functional web dashboard

 Home Assistant auto-discovery

 Mobile-friendly UI

 Weather API integration

 Asynchronous task engine

 PCB V2 (production-ready)

 3D-printed enclosure

 Multi-language support (FR/EN)

 Cloud-free option (local only)

🤝 Contributing

PoolConnect is designed as a community-driven project.
Ideas, feedback, beta testers, and contributors are all welcome.

You can help by:

Opening issues

Suggesting new features

Testing early firmware

Sharing your pool configuration

Helping with documentation

❤️ Support the Project

If PoolConnect helps you save time and energy — or if you simply want to support open hardware — you can help fund the PCB production:

👉 Add here your future links (PayPal, Ko-Fi, BuyMeACoffee, GitHub Sponsors)

Your support accelerates hardware production and helps keep the project fully open-source.

📄 License

MIT License — free to use, modify, and improve.

📬 Contact

Feel free to reach out if you want to follow development or contribute.
