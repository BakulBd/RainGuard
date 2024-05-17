# ESP32 Rain Sensor System

This project utilizes an ESP32 microcontroller along with a rain sensor to create a Telegram bot that notifies users when it starts raining. It also integrates with IFTTT to trigger additional actions when rain is detected.

## Features

- **Rain Detection:** The ESP32 continuously monitors the state of the rain sensor. When rain is detected, it sends a notification to the specified Telegram group chat.
  
- **Telegram Bot:** Users can interact with the bot by sending commands such as `/on` to enable the rain sensor, `/off` to disable it, `/ifttt` to trigger IFTTT applets, and `/setwifi` to set WiFi credentials.

- **IFTTT Integration:** The bot can trigger IFTTT applets when rain is detected. This allows for various automation tasks such as turning on smart lights or sending notifications to other devices.

## Hardware Requirements

- ESP32 microcontroller
- Rain sensor module
- LED (optional, for visual indication)
- Jumper wires

## Software Dependencies

- [Arduino IDE](https://www.arduino.cc/en/software)
- Libraries:
  - WiFi.h
  - WiFiClientSecure.h
  - UniversalTelegramBot.h
  - ArduinoJson.h

## Installation and Setup

1. Clone or download the repository to your local machine.
2. Open the project in Arduino IDE.
3. Set up your WiFi credentials in the `ssid` and `password` variables.
4. Configure the Telegram bot settings by replacing the placeholders with your bot token, chat IDs, and root CA certificate for Telegram API.
5. Connect the rain sensor to the ESP32 as per the pin configurations defined in the code.
6. Upload the code to your ESP32 board.
7. Power on the ESP32 and monitor the Serial Monitor for status messages.

## Usage

1. Once the device is powered on and connected to WiFi, it will automatically start monitoring the rain sensor.
2. Users can interact with the bot by sending commands in the specified Telegram group chat.
3. When rain is detected, the bot will send a notification to the group chat.
4. Optionally, users can enable IFTTT integration to trigger additional actions based on rain detection.

## Contributing

Contributions are welcome! If you have ideas for improvements, new features, or bug fixes, please feel free to open an issue or submit a pull request.

To contribute to this project, follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes.
4. Test your changes thoroughly.
5. Submit a pull request, explaining the purpose of your changes and any considerations for reviewers.

## Troubleshooting

- If the device fails to connect to WiFi, double-check the SSID and password in the code.
- Ensure that the rain sensor is properly connected to the ESP32 and functioning correctly.
- Verify that the Telegram bot token, chat IDs, and root CA certificate are correctly configured in the code.

## License

This project is licensed under the [MIT License](LICENSE).
