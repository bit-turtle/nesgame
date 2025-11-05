#include "neslib.h"
#include "vrambuf.h"
#include <string.h>
#include "offscreen.h"

// number of rows in scrolling playfield (without status bar)
#define PLAYROWS 24

// buffers that hold vertical slices of nametable data
char ntbuf1[PLAYROWS];	// left side
char ntbuf2[PLAYROWS];	// right side

// a vertical slice of attribute table entries
char attrbuf[PLAYROWS/4];

// Clear attribute table buffer
void clear_attrbuf() {
  memset(attrbuf, 0, sizeof(attrbuf));
}

// convert from nametable address to attribute table address
word nt2attraddr(word a) {
  return (a & 0x2c00) | 0x3c0 |
    ((a >> 4) & 0x38) | ((a >> 2) & 0x07);
}

// draw metatile into nametable buffers
// y is the metatile coordinate (row * 2)
// ch is the starting tile index in the pattern table
void set_metatile(byte y, byte ch) {
  ntbuf1[y*2] = ch;
  ntbuf1[y*2+1] = ch+1;
  ntbuf2[y*2] = ch+2;
  ntbuf2[y*2+1] = ch+3;
}

// set attribute table entry in attrbuf
// x and y are metatile coordinates
// pal is the index to set
void set_attr_entry(byte x, byte y, byte pal) {
  if (y&1) pal <<= 4;
  if (x&1) pal <<= 2;
  attrbuf[y/2] |= pal;
}

// write attribute table buffer to vram buffer
void put_attr_entries(word addr) {
  byte i;
  for (i=0; i<PLAYROWS/4; i++) {
    VRAMBUF_PUT(addr, attrbuf[i], 0);
    addr += 8;
  }
  vrambuf_end();
}

// update the nametable offscreen
// called every 8 horizontal pixels
void update_offscreen(byte dir) {
  register word addr;
  byte x;
  // divide x_scroll by 8
  // to get nametable X position
  x = ( (x_scroll>>3) - dir) & 63;
  // get address in either nametable A or B
  if (x < 32)
    addr = NTADR_A(x, 4);
  else
    addr = NTADR_B(x&31, 4);
  // Left Metatile Slice
  vrambuf_put(addr | VRAMBUF_VERT, ntbuf1, PLAYROWS);
  // Right Metatile Slice
  vrambuf_put((addr+1) | VRAMBUF_VERT, ntbuf2, PLAYROWS);
  // Update Attribute Table
  put_attr_entries(nt2attraddr(addr));
}
