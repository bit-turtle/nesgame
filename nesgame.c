
/*
Demonstrates the Famitone2 library for sound and music.
Press controller buttons to hear sound effects.
*/

// Neslib
#include "neslib.h"
// 0 = horizontal mirroring
// 1 = vertical mirroring
#define NES_MIRRORING 1

// link the pattern table into CHR ROM
//#link "chr_generic.s"

// VRAMBuf
#include "vrambuf.h"
//#link "vrambuf.c"

// FamiTone2
//#link "famitone2.s"
//#link "music.s"
extern byte music_data[];

// String
#include <string.h>

// Offscreen Tiles
#include "offscreen.h"
//#link "offscreen.c"

// Area Loader
#include "area.h"
//#link "area.c"

// Metasprite Display
#include "sprite.h"
//#link "sprite.c"

// Global Variables
word t_scroll = 0;
const byte t_scroll_speed = 16;
word x_scroll = 0;
word newx_scroll;
byte dir = RIGHT;
byte playerspeed = 1;
word playerx = 50;
byte playery = 0;
bool moving = false;
word oldplayerx = 0;
byte oldplayery = 0;
byte playerhealth = 12;

void pushable(EntityState* entity) {
  entity->x += playerx-oldplayerx;
  entity->y += playery-oldplayery;
}

const Entity entities[] = {
  // Null entity, nothing is rendered
  {0,0,NULL,NULL},
  // Entities
  {0xfc, 0, NULL,NULL},
  {0xf4, 0, pushable,NULL}
};

#define CENTER 119

EntityState current_entities[MAX_ENTITIES];

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x1A,0x19,0x29,0x00,	// background colors
  0x00,0x16,0x20,0x00,
  0x1A,0x17,0x2D,0x00,
  0x0F,0x27,0x2A,0x00,

  0x06,0x37,0x24,0x00,	// sprite colors
  0x00,0x37,0x25,0x00,
  0x0F,0x2D,0x1A,0x00,
  0x0F,0x27,0x2A
};

bool compareword(word a, word b) {
  return a == b && a>>8 == b>>8;
}

void wait_frame() {
  // reset oam_id
  oam_id = 4;
  vrambuf_end();
  // ensure VRAM buffer is cleared
  ppu_wait_nmi();
  vrambuf_clear();
}
void start_frame() {
  // scroll top bar
  scroll(t_scroll, 0);
  // split at sprite zero and set X scroll
  split(x_scroll, 0);
}

void dialogue(char* name, char* text) {
  register byte counter, val;
  vrambuf_put(NTADR_B(2,1), "                            ", 29);
  vrambuf_put(NTADR_B(2,1), name, strlen(name));
  vrambuf_put(NTADR_B(2,2), "                            ", 29);
  while (t_scroll < 256) {
    t_scroll += t_scroll_speed;
    wait_frame();
  }
  val = strlen(text);
  for (counter = 0; counter < val*2; counter++) {
    vrambuf_put(NTADR_B(2+(counter>>1),2), text+(counter>>1), 1);
    wait_frame();
  }
  counter = 0;
  do {
    counter++;
    if ((counter>>4)&1) vrambuf_put(NTADR_B(29,3), "A", 1);
    else vrambuf_put(NTADR_B(29,3), "\x3", 1);
    val = pad_poll(0);
    wait_frame();
  } while (!val);
  vrambuf_put(NTADR_B(29,3), "\x3", 1);
  while (t_scroll > 0) {
    t_scroll -= t_scroll_speed;
    wait_frame();
  }
}

void player_scroll() {
    newx_scroll = playerx-CENTER;
    // Limits
    if (newx_scroll > 0xfff)
      x_scroll = 0;
    else if (newx_scroll > 16*areas[area].width-0xff)
      x_scroll = 16*areas[area].width-0xff;
    else
      x_scroll = newx_scroll;
}

void load_area(word x, byte y) {
  byte i;
  word final_scroll;
  playerx = x;
  playery = y;
  player_scroll();
  final_scroll = x_scroll;
  x_scroll += 32*8;
  while(x_scroll != final_scroll) {
    // Scroll
    if (t_scroll > 256) t_scroll+=t_scroll_speed;
    else if (t_scroll != 0) t_scroll-=t_scroll_speed;
    x_scroll-=16;
    // Render
    render_collumn(LEFT);
    // Update
    update_offscreen(LEFT);
    // Render
    wait_frame();
  }
  for (i = 0; i < MAX_ENTITIES; i++) {
    current_entities[i] = areas[area].entities[i];
  }
}

