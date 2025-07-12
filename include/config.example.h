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

// Request period in milliseconds, how long to wait before asking for more data
// 200 requests / min is rate limit for free account
// One request is used to download all symbol data
const uint32_t requestPeriod = 60 * 1000;

#endif
