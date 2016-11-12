#ifndef HURDLE_H
#define HURDLE_H

const uint8_t hurdleFrameCount = 1;
const uint8_t hurdleFrameWidth = 4;
const uint8_t hurdleFrameHeight = 8;
const uint8_t PROGMEM hurdleFrames[][4] = {
  /* Frame number 0
       ## 
      #  #
      ## #
      # ##
      ## #
      # ##
      #  #
      #  #
  */
  {0xfe,0x15,0x29,0xfe}
};

#endif
