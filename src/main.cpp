#include "config.h"
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <MD_Parola.h>
#include <StreamUtils.h>
#include <WiFi.h>

// Latest bars URL is https://data.alpaca.markets/v2/stocks/bars/latest
const char* SERVER_ADDRESS = "data.alpaca.markets";
const uint16_t SERVER_PORT = 443; // HTTPS port
const char* PATH = "/v2/stocks/bars/latest";

// Latest bars response is something like this:
// "AMZN": {
//     "c": 223.07,  (close price)
//     "h": 223.24,  (high price)
//     "l": 222.86,  (low price)
//     "n": 564,  (number of trades)
//     "o": 222.91,  (open price)
//     "t": "2025-06-27T19:59:00Z",  (RFC-3339 formatted timestamp)
//     "v": 45662,  (volume)
//     "vw": 223.0137  (volume-weighted average price)
// },
// We only care about the open and close price to display the current price and
// change in price
const size_t maxSymbolIDLen = 16;
// clang-format off
struct SymbolData {
  char id[maxSymbolIDLen]; // Symbol name, e.g. "AAPL"
  float openPrice;         // Open price
  float closePrice;        // Close price
  // We'll calculate change and change percent on the fly
};
// clang-format on
SymbolData allSymbols[maxSymbols];
uint16_t symbolsCount = 0;

void initializeSymbolData() {
  memset(allSymbols, 0, sizeof(allSymbols));
  char str[maxSymbolsStringLen];
  strncpy(str, symbols, maxSymbolsStringLen);
  char* token;
  char* rest = str;
  symbolsCount = 0;
  while ((token = strtok_r(rest, ",", &rest)) && symbolsCount < maxSymbols) {
    if (strlen(token) < maxSymbolIDLen) {
      strncpy(allSymbols[symbolsCount].id, token, maxSymbolIDLen);
      // If price is negative than no data yet
      allSymbols[symbolsCount].openPrice = -1.0f;
      allSymbols[symbolsCount].closePrice = -1.0f;
      Serial1.printf("Symbol '%s' initialized at index %d\n", token,
                     symbolsCount);
      symbolsCount++;
    } else {
      Serial1.printf("Symbol '%s' is too long, skipping.\n", token);
    }
  }
}

void updateSymbolData(const char* id, float openPrice, float closePrice) {
  for (uint16_t i = 0; i < symbolsCount; i++) {
    if (strcmp(allSymbols[i].id, id) == 0) {
      allSymbols[i].openPrice = openPrice;
      allSymbols[i].closePrice = closePrice;
      Serial1.printf("Updated symbol '%s' at index %d: open=%.2f, close=%.2f\n",
                     id, i, openPrice, closePrice);
      return;
    }
  }
  Serial1.printf("Symbol '%s' not found, cannot update.\n", id);
}

