//
// Created by ckyiu on 7/7/2025.
//

#include "MD_MAX72xx_Scrolling.h"

void MD_MAX72XX_Scrolling::update() {
  if (this->display == nullptr || this->strToDisplay == nullptr ||
      strlen(this->strToDisplay) == 0) {
    return; // Nothing to display
  }

  this->display->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  this->display->clear();
  int16_t thisCurCol = this->curCol;
  for (size_t i = 0; i < strlen(this->strToDisplay); i++) {
    thisCurCol -= this->display->setChar(thisCurCol, this->strToDisplay[i]) +
                  this->spaceBetweenChars;
  }
  this->display->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  this->curCol += 1; // How much to shift for every scroll
  if (this->curCol >= this->strWidth + this->display->getColumnCount()) {
    this->curCol = 0; // Reset to start if scrolled past end
  }
}

uint16_t MD_MAX72XX_Scrolling::getTextWidth() {
  if (this->strToDisplay == nullptr) {
    return 0;
  }
  uint16_t width = 0;
  const size_t tempBufSize = 16; // Useless buffer, just to get column width
  uint8_t tempBuf[tempBufSize];
  for (size_t i = 0; i < strlen(this->strToDisplay); i++) {
    width +=
      this->display->getChar(this->strToDisplay[i], tempBufSize, tempBuf) +
      this->spaceBetweenChars;
  }
  return width;
}