bool player_collision(word x, byte y) {
  return (playerx+8 < x+16 && playerx+8 >= x && playery+8 < y+16 && playery+8 >= y);
}

void controls() {
  byte state = pad_poll(0);
  oldplayerx = playerx;
  oldplayery = playery;
  moving = false;
  if (state&PAD_RIGHT) {
    playerx+=playerspeed;
    dir = RIGHT;
    if (playerx>>4>=areas[area].width-1)
      playerx = areas[area].width-1<<4;
    moving = true;
  }
  if (state&PAD_LEFT) {
    playerx-=playerspeed;
    dir = LEFT;
    if (playerx > 256<<4)
      playerx = 0;
    moving = true;
  }
  if (state&PAD_UP) {
    playery-=playerspeed;
    if (playery > oldplayery)
      playery = 0;
    moving = true;
  }
  if (state&PAD_DOWN) {
    playery+=playerspeed;
    if (playery > 22*8)
      playery = 22*8;
    moving = true;
  }
}

void main(void) {
  word renderx_scroll;
  byte playerframe = 0;
  byte i,j;
  // Set Pallete
  pal_all(PALETTE);
  // VRAM Initialization
  vram_adr(nt2attraddr(NTADR_A(0,1)));
  vram_fill(ATTRIBUTE(0,0,1,1),16);
  vram_adr(nt2attraddr(NTADR_B(0,1)));
  vram_fill(ATTRIBUTE(0,0,1,1),16);
  vram_adr(NTADR_A(0,3));
  vram_fill(0x5, 32);
  vram_adr(NTADR_B(0,3));
  vram_fill(0x3, 32);
  // top bar split sprite
  oam_clear();
  oam_spr(0, 30, 0x80, 2, 0);
  // vrambuf initialization
  vrambuf_clear();
  set_vram_update(updbuf);
  // display
  nmi_set_callback(start_frame);
  //enable rendering
  ppu_on_all();
  // repeat forever
  load_area(5*TILE_SIZE,6*TILE_SIZE-8);
  // Reset newx_scroll
  newx_scroll = x_scroll;
  renderx_scroll = x_scroll;
  while(1) {
    // Scroll
    // Update Offscreen Tiles
    if (x_scroll >= renderx_scroll+16 || x_scroll <= renderx_scroll-16) {
      // Render
      render_collumn(dir);
      // Update
      update_offscreen(dir);
      // Renderx
      renderx_scroll = x_scroll;
    }
    // Player health
    for (i = 0; i < playerhealth; i+=4) {
      j = 0x14+(playerhealth-i)%4;
      vrambuf_put(NTADR_A(2+(i>>2),2), &j, 1);
    }
    // Render
    wait_frame();
    
    // Update Game
    // Controls
    controls();
    player_scroll();
    playerframe = (playerx != oldplayerx) ? playerx : ((playery != oldplayery) ? playery : 0);
    playerframe*=2;
    playerframe = (((playerframe&16) ? ((playerframe&8) ? 0xd8 : 0xdc) : ((playerframe&8) ? 0xd8 : 0xe0)));
    if (!moving)
      playerframe = 0xd8;
    metasprite(playerframe, 0 | (dir == LEFT ? OAM_FLIP_H : 0), playerx, playery);
    
    // Render entities
    for (i = 0; i < MAX_ENTITIES; i++) {
      if (current_entities[i].entity == 0)
        continue;
      // Render
      metasprite(entities[current_entities[i].entity].chr, entities[current_entities[i].entity].attr, current_entities[i].x, current_entities[i].y);
    }
    
    // Hide unused sprites
    oam_hide_rest(oam_id);
    // Display again because my code is slow for some reason and can't render in time
    wait_frame();
    
    // Process entities
    for (i = 0; i < MAX_ENTITIES; i++) {
      if (current_entities[i].entity == 0)
        continue;
      // Process
      if (entities[current_entities[i].entity].collide != NULL && player_collision(current_entities[i].x, current_entities[i].y)) {
        entities[current_entities[i].entity].collide(&current_entities[i]);
      }
      if (entities[current_entities[i].entity].tick != NULL) {
        entities[current_entities[i].entity].tick(&current_entities[i]);
      }
    }
  }
}
