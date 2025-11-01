// Game

// NESLib
#include "neslib.h"

// VRAM Buffer
#include "vrambuf.h"
//#link "vrambuf.c"

// CHR ROM
//#link "chr_rom.s"

// FamiStudio
#include "famistudio_cc65.h"
//#link "famistudio_ca65.s"

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x03,			// screen color

  0x25,0x30,0x27,0x00,	// background colors
  0x1C,0x20,0x2C,0x00,
  0x00,0x10,0x20,0x00,
  0x06,0x16,0x26,0x00,

  0x16,0x35,0x24,0x00,	// sprite colors
  0x00,0x37,0x25,0x00,
  0x0D,0x2D,0x1A,0x00,
  0x0D,0x27,0x2A
};

// main function, run after console reset
void main(void) {
  // set palette colors
  pal_all(PALETTE);
  
  // enable PPU rendering (turn on screen)
  ppu_on_all();

  while(1){}
}
