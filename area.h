
#ifndef _AREA_H
#define _AREA_H

#include "neslib.h"

typedef struct Tile {
  byte chr;
  byte attr;
  byte behavior;
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

typedef struct Area {
  byte width;
  byte collumns[64];	// list of collumns from list above
  EntityState entities[MAX_ENTITIES];
} Area;

const extern Area areas[32];

extern byte area;

#endif
