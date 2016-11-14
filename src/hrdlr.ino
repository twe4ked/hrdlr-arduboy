#include <Math.h>
#include <bitmaps/banner.h>
#include <bitmaps/coin.h>
#include <bitmaps/hurdle.h>
#include <bitmaps/player.h>
#include <bitmaps/speaker.h>

#include <globals.h>
#include <game.h>

boolean nextFrame() {
  if (arduboy.nextFrame()) {
    buttons.poll();
    return true;
  }
  return false;
}

void setup() {
  reset();
  arduboy.beginNoLogo();
}

void loop() {
  if (!(nextFrame())) return;

  arduboy.clear();
  run();
  arduboy.display();
}
