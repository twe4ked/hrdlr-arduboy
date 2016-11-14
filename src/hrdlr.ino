#include <Math.h>
#include <bitmaps/banner.h>
#include <bitmaps/hurdle.h>
#include <bitmaps/player.h>
#include <bitmaps/speaker.h>

#include <globals.h>
#include <game.h>

void setup() {
  resetHurdles();
  arduboy.beginNoLogo();
}

void loop() {
  if (!(arduboy.nextFrame())) return;

  arduboy.clear();
  run();
  arduboy.display();
}
