// Rename to config.h
// Temporary, will later use WiFi provisinioning

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char* ssid = "your-ssid";         // Replace with your WiFi SSID
const char* password = "your-password"; // Replace with your WiFi password

const char* host = "streamer.finance.yahoo.com";
const uint16_t port = 443;

// Comma-separated list of symbols to subscribe to
const size_t maxSymbolsStringLen = 256; // Adjust as needed
char symbols[maxSymbolsStringLen] =
  "AAPL,GOOGL,MSFT,AMZN,TSLA,NVDA,BTC-USD,ETH-USD";

#endif
