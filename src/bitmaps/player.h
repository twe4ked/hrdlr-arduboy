#ifndef PLAYER_H
#define PLAYER_H

const uint8_t playerFrameCount = 1;
const uint8_t playerFrameWidth = 8;
const uint8_t playerFrameHeight = 16;
const uint8_t PROGMEM playerFrames[][16] = {
  /* Frame number 0
          #   
         ###  
        # # # 
        # #   
       #####  
       ### #  
        ####  
       # ## # 
      ## ## # 
      ## ## ##
      ## ## ##
        ####  
        ## #  
        #   # 
       ##  ## 
       ### ###
  */
  {0x00,0xb0,0x7c,0xf2,0xdf,0x72,0x84,0x00,0x07,0xc7,0xf8,0x9f,0x0f,0xd8,0xe7,0x86}
};

#endif
