//
// Created by ckyiu on 7/11/2025.
//

#ifndef PICO2W_STOCK_TICKER_BASESETTINGS_H
#define PICO2W_STOCK_TICKER_BASESETTINGS_H

#ifndef LOG_FREE_MEMORY
  #define LOG_FREE_MEMORY
#endif
#ifndef LOG_JSON_PARSED
  #define LOG_JSON_PARSED
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FatFS.h>
#include <FatFSUSB.h>
#include <StreamUtils.h>

namespace Settings {

  enum class SaveToDiskResult {
    OK,
    ERROR_FATFS_INIT_FAILED,
    ERROR_FILE_OPEN_FAILED
  };

  enum class LoadFromDiskResult {
    OK,
    ERROR_FATFS_INIT_FAILED,
    ERROR_FILE_OPEN_FAILED,
    ERROR_JSON_PARSE_TOO_DEEP,
    ERROR_JSON_PARSE_NO_MEMORY,
    ERROR_JSON_PARSE_INVALID_INPUT,
    ERROR_JSON_PARSE_INCOMPLETE_INPUT,
    ERROR_JSON_PARSE_EMPTY_INPUT,
    ERROR_JSON_PARSE_UNKNOWN_ERROR,
    ERROR_VALIDATION_FAILED
  };

  extern bool usbConnected;

  class BaseSettings {
    public:
      BaseSettings() = default;
      ~BaseSettings() = default;

      SaveToDiskResult saveToDisk();
      LoadFromDiskResult loadFromDisk();

      /**
       * @brief Validate the settings. This method should be implemented by
       *  classes inheriting from BaseSettings to validate their specific
       *  settings.
       *
       * @param doc The JSON document containing the settings to validate.
       * @return 0 if the settings are valid, otherwise return a class-specific
       * enum specifying what's wrong.
       */
      virtual uint8_t validateSettings(JsonDocument& doc) = 0;

      /**
       * @brief Get the last validation result.
       *
       * @return uint8_t The last validation result. 0 means validation passed,
       *  nonzero if validation failed.
       */
      uint8_t getLastValidationResult() {
        return lastValidationResult;
      }

      void fatFSUSBBegin();
      void fatFSUSBEnd();
      /**
       * @brief Check if the Pico is being exposed to the computer as a USB
       *  drive.
       *
       * @return
       */
      bool fatFSUSBConnected() const {
        return usbConnected;
      }

    protected:
      uint8_t lastValidationResult = 0;

      /**
       * @brief Classes inheriting from BaseSettings must implement this method
       *  to save their specific values to the JSON document.
       *
       * @param doc The JSON document to save values to.
       */
      virtual void saveValuesToDocument(JsonDocument& doc) = 0;
      /**
       * @brief Classes inheriting from BaseSettings must implement this method
       *  to load their specific values from the JSON document.
       *
       * @param doc The JSON document to load values from.
       */
      virtual void loadValuesFromDocument(const JsonDocument& doc) = 0;

      /**
       * @brief Get the name of the settings. Ex. for WiFi settings it might
       *  return "WiFi" (which would be used in logs like "Saving WiFi settings
       *  to disk")
       *
       * @return The name of the settings.
       */
      virtual const char* getSettingsName() const = 0;
      /**
       * @brief Get the path of the settings file. Ex. for WiFi settings it
       *  might be "/wifi_settings.json"
       *
       * @return The path of the settings file.
       */
      virtual const char* getSettingsFilePath() const = 0;
  };

} // Settings

#endif // PICO2W_STOCK_TICKER_BASESETTINGS_H
