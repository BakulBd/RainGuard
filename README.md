# ESP32 Rain Sensor System

This project utilizes an ESP32 microcontroller along with a rain sensor and DHT11 sensor to create a Telegram bot that notifies users when it starts raining and provides temperature and humidity updates.

## Features

- **Rain Detection:** The ESP32 continuously monitors the state of the rain sensor. When rain is detected, it sends a notification to the specified Telegram group chat.
  
- **Telegram Bot:** Users can interact with the bot by sending commands such as `/status` to check the current status, `/restart` to restart the system, `/setwifi` to set WiFi credentials, `/authorize` to authorize new users, `/rainhistory` to get the rain detection history, and `/temp_humid` to get temperature and humidity updates.

## Hardware Requirements

- ESP32 microcontroller
- Rain sensor module
- DHT11 sensor
- Jumper wires

## Software Dependencies

- [Arduino IDE](https://www.arduino.cc/en/software)
- Libraries:
  - WiFi.h
  - WiFiClientSecure.h
  - UniversalTelegramBot.h
  - ArduinoJson.h
  - DHT.h
  - FastBot.h

## Installation and Setup

1. **Clone or download the repository** to your local machine.
2. **Open the project in Arduino IDE.**
3. **Set up your WiFi credentials** in the `ssid` and `password` variables.
4. **Configure the Telegram bot settings** by replacing the placeholders with your bot token and chat IDs.
5. **Connect the rain sensor and DHT11 sensor to the ESP32** as per the pin configurations defined in the code.
6. **Upload the code to your ESP32 board.**
7. **Power on the ESP32 and monitor the Serial Monitor** for status messages.

## Usage

1. **Power on the device and connect to WiFi.** It will automatically start monitoring the rain sensor and the DHT11 sensor.
2. **Interact with the bot** by sending commands in the specified Telegram group chat.
3. **Receive notifications** when rain is detected.
4. **Get temperature and humidity updates, check sensor status, restart the system, set WiFi credentials, authorize new users, and view rain detection history** via Telegram commands.

## Commands

- `/status` - Check sensor status
- `/restart` - Restart the system
- `/setwifi <SSID> <PASSWORD>` - Set Wi-Fi credentials
- `/authorize <CHAT_ID>` - Authorize a new user
- `/rainhistory` - Get the rain detection history
- `/temp_humid` - Get temperature and humidity status

## Contributing

Contributions are welcome! If you have ideas for improvements, new features, or bug fixes, please feel free to open an issue or submit a pull request.

To contribute to this project, follow these steps:

1. **Fork the repository.**
2. **Create a new branch** for your feature or bug fix.
3. **Make your changes.**
4. **Test your changes thoroughly.**
5. **Submit a pull request,** explaining the purpose of your changes and any considerations for reviewers.

## Troubleshooting

- **WiFi Connection Issues:** Double-check the SSID and password in the code.
- **Sensor Connection Issues:** Ensure that the rain sensor and DHT11 sensor are properly connected to the ESP32 and functioning correctly.
- **Telegram Bot Issues:** Verify that the Telegram bot token and chat IDs are correctly configured in the code.

## License

This project is licensed under the [MIT License](LICENSE).