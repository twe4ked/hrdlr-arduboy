#include <Math.h>
#include <bitmaps/banner.h>
#include <bitmaps/hurdle.h>
#include <bitmaps/player.h>

#include <globals.h>
#include <game.h>

void setup() {
  for (int i = 0; i < maxHurdles; i++) {
    hurdles[i] = -hurdleFrameWidth;
  }

  arduboy.beginNoLogo();
}

void loop() {
  if (!(arduboy.nextFrame())) return;

  arduboy.clear();
  run();
  arduboy.display();
}
