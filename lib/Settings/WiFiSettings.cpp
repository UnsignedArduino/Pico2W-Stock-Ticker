//
// Created by ckyiu on 7/11/2025.
//

#include <WiFiSettings.h>

namespace Settings {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
  uint8_t WiFiSettings::validateSettings(JsonDocument& doc) {
    const char* parsedSSID = doc["ssid"];
    const char* parsedPassword = doc["password"];
    if (strlen(parsedSSID) == 0 || strlen(parsedSSID) >= MAX_SSID_LENGTH) {
      return static_cast<uint8_t>(
        WiFiSettingsValidationResult::ERROR_INVALID_SSID);
    }
    if (strlen(parsedPassword) >= MAX_PASSWORD_LENGTH) {
      return static_cast<uint8_t>(
        WiFiSettingsValidationResult::ERROR_INVALID_PASSWORD);
    }
    return static_cast<uint8_t>(WiFiSettingsValidationResult::OK);
  }
#pragma clang diagnostic pop

  void WiFiSettings::saveValuesToDocument(JsonDocument& doc) {
    doc["ssid"] = this->ssid;
    doc["password"] = this->password;
  }

  void WiFiSettings::loadValuesFromDocument(const JsonDocument& doc) {
    strncpy(this->ssid, doc["ssid"].as<const char*>(), MAX_SSID_LENGTH);
    strncpy(this->password, doc["password"].as<const char*>(),
            MAX_PASSWORD_LENGTH);
  }
} // Settings
