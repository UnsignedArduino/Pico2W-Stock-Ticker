// Rename to config.h
// Temporary, will later use WiFi provisioning to make this unnecessary

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// How many **groups of four** 8x8 MAX7219 modules are connected
const uint8_t matrixModulesCount = 4;
// How long to wait (in ms) between each shift while scrolling text
// (lower is faster)
const uint16_t scrollPeriodSpeed = 30;

const char* ssid = "your-ssid";         // Replace with your WiFi SSID
const char* password = "your-password"; // Replace with your WiFi password
// Replace with your Alpaca Markets API key ID
const char* apcaApiKeyId = "your-alpaca-markets-api-key-id";
// Replace with your Alpaca Markets API secret Key
const char* apcaApiSecretKey = "your-alpaca-markets-api-secret-key";

// Maximum length of symbols string
const size_t maxSymbolsStringLen = 256;
// Maximum number of symbols to reserve memory for
const uint16_t maxSymbols = 32;
// Comma-separated list of symbols to subscribe to
const char symbols[maxSymbolsStringLen] = "AAPL,GOOGL,MSFT,AMZN,TSLA,NVDA";

// Feed to subscribe to
// Either sip, iex, delayed_sip, boats, overnight, or otc
// Only iex or delayed_sip are available with free account
const char* sourceFeed = "iex";

// Request period in milliseconds, how long to wait before asking for more data
// 200 requests / min is rate limit for free account
// One request is used to download all symbol data
const uint32_t requestPeriod = 60 * 1000;

#endif
