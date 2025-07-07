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
const uint16_t maxSymbols = 32;
const char symbols[maxSymbolsStringLen] = "AAPL,GOOGL,MSFT,AMZN,TSLA,NVDA";

#endif
