
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
  {0x90, 0 | GROUND | OFFROAD},	// 0: Grass
  {0x94, 2 | GROUND},	// 1: Path Top
  {0x98, 2 | GROUND},	// 2: Path Bottom
  {0x9c, 2 | GROUND},	// 3: Path
  {0xa0, 1 | SOLID},	// 4: Brick
  {0xa4, 1 | DOOR1},	// 5: Door
  {0xa4, 1 | DOOR2 | LOCKED},	// 6: Locked Door
  {0x1a, 0 | SOLID},	// 7: Void
  {0xac, 2 | GROUND},	// 8: Wood floor
  {0x9c, 2 | LARGE_DOOR1},	// 9: Path Door1
  {0x9c, 2 | LARGE_DOOR2},	// 10: Path Door2
  {0xa4, 1 | DOOR2 | LOCKED},	// 11: Locked Door
  {0xcc, 0 | SOLID},	// 12:  Tree Top
  {0xd0, 0 | SOLID},	// 13: Tree Bottom
  {0xd4, 0 | SOLID},	// 14: Tree center
  {0x90, 0 | LARGE_DOOR1},	// 15: Grass door 1
  {0x90, 0 | LARGE_DOOR2},	// 16: Grass door 2
  {0x90, 0 | LARGE_DOOR3},	// 17: Grass door 3
  {0x90, 0 | LARGE_DOOR4},	// 18: Grass door 4
  {0xcc, 0 | DOOR3},	// 19: Hidden door
  {0x90, 2 | GROUND | OFFROAD},	// 20: Dirty grass
  {0xc4, 1 | DOOR1},	// 21: Staircase
  {0x90, 1 | GROUND | OFFROAD},	// 22: Underground Land
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
  // 14: Bottom path
  {{0,0,0,0,1,3,3,3,3,3,3,10}},
  // 15: Second door with no path
  {{11,0,0,0,1,3,3,2,0,0,0,0}},
  // 16: Tree no path
  {{14,14,12,0,0,0,0,0,0,13,14,14}},
  // 17: Tree small path
  {{14,14,12,0,0,1,2,0,0,13,14,14}},
  // 18: Tree Normal path
  {{14,14,12,0,1,3,3,2,0,13,14,14}},
  // 19: Tree smaller no path
  {{14,14,14,12,0,0,0,0,13,14,14,14}},
  // 20: Tree top path (Large door 1)
  {{15,0,0,0,0,0,0,0,13,14,14,14}},
  // 21: Tree bottom path (Large door 2)
  {{14,14,14,12,0,0,0,0,0,0,0,16}},
  // 22: Tree top path 2 (Large door 3)
  {{17,0,0,0,0,0,0,0,13,14,14,14}},
  // 23: Tree bottom path 2 (Large door 4)
  {{14,14,14,12,0,0,0,0,0,0,0,18}},
  // 24: Tree intersection path (Large door 1 & 2)
  {{15,0,0,0,0,0,0,0,0,0,0,16}},
  // 25: Tree almost no path
  {{14,14,14,14,12,0,0,13,14,14,14,14}},
  // 26: Tree no path
  {{14,14,14,14,14,14,14,14,14,14,14,14}},
  // 27: Hidden tree path
  {{14,14,14,19,20,0,0,0,13,14,14,14}},
  // 28: Staircase no path
  {{14,14,14,12,0,0,21,0,13,14,14,14}},
  // 29: Underground Staircase
  {{7,7,7,7,22,22,21,22,7,7,7,7}},
  // 30: Underground
  {{7,7,7,7,22,22,22,22,7,7,7,7}},
  // 31: Locked Underground Door
  {{7,7,7,7,22,22,6,22,7,7,7,7}},
};

