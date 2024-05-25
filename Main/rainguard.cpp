#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <vector>
#include <algorithm>
#include <DHT.h>
#include <FastBot.h>
#include "time.h"

// Replace with your network credentials
const char* ssid = "GUB-232";
const char* password = "momin123";

FastBot fastBot;

// Initialize Telegram BOT
#define BOTtoken "BOT_token" // Replace with your bot token
#define AUTHORIZED_CHAT_ID "656098264"
#define GROUP_CHAT_ID "-1002020001615" // Replace with your group chat ID

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int WIFI_CONNECTION_TIMEOUT = 10000; // 10 seconds
#define RAIN_SENSOR_PIN 12 
#define DHT_PIN 13
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

bool rainDetected = false;
bool forceBoot = false;
unsigned long rainStartTime;
unsigned long lastTempHumidUpdateTime = 0;
const unsigned long TEMP_HUMID_UPDATE_INTERVAL = 600000; // 10 minutes in milliseconds

std::vector<String> authorizedUsers = {AUTHORIZED_CHAT_ID};
std::vector<String> rainHistory;

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 6 * 3600;  // GMT+6 for Bangladesh
const int daylightOffset_sec = 0;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Function to get the current timestamp in a readable format
String getCurrentTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time not available";
  }
  char timestamp[100];
  strftime(timestamp, sizeof(timestamp), "%A, %B %d %Y %H:%M:%S", &timeinfo); // More readable format
  return String(timestamp);
}

void sendWelcomeMessage(String chat_id) {
  Serial.println("Sending welcome message...");

  String welcome = "🌧️ Welcome to RainGuard! 🌧️\n\n";
  welcome += "RainGuard is your reliable friend for detecting and monitoring rain and environmental conditions. Here are the available commands:\n\n";
  welcome += "/status - Check sensor status\n";
  welcome += "/restart - Restart the system\n";
  welcome += "/setwifi <SSID> <PASSWORD> - Set Wi-Fi credentials\n";
  welcome += "/authorize <CHAT_ID> - Authorize a new user\n";
  welcome += "/rainhistory - Get the rain detection history\n";
  welcome += "/temp_humid - Get temperature and humidity status\n";

  bot.sendMessage(chat_id, welcome, "");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (chat_id != AUTHORIZED_CHAT_ID && std::find(authorizedUsers.begin(), authorizedUsers.end(), chat_id) == authorizedUsers.end()) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      bot.sendMessage(AUTHORIZED_CHAT_ID, "Unauthorized user tried to access: " + from_name + " (" + chat_id + ")", "");
      continue;
    }

    if (text == "/start" && chat_id != AUTHORIZED_CHAT_ID) {
      sendWelcomeMessage(chat_id);
    }

    if (text == "/status" || text == "/status@rainguard_bot") {
      String statusMessage = "🌧️ RainGuard Status 🌧️\n\n";
      statusMessage += "Currently, it is ";
      statusMessage += rainDetected ? "raining 🌧️\n" : "not raining ☀️\n\n";

      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

      statusMessage += "🌡️ Temperature: ";
      if (isnan(temperature)) {
        statusMessage += "Failed to read";
      } else {
        statusMessage += String(temperature) + " °C";
      }
      statusMessage += "\n";

      statusMessage += "💧 Humidity: ";
      if (isnan(humidity)) {
        statusMessage += "Failed to read";
      } else {
        statusMessage += String(humidity) + " %";
      }

      bot.sendMessage(chat_id, statusMessage, "");
    }

    if (text == "/restart") {
      bot.sendMessage(chat_id, "Restarting...");
      forceBoot = true;
    }

    if (text.startsWith("/setwifi")) {
      int firstSpace = text.indexOf(' ');
      int secondSpace = text.indexOf(' ', firstSpace + 1);
      if (firstSpace == -1 || secondSpace == -1) {
        bot.sendMessage(chat_id, "Invalid format. Use /setwifi <SSID> <PASSWORD>", "");
      } else {
        String newSSID = text.substring(firstSpace + 1, secondSpace);
        String newPassword = text.substring(secondSpace + 1);
        bot.sendMessage(chat_id, "Setting new Wi-Fi credentials...", "");
        WiFi.begin(newSSID.c_str(), newPassword.c_str());
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECTION_TIMEOUT) {
          delay(1000);
          Serial.println("Connecting to WiFi with new credentials...");
        }
        if (WiFi.status() == WL_CONNECTED) {
          bot.sendMessage(chat_id, "Connected to new Wi-Fi network", "");
        } else {
          bot.sendMessage(chat_id, "Failed to connect to new Wi-Fi network", "");
        }
      }
    }

    if (text.startsWith("/authorize")) {
      if (chat_id == AUTHORIZED_CHAT_ID) {
        int firstSpace = text.indexOf(' ');
        if (firstSpace == -1) {
          bot.sendMessage(chat_id, "Invalid format. Use /authorize <CHAT_ID>", "");
        } else {
          String newChatID = text.substring(firstSpace + 1);
          if (std::find(authorizedUsers.begin(), authorizedUsers.end(), newChatID) != authorizedUsers.end()) {
            bot.sendMessage(chat_id, "User " + newChatID + " is already authorized", "");
          } else {
            authorizedUsers.push_back(newChatID);
            bot.sendMessage(chat_id, "User " + newChatID + " has been authorized", "");
            sendWelcomeMessage(newChatID);
          }
        }
      } else {
        bot.sendMessage(chat_id, "You are not authorized to authorize new users", "");
      }
    }

    if (text == "/rainhistory" || text == "/rainhistory@rainguard_bot") {
      if (rainHistory.empty()) {
        bot.sendMessage(chat_id, "No rain detection history available 🌤️", "");
      } else {
        const size_t maxEntriesPerMessage = 10;
        size_t totalEntries = rainHistory.size();
        size_t messageCount = (totalEntries + maxEntriesPerMessage - 1) / maxEntriesPerMessage; // Calculate number of messages needed

        for (size_t i = 0; i < messageCount; ++i) {
          String historyMessage = "🌧️ Rain Detection History (" + String(i + 1) + "/" + String(messageCount) + "):\n\n";
          for (size_t j = i * maxEntriesPerMessage; j < (i + 1) * maxEntriesPerMessage && j < totalEntries; ++j) {
            historyMessage += "• " + rainHistory[j] + "\n";
          }
          bot.sendMessage(chat_id, historyMessage, "");
        }
      }
    }

    if (text == "/temp_humid" || text == "/temp_humid@rainguard_bot") {
      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

      if (isnan(temperature) || isnan(humidity)) {
        bot.sendMessage(chat_id, "Failed to read from DHT sensor!", "");
      } else {
        String tempHumidMessage = "🌡️ Temperature: " + String(temperature) + " °C\n";
        tempHumidMessage += "💧 Humidity: " + String(humidity) + " %";
        bot.sendMessage(chat_id, tempHumidMessage, "");
      }
    }
  }
}

