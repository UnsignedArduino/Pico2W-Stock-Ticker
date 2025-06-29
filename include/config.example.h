// Rename to config.h
// Temporary, will later use WiFi provisioning to make this unnecessary

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char* ssid = "your-ssid";         // Replace with your WiFi SSID
const char* password = "your-password"; // Replace with your WiFi password

// Comma-separated list of symbols to subscribe to
const size_t maxSymbolsStringLen = 256; // Maximum length of symbols string
// Maximum number of symbols to subscribe to
// 30 is the limit for Alpaca Markets free plan on websocket
const uint16_t maxSymbols = 30;
char symbols[maxSymbolsStringLen] = "AAPL,GOOGL,MSFT,AMZN,TSLA,NVDA";

#endif
