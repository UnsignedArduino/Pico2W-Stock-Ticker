//
// Created by ckyiu on 7/11/2025.
//

#ifndef PICO2W_STOCK_TICKER_TICKERSETTINGS_H
#define PICO2W_STOCK_TICKER_TICKERSETTINGS_H

#include <Arduino.h>
#include <BaseSettings.h>
#include <StockTicker.h>

namespace Settings {
  const size_t APCA_API_KEY_ID_MAX_LEN = 32;
  const size_t APCA_API_SECRET_KEY_MAX_LEN = 64;
  const size_t SYMBOLS_STRING_MAX_LEN = 256;
  const uint16_t MAX_SYMBOLS_COUNT = 32;
  const size_t SOURCE_FEED_MAX_LEN = 16;

  enum class TickerSettingsValidationResult {
    OK = 0,
    ERROR_INVALID_APCA_API_KEY_ID = 1,
    ERROR_INVALID_APCA_API_SECRET_KEY = 2,
    ERROR_INVALID_SYMBOLS = 3,
    ERROR_INVALID_SOURCE_FEED = 4
  };

  class TickerSettings : public BaseSettings {
    public:
      TickerSettings() = default;
      ~TickerSettings() = default;

      /**
       * @brief Validate a JSON document containing Stock Ticker settings.
       *
       * @return 0 on success, nonzero otherwise.
       */
      uint8_t validateSettings(JsonDocument& doc) override;

      /**
       * @brief Alpaca Markets API key ID.
       */
      char apcaApiKeyId[APCA_API_KEY_ID_MAX_LEN] = "";
      /**
       * @brief Alpaca Markets API secret key.
       */
      char apcaApiSecretKey[APCA_API_SECRET_KEY_MAX_LEN] = "";
      /**
       * @brief Comma-separated list of symbols to subscribe to.
       */
      char symbols[SYMBOLS_STRING_MAX_LEN] = "";
      /**
       * @brief Feed to subscribe to, either "sip", "iex", "delayed_sip",
       * "boats", "overnight", or "otc".
       */
      char sourceFeed[SOURCE_FEED_MAX_LEN] = "iex";

    protected:
      void saveValuesToDocument(JsonDocument& doc) override;
      void loadValuesFromDocument(const JsonDocument& doc) override;

      const char* getSettingsName() const override {
        return "Ticker";
      }

      const char* getSettingsFilePath() const override {
        return "ticker_settings.json";
      }
  };
} // Settings

#endif // PICO2W_STOCK_TICKER_TICKERSETTINGS_H
