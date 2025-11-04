
/*
Demonstrates the Famitone2 library for sound and music.
Press controller buttons to hear sound effects.
*/

// Neslib
#include "neslib.h"

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


/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x17,0x1A,0x2A,0x00,	// background colors
  0x00,0x10,0x20,0x00,
  0x06,0x16,0x26,0x00,

  0x16,0x35,0x24,0x00,	// sprite colors
  0x00,0x37,0x25,0x00,
  0x0F,0x2D,0x1A,0x00,
  0x0F,0x27,0x2A
};

void render_collumn(Collumn* collumn){
  register byte y;
  register Tile* tile;
  for (y = 0; y < 12; y++) {
    tile = (Tile*)&tiles[collumn->rows[y]];
    set_metatile(y, tile->chr);
    set_attr_entry(x_scroll>>3, y, tile->attr);
  }
}

void dialogue(char* name, char* text) {
  vrambuf_put(NTADR_B(2,1), name, strlen(name));
  vrambuf_put(NTADR_B(2,2), text, strlen(text));
}

word t_scroll = 0;
word x_scroll = 0;

byte area = 0;
byte dir = RIGHT;

void main(void) {
  // Set Pallete
  pal_all(PALETTE);
  // VRAM Initialization
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
  oam_spr(0, 30, 0xa0, 0, 0);
  // vrambuf initialization
  vrambuf_clear();
  set_vram_update(updbuf);
  //enable rendering
  ppu_on_all();
  // repeat forever
  while(1) {
    // ensure VRAM buffer is cleared
    ppu_wait_nmi();
    vrambuf_clear();
    // scroll top bar
    scroll(t_scroll, 0);
    // split at sprite zero and set X scroll
    split(x_scroll, 0);
    // Scroll
    t_scroll++;
    x_scroll++;
    // Update Offscreen Tiles
    if (x_scroll&8) {
      // Render
      render_collumn((Collumn*)&collumns[areas[area].collumns[x_scroll>>4]]);
      // Update
      update_offscreen(RIGHT);
    }
    // End VRAM Update
    vrambuf_end();
  }
}
