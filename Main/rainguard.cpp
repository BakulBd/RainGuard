#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "GUB-232";
const char* password = "momin123";

// Initialize Telegram BOT
#define BOTtoken "7158360864:AAEZhKGgtiH8Al3oyBU5_HSHZS24hz74gIs"
#define CHAT_ID "656098264"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin1 = 0;
const int ledPin2 = 4;

bool ledState1 = LOW;
bool ledState2 = LOW;

// Handle received messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands:.\n\n";

      welcome += "/ledOne_on to turn ON LED 1 \n";
      welcome += "/ledOne_off to turn OFF LED 1 \n";

      welcome += "/ledTwo_on to turn ON LED 2 \n";
      welcome += "/ledTwo_off to turn OFF LED 2 \n";

      bot.sendMessage(chat_id, welcome, "");
    }

    // LED 1
    if (text == "/ledOne_on") {
      bot.sendMessage(chat_id, "LEDone ON", "");
      ledState1 = HIGH;
      digitalWrite(ledPin1, ledState1);
    }

    if (text == "/ledOne_off") {
      bot.sendMessage(chat_id, "LEDone OFF", "");
      ledState1 = LOW;
      digitalWrite(ledPin1, ledState1);
    }

    // LED 2
    if (text == "/ledTwo_on") {
      bot.sendMessage(chat_id, "LEDTwo ON", "");
      ledState2 = HIGH;
      digitalWrite(ledPin2, ledState2);
    }

    if (text == "/ledTwo_off") {
      bot.sendMessage(chat_id, "LEDTwo OFF", "");
      ledState2 = LOW;
      digitalWrite(ledPin2, ledState2);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  digitalWrite(ledPin1, ledState1);
  digitalWrite(ledPin2, ledState2);

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
    bot.sendMessage(CHAT_ID, "ESP32 connected to Telegram server.", "");
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
}
