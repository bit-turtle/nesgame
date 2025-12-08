
#ifndef _AREA_H
#define _AREA_H

#include "neslib.h"

// Tile Attributes
#define GROUND		0b00000000
#define SOLID		0b10000000
#define FIRE		0b01000000
#define DOOR		0b00111000
#define DOOR1		0b00001000
#define DOOR2		0b00010000
#define DOOR3		0b00011000
#define LARGE_DOOR1	0b00100000
#define LARGE_DOOR2	0b00101000
#define LARGE_DOOR3	0b00110000
#define LARGE_DOOR4	0b00111000
#define LARGE_DOOR	0b00100000	// Doors 1-3: Small, Doors 4-7 Large
#define DoorIndex(attr) ( ((attr&DOOR) >> 3) - 1)
#define LOCKED	0b00000100	// Offroad if not used on a door
#define OFFROAD LOCKED
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
  byte health;
  byte chr_offset;
  byte memory;
} EntityState;

typedef struct Entity {
  byte chr;
  byte attr;
  void (*collide)(EntityState*);
  void (*tick)(EntityState*);
  void (*die)(EntityState*);
  void (*init)(EntityState*);
  void (*hurt)(EntityState*);
} Entity;

const extern Entity entities[64];

#define MAX_ENTITIES 6

#define NULL_AREA 0xff

typedef struct Door {
  byte area;
  word x;
  byte y;
} Door;

#define RIGHT_AREA 7
#define LEFT_AREA 8

typedef struct Area {
  byte width;
  byte collumns[64];	// list of collumns from list above
  EntityState entities[MAX_ENTITIES];
  Door doors[9];
  byte song;
} Area;

const extern Area areas[32];

extern byte area;

#endif
