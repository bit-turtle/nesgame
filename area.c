
#include "area.h"

const Tile tiles[] = {
  {0xf4, 0, 0},
  {0xe0, 0, 0}
};

const Collumn collumns[] = {
  {{1,1,1,0,0,0,0,0,0,1,1,1}},
  {{0,0,0,0,1,1,1,1,0,0,0,0}},
  {{0,1,0,0,1,1,1,1,0,0,0,0}}
};

const Entity entities[] = {
  {0xba, 0, NULL,NULL}
};

const Area areas[] = {
  {
    4, {0, 1, 1, 0, 2, 1, 0, 0, 1, 1},
    { {0, 40, 30} }
  }
};
