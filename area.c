
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
  {0xa4, 1 | DOOR1},	// 5: Door
  {0xa4, 1 | DOOR1 | LOCKED},	// 6: Locked Door
  {0x1a, 0 | SOLID},	// 7: Void
  {0xac, 2 | GROUND},	// 8: Wood floor
  {0x9c, 2 | LARGE_DOOR1},	// 9: Path Door1
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
  // 10: Void
  {{7,7,7,7,7,7,7,7,7,7,7,7}},
  // 11: Brick wall,
  {{4,4,4,4,4,4,4,4,4,4,4,4}},
  // 12: Brick room
  {{4,8,8,8,8,8,8,8,8,8,8,4}},
  // 13: Top path
  {{9,3,3,3,3,3,3,2,0,0,0,0}},
};

const Area areas[] = {
  { // 0: Starting Area
    42, {0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 8, 9, 8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    { // Entities
      {1, 31*TILE_SIZE-8, 2*TILE_SIZE},	// Bobbert's Horse
      {4, 30*TILE_SIZE, 8*TILE_SIZE}, // Bobbert
      // Spiders
      {7, 16*TILE_SIZE, 3*TILE_SIZE},
      {7, 16*TILE_SIZE, 6*TILE_SIZE-8},
      {7, 16*TILE_SIZE, 8*TILE_SIZE},
    },
    { // Doors
      {1,1*TILE_SIZE,6*TILE_SIZE-8},
      {},{},{},{},{},{},
      {2,32*TILE_SIZE-8,1*TILE_SIZE},{NULL_AREA}
    }
  },
  { // 1: Bobbert's House
    12, {12,12,12,12,12,12,12,12,12,12,12,11,10,10,10,10,10,10,10,10,10,10},
    {	// Entities
      {5, 5*TILE_SIZE, 5*TILE_SIZE-8},	// Bobbert
      {6, 5*TILE_SIZE, 6*TILE_SIZE-8}	// Soup
    },
    {
      {},{},{},{},{},{},{},
      {NULL_AREA}, {0, 28*TILE_SIZE, 1*TILE_SIZE}
    }
  },
  {	// 2: Road
    64, {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,13,13,13,13,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {// Entities
      {10, 32*TILE_SIZE-8, TILE_SIZE*8}
    },
    {// Doors
      {},{},{},
      {0, 40*TILE_SIZE, 6*TILE_SIZE-8},{},{},{},
      {NULL_AREA},{NULL_AREA}
    }
  }
};
