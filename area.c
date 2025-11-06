
#include "area.h"
#include "offscreen.h"

byte area = 0;

void render_collumn_ptr(const Collumn* collumn, const Collumn* collumn2, byte x, byte x2) {
  register byte y;
  clear_attrbuf();
  for (y = 0; y < 12; y++) {
    set_metatile(y, tiles[collumn->rows[y]].chr);
    set_attr_entry(x, y, tiles[collumn->rows[y]].attr);
    set_attr_entry(x2, y, tiles[collumn2->rows[y]].attr);
  }
}

void render_collumn(byte dir){
  const Collumn* collumn;
  const Collumn* collumn2;
  byte x = (x_scroll>>4)+(dir>>1);
  byte x2 = (x&1) ? x-1 : x+1;
  clear_attrbuf();
  collumn = &collumns[areas[area].collumns[x]];
  collumn2 = &collumns[areas[area].collumns[x2]];
  render_collumn_ptr(collumn, collumn2, x, x2);
}

const Tile tiles[] = {
  {0x90, 0, 0},
  {0x94, 2, 0},
  {0x98, 2, 0},
  {0x9c, 2, 0}
};

const Collumn collumns[] = {
  {{0,0,0,0,0,0,0,0,0,0,0,0}},
  {{0,0,0,0,0,1,2,0,0,0,0,0}},
  {{0,0,0,0,1,3,3,2,0,0,0,0}},
  {{0,0,0,1,3,3,3,3,2,0,0,0}},
  {{0,0,1,3,3,3,3,3,3,2,0,0}},
  {{0,1,3,3,3,3,3,3,3,3,2,0}},
  {{3,3,3,3,3,3,3,3,3,3,3,3}}
};

const Entity entities[] = {
  // Null entity, nothing is rendered
  {0,0,NULL,NULL},
  // Entities
  {0xfc, 0, NULL,NULL},
  {0xf4, 0, NULL,NULL}
};

const Area areas[] = {
  {
    31, {1, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 2, 2, 2, 2, 3, 4, 5, 6, 0, 6, 5, 3, 4, 4, 5, 5, 4, 5, 2, 1},
    {
      {1, 4*TILE_SIZE, 2*TILE_SIZE},
      {1, 8*TILE_SIZE, 2*TILE_SIZE},
      {1, 4*TILE_SIZE, 9*TILE_SIZE},
      {1, 8*TILE_SIZE, 9*TILE_SIZE},
      {2, 6*TILE_SIZE, 6*TILE_SIZE},
      {2, 6*TILE_SIZE, 5*TILE_SIZE}
    }
  }
};
