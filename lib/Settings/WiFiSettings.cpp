//
// Created by ckyiu on 7/11/2025.
//

#include <WiFiSettings.h>

namespace Settings {
  bool usbConnected = false;

  const char* wiFiSaveToDiskResultToMessage(WiFiSaveToDiskResult r) {
    switch (r) {
      case WiFiSaveToDiskResult::OK: {
        return "OK";
      }
      case WiFiSaveToDiskResult::ERROR_FATFS_INIT_FAILED: {
        return "FatFS initialization failed";
      }
      case WiFiSaveToDiskResult::ERROR_FILE_OPEN_FAILED: {
        return "Failed to open file for writing";
      }
      default: {
        return "Unknown error";
      }
    }
  }

  const char* wiFiLoadFromDiskResultToMessage(WiFiLoadFromDiskResult r) {
    switch (r) {
      case WiFiLoadFromDiskResult::OK: {
        return "OK";
      }
      case WiFiLoadFromDiskResult::ERROR_FATFS_INIT_FAILED: {
        return "FatFS initialization failed";
      }
      case WiFiLoadFromDiskResult::ERROR_FILE_OPEN_FAILED: {
        return "Failed to open file for reading";
      }
      case WiFiLoadFromDiskResult::ERROR_JSON_PARSE_TOO_DEEP: {
        return "JSON parsing failed due to too deep structure - please modify "
               "wifi_settings.json and eject USB drive to retry.";
      }
      case WiFiLoadFromDiskResult::ERROR_JSON_PARSE_NO_MEMORY: {
        return "JSON parsing failed due to insufficient memory - please check "
               "wifi_settings.json";
      }
      case WiFiLoadFromDiskResult::ERROR_JSON_PARSE_INVALID_INPUT: {
        return "JSON parsing failed due to invalid input - please check "
               "wifi_settings.json";
      }
      case WiFiLoadFromDiskResult::ERROR_JSON_PARSE_INCOMPLETE_INPUT: {
        return "JSON parsing failed due to incomplete input - please check "
               "wifi_settings.json";
      }
      case WiFiLoadFromDiskResult::ERROR_JSON_PARSE_EMPTY_INPUT: {
        return "JSON parsing failed due to empty input - please check "
               "wifi_settings.json";
      }
      case WiFiLoadFromDiskResult::ERROR_JSON_PARSE_UNKNOWN_ERROR: {
        return "JSON parsing failed due to unknown error - please check "
               "wifi_settings.json";
      }
      case WiFiLoadFromDiskResult::ERROR_INVALID_SSID: {
        return "Invalid SSID length - please check \"ssid\" key in "
               "wifi_settings.json";
      }
      case WiFiLoadFromDiskResult::ERROR_INVALID_PASSWORD: {
        return "Invalid password length - please check \"password\" key in "
               "wifi_settings.json";
      }
      default: {
        return "Unknown error";
      }
    }
  }

