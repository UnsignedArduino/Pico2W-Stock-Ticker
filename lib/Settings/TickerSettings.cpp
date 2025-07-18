//
// Created by ckyiu on 7/11/2025.
//

#include <TickerSettings.h>

namespace Settings {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
  uint8_t TickerSettings::validateSettings(JsonDocument& doc) {
    const char* parsedApcaApiKeyId = doc["apcaApiKeyId"];
    const char* parsedApcaApiSecretKey = doc["apcaApiSecretKey"];
    const char* parsedSymbols = doc["symbols"];
    const char* parsedSourceFeed = doc["sourceFeed"];
    const uint16_t parsedScrollPeriod =
      doc["scrollPeriod"] | 30; // Default to 30 ms
    const uint32_t parsedRequestPeriod =
      doc["requestPeriod"] | 60; // Default to 60 seconds
    const uint8_t parsedDisplayBrightness =
      doc["displayBrightness"] | 7; // Default to 7
    if (strlen(parsedApcaApiKeyId) == 0 ||
        strlen(parsedApcaApiKeyId) >= APCA_API_KEY_ID_MAX_LEN) {
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_APCA_API_KEY_ID);
    }
    if (strlen(parsedApcaApiSecretKey) == 0 ||
        strlen(parsedApcaApiSecretKey) >= APCA_API_SECRET_KEY_MAX_LEN) {
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_APCA_API_SECRET_KEY);
    }
    if (strlen(parsedSymbols) == 0 ||
        strlen(parsedSymbols) >= SYMBOLS_STRING_MAX_LEN) {
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_SYMBOLS);
    }
    const uint16_t symbolsCount = StockTicker::stockSymbolsCount(parsedSymbols);
    if (symbolsCount > MAX_SYMBOLS_COUNT || symbolsCount == 0) {
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_SYMBOLS);
    }
    if (strcmp(parsedSourceFeed, "sip") != 0 &&
        strcmp(parsedSourceFeed, "iex") != 0 &&
        strcmp(parsedSourceFeed, "delayed_sip") != 0 &&
        strcmp(parsedSourceFeed, "boats") != 0 &&
        strcmp(parsedSourceFeed, "overnight") != 0 &&
        strcmp(parsedSourceFeed, "otc") != 0) {
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_SOURCE_FEED);
    }
    if (parsedScrollPeriod <
        1) { // Must be a natural number and already uint16_t
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_SCROLL_PERIOD);
    }
    if (parsedRequestPeriod <
        1) { // Must be a natural number and already uint32_t
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_REQUEST_PERIOD);
    }
    if (parsedDisplayBrightness < 1 ||
        parsedDisplayBrightness > 15) { // Must be a natural number between 1
                                        // and 15 inclusive and already uint8_t
      return static_cast<uint8_t>(
        TickerSettingsValidationResult::ERROR_INVALID_DISPLAY_BRIGHTNESS);
    }
    return static_cast<uint8_t>(TickerSettingsValidationResult::OK);
  }
#pragma clang diagnostic pop

  void TickerSettings::saveValuesToDocument(JsonDocument& doc) {
    doc["apcaApiKeyId"] = this->apcaApiKeyId;
    doc["apcaApiSecretKey"] = this->apcaApiSecretKey;
    doc["symbols"] = this->symbols;
    doc["sourceFeed"] = this->sourceFeed;
    doc["requestPeriod"] = this->requestPeriod;
    doc["scrollPeriod"] = this->scrollPeriod;
    doc["displayBrightness"] = this->displayBrightness;
  }

  void TickerSettings::loadValuesFromDocument(const JsonDocument& doc) {
    strncpy(this->apcaApiKeyId, doc["apcaApiKeyId"].as<const char*>(),
            APCA_API_KEY_ID_MAX_LEN);
    strncpy(this->apcaApiSecretKey, doc["apcaApiSecretKey"].as<const char*>(),
            APCA_API_SECRET_KEY_MAX_LEN);
    strncpy(this->symbols, doc["symbols"].as<const char*>(),
            SYMBOLS_STRING_MAX_LEN);
    strncpy(this->sourceFeed, doc["sourceFeed"].as<const char*>(),
            SOURCE_FEED_MAX_LEN);
    this->requestPeriod = doc["requestPeriod"] | 60; // Default to 60 seconds
    this->scrollPeriod = doc["scrollPeriod"] | 30;   // Default to 30 ms
    this->displayBrightness = doc["displayBrightness"] | 7; // Default to 7
  }
} // Settings
