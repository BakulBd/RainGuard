#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <vector>

// Replace with your network credentials
const char* ssid = "GUB-232";
const char* password = "momin123";

// Initialize Telegram BOT
#define BOTtoken "7158360864:AAEZhKGgtiH8Al3oyBU5_HSHZS24hz74gIs"
#define AUTHORIZED_CHAT_ID "656098264"
#define GROUP_CHAT_ID "-123456789" // Replace with your group chat ID

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int rainSensorPin = 0;
bool rainSensorState = LOW;
bool rainDetected = false;
unsigned long rainStartTime;
unsigned long rainEndTime;

std::vector<String> authorizedUsers;
std::vector<String> rainHistory;

void sendWelcomeMessage(String chat_id) {
  String welcome = "🌧️ Welcome to RainGuard! 🌧️\n\n";
  welcome += "RainGuard is your reliable project for detecting and monitoring rain. Here are the available commands:\n\n";
  welcome += "/rain_on - Turn ON the Rain Sensor\n";
  welcome += "/rain_off - Turn OFF the Rain Sensor\n";
  welcome += "/restart - Restart the system\n";
  welcome += "/setwifi <SSID> <PASSWORD> - Set Wi-Fi credentials\n";
  welcome += "/authorize <CHAT_ID> - Authorize a new user\n";
  welcome += "/rainhistory - Get the rain detection history\n";
  bot.sendMessage(chat_id, welcome, "");
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Message from " + chat_id + ": " + text);

    if (chat_id != AUTHORIZED_CHAT_ID && std::find(authorizedUsers.begin(), authorizedUsers.end(), chat_id) == authorizedUsers.end()) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      bot.sendMessage(AUTHORIZED_CHAT_ID, "Unauthorized user tried to access: " + from_name + " (" + chat_id + ")", "");
      continue;
    }

    if (text == "/start") {
      sendWelcomeMessage(chat_id);
    }

    if (text == "/rain_on") {
      bot.sendMessage(chat_id, "Rain Sensor ON", "");
      rainSensorState = HIGH;
      digitalWrite(rainSensorPin, rainSensorState);
      rainHistory.push_back("Rain Sensor turned ON at " + String(millis() / 1000) + " seconds");
    }

    if (text == "/rain_off") {
      bot.sendMessage(chat_id, "Rain Sensor OFF", "");
      rainSensorState = LOW;
      digitalWrite(rainSensorPin, rainSensorState);
      rainHistory.push_back("Rain Sensor turned OFF at " + String(millis() / 1000) + " seconds");
    }

    if (text == "/restart") {
      bot.sendMessage(chat_id, "Restarting the system...", "");
      ESP.restart();
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
        while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
          Serial.println("Connecting to WiFi with new credentials..");
        }
        bot.sendMessage(chat_id, "Connected to new Wi-Fi network", "");
      }
    }

    if (text.startsWith("/authorize")) {
      if (chat_id == AUTHORIZED_CHAT_ID) {
        int firstSpace = text.indexOf(' ');
        if (firstSpace == -1) {
          bot.sendMessage(chat_id, "Invalid format. Use /authorize <CHAT_ID>", "");
        } else {
          String newChatID = text.substring(firstSpace + 1);
          authorizedUsers.push_back(newChatID);
          bot.sendMessage(chat_id, "User " + newChatID + " has been authorized", "");
          sendWelcomeMessage(newChatID); // Send welcome message to newly authorized user
        }
      } else {
        bot.sendMessage(chat_id, "You are not authorized to authorize new users", "");
      }
    }

    if (text == "/rainhistory") {
      String history = "Rain Detection History:\n";
      for (const auto& entry : rainHistory) {
        history += entry + "\n";
      }
      bot.sendMessage(GROUP_CHAT_ID, history, "");
    }
  }
}

void checkRainSensor() {
  int rainValue = digitalRead(rainSensorPin);
  if (rainValue == HIGH && !rainDetected) {
    rainDetected = true;
    rainStartTime = millis();
    bot.sendMessage(GROUP_CHAT_ID, "🌧️ Rain started!", "");
  } else if (rainValue == LOW && rainDetected) {
    rainDetected = false;
    rainEndTime = millis();
    unsigned long duration = (rainEndTime - rainStartTime) / 1000;
    String durationMessage = "Rain stopped. Duration: " + String(duration) + " seconds.";
    bot.sendMessage(GROUP_CHAT_ID, durationMessage, "");
    rainHistory.push_back(durationMessage);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(rainSensorPin, INPUT);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("WiFi connected.");
  Serial.println(WiFi.localIP());

  // Connect to Telegram server
  client.setInsecure();
  if (client.connect("api.telegram.org", 443)) {
    Serial.println("Connected to Telegram server.");
    bot.sendMessage(AUTHORIZED_CHAT_ID, "ESP32 connected to Telegram server.", "");

    // Initial authorization and sending welcome message to all authorized users
    authorizedUsers.push_back(AUTHORIZED_CHAT_ID);
    sendWelcomeMessage(AUTHORIZED_CHAT_ID);
  } else {
    Serial.println("Failed to connect to Telegram server.");
  }
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  
  checkRainSensor();
}