const Area areas[] = {
  { // 0: Starting Area
    42, {0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 8, 9, 8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    { // Entities
      {1, 31*TILE_SIZE-8, 2*TILE_SIZE},	// Bobbert's Horse
      {4, 30*TILE_SIZE, 8*TILE_SIZE}, // Bobbert
      // Spiders
      {7, 16*TILE_SIZE, 3*TILE_SIZE, 3},
      {7, 16*TILE_SIZE, 6*TILE_SIZE-8, 3},
      {7, 16*TILE_SIZE, 8*TILE_SIZE, 3},
    },
    { // Doors
      {1,1*TILE_SIZE,6*TILE_SIZE-8},
      {},{},{},{},{},{},
      {2,32*TILE_SIZE-8,1*TILE_SIZE},{NULL_AREA}
    },
    0,
    NORTH
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
    },
    0,
    EAST
  },
  {	// 2: Road
    64, {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,13,13,13,13,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,14,14,14,14,2,2,2,2,2},
    {// Entities
      {10, 32*TILE_SIZE-8, TILE_SIZE*8}
    },
    {// Doors
      {},{},{},
      {0, 40*TILE_SIZE, 6*TILE_SIZE-8},{},{},{},
      {3, 16*TILE_SIZE-8,1*TILE_SIZE},{5, 23*TILE_SIZE, 6*TILE_SIZE-8}
    },
    0,
    EAST
  },
  {	// 3: Turlin
    32, {0,1,2,2,8,8,8,8,9,8,8,8,8,2,13,13,13,13,2,8,8,8,8,15,8,8,8,8,2,2,1,0},
    {// Entities
      {10, 18*TILE_SIZE, 1*TILE_SIZE},
      {12, 16*TILE_SIZE-8, 6*TILE_SIZE-8}
    },
    {// Doors
      {4, 1*TILE_SIZE, 6*TILE_SIZE-8},{},{},
      {2, 62*TILE_SIZE, 6*TILE_SIZE-8},{},{},{},
      {NULL_AREA}, {NULL_AREA}
    },
    4,
    NORTH
  },
  {	// 4: Mayor Turt Le's House
    12, {12,12,12,12,12,12,12,12,12,12,12,11,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {	// Entities
      {13, TILE_SIZE*5, 6*TILE_SIZE-8}
    },
    {	// Door
      {},{},{},
      {},{},{},{},
      {NULL_AREA},{3, TILE_SIZE*8, TILE_SIZE*1}
    },
    4,
    WEST
  },
  {	// 5: Entrance to forest
    25, {19,19,19,19,16,16,16,16,16,16,16,16,16,17,18,18,18,18,18,2,2,2,2,2,2},
    {	// Entities
      {10, 12*TILE_SIZE-8, 6*TILE_SIZE-8}
    },
    {	// Doors
      {},{},{},
      {},{},{},{},
      {2, TILE_SIZE*1, TILE_SIZE*6-8},{6, 32*TILE_SIZE-8, 1*TILE_SIZE}
    },
    1,
    EAST
  },
  {	// 6: Maze part 1
    64, {26,25,19,19,19,19,21,21,21,21,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,23,23,23,23,19,19,19,19,19,19,19,25,26},
    {	// Entities
      {17, TILE_SIZE*3, TILE_SIZE*6-8}
    },
    {// Doors
      {},{},{},
      {5, TILE_SIZE*1, TILE_SIZE*6-8},{10,TILE_SIZE*1,TILE_SIZE*6-8},{},{7, TILE_SIZE*1, TILE_SIZE*6-8},
      {NULL_AREA},{NULL_AREA}
    },
    1,
    NORTH
  },
  
  { // 7: Maze part 2
    64, {19,19,19,19,19,19,19,19,19,19,19,19,27,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,24,24,24,24,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,23,23,23,23,19,19,19,19,19,19,19,19,19},
    {	// Entities
      {17, TILE_SIZE*12, TILE_SIZE*6-8},
      {17, TILE_SIZE*40, TILE_SIZE*6-8}
    },
    {// Doors
      {},{},{8, TILE_SIZE*1, TILE_SIZE*6-8},
      {12, TILE_SIZE*1, TILE_SIZE*6-8},{9, TILE_SIZE*1, TILE_SIZE*6-8},{},{11, 62*TILE_SIZE, TILE_SIZE*6-8},
      {2, TILE_SIZE*1, TILE_SIZE*6-8},{6, TILE_SIZE*53-8, TILE_SIZE*10}
    },
    1,
    WEST
  },
  {	// 8: Compass room
    15, {25,25,25,25,25,25,25,25,19,19,19,19,19,25,26,26},
    {	// Entities
      {14, TILE_SIZE*10, TILE_SIZE*6-8},
      {15, TILE_SIZE*8, TILE_SIZE*4-4},
      {15, TILE_SIZE*8, TILE_SIZE*7}
    },
    {//Doors
      {},{},{},
      {},{},{},{},
      {NULL_AREA},{7, TILE_SIZE*12, TILE_SIZE*4}
    },
    1,
    NORTH
  },
  {	// 9: Dead End
    15, {25,25,25,19,19,19,19,19,25,26,26,26,26,26,26,26},
    {	// Entities
      {10, TILE_SIZE*5, TILE_SIZE*6-8}
    },
    {// Doors
      {},{},{},
      {},{},{},{},
      {NULL_AREA},{7, TILE_SIZE*32-8, TILE_SIZE*1},
    },
    1,
    UNKNOWN
  },
  {	// 10: Left path
    64, {19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,25,26},
  {	// Entities
    {14, TILE_SIZE*60, TILE_SIZE*6-8},
    {7, TILE_SIZE*50, TILE_SIZE*7},
    {7, TILE_SIZE*50, TILE_SIZE*4},
    {7, TILE_SIZE*40, TILE_SIZE*6-8},
    {17, TILE_SIZE*20, TILE_SIZE*6-8}
  },
  {	// Doors
    {},{},{},
    {9, TILE_SIZE*1, TILE_SIZE*6-8},{},{},{},
    {NULL_AREA},{6, TILE_SIZE*8-8, TILE_SIZE*10}
  },
    1,
    WEST
  },
  {	// 11: Monster Jumpscare
    64, {26,25,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19},
    {	// Entities
      {10, TILE_SIZE*3, TILE_SIZE*6-8},
      
    },
    {	// Doors
      {},{},{},
      {},{},{},{},
      {7, TILE_SIZE*53-8, TILE_SIZE*10}, {NULL_AREA}
    },
    1,
    SOUTH
  }, 
  {	// 12: Coin room
    15, {25,25,25,25,25,25,25,25,19,19,19,19,19,25,26,26},
    {	// Entities
      {14, TILE_SIZE*10, TILE_SIZE*6-8}
    },
    {//Doors
      {},{},{},
      {},{},{},{},
      {NULL_AREA},{7, TILE_SIZE*32-8, TILE_SIZE*10}
    },
    1,
    NORTH
  },
  {	// 13: Entrance to the tunnel
    15, {25,25,25,25,25,25,25,25,19,19,28,19,19,25,26,26},
    {	// Entities
      {19, TILE_SIZE*10, TILE_SIZE*6},
    },
    {//Doors
      {14, TILE_SIZE*2, TILE_SIZE*6},{},{},
      {},{},{},{},
      {NULL_AREA},{7, TILE_SIZE*62, TILE_SIZE*6-8}
    },
    1,
    NORTH
  },
  {	// 14: Underground Tunnel
    40, {10,29,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,31,10},
    {	// Entities
    },
    { // Doors
      {13, TILE_SIZE*10, TILE_SIZE*5},{},{},
      {},{},{},{},
      {NULL_AREA}, {NULL_AREA}
    },
    1,
    NORTH
  }
  
};
