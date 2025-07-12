//
// Created by ckyiu on 7/8/2025.
//

#ifndef PICO2W_STOCK_TICKER_STOCKTICKER_H
#define PICO2W_STOCK_TICKER_STOCKTICKER_H

#ifndef LOG_FREE_MEMORY
// #define LOG_FREE_MEMORY
#endif
#ifndef LOG_JSON_PARSED
// #define LOG_JSON_PARSED
#endif
#ifndef BUFFER_JSON_READING
  #define BUFFER_JSON_READING
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <StreamUtils.h>
#include <WiFi.h>

namespace StockTicker {
  const size_t MAX_ID_LEN = 32;
  const size_t MAX_SYMBOLS_STRING_LEN = 256;
  const uint16_t MAX_SYMBOLS = 64;
  const size_t MAX_SYMBOL_DISPLAY_STR_LEN = 64;
  const size_t MAX_DISPLAY_STR_LEN = MAX_SYMBOLS * MAX_SYMBOL_DISPLAY_STR_LEN;

  // clang-format off
  struct SymbolPrice {
    char id[MAX_ID_LEN];
    float price;
    float change;
    float changePercent;
  };
  // clang-format on

  /**
   * @brief Status codes for the StockTicker class.
   */
  enum class StockTickerStatus {
    OK,
    ERROR_NO_WIFI,
    ERROR_INIT_REQUEST_FAILED,
    ERROR_CONNECTION_FAILED,
    ERROR_SEND_HEADER_FAILED,
    ERROR_SEND_PAYLOAD_FAILED,
    ERROR_BAD_JSON_RESPONSE,
    ERROR_BAD_REQUEST,
    ERROR_FORBIDDEN,
    ERROR_TOO_MANY_REQUESTS,
    ERROR_INTERNAL_SERVER_ERROR,
    ERROR_UNKNOWN
  };

  uint16_t stockSymbolsCount(const char* symbolsString);

  /**
   * @brief StockTicker class to fetch and display stock prices from Alpaca
   *  Markets' Market Data API
   */
  class StockTicker {
    public:
      StockTicker() = default;
      ~StockTicker() = default;

      void begin(const char* apiKeyId, const char* apiSecretKey,
                 const char* symbolsString, const char* feed = "iex",
                 uint32_t request = 60 * 1000);
      /**
       * @brief Deinitialize.
       */
      void end() {
        // Nothing to do here, no dynamic memory allocation
      };

      void update();

      /**
       * @brief Get a pointer to the string to display.
       *
       * @return const char*
       */
      const char* getDisplayStr() const {
        return this->displayStr;
      }

      /**
       * @brief Get the current status of the StockTicker.
       *
       * @return StockTickerStatus
       */
      StockTickerStatus getStatus() const {
        return this->status;
      }

      /**
       * @brief Signal an immediate refresh of the stock prices on the next
       *  StockTicker::StockTicker.update();
       */
      void refreshOnNextUpdate() {
        this->nextRequestTime = 0; // Force immediate refresh
      }

    protected:
      const char* apcaApiKeyId;
      const char* apcaApiSecretKey;

      const char* symbols;
      SymbolPrice allSymbolPrices[MAX_SYMBOLS];
      uint16_t symbolCount = 0;

      void updateSymbolPriceInMemory(const char* id, float price, float change,
                                     float changePercent);

      const char* sourceFeed;
      uint32_t requestPeriod;
      uint32_t nextRequestTime = 0;

      StockTickerStatus status = StockTickerStatus::OK;

      char displayStr[MAX_DISPLAY_STR_LEN];

      void updateDisplayStr();
  };
} // StockTicker

#endif // PICO2W_STOCK_TICKER_STOCKTICKER_H
