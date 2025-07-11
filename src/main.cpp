#include "config.h"
#include "pins.h"
#include <Arduino.h>
#include <Button.h>
#include <MD_MAX72xx.h>
#include <MD_MAX72xx_Text.h>
#include <SPI.h>
#include <StockTicker.h>
#include <WiFi.h>
#include <WiFiSettings.h>

Button configBtn(CONFIG_BTN_PIN);

Settings::WiFiSettings wifiSettings;
StockTicker::StockTicker stockTicker;

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

void startWiFiConfigurationOverUSB() {
  Serial1.println("Exposing FatFSUSB for WiFi settings editing");
  wifiSettings.startFatFSUSB();
  Serial1.println("USB connected, waiting for eject...");
  while (wifiSettings.isFatFSUSBConnected()) {
    yield();
  }
  wifiSettings.stopFatFSUSB();
  Serial1.println("Rebooting to try loading settings again");
  rp2040.reboot();
}

void setup() {
  Serial1.begin(115200);
  Serial1.println("\n");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  configBtn.begin();

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

  const Settings::WiFiLoadFromDiskResult r = wifiSettings.loadFromDisk();
  // If fail to load settings, start WiFi configuration over USB
  if (r != Settings::WiFiLoadFromDiskResult::OK) {
    Serial1.printf("Failed to load settings from disk: %s\n",
                   Settings::wiFiLoadFromDiskResultToMessage(r));
    if (r == Settings::WiFiLoadFromDiskResult::ERROR_FILE_OPEN_FAILED) {
      // Write default settings because file not found
      wifiSettings.saveToDisk();
    }
    startWiFiConfigurationOverUSB();
  }
  // If configuration buton pressed, start WiFi configuration over USB
  if (configBtn.pressed()) {
    Serial1.println("Config button pressed");
    startWiFiConfigurationOverUSB();
  }
  // If WiFi fail to connect, start WiFi configuration over USB
  Serial1.println("Connecting to WiFi...");
  WiFi.begin(wifiSettings.ssid, wifiSettings.password);
  if (WiFi.status() != WL_CONNECTED) {
    Serial1.println("WiFi connection failed");
    startWiFiConfigurationOverUSB();
  }
  delay(1000);
  Serial1.println("Connected to WiFi");
  Serial1.print("IP Address: ");
  Serial1.println(WiFi.localIP());
  // Connected!

  Serial1.println(symbols);
  stockTicker.begin(apcaApiKeyId, apcaApiSecretKey, symbols, sourceFeed,
                    requestPeriod);

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
    Serial1.println("Reconnecting to WiFi...");
    WiFi.begin(wifiSettings.ssid, wifiSettings.password);
    delay(1000);
    Serial1.println("Connected to WiFi");
    Serial1.print("IP Address: ");
    Serial1.println(WiFi.localIP());
    delay(1000);
    scrollingDisplay.reset();
    stockTicker.refreshOnNextUpdate();
  }
  if (configBtn.pressed()) {
    Serial1.println("Config button pressed, rebooting");
    Serial1.println("Stopping WiFi");
    WiFi.end();
    rp2040.reboot();
  }
}
