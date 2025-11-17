
#include "neslib.h"
#include "sprite.h"
#include "offscreen.h"

byte oam_id = 4;

void sprite(word x, byte y, byte ch, byte attr) {
  oam_id = oam_spr(x-x_scroll, 30+y, (x < x_scroll || x >= x_scroll + 32*8) ? 0x00 : (ch), attr, oam_id);
}

void metasprite(byte ch, byte attr, word x, byte y) {
  sprite(x, y, ch + (attr&OAM_FLIP_H ? 2 : 0), attr);
  sprite(x, y+8, ch + (attr&OAM_FLIP_H ? 3 : 1), attr);
  sprite(x+8, y, ch + (attr&OAM_FLIP_H ? 0 : 2), attr);
  sprite(x+8, y+8, ch + (attr&OAM_FLIP_H ? 1 : 3), attr);
}