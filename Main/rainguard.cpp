#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <vector>
#include <algorithm>
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "GUB-232";
const char* password = "momin123";

// Initialize Telegram BOT
#define BOTtoken "7158360864:AAEZhKGgtiH8Al3oyBU5_HSHZS24hz74gIs"  // Replace with your Bot Token
#define AUTHORIZED_CHAT_ID "656098264"
#define GROUP_CHAT_ID "-1002020001615"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int WIFI_CONNECTION_TIMEOUT = 10000; // 10 seconds
const int RAIN_SENSOR_PIN = 0; // Assuming GPIO 0
#define DHT_PIN 13
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

bool rainDetected = false;
bool sensorsOn = true;
bool forceBoot = false;
unsigned long rainStartTime;
unsigned long rainEndTime;

std::vector<String> authorizedUsers = {AUTHORIZED_CHAT_ID};
std::vector<String> rainHistory;

void sendWelcomeMessage(String chat_id) {
  Serial.println("Sending welcome message...");

  String welcome = "🌧️ Welcome to RainGuard! 🌧️\n\n";
  welcome += "RainGuard is your reliable friend for detecting and monitoring rain. Here are the available commands:\n\n";
  welcome += "/status - Check rain status\n";
  welcome += "/rain_on - Turn ON the Rain Sensor\n";
  welcome += "/rain_off - Turn OFF the Rain Sensor\n";
  welcome += "/restart - Restart the system\n";
  welcome += "/setwifi <SSID> <PASSWORD> - Set Wi-Fi credentials\n";
  welcome += "/authorize <CHAT_ID> - Authorize a new user\n";
  welcome += "/rainhistory - Get the rain detection history\n";

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

    if (text == "/rain_on" || text == "/rain_on@rainguard_bot") {
      bot.sendMessage(chat_id, "Rain Sensor ON", "");
      rainDetected = true;
      rainHistory.push_back("Rain Sensor turned ON at " + String(millis() / 1000) + " seconds");
    }

    if (text == "/rain_off" || text == "/rain_off@rainguard_bot") {
      bot.sendMessage(chat_id, "Rain Sensor OFF", "");
      rainDetected = false;
      rainHistory.push_back("Rain Sensor turned OFF at " + String(millis() / 1000) + " seconds");
    }

    if (text == "/status" || text == "/status@rainguard_bot") {
      String statusMessage = "🌧️ Rain Status 🌧️\n";
      statusMessage += "Currently ";
      statusMessage += rainDetected ? "raining." : "not raining.";
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
        bot.sendMessage(chat_id, "No rain detection history available", "");
      } else {
        String history = "🌧️ Rain Detection History:\n";
        for (const auto& entry : rainHistory) {
          history += entry + "\n";
        }
        bot.sendMessage(chat_id, history, "");
      }
    }
  }
}

void checkRainSensor() {
  int rainValue = digitalRead(RAIN_SENSOR_PIN);
  if (rainValue == HIGH && !rainDetected) {
    rainDetected = true;
    rainStartTime = millis();
    bot.sendMessage(GROUP_CHAT_ID, "🌧️ Rain started!", "");
  } else if (rainValue == LOW && rainDetected) {
    rainDetected = false;
    rainEndTime = millis();
    unsigned long duration = (rainEndTime - rainStartTime) / 1000;
    unsigned long hours = duration / 3600;
    unsigned long minutes = (duration % 3600) / 60;
    unsigned long seconds = duration % 60;
    String durationMessage = "🌦️ Rain stopped. Duration: ";
    if (hours > 0) {
      durationMessage += String(hours) + " hours, ";
    }
    if (minutes > 0) {
      durationMessage += String(minutes) + " minutes, ";
    }
    durationMessage += String(seconds) + " seconds.";
    bot.sendMessage(GROUP_CHAT_ID, durationMessage, "");
    rainHistory.push_back(durationMessage);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(DHT_PIN, INPUT_PULLUP);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECTION_TIMEOUT) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to WiFi.");
  }

  lastTimeBotRan = millis();
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

  // Delay to avoid flooding the server
  delay(1000);
}
