//
// Created by ckyiu on 7/7/2025.
//

#include "MD_MAX72xx_Print.h"

size_t MD_MAX72XX_Print::write(uint8_t c) {
  if (this->display == nullptr) {
    return 0;
  }
  if (c == '\r') { // Return to start of line, but do not clear
    this->carriageReturn();
  } else if (c == '\n') { // Return to start of line and clear
    this->newline();      // Automatic carriage return;
  } else {
    this->curCol -= this->display->setChar(curCol, c) + 1;
  }
  return 1;
}
