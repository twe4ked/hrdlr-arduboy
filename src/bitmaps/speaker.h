#ifndef SPEAKER_H
#define SPEAKER_H

const uint8_t speakerFrameCount = 2;
const uint8_t speakerFrameWidth = 8;
const uint8_t speakerFrameHeight = 8;
const uint8_t PROGMEM speakerFrames[][8] = {
  /* Frame number 0
         #    
        ##   #
      ## # # #
      # ## # #
      #### # #
        ##   #
         #    
              
  */
  {0x1c,0x14,0x3a,0x7f,0x00,0x1c,0x00,0x3e},
  /* Frame number 1
      #  #    
       # #    
      # #     
      ## #    
      ### #   
        ## #  
         #  # 
              
  */
  {0x1d,0x1a,0x34,0x6b,0x10,0x20,0x40,0x00}
};

#endif
