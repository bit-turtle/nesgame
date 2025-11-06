
#include "neslib.h"
#include "sprite.h"
#include "offscreen.h"

byte oam_id = 4;

void sprite(byte ch, byte attr, word x, byte y) {
  if (x < x_scroll || x > x_scroll + 32*8)
    return;
  oam_id = oam_spr(x-x_scroll, 30+y, ch, attr, oam_id);
}

void metasprite(byte ch, byte attr, word x, byte y) {
  sprite(ch + (attr&OAM_FLIP_H ? 2 : 0), attr, x, y);
  sprite(ch + (attr&OAM_FLIP_H ? 3 : 1), attr, x, y+8);
  sprite(ch + (attr&OAM_FLIP_H ? 0 : 2), attr, x+8, y);
  sprite(ch + (attr&OAM_FLIP_H ? 1 : 3), attr, x+8, y+8);
}