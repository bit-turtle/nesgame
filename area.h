
#ifndef _AREA_H
#define _AREA_H

#include "neslib.h"

// Tile Types
#define AIR	0b00000000
#define SOLID	0b10000000
#define FIRE	0b01000000
#define DOOR	0b00100000
//		    |-|
#define DOOR0	0b00000000 | DOOR
#define DOOR1	0b00001000 | DOOR
#define DOOR2	0b00010000 | DOOR
#define DOOR3	0b00011000 | DOOR
#define LOCKED	0b00000100
// Attribute Mask
#define MASK	0b00000011

typedef struct Tile {
  byte chr;
  byte attr;
} Tile;

#define TILE_SIZE 16

const extern Tile tiles[256];

typedef struct Collumn {
  byte rows[12];  // List of tiles from tiles list above
} Collumn;

void render_collumn(byte dir);

const extern Collumn collumns[64];

typedef struct EntityState {
  byte entity;
  word x;
  byte y;
} EntityState;

typedef struct Entity {
  byte chr;
  byte attr;
  void (*collide)(EntityState*);
  void (*tick)(EntityState*);
} Entity;

const extern Entity entities[64];

#define MAX_ENTITIES 10

typedef struct Door {
  byte area;
  word x;
  byte y;
} Door;

typedef struct Area {
  byte width;
  byte collumns[64];	// list of collumns from list above
  EntityState entities[MAX_ENTITIES];
  Door doors[4];
} Area;

const extern Area areas[32];

extern byte area;

#endif
