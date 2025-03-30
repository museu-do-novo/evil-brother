setup:
  install arduino-cli:
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh 

  add board url:
    arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json

  install cores:
    arduino-cli core install deauther:esp8266:generic

  compile and upload:
    arduino-cli compile -e -b deauther:esp8266:generic -p /dev/ttyUSB0 -u

    
