//
// Created by ckyiu on 7/11/2025.
//

#ifndef PICO2W_STOCK_TICKER_WIFISETTINGS_H
#define PICO2W_STOCK_TICKER_WIFISETTINGS_H

#ifndef LOG_FREE_MEMORY
  #define LOG_FREE_MEMORY
#endif
#ifndef LOG_JSON_PARSED
  #define LOG_JSON_PARSED
#endif

#include "ArduinoJson.h"
#include "StreamUtils.h"
#include <Arduino.h>
#include <FatFS.h>
#include <FatFSUSB.h>

namespace Settings {
  const size_t MAX_SSID_LENGTH = 32 + 1; // +1 for null terminator
  const size_t MAX_PASSWORD_LENGTH = 63 + 1;

  enum class WiFiSaveToDiskResult {
    OK,
    ERROR_FATFS_INIT_FAILED,
    ERROR_FILE_OPEN_FAILED
  };

  enum class WiFiLoadFromDiskResult {
    OK,
    ERROR_FATFS_INIT_FAILED,
    ERROR_FILE_OPEN_FAILED,
    ERROR_JSON_PARSE_TOO_DEEP,
    ERROR_JSON_PARSE_NO_MEMORY,
    ERROR_JSON_PARSE_INVALID_INPUT,
    ERROR_JSON_PARSE_INCOMPLETE_INPUT,
    ERROR_JSON_PARSE_EMPTY_INPUT,
    ERROR_JSON_PARSE_UNKNOWN_ERROR,
    ERROR_INVALID_SSID,
    ERROR_INVALID_PASSWORD
  };

  const char* wiFiSaveToDiskResultToMessage(WiFiSaveToDiskResult r);
  const char* wiFiLoadFromDiskResultToMessage(WiFiLoadFromDiskResult r);

  extern bool usbConnected;

  class WiFiSettings {
    public:
      WiFiSettings() = default;
      ~WiFiSettings() = default;

      WiFiSaveToDiskResult saveToDisk();
      WiFiLoadFromDiskResult loadFromDisk();

      void startFatFSUSB();
      bool isFatFSUSBConnected() const {
        return usbConnected;
      }
      void stopFatFSUSB() {
        FatFSUSB.end();
        Serial1.println("FatFSUSB stopped");
      }

      /**
       * @brief WiFi SSID
       */
      char ssid[MAX_SSID_LENGTH] = "";
      /**
       * @brief WiFi password
       */
      char password[MAX_PASSWORD_LENGTH] = "";
  };
} // Settings

#endif // PICO2W_STOCK_TICKER_WIFISETTINGS_H
