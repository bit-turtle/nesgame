
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
  if (x >= x_scroll && x <= x_scroll + 32*8) {
    oam_id = oam_spr(x-x_scroll, 30+y, ch + (attr&OAM_FLIP_H ? 2 : 0), attr, oam_id);
    oam_id = oam_spr(x-x_scroll, 30+y+8, ch + (attr&OAM_FLIP_H ? 3 : 1), attr, oam_id);
  }
  if (x+8 >= x_scroll && x+8 <= x_scroll + 32*8) {
    oam_id = oam_spr(x-x_scroll+8, 30+y, ch + (attr&OAM_FLIP_H ? 0 : 2), attr, oam_id);
    oam_id = oam_spr(x-x_scroll+8, 30+y+8, ch + (attr&OAM_FLIP_H ? 1 : 3), attr, oam_id);
  }
}