void sendTempHumidUpdate() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity)) {
    String tempHumidMessage = "🌡️ Temperature: " + String(temperature) + " °C\n";
    tempHumidMessage += "💧 Humidity: " + String(humidity) + " %";
    for (const auto& chat_id : authorizedUsers) {
      bot.sendMessage(chat_id, tempHumidMessage, "");
    }
  }
}

void checkRainSensor() {
  int rainValue = digitalRead(RAIN_SENSOR_PIN);
  if (rainValue == LOW && !rainDetected) {
    rainDetected = true;
    rainStartTime = millis();
    String startMessage = "🌧️ Rain started at " + getCurrentTimestamp() + "!";

    // Send the rain start alert to every authorized user
    for (const auto& chat_id : authorizedUsers) {
      bot.sendMessage(chat_id, startMessage, "");
    }

    Serial.println("Rain detected!");
  } else if (rainValue == HIGH && rainDetected) {
    rainDetected = false;
    unsigned long rainEndTime = millis();
    unsigned long duration = (rainEndTime - rainStartTime) / 1000;
    unsigned long hours = duration / 3600;
    unsigned long minutes = (duration % 3600) / 60;
    unsigned long seconds = duration % 60;

    String timestamp = getCurrentTimestamp();
    String durationMessage = "🌦️ Rain stopped at " + timestamp + ". Duration: ";
    if (hours > 0) {
      durationMessage += String(hours) + " hours, ";
    }
    if (minutes > 0) {
      durationMessage += String(minutes) + " minutes, ";
    }
    durationMessage += String(seconds) + " seconds.";

    // Send the rain stop alert to every authorized user
    for (const auto& chat_id : authorizedUsers) {
      bot.sendMessage(chat_id, durationMessage, "");
    }

    rainHistory.push_back(durationMessage);
    Serial.println("No rain detected.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECTION_TIMEOUT) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to WiFi.");
  }

  // Connect to Telegram server
  client.setInsecure();
  if (client.connect("api.telegram.org", 443)) {
    Serial.println("Connected to Telegram server.");
    bot.sendMessage(AUTHORIZED_CHAT_ID, "ESP32 connected to Telegram server.", "");

    // Initial authorization and sending welcome message to all authorized users
    sendWelcomeMessage(AUTHORIZED_CHAT_ID);
  } else {
    Serial.println("Failed to connect to Telegram server.");
  }

  // Set up time synchronization
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void loop() {
  // Restart the ESP if forceBoot is set to true
  if (forceBoot) {
    forceBoot = false;
    ESP.restart();
  }

  // Check for new messages from the bot at regular intervals
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("[Telegram] Got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // Continuously check the rain sensor
  checkRainSensor();

  // Send temperature and humidity update every 10 minutes during rain
  if (rainDetected && millis() - lastTempHumidUpdateTime >= TEMP_HUMID_UPDATE_INTERVAL) {
    sendTempHumidUpdate();
    lastTempHumidUpdateTime = millis();
  }

  // Delay to avoid flooding the server
  delay(1000);
}
