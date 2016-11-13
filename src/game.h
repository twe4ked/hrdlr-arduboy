#include <globals.h>

const uint8_t playerXDefault = 5;
const uint8_t playerYDefault = HEIGHT - playerFrameHeight - 1;
const uint8_t jumpHeight = 20;
const uint8_t jumpFrame = 30;
const uint8_t maxHurdles = 5;
const uint8_t hurdleY = HEIGHT - hurdleFrameHeight - 1;

uint8_t currentJumpFrame = 0;
uint8_t introFrameCount = FRAMERATE * 2;
int16_t hurdles[maxHurdles];
uint16_t score = 0;

struct Player {
  uint8_t X;
  uint8_t Y;
  uint8_t idleAnimationFrame;
  uint8_t runningAnimationFrame;
};

Player player = {playerXDefault, playerYDefault, 0, 0};

double jumpCurve(double currentJumpFrame) {
  double n = (currentJumpFrame * (1.0 / jumpFrame));
  return sin((n * 180) * PI / 180) * jumpHeight;
}

uint8_t randInRange(int minN, int maxN) {
  return (rand() % (maxN - minN)) + minN;
}

bool collision(
  int x1, int y1, int height1, int width1,
  int x2, int y2, int height2, int width2
) {
  return (
    x1 < x2 + width2-1 &&
    x1 + width1-1 > x2 &&
    y1 < y2 + height2-1 &&
    height1-1 + y1 > y2
  );
}

void handleInput() {
  // Jump
  if (arduboy.pressed(B_BUTTON) && currentJumpFrame == 0) {
    currentJumpFrame = jumpFrame;
  }
}

void run() {
  if (introFrameCount > 0) {
    introFrameCount--;
    arduboy.drawBitmap(0, 0, bannerFrames[0], bannerFrameWidth, bannerFrameHeight, WHITE);
    arduboy.display();
    return;
  }

  // Draw player
  arduboy.drawBitmap(player.X, player.Y, playerFrames[player.runningAnimationFrame], playerFrameWidth, playerFrameHeight, WHITE);

  // Draw floor
  arduboy.drawFastHLine(0, HEIGHT-1, WIDTH-1, WHITE);

  // Draw score
  arduboy.setCursor(100, 1);
  char scoreBuffer[16];
  sprintf(scoreBuffer, "% 3d", score);
  arduboy.print(scoreBuffer);

  for (int i = 0; i < maxHurdles; i++) {
    if (hurdles[i] < -hurdleFrameWidth) {
      int minDistance;

      if (i == 0) {
        minDistance = hurdles[maxHurdles-1];
      } else {
        minDistance = hurdles[i-1];
      }

      if (minDistance < WIDTH) {
        minDistance = WIDTH;
      }

      hurdles[i] = minDistance + randInRange(40, 80);
    }

    arduboy.drawBitmap(hurdles[i], hurdleY, hurdleFrames[0], hurdleFrameWidth, hurdleFrameHeight, WHITE);

    hurdles[i]--;
  }

  handleInput();

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

  if (arduboy.everyXFrames(6)) {
    player.runningAnimationFrame++;
    player.runningAnimationFrame = (player.runningAnimationFrame % 4) + 4;
  }

  if (currentJumpFrame > 0) {
    player.idleAnimationFrame = 3;
    player.runningAnimationFrame = 3;
  }

  // Check for collision
  for (int i = 0; i < maxHurdles; i++) {
    if (
      collision(
        player.X, player.Y, playerFrameHeight, playerFrameWidth,
        hurdles[i], hurdleY, hurdleFrameHeight, hurdleFrameWidth
      )
    ) {
      // Kill player
      score = 0;
      arduboy.tunes.tone(300, 50);
    }

    // Check for score collision
    if (
      collision(
        player.X, 0, 2, 2,
        hurdles[i], 0, 2, 2
      )
    ) {
      score++;
    }
  }
}
