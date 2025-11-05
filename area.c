
#include "area.h"
#include "offscreen.h"

byte area = 0;

void render_collumn_ptr(const Collumn* collumn, word x) {
  register byte y;
  for (y = 0; y < 12; y++) {
    set_metatile(y, tiles[collumn->rows[y]].chr);
    set_attr_entry(x, y, tiles[collumn->rows[y]].attr);
  }
}

void render_collumn(byte dir){
  register const Collumn* collumn;
  collumn = &collumns[areas[area].collumns[(x_scroll>>4)+dir]];
  render_collumn_ptr(collumn, (x_scroll>>4)-dir);
}

const Tile tiles[] = {
  {0x90, 0, 0},
  {0x94, 2, 0},
  {0x98, 2, 0},
  {0x9c, 2, 0}
};

const Collumn collumns[] = {
  {{0,0,0,0,1,3,3,2,0,0,0,0}},
  {{0,0,1,3,3,3,3,3,3,2,0,0}},
  {{0,1,3,3,3,3,3,3,3,3,2,0}}
};

const Entity entities[] = {
  {0xba, 0, NULL,NULL}
};

const Area areas[] = {
  {
    4, {0, 1, 2, 0, 1, 2, 0, 1, 2, },
    { {0, 40, 30} }
  }
};
