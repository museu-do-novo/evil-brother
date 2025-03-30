setup:
  install arduino-cli:
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh 

  add board url:
    arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json

  install cores:
    arduino-cli core install deauther:esp8266:generic

  compile and upload:
    arduino-cli compile -e -b deauther:esp8266:generic -p /dev/ttyUSB0 -u

    
Description:

Tool developed for academic purposes as part of a Final Course Work (TCC) that demonstrates the operation of an EvilTwin attack, allowing:

Detection of nearby WiFi networks

Creation of a fake access point (EvilTwin)

Credential capture (WiFi or router mode)

Deauthentication attack (Deauth)

Main Features

WiFi Scanner: Detects nearby networks with SSID, BSSID and channel information

Dual Mode:

WiFi Mode: Captures only the WiFi network password

Router Mode: Simulates router administrator login

Dual Interface:

Web interface (192.168.1.1)

Control via serial terminal

Deauth attack: Allows deauthenticating clients on the target network

Initial Configuration
Required Hardware

NodeMCU ESP8266

USB power supply

Computer for programming and monitoring
Usage Basics
Via Web Interface

Connect to AP "WiFi_Setup" (password: setup1234)

Go to http://192.168.1.1

Select a target network

Choose the attack mode (WiFi or Router)

Start EvilTwin or Deauth attack

Via Serial Terminal (115200 baud)

Available commands:
Copy

scan - Scan WiFi networks
list - List available networks
select [n] - Select network by index
deauth [on|off] - Turn Deauth attack on/off
hotspot [on|off] - Turn EvilTwin on/off
mode [wifi|router] - Toggle between modes
status - Show current status
help - Show help

Customizable Configuration

Edit at the beginning of the code:
cpp
Copy

// ============= CONFIGURABLE =============
const IPAddress AP_IP(192, 168, 1, 1); // AP IP
const char* AP_SSID = "WiFi_Setup"; // AP SSID
const char* AP_PASSWORD = "setup1234"; // AP password
const unsigned long SCAN_INTERVAL = 30000; // Scan interval (ms)
const unsigned long DEAUTH_INTERVAL = 1000; // Deauth Packet Range
// =======================================

Code Structure

setup(): Initializes WiFi, Web Server and DNS interfaces

loop(): Manages requests and attacks

Web interface functions (handleRoot(), handleSelect(), etc.)

Serial terminal functions (handleSerialCommand())

Network functions (scanNetworks(), sendDeauthPacket())

Ethical Considerations

This project was developed exclusively for academic and security research purposes. Using this tool to attack networks without prior authorization is illegal and unethical.
    
