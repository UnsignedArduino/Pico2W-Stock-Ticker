#include "config.h"
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <MD_Parola.h>
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

const size_t maxIDLen = 32;
// clang-format off
struct SymbolPrice {
  char id[maxIDLen];
  float price;
  float change;
  float changePercent;
};
// clang-format on
SymbolPrice allSymbolPrices[maxSymbols];
uint16_t symbolsCount = 0;

void initializeAllSymbolPrices() {
  memset(allSymbolPrices, 0, sizeof(allSymbolPrices));
  char str[maxSymbolsStringLen];
  strncpy(str, symbols, maxSymbolsStringLen);
  char* token;
  char* rest = str;
  symbolsCount = 0;
  while ((token = strtok_r(rest, ",", &rest)) && symbolsCount < maxSymbols) {
    if (strlen(token) < maxIDLen) {
      strncpy(allSymbolPrices[symbolsCount].id, token, maxIDLen);
      // If price is negative than no data yet
      allSymbolPrices[symbolsCount].price = -1;
      Serial1.printf("Symbol '%s' initialized at index %d\n", token,
                     symbolsCount);
      symbolsCount++;
    } else {
      Serial1.printf("Symbol '%s' is too long, skipping.\n", token);
    }
  }
}

void updateSymbolPrice(const char* id, float price, float change,
                       float changePercent) {
  for (uint16_t i = 0; i < maxSymbols; i++) {
    if (strcmp(allSymbolPrices[i].id, id) == 0) {
      allSymbolPrices[i].price = price;
      allSymbolPrices[i].change = change;
      allSymbolPrices[i].changePercent = changePercent;
      Serial1.printf("Updated symbol %s in symbol data list\n",
                     allSymbolPrices[i].id);
      return;
    }
  }
  Serial1.printf("Symbol %s not found in symbol data list\n", id);
}

const size_t maxSymbolDisplayStrLen = 64;
const size_t maxDisplayStrLen = maxSymbolDisplayStrLen * maxSymbols;
char displayStr[maxDisplayStrLen];

void updateDisplayStr() {
  memset(displayStr, 0, maxDisplayStrLen);
  char* ptr = displayStr;
  for (uint16_t i = 0; i < symbolsCount; i++) {
    size_t charsWritten = 0;
    if (allSymbolPrices[i].price > 0) {
      char sign = '+';
      if (allSymbolPrices[i].change < 0) {
        sign = '-';
      }
      charsWritten = snprintf(
        ptr, maxSymbolDisplayStrLen, "%s: $%.2f %+.2f%% (%c$%.2f)    ",
        allSymbolPrices[i].id, allSymbolPrices[i].price,
        allSymbolPrices[i].changePercent, sign, abs(allSymbolPrices[i].change));
    } else {
      // No data yet cause price is negative
      charsWritten = snprintf(ptr, maxSymbolDisplayStrLen, "%s: No data yet...    ",
                              allSymbolPrices[i].id);
    }
    ptr += charsWritten;
    if (ptr - displayStr >= maxDisplayStrLen - maxSymbolDisplayStrLen) {
      break; // Prevent overflow
    }
  }
  Serial1.println("Display string updated:");
  Serial1.println(displayStr);
}

const MD_MAX72XX::moduleType_t HARDWARE_TYPE = MD_MAX72XX::FC16_HW;
uint8_t MATRIX_COUNT = 16;

#define USE_HARDWARE_SPI
const uint8_t CLK_PIN = 2;
const uint8_t DATA_PIN = 3;
const uint8_t CS_PIN = 5;

#ifdef USE_HARDWARE_SPI
MD_Parola p = MD_Parola(HARDWARE_TYPE, SPI, CS_PIN, MATRIX_COUNT);
#else
MD_Parola p = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
#endif

void setup() {
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  Serial1.println("\n");

  Serial1.println(symbols);
  initializeAllSymbolPrices();
  updateDisplayStr();

  Serial1.println("Initializing display...");
  SPI.setSCK(CLK_PIN);
  SPI.setTX(DATA_PIN);
  SPI.setCS(CS_PIN);
  SPI.begin();
  p.begin();
  p.setIntensity(8);
  p.displayClear();
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial1.println("Connecting to WiFi...");
    p.displayClear();
    p.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(1000);
  }
  Serial1.println("Connected to WiFi");
  Serial1.print("IP Address: ");
  Serial1.println(WiFi.localIP());

  wifiClient.setInsecure();
  wsClient.begin(PATH);

  do {
    Serial1.println("Connecting to websocket...");
    p.displayClear();
    p.print("Connecting to server...");
    delay(1000);
  } while (!wsClient.connected());
  Serial1.println("Connected to websocket");

  Serial1.println("Subscribing to tickers...");
  wsClient.beginMessage(TYPE_TEXT);
  // char symbols[maxSymbolsStringLen] = "AAPL,GOOGL,...";
  //                       vvv
  // {"subscribe": ["AAPL", "GOOGL", ...]}
  wsClient.print("{\"subscribe\": [");
  Serial1.print("{\"subscribe\": [");
  for (uint16_t i = 0; i < symbolsCount; i++) {
    if (i > 0) {
      wsClient.print(", ");
      Serial1.print(", ");
    }
    wsClient.printf("\"%s\"", allSymbolPrices[i].id);
    Serial1.printf("\"%s\"", allSymbolPrices[i].id);
  }
  wsClient.print("]}");
  Serial1.println("]}");
  wsClient.endMessage();
  Serial1.println("Subscribed to tickers");

  p.displayClear();
  p.print("Connected!");
  delay(1000);

  while (wsClient.connected()) {
    static uint32_t turnOffBuiltinAt = 0;

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
          updateSymbolPrice(pricingData.id, pricingData.price,
                            pricingData.change, pricingData.change_percent);
          updateDisplayStr();

          digitalWrite(LED_BUILTIN, HIGH);
          turnOffBuiltinAt = millis() + 100;
        } else {
          Serial1.println("Received non-pricing message, ignoring.");
        }
      }
    }

    if (turnOffBuiltinAt != 0 && millis() > turnOffBuiltinAt) {
      digitalWrite(LED_BUILTIN, LOW);
      turnOffBuiltinAt = 0;
    }
  }
  Serial1.println("Disconnected");
  delay(5000);
}
