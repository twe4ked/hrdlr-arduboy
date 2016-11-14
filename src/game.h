#include <globals.h>
#include <music.h>

const uint8_t playerXDefault = 5;
const uint8_t playerYDefault = HEIGHT - playerFrameHeight - 1;
const uint8_t jumpHeight = 20;
const uint8_t jumpFrame = 30;
const uint8_t maxHurdles = 5;
const uint8_t maxCoins = 5;
const uint8_t hurdleY = HEIGHT - hurdleFrameHeight - 1;
const uint8_t coinY = 30;

uint8_t currentJumpFrame = 0;
uint8_t introFrameCount = arduboy.frameRate * 2;
uint8_t resetFrameCount;
int16_t hurdles[maxHurdles];
int16_t coins[maxCoins];
uint8_t currentCoinFrame = 0;
uint16_t score = 0;
uint16_t highScore = 0;
uint8_t deadCounter = 0;
bool muted = false;
bool gamePaused = false;

struct Player {
  uint8_t X;
  uint8_t Y;
  uint8_t idleAnimationFrame;
  uint8_t runningAnimationFrame;
  uint8_t deathAnimationFrame;
  bool isAlive;
  bool running;
};

Player player = {playerXDefault, playerYDefault, 0, 0, 0, true, false};

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
  if (!gamePaused && player.isAlive && arduboy.pressed(B_BUTTON) && currentJumpFrame == 0) {
    currentJumpFrame = jumpFrame;
  }

  if (buttons.justPressed(LEFT_BUTTON)) {
    muted = !muted;
  }

  if (buttons.justPressed(UP_BUTTON)) {
    gamePaused = !gamePaused;
  }
}

void resetHurdles() {
  for (int i = 0; i < maxHurdles; i++) {
    hurdles[i] = -hurdleFrameWidth;
  }
}

void resetCoins() {
  for (int i = 0; i < maxCoins; i++) {
    coins[i] = -coinFrameWidth;
  }
}

void reset() {
  resetHurdles();
  resetCoins();

  resetFrameCount = arduboy.frameRate;
  player.running = false;
}

void drawScore() {
  arduboy.setCursor(105, 0);
  char scoreBuffer[16];
  sprintf(scoreBuffer, "%04d", score);
  arduboy.print(scoreBuffer);

  arduboy.setCursor(105, 8);
  sprintf(scoreBuffer, "%04d", highScore);
  arduboy.print(scoreBuffer);
}

void drawMute() {
  arduboy.drawBitmap(95, 0, speakerFrames[muted ? 1 : 0], speakerFrameWidth, speakerFrameHeight, WHITE);
}

void drawHurdles() {
  for (int i = 0; i < maxHurdles; i++) {
    arduboy.drawBitmap(hurdles[i], hurdleY, hurdleFrames[0], hurdleFrameWidth, hurdleFrameHeight, WHITE);
  }
}

void drawCoins() {
  for (int i = 0; i < maxCoins; i++) {
    arduboy.drawBitmap(coins[i], coinY, coinFrames[currentCoinFrame], coinFrameWidth, coinFrameHeight, WHITE);
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

void updateCoins() {
  for (int i = 0; i < maxCoins; i++) {
    if (coins[i] < -coinFrameWidth) {
      int minDistance;

      if (i == 0) {
        minDistance = coins[maxCoins-1];
      } else {
        minDistance = coins[i-1];
      }

      if (minDistance < WIDTH) {
        minDistance = WIDTH;
      }

      coins[i] = minDistance + randInRange(40, 80);
    }

    coins[i]--;
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
  } else if (!player.isAlive) {
    return (2 - player.deathAnimationFrame) + 10;
  } else if (player.running) {
    return player.runningAnimationFrame;
  } else {
    return player.idleAnimationFrame;
  }
}

void drawPlayer() {
  if (player.isAlive || player.deathAnimationFrame != 0) {
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
  if (arduboy.everyXFrames(16)) {
    currentCoinFrame = currentCoinFrame == 0 ? 1 : 0;
  }

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

void incScore() {
  score++;
  if (score >= highScore) {
    highScore = score;
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
      player.isAlive = false;
      player.deathAnimationFrame = 2;
      // 3 frames * updates every 8 = 24
      deadCounter = 24;
      score = 0;

      if (!muted) {
        arduboy.tunes.playScore(deathMusic);
      }
      return;
    }

    // Check for score collision
    if (
      collision(
        player.X, 0, 2, 2,
        hurdles[i], 0, 2, 2
      )
    ) {
      incScore();
    }
  }

  for (int i = 0; i < maxCoins; i++) {
    if (
      collision(
        player.X, player.Y, playerFrameHeight, playerFrameWidth,
        coins[i], coinY, 7, 4
      )
    ) {
      incScore();
      coins[i] = -coinFrameWidth;

      if (!muted) {
        arduboy.tunes.tone(300, 50);
      }
    }
  }
}

void printCenter(const uint8_t y, const __FlashStringHelper *string) {
  uint8_t widthOfChar = 5;
  uint8_t gutter = 1;

  uint8_t numberOfCharacters = strlen_P(reinterpret_cast<const char *>(string));
  uint8_t widthOfPrintedString = (widthOfChar * numberOfCharacters) + (gutter * (numberOfCharacters-1));

  uint8_t x = (WIDTH - widthOfPrintedString) / 2;

  arduboy.setCursor(x, y);
  arduboy.print(string);
}

void drawPauseScreen() {
  arduboy.tunes.stopScore();

  drawScore();
  drawMute();

  arduboy.drawFastHLine(2, 20, 124, WHITE);   // top line
  arduboy.drawFastHLine(2, 54, 124, WHITE);   // bottom line
  arduboy.drawFastVLine(0, 22, 31, WHITE);    // left line
  arduboy.drawFastVLine(127, 22, 31, WHITE);  // right line
  arduboy.drawPixel(1, 21, WHITE);            // top left
  arduboy.drawPixel(126, 21, WHITE);          // top right
  arduboy.drawPixel(1, 53, WHITE);            // bottom left
  arduboy.drawPixel(126, 53, WHITE);          // bottom right

  printCenter(24, F("PAUSED"));
  printCenter(34, F("Press UP to resume!"));
  printCenter(44, F("LEFT to toggle sound"));
}

void run() {
  if (introFrameCount > 0) {
    introFrameCount--;
    arduboy.drawBitmap(0, 0, bannerFrames[0], bannerFrameWidth, bannerFrameHeight, WHITE);
    arduboy.display();
    return;
  }

  if (gamePaused) {
    drawPauseScreen();
    handleInput();
    return;
  }

  if (resetFrameCount > 0) {
    resetFrameCount--;
    if (resetFrameCount == 0) {
      player.running = true;
    }
  }

  drawPlayer();
  drawFloor();
  drawScore();
  drawMute();
  drawHurdles();
  drawCoins();

  if (player.isAlive && resetFrameCount == 0) {
    updateHurdles();
    updateCoins();
  }

  handleInput();

  if (deadCounter > 0) {
    deadCounter--;
    if (deadCounter == 0) {
      player.isAlive = true;
      arduboy.tunes.stopScore();
      reset();
    }
  }

  updateJumpFrame();
  updateAnimationFrames();

  if (player.isAlive) {
    checkForCollision();
  }
}
