
#ifndef _FLAGS_H
#define _FLAGS_H

#include "neslib.h"

extern byte flags[16];

// Bobbert flags
#define BOBBERT 0
#define HORSE_LENT	0b10000000
#define HORSE_STOLEN	0b01000000
#define HORSE_WARN	0b00010000
#define PLAYER_SLOW	0b00100000
#define PLAYER_SAVED	0b00000001
#define PLAYER_FED	0b00000010
#define HAS_MESSAGE	0b00000100

// Weapon Flags
#define WEAPON 1
#define BASIC_SWORD 0b00000001
#define SHORT_SWORD 0b00000010
#define LARGE_SWORD 0b00000100
#define LIGHT_SWORD 0b00001000

#endif