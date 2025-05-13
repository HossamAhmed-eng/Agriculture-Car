🚜 Smart Agriculture Car with Pesticide Sprayer
This project is a Wi-Fi-controlled agriculture car designed for remote pesticide spraying in fields. It uses two ESP32 modules:

ESP32-CAM: Captures real-time video and sends it to the main controller.

ESP32-WROOM: Hosts a web-based control interface, receives the video stream, and controls the motors and pesticide sprayer based on user input.

🔧 Features
Live video streaming for remote navigation

Manual control via web interface (no need for external apps)

Pesticide spraying mechanism controlled remotely

Designed for small to medium-sized farming applications

🧠 Technologies
ESP32-CAM (for video capture)

ESP32-WROOM (for server, motor, and sprayer control)

HTML + JavaScript interface hosted on ESP32

TCP communication between the two ESPs
