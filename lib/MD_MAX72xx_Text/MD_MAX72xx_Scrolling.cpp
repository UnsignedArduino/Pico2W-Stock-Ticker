//
// Created by ckyiu on 7/7/2025.
//

#include "MD_MAX72xx_Scrolling.h"

void MD_MAX72XX_Scrolling::update() {
  if (this->display == nullptr || this->strToDisplay == nullptr ||
      strlen(this->strToDisplay) == 0) {
    return; // Nothing to display
  }

  if (this->nextShiftTime > millis()) {
    return; // Not time to shift yet
  }
  this->nextShiftTime = millis() + this->periodBetweenShifts;
  this->display->update(); // Update now, which will be more precise than
                           // waiting till after we do all the computation

  const size_t strToDisplayLen = strlen(this->strToDisplay);
  const uint16_t colCount = this->display->getColumnCount();
  this->display->clear();
  int16_t thisCurCol = this->curCharColOffset;
  for (size_t i = this->curCharIndex; // Start from the current character index
       i < strToDisplayLen && // Keep going as long as we have characters to
       thisCurCol < colCount; // display or columns left to fill
       i++) {
    thisCurCol +=
      this->display->setChar(colCount - thisCurCol, this->strToDisplay[i]) +
      this->spaceBetweenChars;
  }
  // This column offset is from the left instead of from the right
  // So to move text left, we subtract
  this->curCharColOffset -= 1;
  // If the current character is scrolled completely past the left edge of the
  // display, then focus on the next character and set it's offset to 0
  const char curChar = this->strToDisplay[this->curCharIndex];
  if (this->curCharColOffset <= -this->getTextWidth(curChar)) {
    this->curCharColOffset = 0;
    this->curCharIndex++;
  }
  if (this->curCharIndex >= strToDisplayLen) {
    this->reset();
  }
}

uint16_t MD_MAX72XX_Scrolling::getTextWidth(const char* text) {
  uint16_t width = 0;
  const size_t tempBufSize = 16; // Useless buffer, just to get column width
  uint8_t tempBuf[tempBufSize];
  for (size_t i = 0; i < strlen(text); i++) {
    width += this->display->getChar(text[i], tempBufSize, tempBuf) +
             this->spaceBetweenChars;
  }
  return width;
}

uint16_t MD_MAX72XX_Scrolling::getTextWidth(char c) {
  const size_t tempBufSize = 16; // Useless buffer, just to get column width
  uint8_t tempBuf[tempBufSize];
  return this->display->getChar(c, tempBufSize, tempBuf) +
         this->spaceBetweenChars;
}
