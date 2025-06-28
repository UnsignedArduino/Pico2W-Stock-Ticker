#include "config.h"
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <WiFi.h>

WiFiClientSecure wifiClient;
WebSocketClient wsClient = WebSocketClient(wifiClient, host, port);

void setup() {
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  Serial1.println("\n");
  while (WiFi.status() != WL_CONNECTED) {
    Serial1.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(1000);
  }
  Serial1.println("Connected to WiFi");
  Serial1.print("IP Address: ");
  Serial1.println(WiFi.localIP());
}

void loop() {
  wifiClient.setInsecure();
  wsClient.begin("/?version=2");

  while (!wsClient.connected()) {
    Serial1.println("Connecting to websocket...");
    delay(1000);
  }
  Serial1.println("Connected to websocket");

  Serial1.println("Subscribing to BTC-USD ticker...");
  wsClient.beginMessage(TYPE_TEXT);
  wsClient.print("{\"subscribe\": [\"BTC-USD\"]}");
  wsClient.endMessage();

  while (wsClient.connected()) {
    if (wsClient.parseMessage() > 0) {
      Serial1.print("Received message: ");
      Serial1.println(wsClient.readString());
    } else {
      Serial1.println("No message received");
    }

    delay(1000);
  }
  Serial1.println("Disconnected");
  delay(5000);
}
