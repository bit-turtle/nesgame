
#include "area.h"
#include "offscreen.h"

byte area = 0;

void render_collumn(byte dir){
  register byte y;
  register const Collumn* collumn;
  collumn = &collumns[areas[area].collumns[(x_scroll>>3)-dir]];
  for (y = 0; y < 12; y++) {
    set_metatile(y, tiles[collumn->rows[y]].chr);
    set_attr_entry(x_scroll>>3, y, tiles[collumn->rows[y]].attr);
  }
}

const Tile tiles[] = {
  {0x90, 0, 0},
  {0x94, 0, 0},
  {0x98, 0, 0},
  {0x9c, 0, 0}
};

const Collumn collumns[] = {
  {{0,0,0,1,3,3,3,3,2,0,0,0}},
  {{0,0,0,0,1,2,2,3,4,0,0,0}},
  {{0,1,0,0,1,2,2,3,4,0,0,0}}
};

const Entity entities[] = {
  {0xba, 0, NULL,NULL}
};

const Area areas[] = {
  {
    4, {1, 1, 1, 1, 2, 1, 0, 0, 1, 1},
    { {0, 40, 30} }
  }
};
