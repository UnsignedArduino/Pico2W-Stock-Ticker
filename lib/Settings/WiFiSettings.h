//
// Created by ckyiu on 7/11/2025.
//

#ifndef PICO2W_STOCK_TICKER_WIFISETTINGS_H
#define PICO2W_STOCK_TICKER_WIFISETTINGS_H

#include <Arduino.h>
#include <BaseSettings.h>

namespace Settings {
  const size_t MAX_SSID_LENGTH = 32 + 1; // +1 for null terminator
  const size_t MAX_PASSWORD_LENGTH = 63 + 1;

  enum class WiFiSettingsValidationResult {
    OK = 0,
    ERROR_INVALID_SSID = 1,
    ERROR_INVALID_PASSWORD = 2
  };

  class WiFiSettings : public BaseSettings {
    public:
      WiFiSettings() = default;
      ~WiFiSettings() = default;

      /**
       * @brief Validate a JSON document containing WiFi settings.
       *
       * @return 0 on success, nonzero otherwise.
       */
      uint8_t validateSettings(JsonDocument& doc) override;

      /**
       * @brief WiFi SSID
       */
      char ssid[MAX_SSID_LENGTH] = "";
      /**
       * @brief WiFi password
       */
      char password[MAX_PASSWORD_LENGTH] = "";

    protected:
      void saveValuesToDocument(JsonDocument& doc) override;
      void loadValuesFromDocument(const JsonDocument& doc) override;

      const char* getSettingsName() const override {
        return "WiFi";
      }

      const char* getSettingsFilePath() const override {
        return "wifi_settings.json";
      }
  };
} // Settings

#endif // PICO2W_STOCK_TICKER_WIFISETTINGS_H
