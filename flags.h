
#ifndef _FLAGS_H
#define _FLAGS_H

#include "neslib.h"

extern byte flags[16];

// Bobbert flags
#define BOBBERT 0
#define HORSE_LENT	0b10000000
#define HORSE_STOLEN	0b01000000
#define PLAYER_SAVED	0b00000001

#endif