
// Offscreen Tiles

#define RIGHT 32
#define LEFT 0

extern word x_scroll;

// Get Attribute Table Address from Nametable Address
word nt2attraddr(word a);

// Set Metatile, y = height, ch = index in CHR ROM
void set_metatile(byte y, byte ch);

// Set Attribute Table Entry
void set_attr_entry(byte x, byte y, byte pal);

// Update the VRAM buffer with offscreen changes
void update_offscreen(byte dir);