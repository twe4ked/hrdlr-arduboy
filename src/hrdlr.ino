#include <Arduboy.h>
#include <Math.h>
#include <bitmaps/player.h>

Arduboy arduboy;

const uint8_t playerYDefault = 47;
const uint8_t jumpHeight = 20;
const uint8_t jumpFrame = 30;

uint8_t currentJumpFrame = 0;

struct Player {
  uint8_t X;
  uint8_t Y;
  uint8_t idleAnimationFrame;
};

Player player = {5, playerYDefault, 0};

double jumpCurve(double currentJumpFrame) {
  double n = (currentJumpFrame * (1.0 / jumpFrame));
  return sin((n * 180) * PI / 180) * jumpHeight;
}

void setup() {
  arduboy.beginNoLogo();
}

void loop() {
  if (!(arduboy.nextFrame())) return;

  arduboy.clear();

  // Draw player
  arduboy.drawBitmap(player.X, player.Y, playerFrames[player.idleAnimationFrame], playerFrameWidth, playerFrameHeight, WHITE);

  // Draw floor
  arduboy.drawFastHLine(0, HEIGHT-1, WIDTH-1, WHITE);

  // Jump
  if (arduboy.pressed(B_BUTTON) && currentJumpFrame == 0) {
    currentJumpFrame = jumpFrame; //....
  }

  if (currentJumpFrame > 0) {
    currentJumpFrame--;
    player.Y = playerYDefault - jumpCurve(currentJumpFrame);
  } else {
    player.Y = playerYDefault;
  }

  // Idle animation
  if (arduboy.everyXFrames(8)) {
    player.idleAnimationFrame++;
    player.idleAnimationFrame = player.idleAnimationFrame % 3;
  }

  arduboy.display();
}