  WiFiSaveToDiskResult WiFiSettings::saveToDisk() {
    Serial1.println("Saving WiFi settings to disk");

#ifdef LOG_FREE_MEMORY
    Serial1.printf("Free memory before save to disk: heap %d kb, stack %d kb\n",
                   rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    { // Scope for JsonDocument
      JsonDocument doc;
      doc["ssid"] = this->ssid;
      doc["password"] = this->password;
      doc.shrinkToFit();

#ifdef LOG_FREE_MEMORY
      Serial1.printf(
        "Free memory after building document: heap %d kb, stack %d kb\n",
        rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif

      { // Scope for file operations
        Serial1.println("Starting FatFS and opening file");
        if (!FatFS.begin()) {
          Serial1.println("Failed to init FatFS");
          return WiFiSaveToDiskResult::ERROR_FATFS_INIT_FAILED;
        }
        File file = FatFS.open("wifi_settings.json", "w");
        if (!file) {
          Serial1.println("Failed to open WiFi settings file for writing");
          return WiFiSaveToDiskResult::ERROR_FILE_OPEN_FAILED;
        }

        Serial1.println("Serializing JSON to file");
#ifdef LOG_JSON_PARSED
        Serial1.println("JSON:");
        WriteLoggingStream loggingStream(file, Serial1);
        serializeJsonPretty(doc, loggingStream);
        Serial1.println("");
#else
        serializeJsonPretty(doc, file);
#endif

#ifdef LOG_FREE_MEMORY
        Serial1.printf(
          "Free memory after serialization: heap %d kb, stack %d kb\n",
          rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif

        Serial1.println("Closing file and stopping FatFS");
        file.close();
        FatFS.end();
      }
    }
#ifdef LOG_FREE_MEMORY
    Serial1.printf("Free memory after save to disk: heap %d kb, stack %d kb\n",
                   rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    Serial1.println("WiFi settings saved to disk successfully");
    return WiFiSaveToDiskResult::OK;
  }

  WiFiLoadFromDiskResult WiFiSettings::loadFromDisk() {
    Serial1.println("Loading WiFi settings from disk");

#ifdef LOG_FREE_MEMORY
    Serial1.printf(
      "Free memory before load from disk: heap %d kb, stack %d kb\n",
      rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    {
      Serial1.println("Starting FatFS and opening file");
      if (!FatFS.begin()) {
        Serial1.println("Failed to init FatFS");
        return WiFiLoadFromDiskResult::ERROR_FATFS_INIT_FAILED;
      }
      File file = FatFS.open("wifi_settings.json", "r");
      if (!file) {
        Serial1.println("Failed to open WiFi settings file for reading");
        return WiFiLoadFromDiskResult::ERROR_FILE_OPEN_FAILED;
      }

      {
        Serial1.println("Deserializing JSON from file");
        JsonDocument doc;
#ifdef LOG_JSON_PARSED
        Serial1.println("JSON:");
        ReadLoggingStream loggingStream(file, Serial1);
        DeserializationError error = deserializeJson(doc, loggingStream);
        Serial1.println("");
#else
        DeserializationError error = deserializeJson(doc, file);
#endif
        Serial1.println("Closing file and stopping FatFS");
        file.close();
        FatFS.end();

        if (error) {
          Serial1.printf("Failed to deserialize JSON: %s\n", error.c_str());
          switch (error.code()) {
            case DeserializationError::TooDeep: {
              Serial1.println("JSON is too deep, please check the file");
              return WiFiLoadFromDiskResult::ERROR_JSON_PARSE_TOO_DEEP;
            }
            case DeserializationError::NoMemory: {
              Serial1.println("JSON parsing failed due to insufficient memory");
              return WiFiLoadFromDiskResult::ERROR_JSON_PARSE_NO_MEMORY;
            }
            case DeserializationError::InvalidInput: {
              Serial1.println("JSON parsing failed due to invalid input");
              return WiFiLoadFromDiskResult::ERROR_JSON_PARSE_INVALID_INPUT;
            }
            case DeserializationError::IncompleteInput: {
              Serial1.println("JSON parsing failed due to incomplete input");
              return WiFiLoadFromDiskResult::ERROR_JSON_PARSE_INCOMPLETE_INPUT;
            }
            case DeserializationError::EmptyInput: {
              Serial1.println("JSON parsing failed due to empty input");
              return WiFiLoadFromDiskResult::ERROR_JSON_PARSE_EMPTY_INPUT;
            }
            case DeserializationError::Ok:
            default: {
              Serial1.println("Unknown error");
              return WiFiLoadFromDiskResult::ERROR_JSON_PARSE_UNKNOWN_ERROR;
            }
          }
        }

#ifdef LOG_FREE_MEMORY
        Serial1.printf(
          "Free memory after deserialization: heap %d kb, stack %d kb\n",
          rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif

        const char* parsedSSID = doc["ssid"];
        const char* parsedPassword = doc["password"];

        if (strlen(parsedSSID) >= MAX_SSID_LENGTH || strlen(parsedSSID) == 0) {
          Serial1.println("Invalid SSID length");
          return WiFiLoadFromDiskResult::ERROR_INVALID_SSID;
        }
        if (strlen(parsedPassword) >= MAX_PASSWORD_LENGTH) {
          Serial1.println("Invalid password length");
          return WiFiLoadFromDiskResult::ERROR_INVALID_PASSWORD;
        }

        strncpy(this->ssid, parsedSSID, MAX_SSID_LENGTH);
        strncpy(this->password, parsedPassword, MAX_PASSWORD_LENGTH);
        Serial1.printf("Loaded WiFi settings: SSID='%s', Password='%s'\n",
                       this->ssid, this->password);
      }
    }
#ifdef LOG_FREE_MEMORY
    Serial1.printf(
      "Free memory after load from disk: heap %d kb, stack %d kb\n",
      rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    Serial1.println("WiFi settings loaded from disk successfully");
    return WiFiLoadFromDiskResult::OK;
  }

  void WiFiSettings::startFatFSUSB() {
    Serial1.println("Exposing FatFS to USB");
    FatFSUSB.onUnplug([](uint32_t i) {
      usbConnected = false;
      Serial1.println("USB unplugged");
    });
    FatFSUSB.onPlug([](uint32_t i) {
      usbConnected = true;
      Serial1.println("USB plugged in");
    });
    FatFSUSB.driveReady([](uint32_t i) { return true; });
    FatFSUSB.begin();
    usbConnected = true;
    Serial1.println("FatFSUSB started");
  }
} // Settings
