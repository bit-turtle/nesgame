
#ifndef _AREA_H
#define _AREA_H

#include "neslib.h"

// Tile Attributes
#define GROUND	0b00000000
#define SOLID	0b10000000
#define FIRE	0b01000000
#define DOOR	0b00111000
#define DOOR1	0b00001000
#define DOOR2	0b00010000
#define DOOR3	0b00011000
#define DOOR4	0b00100000
#define DOOR5	0b00101000
#define DOOR6	0b00110000
#define DOOR7	0b00111000
#define DoorIndex(attr) ( ((attr&DOOR) >> 3) - 1)
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
  Door doors[7];
} Area;

const extern Area areas[32];

extern byte area;

#endif
