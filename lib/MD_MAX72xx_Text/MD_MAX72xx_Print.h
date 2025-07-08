//
// Created by ckyiu on 7/7/2025.
//

#ifndef PICO2W_STOCK_TICKER_MD_MAX72XX_PRINT_H
#define PICO2W_STOCK_TICKER_MD_MAX72XX_PRINT_H

#include <Arduino.h>
#include <MD_MAX72xx.h>

// This class extends the Print class to allow printing text to an MD_MAX72XX
// display.
class MD_MAX72XX_Print : public Print {
  public:
    /**
     * @brief Constructor for MD_MAX72XX_Print, allowing "print"ing and other
     *  common functions to be used with MD_MAX72XX displays.
     *
     * This enables using Arduino print functions like print, println, printf on
     * supported builds to make printing text easy. \r sets the current column
     * to the left most column. \n will clear the display. (and also do a
     * carriage return as well)
     *
     * @param display A pointer to the MD_MAX72XX display object to print to.
     */
    MD_MAX72XX_Print(MD_MAX72XX* display) {
      this->display = display;
      this->carriageReturn();
    }
    ~MD_MAX72XX_Print() = default;

    size_t write(uint8_t c) override;

  protected:
    MD_MAX72XX* display = nullptr;
    uint16_t curCol = 0;

    void carriageReturn() {
      this->curCol = this->display->getColumnCount() - 1;
    }

    void newline() {
      this->carriageReturn();
      this->display->clear();
    }
};

#endif // PICO2W_STOCK_TICKER_MD_MAX72XX_PRINT_H
