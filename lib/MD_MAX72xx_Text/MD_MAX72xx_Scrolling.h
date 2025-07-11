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
    /**
     * @brief Constructor for MD_MAX72XX_Print, allowing scrolling text from the
     *  right to the left.
     *
     * This enables easy scrolling at a configurable speed.
     *
     * @param display A pointer to the MD_MAX72XX display object to print to.
     */
    MD_MAX72XX_Scrolling(MD_MAX72XX* display) {
      this->display = display;
    }
    ~MD_MAX72XX_Scrolling() = default;

    /**
     * @brief Set the text to display on the scrolling display. Does not copy,
     *  only maintains a pointer. This allows live updating of the text.
     *
     * @param text The pointer to the text to display, the string can be
     *  modified.
     */
    void setText(const char* text) {
      this->strToDisplay = text;
      this->reset();
    }

    /**
     * @brief Get the text currently being displayed.
     *
     * @return const char* The pointer to the text currently being displayed.
     */
    const char* getText() const {
      return this->strToDisplay;
    }

    void update();

    /**
     * @brief Reset the scrolling display to the initial state. (text to the
     *  right of the screen, about to scroll in)
     */
    void reset() {
      this->curCharIndex = 0;
      this->curCharColOffset = this->display->getColumnCount();
      this->nextShiftTime = 0; // Reset next shift time to 0 so it will shift
                               // immediately on next update
    }

    /**
     * @brief The time in milliseconds between each shift of the text.
     *
     * This is how long to wait before shifting the text to the left by one
     * column. Lower values will make the text scroll faster.
     */
    uint32_t periodBetweenShifts = 30;

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
