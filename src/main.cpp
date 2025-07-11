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

void startWiFiConfigOverUSBAndReboot(const char* msg) {
  Serial1.println("Exposing FatFSUSB for WiFi settings editing");
  wifiSettings.startFatFSUSB();
  Serial1.println("USB connected, waiting for eject...");
  scrollingDisplay.setText(msg, true);
  while (wifiSettings.isFatFSUSBConnected()) {
    scrollingDisplay.update();
    if (configBtn.released()) {
      Serial1.println("Config button released, stopping FatFSUSB");
      break;
    }
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch (r) {
      case Settings::WiFiLoadFromDiskResult::ERROR_FATFS_INIT_FAILED:
        startWiFiConfigOverUSBAndReboot(
          "Failed to initialize filesystem, eject USB drive to try again.");

      case Settings::WiFiLoadFromDiskResult::ERROR_FILE_OPEN_FAILED:
        startWiFiConfigOverUSBAndReboot(
          "Modify wifi_settings.json on USB drive and eject to finish.");

      case Settings::WiFiLoadFromDiskResult::ERROR_JSON_PARSE_TOO_DEEP:
        startWiFiConfigOverUSBAndReboot(
          "JSON too deep, modify wifi_settings.json on USB drive and eject to "
          "finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_JSON_PARSE_NO_MEMORY:
        startWiFiConfigOverUSBAndReboot(
          "JSON parsing failed due to insufficient memory, modify "
          "wifi_settings.json on USB drive and eject to finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_JSON_PARSE_INVALID_INPUT:
        startWiFiConfigOverUSBAndReboot(
          "JSON parsing failed due to invalid input, modify wifi_settings.json "
          "on USB drive and eject to finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_JSON_PARSE_INCOMPLETE_INPUT:
        startWiFiConfigOverUSBAndReboot(
          "JSON parsing failed due to incomplete input, modify "
          "wifi_settings.json on USB drive and eject to finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_JSON_PARSE_EMPTY_INPUT:
        startWiFiConfigOverUSBAndReboot(
          "JSON parsing failed due to empty input, modify wifi_settings.json "
          "on USB drive and eject to finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_JSON_PARSE_UNKNOWN_ERROR:
        startWiFiConfigOverUSBAndReboot(
          "JSON parsing failed due to unknown error, modify wifi_settings.json "
          "on USB drive and eject to finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_INVALID_SSID:
        startWiFiConfigOverUSBAndReboot(
          "Invalid SSID length, modify \"ssid\" key in wifi_settings.json on "
          "USB drive and eject to finish.");
      case Settings::WiFiLoadFromDiskResult::ERROR_INVALID_PASSWORD:
        startWiFiConfigOverUSBAndReboot(
          "Invalid password length, modify \"password\" key in "
          "wifi_settings.json on USB drive and eject to finish.");
    }
#pragma clang diagnostic pop
  }
  // If configuration buton pressed, start WiFi configuration over USB
  if (configBtn.pressed()) {
    Serial1.println("Config button pressed");
    startWiFiConfigOverUSBAndReboot(
      "Configuration button pressed, modify wifi_settings.json on USB drive "
      "and eject to finish.");
  }

  Serial1.println(symbols);
  stockTicker.begin(apcaApiKeyId, apcaApiSecretKey, symbols, sourceFeed,
                    requestPeriod);

  scrollingDisplay.setText(stockTicker.getDisplayStr());
  scrollingDisplay.periodBetweenShifts = scrollPeriodSpeed;
}

void loop() {
  static StockTicker::StockTickerStatus lastStatus =
    StockTicker::StockTickerStatus::OK;

  // If configuration button pressed, start WiFi configuration over USB
  if (configBtn.pressed()) {
    Serial1.println("Config button pressed");
    Serial1.println("Stopping WiFi and rebooting");
    WiFi.end();
    rp2040.reboot();
  }
  if (WiFi.status() == WL_CONNECTED) {
    stockTicker.update();
    if (stockTicker.getStatus() != lastStatus) {
      lastStatus = stockTicker.getStatus();
      const char* msg = stockTickerStatusToMessage(lastStatus);
      Serial1.printf("Stock ticker status changed: %s\n", msg);
    }
    scrollingDisplay.update();
  } else {
    Serial1.println("Connecting to WiFi...");
    textDisplay.print("Connecting to WiFi...");
    display.update();
    WiFi.begin(wifiSettings.ssid, wifiSettings.password);
    delay(1000);
    // If WiFi connection fails, start WiFi configuration over USB
    if (WiFi.status() != WL_CONNECTED) {
      Serial1.println("WiFi connection failed");
      startWiFiConfigOverUSBAndReboot(
        "WiFi connection failed, modify \"ssid\" and \"password\" in "
        "wifi_settings.json on USB drive and eject to finish.");
    }
    Serial1.println("Connected to WiFi");
    textDisplay.print("\nConnected to WiFi");
    display.update();
    Serial1.print("IP Address: ");
    Serial1.println(WiFi.localIP());
    delay(1000);
    scrollingDisplay.reset();
    stockTicker.refreshOnNextUpdate();
  }
}
