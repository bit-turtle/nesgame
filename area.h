
#ifndef _AREA_H
#define _AREA_H

#include "neslib.h"

typedef struct Tile {
  byte chr;
  byte attr;
  byte behavior;
} Tile;

const extern Tile tiles[256];

typedef struct Collumn {
  byte rows[12];  // List of tiles from tiles list above
} Collumn;

const extern Collumn collumns[64];

typedef struct Entity {
  byte chr;
  byte attr;
  void (*move)();
  void (*interact)();
} Entity;

const extern Entity entities[64];

typedef struct EntityState {
  byte entity;
  word x;
  byte y;
} EntityState;

typedef struct Area {
  byte width;
  byte collumns[64];	// list of collumns from list above
  EntityState entities[16];
} Area;

const extern Area areas[32];

#endif
