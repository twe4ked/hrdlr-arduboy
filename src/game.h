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
const uint16_t maxScore = 9999;

uint8_t currentJumpFrame = 0;
uint8_t introFrameCount = arduboy.frameRate * 2;
uint8_t resetFrameCount;
int16_t hurdles[maxHurdles];
int16_t coins[maxCoins];
uint8_t currentCoinFrame = 0;
uint16_t score = 0;
uint8_t deadCounter = 0;
bool muted = false;
bool gamePaused = false;
bool deathScreen = false;
bool hasNewHighScore = false;

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

struct HighScore {
  uint16_t score;
  char initials[4];
};

HighScore highScores[3];

double jumpCurveInternal(double x) {
  return sin((x * 180) * PI / 180);
}

double jumpCurve(double currentJumpFrame) {
  double n = 1 - (currentJumpFrame * (1.0 / jumpFrame));
  return jumpCurveInternal(n) * jumpHeight;
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

void saveHighScore(const uint8_t rank, HighScore highScore) {
  EEPROM.put(
    EEPROM_STORAGE_SPACE_START + (sizeof(highScore) * rank),
    highScore
  );
}

HighScore getHighScore(const uint8_t rank) {
  HighScore highScore;
  EEPROM.get(
    EEPROM_STORAGE_SPACE_START + (sizeof(highScore) * rank),
    highScore
  );
  return highScore;
}

void ensureValidHighScore() {
  for (int i = 0; i < 3; i++) {
    if (getHighScore(i).score > maxScore) {
      saveHighScore(i, HighScore {0, "AAA"});
    }
  }
}

void drawScore() {
  arduboy.setCursor(105, 0);
  char scoreBuffer[16];
  sprintf(scoreBuffer, "%04d", score);
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
      int minDistance = max(hurdles[(i == 0 ? maxHurdles : i) - 1], WIDTH);
      hurdles[i] = minDistance + randInRange(40, 80);
    }
    hurdles[i]--;
  }
}

