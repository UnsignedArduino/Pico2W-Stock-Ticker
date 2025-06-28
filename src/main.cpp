#include "config.h"
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <WiFi.h>
#include <base64.hpp>
#include <pb_decode.h>
#include <pricing.pb.h>

// wss://streamer.finance.yahoo.com/?version=2
const char* HOST = "streamer.finance.yahoo.com";
const uint16_t PORT = 443;
const char* PATH = "/?version=2";

WiFiClientSecure wifiClient;
WebSocketClient wsClient = WebSocketClient(wifiClient, HOST, PORT);

void setup() {
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  Serial1.println("\n");
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial1.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(1000);
  }
  Serial1.println("Connected to WiFi");
  Serial1.print("IP Address: ");
  Serial1.println(WiFi.localIP());

  wifiClient.setInsecure();
  wsClient.begin(PATH);

  while (!wsClient.connected()) {
    Serial1.println("Connecting to websocket...");
    delay(1000);
  }
  Serial1.println("Connected to websocket");

  Serial1.println("Subscribing to tickers...");
  wsClient.beginMessage(TYPE_TEXT);
  // char symbols[maxSymbolsStringLen] = "AAPL,GOOGL,...";
  //                       vvv
  // {"subscribe": ["AAPL", "GOOGL", ...]}
  wsClient.print("{\"subscribe\": [");
  Serial1.print("{\"subscribe\": [");
  char str[maxSymbolsStringLen];
  strncpy(str, symbols, maxSymbolsStringLen);
  char* token;
  char* rest = str;
  while ((token = strtok_r(rest, ",", &rest))) {
    if (token != str) {
      wsClient.print(", ");
      Serial1.print(", ");
    }
    wsClient.printf("\"%s\"", token);
    Serial1.printf("\"%s\"", token);
  }
  wsClient.print("]}");
  Serial1.println("]}");
  wsClient.endMessage();
  Serial1.println("Subscribed to tickers");

  while (wsClient.connected()) {
    if (wsClient.parseMessage() > 0) {
      JsonDocument doc;

      ReadBufferingClient bufferedClient(wsClient, 256);
      DeserializationError error = deserializeJson(doc, bufferedClient);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      } else {
        const char* type = doc["type"];
        if (strcmp(type, "pricing") == 0) {
          const char* message = doc["message"];
          Serial1.print("Received pricing message: (base64 of protobuf)\n  ");
          Serial1.println(message);
          // First decode from base64 into a byte array and length
          // The messages are usually 160 characters long, so decoded will be
          // 120 bytes long. We'll have 192 bytes to be safe.
          const size_t decodedMaxSize = 192;
          uint8_t decoded[decodedMaxSize];
          const size_t decodedLength = decode_base64(
            reinterpret_cast<const unsigned char*>(message), decoded);
          // Then decode the protobuf message
          pb_istream_t istream = pb_istream_from_buffer(decoded, decodedLength);
          PricingData pricingData = PricingData_init_zero;
          Serial1.println("Decoded Pricing Data:");
          pb_decode(&istream, PricingData_fields, &pricingData);
          Serial1.printf("  ID: %s\n", pricingData.id);
          Serial1.printf("  Price: %.2f\n", pricingData.price);
          Serial1.printf("  Change: %+.2f\n", pricingData.change);
          Serial1.printf("  Change percent: %+.2f%%\n",
                         pricingData.change_percent);
        } else {
          Serial1.println("Received non-pricing message, ignoring.");
        }
      }
    }

    delay(1000);
  }
  Serial1.println("Disconnected");
  delay(5000);
}
