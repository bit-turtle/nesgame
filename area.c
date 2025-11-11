
#include "area.h"
#include "offscreen.h"

byte area = 0;

void render_collumn_ptr(const Collumn* collumn, const Collumn* collumn2, byte x, byte x2) {
  register byte y;
  clear_attrbuf();
  for (y = 0; y < 12; y++) {
    set_metatile(y, tiles[collumn->rows[y]].chr);
    set_attr_entry(x, y, tiles[collumn->rows[y]].attr & MASK);
    set_attr_entry(x2, y, tiles[collumn2->rows[y]].attr & MASK);
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
  {0x90, 0 | GROUND},	// 0: Grass
  {0x94, 2 | GROUND},	// 1: Path Top
  {0x98, 2 | GROUND},	// 2: Path Bottom
  {0x9c, 2 | GROUND},	// 3: Path
  {0xa0, 1 | SOLID},	// 4: Brick
  {0xa4, 1 | DOOR1 | LOCKED},	// 5: Door
};

const Collumn collumns[] = {
  // 0-7: Grass and Path
  {{0,0,0,0,0,0,0,0,0,0,0,0}},
  {{0,0,0,0,0,1,2,0,0,0,0,0}},
  {{0,0,0,0,1,3,3,2,0,0,0,0}},
  {{0,0,0,1,3,3,3,3,2,0,0,0}},
  {{0,0,1,3,3,3,3,3,3,2,0,0}},
  {{0,1,3,3,3,3,3,3,3,3,2,0}},
  {{1,3,3,3,3,3,3,3,3,3,3,2}},
  {{3,3,3,3,3,3,3,3,3,3,3,3}},
  // 8-9: Brick House
  {{4,0,0,0,1,3,3,2,0,0,0,0}},
  {{5,3,3,3,3,3,3,2,0,0,0,0}},
};

const Area areas[] = {
  { // Starting Area
    64, {0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 8, 9, 8, 8, 2, 2},
    { // Entities
      {1, 31*TILE_SIZE-8, 2*TILE_SIZE},	// Bobbert's Horse
      {3, 1*TILE_SIZE, 8*TILE_SIZE}, // Bobbert
    },
    { // Doors
      {0,0,0},
      {NULL_AREA},{NULL_AREA},	// Unused
      {}, // Right
      {NULL_AREA},{NULL_AREA},{NULL_AREA} // Unused
    }
  }
};
