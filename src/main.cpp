#include "config.h"
#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <MD_MAX72xx_Text.h>
#include <SPI.h>
#include <StockTicker.h>
#include <WiFi.h>

StockTicker::StockTicker stockTicker;

const MD_MAX72XX::moduleType_t HARDWARE_TYPE = MD_MAX72XX::FC16_HW;

#define USE_HARDWARE_SPI
const uint8_t CLK_PIN = 2;
const uint8_t DATA_PIN = 3;
const uint8_t CS_PIN = 5;

#ifdef USE_HARDWARE_SPI
// Not using Parola for manual control
MD_MAX72XX display =
  MD_MAX72XX(HARDWARE_TYPE, SPI, CS_PIN, matrixModulesCount * 4);
#else
MD_MAX72XX display =
  MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, matrixModulesCount * 4);
#endif

MD_MAX72XX_Print textDisplay(&display);
MD_MAX72XX_Scrolling scrollingDisplay(&display);

void setup() {
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  Serial1.println("\n");

  Serial1.println(symbols);
  stockTicker.begin(apcaApiKeyId, apcaApiSecretKey, symbols, sourceFeed,
                    requestPeriod);

#ifdef USE_HARDWARE_SPI
  SPI.setSCK(CLK_PIN);
  SPI.setTX(DATA_PIN);
  SPI.setCS(CS_PIN);
  SPI.begin();
#endif
  display.begin();
  display.clear();
  display.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  display.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);

  scrollingDisplay.setText(stockTicker.getDisplayStr());
  scrollingDisplay.periodBetweenShifts = scrollPeriodSpeed;
}

void loop() {
  static StockTicker::StockTickerStatus lastStatus =
    StockTicker::StockTickerStatus::OK;

  if (WiFi.status() == WL_CONNECTED) {
    stockTicker.update();
    if (stockTicker.getStatus() != lastStatus) {
      lastStatus = stockTicker.getStatus();
      const char* msg = stockTickerStatusToMessage(lastStatus);
      Serial1.printf("Stock ticker status changed: %s\n", msg);
      if (lastStatus == StockTicker::StockTickerStatus::OK) {
        scrollingDisplay.setText(stockTicker.getDisplayStr());
      } else {
        scrollingDisplay.setText(msg);
      }
    }
    scrollingDisplay.update();
  } else {
    Serial1.println("Connecting to WiFi...");
    textDisplay.print("Connecting to WiFi...");
    display.update();
    WiFi.begin(ssid, password);
    delay(1000);

    Serial1.println("Connected to WiFi");
    textDisplay.print("\nConnected to WiFi!");
    display.update();
    Serial1.print("IP Address: ");
    Serial1.println(WiFi.localIP());
    delay(1000);
    scrollingDisplay.reset();
    stockTicker.refreshOnNextUpdate();
  }
}
