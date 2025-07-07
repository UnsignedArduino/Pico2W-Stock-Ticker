//
// Created by ckyiu on 7/7/2025.
//

#ifndef PICO2W_STOCK_TICKER_MD_MAX72XX_SCROLLING_H
#define PICO2W_STOCK_TICKER_MD_MAX72XX_SCROLLING_H

#include <Arduino.h>
#include <MD_MAX72xx.h>

// Manages continually scrolling a string of text across the display.
class MD_MAX72XX_Scrolling {
  public:
    MD_MAX72XX_Scrolling(MD_MAX72XX* display) {
      this->display = display;
    }
    ~MD_MAX72XX_Scrolling() = default;

    void setText(const char* text) {
      this->strToDisplay = text;
      this->curCol = 0;
      this->textChangedSize();
    }

    const char* getText() const {
      return this->strToDisplay;
    }

    void textChangedSize() {
      this->strWidth = this->getTextWidth();
    }

    void update();

  protected:
    MD_MAX72XX* display = nullptr;
    const char* strToDisplay = nullptr;
    int16_t curCol = 0;
    int16_t strWidth = 0;

    const uint16_t spaceBetweenChars = 1;

    uint16_t getTextWidth();
};

#endif // PICO2W_STOCK_TICKER_MD_MAX72XX_SCROLLING_H