const size_t maxSymbolDisplayStrLen = 64;
const size_t maxDisplayStrLen = maxSymbolDisplayStrLen * maxSymbols;
char displayStr[maxDisplayStrLen];

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
  initializeSymbolData();

  SPI.setSCK(CLK_PIN);
  SPI.setTX(DATA_PIN);
  SPI.setCS(CS_PIN);
  SPI.begin();
  p.begin();
  p.setIntensity(8);
  p.displayClear();
  p.setSpeed(15);
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

  while (WiFi.status() == WL_CONNECTED) {
    static uint32_t lastRequestTime = -requestPeriod;
    if (millis() - lastRequestTime >= requestPeriod) {
      digitalWrite(LED_BUILTIN, HIGH);
      // Fetch (network request)
      Serial1.println("Requesting latest stock bars from Alpaca Markets API");
      WiFiClientSecure wifiClient;
      wifiClient.setInsecure();
      HttpClient client = HttpClient(wifiClient, SERVER_ADDRESS, SERVER_PORT);
      const size_t pathMaxLen = sizeof(PATH) + maxSymbolsStringLen + 1;
      char actualPath[pathMaxLen];
      snprintf(actualPath, pathMaxLen, "%s?symbols=%s&feed=%s", PATH, symbols,
               feed);
      Serial1.printf("Requesting %s from %s\n", actualPath, SERVER_ADDRESS);
      client.beginRequest();
      client.get(actualPath);
      client.sendHeader("APCA-API-KEY-ID", apcaApiKeyId);
      client.sendHeader("APCA-API-SECRET-KEY", apcaApiSecretKey);
      client.endRequest();
      int16_t statusCode = client.responseStatusCode();
      if (statusCode == 200) {
        // Parse JSON
        Serial1.println("Status code 200 (ok) - parsing response now");
        client.skipResponseHeaders();
        ReadBufferingClient bufferedClient(client, 256);
        //        while (bufferedClient.available() > 0) {
        //          Serial1.write(bufferedClient.read());
        //        }
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, bufferedClient);
        if (error) {
          Serial1.print("Failed to parse JSON: ");
          Serial1.println(error.c_str());
        } else {
          Serial1.println("JSON parsed successfully");
          for (JsonPair bar : doc["bars"].as<JsonObject>()) {
            const char* bar_key = bar.key().c_str();
            const float bar_value_c = bar.value()["c"];
            const float bar_value_o = bar.value()["o"];
            updateSymbolData(bar_key, bar_value_o, bar_value_c);
          }
        }
      } else if (statusCode == 400) {
        Serial1.println("Status code 400 (invalid) - check symbol or API key "
                        "ID and secret key");
      } else if (statusCode == 403) {
        Serial1.println(
          "Status code 403 (forbidden) - check API key ID and secret key");
      } else if (statusCode == 429) {
        Serial1.println(
          "Status code 429 (too many requests) - check request period");
      } else if (statusCode == 500) {
        Serial1.println(
          "Status code 500 (internal server error) - try again later");
      } else {
        Serial1.printf("Status code %d unexpected \n", statusCode);
      }
      client.stop();
      Serial1.printf("Request finished, requesting again in %u ms\n",
                     requestPeriod);
      lastRequestTime = millis();
      // Update string to display
      memset(displayStr, 0, maxDisplayStrLen);
      char* ptr = displayStr;
      if (statusCode == 200) { // OK
        for (uint16_t i = 0; i < symbolsCount; i++) {
          size_t charsWritten = 0;
          if (allSymbols[i].openPrice >
              0) { // Non-negative means data is available
            const float change =
              allSymbols[i].closePrice - allSymbols[i].openPrice;
            const float changePercent =
              (change / allSymbols[i].openPrice) * 100.0f;
            char sign = '+';
            if (change < 0) {
              sign = '-';
            }
            charsWritten = snprintf(ptr, maxSymbolDisplayStrLen,
                                    "%s: $%.2f %+.2f%% (%c$%.2f)    ",
                                    allSymbols[i].id, allSymbols[i].closePrice,
                                    changePercent, sign, abs(change));
          } else {
            // No data yet cause price is negative
            charsWritten = snprintf(ptr, maxSymbolDisplayStrLen,
                                    "%s: Loading...    ", allSymbols[i].id);
          }
          ptr += charsWritten;
          if (ptr - displayStr >= maxDisplayStrLen - maxSymbolDisplayStrLen) {
            break; // Prevent overflow
          }
        }
      } else if (statusCode == 400) {
        snprintf(displayStr, maxDisplayStrLen,
                 "Error - Invalid request, check symbols or API key ID and "
                 "secret key");
      } else if (statusCode == 403) {
        snprintf(displayStr, maxDisplayStrLen,
                 "Error - Forbidden request, check API key ID and secret key");
      } else if (statusCode == 429) {
        snprintf(displayStr, maxDisplayStrLen,
                 "Error - Too many requests, check request period");
      } else if (statusCode == 500) {
        snprintf(displayStr, maxDisplayStrLen,
                 "Error - Internal server error, try again later");
      } else {
        snprintf(
          displayStr, maxDisplayStrLen,
          "Error - Unexpected status code of %d, try again later or check "
          "API status",
          statusCode);
      }
      Serial1.println("Display string updated:");
      Serial1.println(displayStr);
      digitalWrite(LED_BUILTIN, LOW);
    }

    if (p.displayAnimate()) {
      static char textToScroll[maxDisplayStrLen];
      strncpy(textToScroll, displayStr, maxDisplayStrLen);
      p.displayText(textToScroll, PA_LEFT, 40, 0, PA_SCROLL_LEFT,
                    PA_SCROLL_LEFT);
    }
  }

  Serial1.println("Disconnected from WiFi");
  delay(5000);
}