void updateCoins() {
  for (int i = 0; i < maxCoins; i++) {
    if (coins[i] < -coinFrameWidth) {
      int minDistance = max(coins[(i == 0 ? maxCoins : i) - 1], WIDTH);
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
  if (score < maxScore) {
    score++;
  }
}

void playerDeath() {
  player.isAlive = false;
  player.deathAnimationFrame = 2;
  // 3 frames * updates every 8 = 24
  deadCounter = 24;

  if (!muted) {
    arduboy.tunes.playScore(deathMusic);
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
      playerDeath();
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

// TODO: De-dupe
void printCenterNormalString(const uint8_t y, const char *string) {
  uint8_t widthOfChar = 5;
  uint8_t gutter = 1;

  uint8_t numberOfCharacters = strlen(string);
  uint8_t widthOfPrintedString = (widthOfChar * numberOfCharacters) + (gutter * (numberOfCharacters-1));

  uint8_t x = (WIDTH - widthOfPrintedString) / 2;

  arduboy.setCursor(x, y);
  arduboy.print(string);
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

void drawSlightlyRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  arduboy.drawRoundRect(x, y, w, h, 3, WHITE);

  // top left
  arduboy.drawPixel(x+1, y, WHITE);
  arduboy.drawPixel(x, y+1, WHITE);

  // top right
  arduboy.drawPixel(x+w-2, y, WHITE);
  arduboy.drawPixel(x+w-1, y+1, WHITE);

  // bottom left
  arduboy.drawPixel(x+1, y+h-1, WHITE);
  arduboy.drawPixel(x, y+h-2, WHITE);

  // bottom right
  arduboy.drawPixel(x+w-2, y+h-1, WHITE);
  arduboy.drawPixel(x+w-1, y+h-2, WHITE);
}

void drawPauseScreen() {
  arduboy.tunes.stopScore();

  drawScore();
  drawMute();

  drawSlightlyRoundRect(0, 17, 127, 42);
  drawSlightlyRoundRect(2, 19, 123, 38);

  printCenter(24, F("PAUSED"));
  printCenter(34, F("UP to resume!"));
  printCenter(44, F("LEFT toggle sound"));
}

void drawInvertedChar(const uint16_t x, const uint16_t y, const uint8_t c) {
  arduboy.drawFastVLine(x, y, 9, WHITE);
  arduboy.drawFastHLine(x+1, y, 6, WHITE);
  arduboy.drawChar(x+1, y+1, c, BLACK, WHITE, 1);
}

uint16_t highScoreRank1 = 0;
uint16_t highScoreRank2 = 0;
uint16_t highScoreRank3 = 0;

uint8_t currentInitial;
uint8_t currentRank;

HighScore editInitialsForRank(HighScore highScore) {
  if (buttons.justPressed(DOWN_BUTTON)) {
    highScore.initials[currentInitial] = min(highScore.initials[currentInitial] + 1, 'Z');
  } else if (buttons.justPressed(UP_BUTTON)) {
    highScore.initials[currentInitial] = max(highScore.initials[currentInitial] - 1, 'A');
  } else if (buttons.justPressed(RIGHT_BUTTON)) {
    currentInitial = min(currentInitial + 1, 2);
  } else if (buttons.justPressed(LEFT_BUTTON)) {
    currentInitial = max(currentInitial - 1, 0);
  }
  return highScore;
}

void drawCurrentInitialForRank(HighScore highScore) {
  drawInvertedChar(
    48 + (currentInitial * 6),
    29 + (currentRank * 10),
    highScore.initials[currentInitial]
  );
}

void drawHighScoreLine(const uint8_t displayRank, const HighScore highScore) {
  char lineBuffer[16];
  sprintf(lineBuffer, "%d.   %s   %04d", displayRank, highScore.initials, highScore.score);
  arduboy.setCursor(19, 20 + (10 * displayRank));
  arduboy.print(lineBuffer);
}

void drawHighScoreScreen() {
  drawSlightlyRoundRect(0, 0, WIDTH, HEIGHT);
  drawSlightlyRoundRect(2, 2, WIDTH-4, HEIGHT-4);

  char scoreBuffer[17];
  sprintf(scoreBuffer, "Your score: %04d", score);
  printCenterNormalString(8, scoreBuffer);

  arduboy.setCursor(19, 20);
  arduboy.print(F("RANK NAME SCORE"));

  drawHighScoreLine(1, highScores[0]);
  drawHighScoreLine(2, highScores[1]);
  drawHighScoreLine(3, highScores[2]);
}

void setupHighScore() {
  player.isAlive = true;
  deathScreen = true;

  highScores[0] = getHighScore(0);
  highScores[1] = getHighScore(1);
  highScores[2] = getHighScore(2);

  if (score > highScores[0].score) {
    currentRank = 0;
    highScores[2] = highScores[1]; // mid -> bot
    highScores[1] = highScores[0]; // top -> mid
  } else if (score > highScores[1].score) {
    currentRank = 1;
    highScores[2] = highScores[1]; // mid -> bot
  } else if (score > highScores[2].score) {
    currentRank = 2;
  }

  if (score > highScores[2].score) {
    hasNewHighScore = true;
    highScores[currentRank] = HighScore {score, "AAA"};
    currentInitial = 0;
  } else {
    hasNewHighScore = false;
  }

  arduboy.tunes.stopScore();
  reset();
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

  if (deathScreen) {
    drawHighScoreScreen();

    if (hasNewHighScore) {
      highScores[currentRank] = editInitialsForRank(highScores[currentRank]);
      drawCurrentInitialForRank(highScores[currentRank]);
    }

    if (buttons.justPressed(B_BUTTON)) {
      if (hasNewHighScore) {
        for (int i = 0; i < 3; i++) {
          saveHighScore(i, highScores[i]);
        }
      }
      deathScreen = false;
      score = 0;
    }
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
      setupHighScore();
    }
  }

  updateJumpFrame();
  updateAnimationFrames();

  if (player.isAlive) {
    checkForCollision();
  }
}
