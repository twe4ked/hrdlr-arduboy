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
uint8_t deadCounter = 0;

struct Player {
  uint8_t X;
  uint8_t Y;
  uint8_t idleAnimationFrame;
  uint8_t runningAnimationFrame;
  uint8_t deathAnimationFrame;
  bool isDead;
};

Player player = {playerXDefault, playerYDefault, 0, 0, 0, false};

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

void resetHurdles() {
  for (int i = 0; i < maxHurdles; i++) {
    hurdles[i] = -hurdleFrameWidth;
  }
}

void drawScore() {
  arduboy.setCursor(100, 1);
  char scoreBuffer[16];
  sprintf(scoreBuffer, "% 3d", score);
  arduboy.print(scoreBuffer);
}

void drawHurdles() {
  for (int i = 0; i < maxHurdles; i++) {
    arduboy.drawBitmap(hurdles[i], hurdleY, hurdleFrames[0], hurdleFrameWidth, hurdleFrameHeight, WHITE);
  }
}

void updateHurdles() {
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

    hurdles[i]--;
  }
}

int currentPlayerJumpAnimationFrameIndex() {
  if (currentJumpFrame > 25) {
    return 3;
  } else if (currentJumpFrame > 20) {
    return 4;
  } else if (currentJumpFrame > 10) {
    return 5;
  } else if (currentJumpFrame > 5) {
    return 4;
  } else {
    return 3;
  }
}

int currentPlayerAnimationFrameIndex() {
  if (currentJumpFrame > 0) {
    return currentPlayerJumpAnimationFrameIndex();
  } else if (player.isDead) {
    return (2 - player.deathAnimationFrame) + 10;
  } else {
    return player.runningAnimationFrame;
  }
}

void drawPlayer() {
  if (!player.isDead || player.deathAnimationFrame != 0) {
    arduboy.drawBitmap(player.X, player.Y, playerFrames[currentPlayerAnimationFrameIndex()], playerFrameWidth, playerFrameHeight, WHITE);
  }
}

void drawFloor() {
  arduboy.drawFastHLine(0, HEIGHT-1, WIDTH-1, WHITE);
}

void updateJumpFrame() {
  if (currentJumpFrame > 0) {
    currentJumpFrame--;
    player.Y = playerYDefault - jumpCurve(currentJumpFrame);
  } else {
    player.Y = playerYDefault;
  }
}

// Player animation frames
//
// 0-2   : idle
// 3-5   : jump
// 6-9   : running
// 10-12 : dying
void updateAnimationFrames() {
  if (arduboy.everyXFrames(8)) {
    // 0-2
    player.idleAnimationFrame++;
    player.idleAnimationFrame = player.idleAnimationFrame % 3;

    // 10-12
    if (player.deathAnimationFrame > 0) {
      player.deathAnimationFrame--;
    }
  }

  if (arduboy.everyXFrames(6)) {
    // 6-9
    player.runningAnimationFrame++;
    player.runningAnimationFrame = (player.runningAnimationFrame % 4) + 6;
  }
}

void checkForCollision() {
  for (int i = 0; i < maxHurdles; i++) {
    if (
      collision(
        player.X, player.Y, playerFrameHeight, playerFrameWidth,
        hurdles[i], hurdleY, hurdleFrameHeight, hurdleFrameWidth
      )
    ) {
      // Kill player
      player.isDead = true;
      player.deathAnimationFrame = 2;
      deadCounter = 10;
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

void run() {
  if (introFrameCount > 0) {
    introFrameCount--;
    arduboy.drawBitmap(0, 0, bannerFrames[0], bannerFrameWidth, bannerFrameHeight, WHITE);
    arduboy.display();
    return;
  }

  drawPlayer();

  drawFloor();

  drawScore();

  drawHurdles();

  if (!player.isDead) {
    updateHurdles();
    handleInput();
  }

  if (deadCounter > 0) {
    if (deadCounter == 1) {
      player.isDead = false;
      resetHurdles();
    }
    deadCounter--;
  }

  updateJumpFrame();

  updateAnimationFrames();

  if (!player.isDead) {
    checkForCollision();
  }
}
