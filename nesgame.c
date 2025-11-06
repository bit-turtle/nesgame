
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
byte dir = RIGHT;
word playerx = 50;
byte playery = 0;
bool moving = false;
byte oldplayerx = 0;
byte oldplayery = 0;

#define CENTER 119

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x1A,0x19,0x29,0x00,	// background colors
  0x00,0x10,0x20,0x00,
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

void display() {
  // reset oam_id
  oam_hide_rest(oam_id);
  oam_id = 4;
  vrambuf_end();
  // ensure VRAM buffer is cleared
  ppu_wait_nmi();
  vrambuf_clear();
  // scroll top bar
  scroll(t_scroll, 0);
  // split at sprite zero and set X scroll
  split(x_scroll, 0);
}

void dialogue(char* name, char* text) {
  register byte counter, val;
  vrambuf_put(NTADR_B(2,1), name, strlen(name));
  vrambuf_put(NTADR_B(2,2), "                            ", 29);
  while (t_scroll < 256) {
    t_scroll += t_scroll_speed;
    display();
  }
  val = strlen(text);
  for (counter = 0; counter < val*2; counter++) {
    vrambuf_put(NTADR_B(2+(counter>>1),2), text+(counter>>1), 1);
    display();
  }
  counter = 0;
  do {
    counter++;
    if ((counter>>4)&1) vrambuf_put(NTADR_B(29,3), "A", 1);
    else vrambuf_put(NTADR_B(29,3), "\x3", 1);
    val = pad_poll(0);
    display();
  } while (!val);
  vrambuf_put(NTADR_B(29,3), "\x3", 1);
  while (t_scroll > 0) {
    t_scroll -= t_scroll_speed;
    display();
  }
}

void load_area(word final_scroll) {
  x_scroll = final_scroll+32*8;
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
    display();
  }
}

void controls() {
  byte state = pad_poll(0);
  oldplayerx = playerx;
  oldplayery = playery;
  moving = false;
  if (state&PAD_RIGHT) {
    playerx++;
    dir = RIGHT;
    if (playerx>>4>=areas[area].width-1)
      playerx = areas[area].width-1<<4;
    moving = true;
  }
  if (state&PAD_LEFT) {
    playerx--;
    dir = LEFT;
    if (playerx > 256<<4)
      playerx = 0;
    moving = true;
  }
  if (state&PAD_UP) {
    playery--;
    if (playery > oldplayery)
      playery = 0;
    moving = true;
  }
  if (state&PAD_DOWN) {
    playery++;
    if (playery > 22*8)
      playery = 22*8;
    moving = true;
  }
}

void main(void) {
  word newx_scroll;
  byte playerframe = 0;
  // Set Pallete
  pal_all(PALETTE);
  // VRAM Initialization
  vram_adr(nt2attraddr(NTADR_A(0,1)));
  vram_fill(ATTRIBUTE(0,0,1,1),16);
  vram_adr(nt2attraddr(NTADR_B(0,1)));
  vram_fill(ATTRIBUTE(0,0,1,1),16);
  vram_adr(NTADR_A(0,3));
  vram_fill(0x6, 32);
  vram_adr(NTADR_B(0,3));
  vram_fill(0x3, 32);
  // initialize music system
  famitone_init(music_data);
  // set music callback function for NMI
  nmi_set_callback(famitone_update);
  // play music
  music_play(0);
  // top bar split sprite
  oam_clear();
  oam_spr(0, 30, 0x80, 2, 0);
  // vrambuf initialization
  vrambuf_clear();
  set_vram_update(updbuf);
  //enable rendering
  ppu_on_all();
  // repeat forever
  load_area(0);
  dialogue("Bobbert", "It works now!");
  // Reset newx_scroll
  newx_scroll = x_scroll;
  while(1) {
    // Scroll
    // Limits
    if (newx_scroll > 0xfff)
      x_scroll = 0;
    else if (newx_scroll > 16*areas[area].width-0xff)
      x_scroll = 16*areas[area].width-0xff;
    else
      x_scroll = newx_scroll;
    // Update Offscreen Tiles
    if (x_scroll%16 == 0) {
      // Render
      render_collumn(dir);
      // Update
      update_offscreen(dir);
    }
    // Render
    display();
    
    // Update Game
    // Controls
    controls();
    newx_scroll = playerx-CENTER;
    playerframe = !(playerx==oldplayerx) ? playerx : (!(playery == oldplayery) ? playery : 0);
    playerframe = (((playerframe&32) ? ((playerframe&16) ? 0xd8 : 0xdc) : ((playerframe&16) ? 0xd8 : 0xe0)));
    if (!moving)
      playerframe = 0xd8;
    metasprite(playerx==oldplayerx ? 0xd8 : playerframe, 0 | (dir == LEFT ? OAM_FLIP_H : 0), playerx, playery);
  }
}
