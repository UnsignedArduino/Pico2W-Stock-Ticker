// #define LOG_FREE_MEMORY
// #define LOG_JSON_PARSED
// #define BUFFER_JSON_READING

#include "config.h"
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#if defined(LOG_FREE_MEMORY) || defined(LOG_JSON_PARSED) ||                    \
  defined(BUFFER_JSON_READING)
  #include <StreamUtils.h>
#endif
#include <WiFi.h>

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
      Serial1.printf("Updated symbol %s in symbol data list (price: %.2f, "
                     "change: %.2f, changePercent: %.2f%%)\n",
                     allSymbolPrices[i].id, price, change, changePercent);
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
      charsWritten = snprintf(ptr, maxSymbolDisplayStrLen,
                              "%s: No data yet...    ", allSymbolPrices[i].id);
    }
    ptr += charsWritten;
    if (ptr - displayStr >= maxDisplayStrLen - maxSymbolDisplayStrLen) {
      break; // Prevent overflow
    }
  }
  Serial1.println("Display string updated:");
  Serial1.println(displayStr);
}

void setup() {
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  Serial1.println("\n");

  Serial1.println(symbols);
  initializeAllSymbolPrices();
  updateDisplayStr();
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

  static uint32_t refreshAtTime;
  refreshAtTime = 0; // Force refresh on first loop
  while (WiFi.status() == WL_CONNECTED) {
    if (refreshAtTime < millis()) {
      // Time to call API
      Serial1.println("Requesting data from Alpaca Markets API...");
#ifdef LOG_FREE_MEMORY
      Serial1.printf("Free memory before request: heap %d kb, stack %d kb\n",
                     rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
      { // Scope to destroy client to print free memory after request
        WiFiClientSecure wifiClient;
        wifiClient
          .setInsecure(); // Disable SSL certificate verification for now
        // The URL is:
        // https://data.alpaca.markets/v2/stocks/snapshots?symbols={SYMBOLS}&feed={FEED}

        // We need to send something like this:
        // GET /v2/stocks/snapshots?symbols=AAPL%2CTSLA&feed=iex HTTP/1.1
        // Accept: application/json
        // Apca-Api-Key-Id: ********************
        // Apca-Api-Secret-Key: ****************************************
        // Host: data.alpaca.markets
        HttpClient client(wifiClient, "data.alpaca.markets", 443);
        client.beginRequest();
        // Length of /v2/stocks/snapshots?symbols=&feed=iex is 38 chars so 64
        // chars + maxSymbolsStringLen is enough
        const size_t maxURLLen = 64 + maxSymbolsStringLen;
        char url[maxURLLen];
        snprintf(url, maxURLLen, "/v2/stocks/snapshots?symbols=%s&feed=%s",
                 symbols, sourceFeed);
        Serial1.printf("Requesting https://data.alpaca.markets%s\n", url);
        client.get(url);
        client.sendHeader("Accept", "application/json");
        client.sendHeader("Apca-Api-Key-Id", apcaApiKeyId);
        client.sendHeader("Apca-Api-Secret-Key", apcaApiSecretKey);
        client.endRequest();
        int statusCode = client.responseStatusCode();
        client.skipResponseHeaders();
#ifdef BUFFER_JSON_READING
        ReadBufferingClient bufferedClient(client, 256);
#endif
#ifdef LOG_FREE_MEMORY
        Serial1.printf(
          "Free memory after sending request: heap %d kb, stack %d kb\n",
          rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
        if (statusCode == 200) { // OK
          JsonDocument doc;
#ifdef LOG_JSON_PARSED
          Serial1.println("JSON read:");
  #ifdef BUFFER_JSON_READING
          ReadLoggingStream loggingStream(bufferedClient, Serial1);
  #else
          ReadLoggingStream loggingStream(client, Serial1);
  #endif
          DeserializationError error = deserializeJson(doc, loggingStream);
          Serial1.println("");
#else
  #ifdef BUFFER_JSON_READING
          DeserializationError error = deserializeJson(doc, bufferedClient);
  #else
          DeserializationError error = deserializeJson(doc, client);
  #endif
#endif
#ifdef LOG_FREE_MEMORY
          Serial1.printf(
            "Free memory after JSON parse: heap %d kb, stack %d kb\n",
            rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
          if (error) {
            Serial1.printf("Failed to parse JSON: %s\n", error.c_str());
          } else {
            for (JsonPair snapshot : doc.as<JsonObject>()) {
              const char* symbol = snapshot.key().c_str();
              JsonObject daily_bar = snapshot.value()["dailyBar"];
              float open_price = daily_bar["o"];  // Start of day price
              float close_price = daily_bar["c"]; // End of day / current price
              updateSymbolPrice(symbol, close_price, close_price - open_price,
                                ((close_price - open_price) / open_price) *
                                  100.0f);
            }
            updateDisplayStr();
          }
        } else {
          Serial1.printf("Bad status code: %d\n", statusCode);
          switch (statusCode) {
            case 400: { // Bad request
              Serial1.println("Bad request, check your API key and secret.");
              break;
            }
            case 403: { // Forbidden
              Serial1.println("Forbidden, check your API key and secret.");
              break;
            }
            case 429: { // Too many requests
              Serial1.println("Too many requests, check your request period.");
              break;
            }
            case 500: { // Internal server error
              Serial1.println("Internal server error, check Alpaca Markets' "
                              "Slack or Community Forum.");
              break;
            }
          }
          Serial1.println("Response: ");
#ifdef BUFFER_JSON_READING
          while (bufferedClient.available()) {
            Serial1.write(bufferedClient.read());
          }
#else
          while (client.available()) {
            Serial1.write(client.read());
          }
#endif
        }
      }
#ifdef LOG_FREE_MEMORY
      Serial1.printf("Free memory after request: heap %d kb, stack %d kb\n",
                     rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
      refreshAtTime = millis() + requestPeriod;
      Serial1.printf("Next request in %d seconds\n\n", requestPeriod / 1000);
    }
  }

  Serial1.println("Disconnected from WiFi");
  delay(5000);
}
