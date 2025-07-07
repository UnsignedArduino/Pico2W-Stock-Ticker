// Rename to config.h
// Temporary, will later use WiFi provisioning to make this unnecessary

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

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

// Request period in milliseconds, how long to wait before asking for more fresh
// data for every symbol Free account gets 200 requests per minute, so for max
// of 32 symbols, we can request data every:
// (60 seconds / 200 hits) * (32 hits / refresh) = 9.6 seconds
// MINIMUM refresh time to not hit limit
// But 30 seconds before requesting again is definitely fast enough for me
const uint32_t requestPeriod = 30 * 1000;

#endif
