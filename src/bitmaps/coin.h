#ifndef COIN_H
#define COIN_H

const uint8_t coinFrameCount = 2;
const uint8_t coinFrameWidth = 4;
const uint8_t coinFrameHeight = 8;
const uint8_t PROGMEM coinFrames[][4] = {
  /* Frame number 0
       ## 
      #  #
      # ##
      # ##
      # ##
      # ##
       ## 
          
  */
  {0x3e,0x41,0x7d,0x3e},
  /* Frame number 1
       ## 
       ## 
       ## 
       ## 
       ## 
       ## 
       ## 
          
  */
  {0x00,0x7f,0x7f,0x00}
};

#endif
