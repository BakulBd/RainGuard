#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Rain sensor and LED pins
const int rainSensorPin = 12;
const int ledPin = 13;

// Rain sensor state variables
bool isRaining = false;
unsigned long rainStartTime;
unsigned long rainDuration;

// Bot and WiFi settings
bool botEnabled = true; 
bool settingSSID = false;
bool settingPassword = false;
char ssid[32] = "GUB-232";
char password[64] = "momin123";

// Telegram Bot settings
const char* telegramBotToken = "7158360864:AAEZhKGgtiH8Al3oyBU5_HSHZS24hz74gIs";
const char* chatId = "656098264";
const char* telegramGroupChatId = "4111837178";

// Root CA for api.telegram.org
const char* telegramCert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDxTCCAq2gAwIBAgIUEdU8pFOMu5U7wE0oTFjeGDX0U9MwDQYJKoZIhvcNAQEL\n" \
  "BQAwVTELMAkGA1UEBhMCVVMxEzARBgNVBAgMCldhc2hpbmd0b24xEzARBgNVBAcM\n" \
  "Ck5vcnRoIFNjb3R0MRAwDgYDVQQKDAdHZW9UcnVzdDEhMB8GA1UEAwwYQ2xvdWRG\n" \
  "bGFyZSBJbmMgUlNBIENBMB4XDTIxMDQxNDAwMDAwMFoXDTMxMDQxMzIzNTk1OVow\n" \
  "gY0xCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJDQTESMBAGA1UEBwwJUGFsbyBBbHRv\n" \
  "MQ4wDAYDVQQKDAVUZWxlczEMMAoGA1UECwwDSVRIMRowGAYDVQQDDBFhcGkudGVs\n" \
  "ZWdyYW0ub3JnMSYwJAYJKoZIhvcNAQkBFhdzdXBwb3J0QHRlbGVncmFtLm9yZzCC\n" \
  "ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANPsbxA3pPj8uHRmh5SdhCsm\n" \
  "Yxegw2wrI8uF0TCZozk/NTnR7IgxV2WzG1tb6XfQHTPQB6B5QOrjQpL0q2ZCmNcC\n" \
  "uFtbshX6mPlg+FEaHv4NjYSwG4K4i1HvmEVGEEXtUfp+zCHcy6P4ApCrXdpGnSST\n" \
  "IDH0J8cIj9kkU1/lo+77Lud8Zfy5v7Eb0DW9b5hd8hSckTjq/lPBHFbf7ql2we73\n" \
  "KO5BPQwLMSL7nAg0mXoaX6LC5N6LPB0tBa4TVOtME4Yag6Hb+jg9dDf8lGrbsUuP\n" \
  "uAvqMnCCTzztUbW3UyMzPcX/MJdJw1/H8CfskrgK38A/qVZX4HwMtTt2Eb84u68C\n" \
  "AwEAAaOB3zCB3DAdBgNVHQ4EFgQUdKnkRZ9mHUp8AlcAF2Lf/WGdazowHwYDVR0j\n" \
  "BBgwFoAU6kEhsIpy7MEaENb2FFd2YEyOVZ0wDAYDVR0TAQH/BAIwADAdBgNVHSUE\n" \
  "FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwHQYDVR0OBBYEFL/MtBhRHlHZRh4lO3mj\n" \
  "X3J3/xTcMB4GA1UdIAEB/wQbMDkwNwYJKwYBBAGCNyoBMCowKAYIKwYBBQUHAgEW\n" \
  "HWh0dHA6Ly9wa2kuZ2VvdHJ1c3QuY29tL0NQUzA3BgNVHR8EMDAuMCygKqAohiZl\n" \
  "d3d3LmNsb3VkZmxhcmUuY29tL2NybC9nZnJzYWNhMS5jcmwwDQYJKoZIhvcNAQEL\n" \
  "BQADggEBAIDkVjE27nPxfLzD/ebvsHMPcRp6vP5CUNjmw1TiAmJeBLmbGtpFJMz9\n" \
  "rAs5I10xxSmHFg76yTDbkYqV82xPTeNtrGlY8z7PtjsOiO2XxlIVw58O61S1oAlm\n" \
  "6aJntWBZB2en0Knxd6eyfnnpR8XdyZSyKoOFg3unOMHssP5bsBdXjGcL0idPXzZx\n" \
  "SzC84bm8aEJsTVd/JIgRF0zDlB2xe0zBJc6HpHsOBPbqY/N65uK8xseCzEdCWl+C\n" \
  "DH9W1oP8HTDCtw5hU5ZjCE9hc12DqoaGnka2FR9m7Z9rHy/x44C9F0Ma9KMWUwlD\n" \
  "R9duT7yQAIHbKFeIEIRUQMQDpDkEkt4=\n" \
  "-----END CERTIFICATE-----\n";

