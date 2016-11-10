#include "Arduboy.h"

Arduboy arduboy;

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(15); // default 60
}

const uint8_t playerYDefault = 55;
const uint8_t jumpCurve[] = {0, 0, 3, 6, 9, 9, 9, 6, 3, 0};

uint8_t jumpFrame = 0;

struct Player {
  uint8_t X;
  uint8_t Y;
};

Player player = {5, playerYDefault};

void loop() {
  if (!(arduboy.nextFrame())) return;

  arduboy.clear();

  // Draw player
  arduboy.fillRect(player.X, player.Y, 4, 8, WHITE);

  // Draw floor
  arduboy.drawFastHLine(0, HEIGHT-1, WIDTH-1, WHITE);

  // Jump
  if (arduboy.pressed(B_BUTTON) && jumpFrame == 0) {
    jumpFrame = 9;
  }

  if (jumpFrame > 0) {
    jumpFrame--;
    player.Y = playerYDefault - jumpCurve[jumpFrame];
  } else {
    player.Y = playerYDefault;
  }

  arduboy.display();
}
