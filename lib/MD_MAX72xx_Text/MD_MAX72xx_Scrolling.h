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
      this->reset();
    }

    const char* getText() const {
      return this->strToDisplay;
    }

    void update();

    void reset() {
      this->curCharIndex = 0;
      this->curCharColOffset = this->display->getColumnCount();
      this->nextShiftTime = 0; // Reset next shift time to 0 so it will shift
                               // immediately on next update
    }

    uint32_t periodBetweenShifts = 50;

  protected:
    MD_MAX72XX* display = nullptr;
    const char* strToDisplay = nullptr;
    int16_t curCharIndex = 0;
    int16_t curCharColOffset =
      0; // Instead of 0 being the right, we'll define 0 as offset from the left
         // edge of the display

    const uint16_t spaceBetweenChars = 1;

    uint32_t nextShiftTime = 0;

    uint16_t getTextWidth(const char* text);
    uint16_t getTextWidth(char c);
};

#endif // PICO2W_STOCK_TICKER_MD_MAX72XX_SCROLLING_H
