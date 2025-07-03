// Rename to config.h
// Temporary, will later use WiFi provisioning to make this unnecessary

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char* ssid = "your-ssid";         // Replace with your WiFi SSID
const char* password = "your-password"; // Replace with your WiFi password
const char* apcaApiKeyId =
  "your-alpaca-markets-api-key-id"; // Replace with your Alpaca Markets API key
                                    // ID
const char* apcaApiSecretKey =
  "your-alpaca-markets-api-secret-key"; // Replace with your Alpaca Markets API
                                        // secret Key

// Comma-separated list of symbols to subscribe to
const size_t maxSymbolsStringLen = 256; // Maximum length of symbols string
// Maximum number of symbols to subscribe to
// 30 is the limit for Alpaca Markets free plan on websocket
const uint16_t maxSymbols = 30;
const char symbols[maxSymbolsStringLen] = "AAPL,GOOGL,MSFT,AMZN,TSLA,NVDA";

const uint16_t requestPeriod = 30 * 1000; // Request period in milliseconds
// How often to call the rest API, when not using the websocket
// Free plan is limited to 200 requests / s

const char* feed = "iex"; // Either "sip", "iex", or "delayed_sip"
// The SIP feed (all exchanges) is paid

#endif
