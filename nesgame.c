
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
byte keys = 4;
void update_keys(byte num) {
  char disp[2] = "\x19 ";
  keys = num;
  disp[1] = 0x30 + keys;
  vrambuf_put(NTADR_A(28,2), disp, 2);
}
byte anim;
word t_scroll = 0;
#define t_scroll_speed 16
word x_scroll = 0;
word newx_scroll;
byte dir = RIGHT;
#define PLAYER_WALK 2
#define PLAYER_RUN 3
#define PLAYER_RUN_MIN_HEALTH 4
#define HORSE 6
byte playerspeed = 6;
word playerx = 50;
byte playery = 0;
bool horse = false;
bool moving = false;
word oldplayerx = 0;
byte oldplayery = 0;
#define MAX_PLAYER_HEALTH 12
byte playerhealth = MAX_PLAYER_HEALTH;
#define DAMAGE_COOLDOWN 16
byte damage_cooldown = 0;
void damage(byte dmg) {
  byte i, j;
  if (damage_cooldown != 0)
    return;
  damage_cooldown = DAMAGE_COOLDOWN;
  i = playerhealth;
  playerhealth -= dmg;
  if (playerhealth > i)
    playerhealth = 0;
  // Player health
  for (i = 0; i < MAX_PLAYER_HEALTH; i+=4) {
    if (i > playerhealth)
      j = 0x14;
    else if (playerhealth - i > 4)
      j = 0x18;
    else
      j = 0x14+(playerhealth-i)%5;
    vrambuf_put(NTADR_A(2+(i>>2),2), &j, 1);
  }
}
// Pre def
void dialogue(char* name, char* text);
void load_area(byte newarea, word x, byte y);
// Entity Callbacks
void pushable(EntityState* entity) {
  entity->x += playerx-oldplayerx;
  entity->y += playery-oldplayery;
}

void horse_mount(EntityState* entity) {
  if (!horse) {
    horse = true;
    entity->entity = 0;
    playery = entity->y;
  }
}

void random_stroll(EntityState* entity) {
  entity->x += rand8()%3-1;
  entity->y += rand8()%3-1;
}

// Entities
const Entity entities[] = {
  // Null entity, nothing is rendered
  {0x1a,0,NULL,NULL},
  // Entities
  {0xe8, 0, horse_mount, NULL},	// 1: Horse
  {0xf4, 0, pushable, 0} // 2: Pushable block
};

#define CENTER 119

#define HORSE_INDEX MAX_ENTITIES
EntityState current_entities[MAX_ENTITIES+1];
byte horse_area = 0;

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x1A,0x19,0x29,0x00,	// background colors
  0x00,0x16,0x20,0x00,
  0x1A,0x17,0x2D,0x00,
  0x0F,0x27,0x2A,0x00,

  0x06,0x37,0x24,0x00,	// sprite colors
  0x16,0x36,0x34,0x00,
  0x29,0x2B,0x1A,0x00,
  0x0F,0x27,0x2A
};

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
  val = 0;
  do {
    counter++;
    if (playerhealth != 0) {
    	if ((counter>>4)&1) vrambuf_put(NTADR_B(29,3), "A", 1);
    	else vrambuf_put(NTADR_B(29,3), "\x3", 1);
    	val = pad_poll(0);
    }
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

void load_area(byte newarea, word x, byte y) {
  byte i;
  word final_scroll;
  area = newarea;
  playerx = x;
  playery = y;
  player_scroll();
  final_scroll = x_scroll;
  x_scroll += 32*9;
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
    // The one horse to rule them all
    if (areas[area].entities[i].entity == 1 && (horse || current_entities[HORSE_INDEX].entity == 1))
      current_entities[i].entity = 0;
    else
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
  moving = true;
  if (state&PAD_B)
    if (horse) {
      current_entities[HORSE_INDEX].entity = 1;
      current_entities[HORSE_INDEX].x = playerx;
      current_entities[HORSE_INDEX].y = playery;
      horse_area = area;
      playery += 8;
      horse = false;
      playerspeed = 0;
      moving = false;
    }
    else if (playerhealth >= PLAYER_RUN_MIN_HEALTH)
      playerspeed = PLAYER_RUN;
    else
      playerspeed = PLAYER_WALK;
  else
    playerspeed = horse ? HORSE : PLAYER_WALK;
  if (moving) {
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
}

void main(void) {
  word renderx_scroll;
  byte i;
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
  load_area(0, 5*TILE_SIZE,6*TILE_SIZE-8);
  // Reset newx_scroll
  newx_scroll = x_scroll;
  renderx_scroll = x_scroll;
  // Initial health
  playerhealth = MAX_PLAYER_HEALTH;
  damage(0);
  damage_cooldown = 0;
  // initial horse
  
  // Initial Keys
  update_keys(keys);
  while(!(playerhealth == 0 && damage_cooldown == 0)) {
    // Scroll
    // Update Offscreen Tiles
    // Render
    render_collumn(dir);
    // Update
    
    // Render
    
    update_offscreen(dir); 
    // Controls
    controls();
    anim++;
    
    wait_frame();
    player_scroll();
    
    // Update Game
    
    // Player walk
    if (horse)
      metasprite((moving && anim&4) ? 0xec : 0xe4, ((damage_cooldown & 2) ? 1 : 0) | (dir == LEFT ? OAM_FLIP_H : 0), playerx, playery);
    else
      metasprite((moving) ? ((( (anim*(playerspeed-1)) &8) ? (( (anim*(playerspeed-1)) &4) ? 0xd8 : 0xdc) : (( (anim*(playerspeed-1)) &4) ? 0xd8 : 0xe0))) : 0xd8, ((damage_cooldown & 2) ? 1 : 0) | (dir == LEFT ? OAM_FLIP_H : 0), playerx, playery);
    
    // Render entities
    for (i = 0; i < MAX_ENTITIES + (area == horse_area ? 1 : 0); i++) {
      // Render
      metasprite(entities[current_entities[i].entity].chr, entities[current_entities[i].entity].attr, current_entities[i].x, current_entities[i].y);
    }
    
    // Hide unused sprites
    oam_hide_rest(oam_id);
    
    // Process entities
    for (i = 0; i < MAX_ENTITIES + (area == horse_area ? 1 : 0); i++) {
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
    
    // Process tile
    i = tiles[collumns[areas[area].collumns[(playerx+8)>>4]].rows[(playery+8)>>4]].attr;
    if (i & FIRE)
      damage(1);
    if (i & SOLID) {
      playerx = oldplayerx;
      playery = oldplayery;
    }
    if (i & DOOR ) {
      wait_frame();
      if (i&LOCKED && keys == 0) {
        dialogue("Door is locked", "It seems you need a key");
        playerx = oldplayerx;
        playery = oldplayery;
      }
      else if (horse) {
        dialogue("Horse is too big", "Your horse can't fit!");
        playerx = oldplayerx;
        playery = oldplayery;
      }
      else {
        if (i&LOCKED) {
          dialogue("Door unlocked!", "The small key turns to dust");
          update_keys(keys-1);
        }
        i = DoorIndex(i);
        load_area(areas[area].doors[i].area,areas[area].doors[i].x,areas[area].doors[i].y);
      }
    }
    
    // Damage Cooldown
    
    if (damage_cooldown != 0)
      damage_cooldown--;
    
  }
  wait_frame();
  // Game Over
  dialogue("GAME OVER", "Better Luck Next Time!");
}