WiFiClientSecure client;
UniversalTelegramBot bot(telegramBotToken, client);

void handleNewMessages(int numNewMessages);
// Other code omitted for brevity

void connectToWiFi();
void sendTelegramMessage(String chatId, String message);
void sendIFTTTCommand();
String formatDuration(unsigned long duration);
String getTime();
void telegramBotCommandHandler(String command);



void setup() {
  pinMode(rainSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  // Initialize WiFi
  connectToWiFi();

  // Initialize Telegram bot after WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    sendTelegramMessage(chatId, "ESP32 Rain Sensor Bot started. Type /on to enable rain sensor or /off to disable.");
    sendTelegramMessage(telegramGroupChatId, "Welcome! I'm the ESP32 Rain Sensor Bot. Use /ifttt command to trigger another bot when rain starts. Use /setwifi to set WiFi credentials. Use /restart to restart the device.");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  bot.longPoll = 60; // Set long polling interval to 60 seconds
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  if (numNewMessages > 0) {
    handleNewMessages(numNewMessages);
  }

  if (botEnabled) {
    if (digitalRead(rainSensorPin) == HIGH) {
      // Rain started
      if (!isRaining) {
        isRaining = true;
        rainStartTime = millis();
        sendTelegramMessage(telegramGroupChatId, "It's raining now. Take cover!");
        digitalWrite(ledPin, HIGH);
        sendIFTTTCommand();
      }
    } else {
      // Rain stopped
      if (isRaining) {
        isRaining = false;
        rainDuration = millis() - rainStartTime;
        sendTelegramMessage(telegramGroupChatId, "Rain stopped after " + formatDuration(rainDuration) + ". Time: " + getTime());
        digitalWrite(ledPin, LOW);
      }
    }
  }

  delay(1000); // Check rain sensor every second
}

void connectToWiFi() {
  if (strlen(ssid) == 0 || strlen(password) == 0) {
    Serial.println("WiFi credentials not set.");
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (millis() - startTime > 10000) {
      Serial.println("\nWiFi connection failed. Invalid credentials or timeout.");
      return;
    }
  }

  Serial.println("\nWiFi connected.");
  client.setCACert(telegramCert); // Set the root certificate for Telegram API
}

void sendTelegramMessage(String chatId, String message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send Telegram message. WiFi not connected.");
    return;
  }

  WiFiClientSecure client;
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Failed to connect to Telegram server.");
    return;
  }

  String url = "/bot" + String(telegramBotToken) + "/sendMessage";
  String postRequest = "POST " + url + " HTTP/1.1\r\n" +
                       "Host: api.telegram.org\r\n" +
                       "User-Agent: ESP32\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + String(message.length()) + "\r\n" +
                       "Connection: close\r\n\r\n" +
                       message;

  client.print(postRequest);
  delay(10);
  client.stop();

  Serial.println("Telegram message sent successfully.");
}

void sendIFTTTCommand() {
  sendTelegramMessage(telegramGroupChatId, "/ifttt");
}

String formatDuration(unsigned long duration) {
  int hours = duration / 3600000;
  int minutes = (duration % 3600000) / 60000;
  int seconds = (duration % 60000) / 1000;
  return String(hours) + " hours, " + String(minutes) + " minutes, " + String(seconds) + " seconds";
}

String getTime() {
  unsigned long currentMillis = millis();
  unsigned long currentSeconds = currentMillis / 1000;
  unsigned long currentMinutes = currentSeconds / 60;
  unsigned long currentHours = currentMinutes / 60;
  return String(currentHours % 24) + ":" + String(currentMinutes % 60) + ":" + String(currentSeconds % 60);
}

void handleNewMessages(int numNewMessages) {
  // Handle new messages from Telegram
}

void telegramBotCommandHandler(String command) {
  // Handle bot commands received from Telegram
}
