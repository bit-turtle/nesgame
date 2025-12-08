
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
extern byte music_data_test_song[];

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

// Flags
#include "flags.h"
//#link "flags.c"

#define INITIAL_AREA 0
#define INITIAL_WEAPON 0
const byte weapon_damage[] = {3, 2, 4, 1};
#define DEFAULT_MAX_HEALTH 12
#define FOUNTAIN_HEALTH 4
// Global Variables
byte coins = 8;
void update_coins(byte new) {
  char disp[2] = "\x10 ";
  coins = new;
  if (coins == 0) {
    vrambuf_put(NTADR_A(22, 2), "  ", 2);
    return;
  }
  disp[1] = 0x30+coins;
  vrambuf_put(NTADR_A(22,2), disp, 2);
}
byte keys = 0;
void update_keys(byte num) {
  char disp[2] = "\x19 ";
  keys = num;
  disp[1] = 0x30 + keys;
  if (keys == 0) {
    vrambuf_put(NTADR_A(28,2), "  ", 2);
    return;
  }
  vrambuf_put(NTADR_A(28,2), disp, 2);
}
byte weapon = 0;
bool bow = false;
byte anim;
word t_scroll = 0;
bool offroad = false;
#define t_scroll_speed 16
word x_scroll = 0;
word newx_scroll;
byte dir = RIGHT;
#define PLAYER_WALK 2
#define PLAYER_RUN 3
#define PLAYER_WALK_ANIM_SPEED 1
#define PLAYER_RUN_ANIM_SPEED 2
#define PLAYER_RUN_MIN_HEALTH 6
#define HORSE 6
#define HORSE_STEAL_DISTANCE 12
#define SPIDER_DAMAGE 3
byte playerspeed = 0;
bool shield = false;
#define ATTACK_TIME 8
#define HIT_TIME (ATTACK_TIME-3)
byte attacktimer = 0;
bool attacking = false;
word playerx = 50;
byte playery = 0;
bool collision(word x1, byte y1, word x2, byte y2) {
  return (x1 < x2+8 && x1+8 >= x2 && y1 < y2+8 && y1+8 >= y2);
}
bool player_collision(word x, byte y) {
  return collision(x,y, playerx, playery);
}
bool horse = false;
bool moving = false;
word oldplayerx = 0;
byte oldplayery = 0;
byte MAX_PLAYER_HEALTH = 12;
#define INITIAL_HEALTH 12
byte playerhealth;
#define DAMAGE_COOLDOWN 12
byte damage_cooldown = 0;
#define PLAYER_KNOCKBACK 3
void damage(byte dmg) {
  byte i, j;
  playerx = oldplayerx+(dir == RIGHT? -PLAYER_KNOCKBACK : PLAYER_KNOCKBACK);
  playery = oldplayery;
  dir = dir==RIGHT? LEFT:RIGHT;
  if (damage_cooldown != 0)
    return;
  damage_cooldown = DAMAGE_COOLDOWN;
  playerhealth -= dmg;
  if (dmg != 255 && playerhealth > i) {
    playerhealth = 0;
  }
  i = playerhealth;
  if (playerhealth <= 1 && !(flags[BOBBERT]&PLAYER_SAVED))
    playerhealth = 1;
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

#define CENTER 119

#define HORSE_INDEX MAX_ENTITIES
EntityState current_entities[MAX_ENTITIES+1];
byte horse_area = 0;
// Pre def
void dialogue(char* name, char* text);
#define YES 1
#define NO 2
#define MAYBE 3
byte decision(char* title);
void load_area(byte newarea, word x, byte y);
void wait_frame();
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
    playerx = entity->x;
  }
}

void bobbert_walk(EntityState* entity) {
  // Anim
  entity->chr_offset = (anim&4) ? 4 : 0;
  if (entity->x > playerx+6*TILE_SIZE) {
    if (!(flags[BOBBERT]&PLAYER_SLOW)) {
      dialogue("?", "Are you coming or not?");
    	flags[BOBBERT] |= PLAYER_SLOW;
    }
    entity->chr_offset = 4;
  }
  else if (entity->x < 27*TILE_SIZE) {
    entity->x+=(playerx<entity->x-TILE_SIZE*2? 1: 2);
    if (entity->y > 6*TILE_SIZE-8)
      entity->y--;
    else if (entity->y < 6*TILE_SIZE-8)
      entity->y++;
  }
  else if (entity->y > 1*TILE_SIZE)
    entity->y-=2;
  else {
    entity->entity = 2;	// Standstill
    entity->chr_offset = 0;
    dialogue("?", "My soup can heal you");
    dialogue("Bobbert", "By the way, I'm Bobbert");
  }
}

void player_pushback() {
  playerx = oldplayerx;
  playery = oldplayery;
}

void bobbert_init(EntityState* entity) {
  if (flags[BOBBERT]&PLAYER_SAVED) {
    entity->x = 27*TILE_SIZE;
    entity->y = 1*TILE_SIZE;
    entity->entity = 2;
  }
}

void bobbert_save(EntityState* entity) {
  byte y;
  word x;
  byte t;
  if (current_entities[2].entity != 7) {
    // Anim
    entity->chr_offset = (anim&4) ? 4 : 0;
    sprite(entity->x-4, entity->y+(anim&8 ? 8 : 4), 0x84, 2 | OAM_FLIP_H | (anim&8 ? OAM_FLIP_V : 0));
    
    t = 0;
    for (t = 2; t < 5 && current_entities[t].entity == 0; t++); 
    if (current_entities[t].entity != 0) {
      y = current_entities[t].y;
      x = current_entities[t].x;
      if (collision(entity->x,entity->y,x,y)) {
        current_entities[t].entity = 0;
      }
    }
    else {
      entity->entity = 3;
      flags[BOBBERT] |= PLAYER_SAVED;
      dialogue("?", "Follow me, I can heal you");
     return;
   }
    
    if (entity->x > x)
      entity->x-=2;
    else if (entity->x < x)
      entity->x+=3;
    if (entity->y > y)
      entity->y-=2;
    else if (entity->y < y)
      entity->y+=2;
    
   
  }
}

void bobbert_soup(EntityState*) {
  if (flags[BOBBERT]&HORSE_STOLEN) {
    dialogue("Bobbert", "Horse thief, you want soup?");
    if (coins >= 2) {
      dialogue("Bobbert", "It seems you can pay.");
    	switch (decision("Give Bobbert the coins?")) {
          case YES:
            update_coins(coins-2);
            while(playerhealth < MAX_PLAYER_HEALTH) {
                  playerhealth++;
                  damage_cooldown = 0;
                  damage(0);
                  wait_frame();
                  wait_frame();
                  wait_frame();
                  wait_frame();
                  wait_frame();
                  wait_frame();
                  wait_frame();
                  wait_frame();
    		}
            wait_frame();
            if (!(flags[BOBBERT]&RESTAURANT_MENTIONED)) {
            	dialogue("Bobbert", "This gives me an idea ...");
              	dialogue("Bobbert", "I could make a restaurant!");
              flags[BOBBERT] |= RESTAURANT_MENTIONED;
            }
            break;
          case NO:
            dialogue("Bobbert", "Oh well...");
            break;
          case MAYBE:
            dialogue("Bobbert", "So you're considering it?");
            break;
        }
    }
    else dialogue("Bobbert", "Give me two coins.");
  }
  else {
    if (playerhealth != MAX_PLAYER_HEALTH) dialogue("Bobbert", "Go on, try the soup!");
    else if (!(flags[BOBBERT]&RESTAURANT_MENTIONED)) {
    	dialogue("Bobbert", "What did you think?");
      switch (decision("Was the soup good?")) {
        case YES:
          dialogue("Bobbert", "Thank you!");
          break;
        case NO:
          dialogue("Bobbert", "I'll try to make it better.");
          break;
        case MAYBE:
          dialogue("Bobbert", "I'll improve it for you!");
          break;
      }
      dialogue("Bobbert", "Anyways, I was thinking ...");
      dialogue("Bobbert", "I could make a restaurant!");
      flags[BOBBERT] |= RESTAURANT_MENTIONED;
    }
  }
  player_pushback();
}

void soup_pot(EntityState* e) {
  if (e != NULL && flags[BOBBERT]&HORSE_STOLEN)
    dialogue("Bobbert", "No soup for you!");
  else {
    while(playerhealth < MAX_PLAYER_HEALTH) {
      playerhealth++;
      damage_cooldown = 0;
      damage(0);
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
    }
    if (e != NULL && !(flags[BOBBERT]&HORSE_LENT)) {
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      wait_frame();
      dialogue("Bobbert", "Could you do a favor for me?");
      dialogue("Bobbert", "I need this letter delivered");
      dialogue("Bobbert", "Go quickly on my horse!");
      vrambuf_put(NTADR_A(26,2), "\x1e", 1);
      flags[BOBBERT] |= HAS_MESSAGE;
      flags[BOBBERT] |= HORSE_LENT;
    }
  }
  player_pushback();
}

void bobbert_house(EntityState*) {
  if (flags[BOBBERT]&HORSE_STOLEN)
    dialogue("Bobbert", "You stole my horse, thief.");
  else if (playerhealth == MAX_PLAYER_HEALTH)
     dialogue("Bobbert", "Come back any time!");
  else
    dialogue("Bobbert", "Go inside and try my soup!");
  player_pushback();
}

void spider_wait(EntityState* entity) {
  if (playerx+4*TILE_SIZE > entity->x && playerx < entity->x+4*TILE_SIZE)
    entity->entity = 8;
}

#define SPIDER_SPEED 4

void spider_attack(EntityState* entity) {
  if (playerx > entity->x)
    entity->x+=SPIDER_SPEED;
  else if (playerx < entity->x)
    entity->x-=SPIDER_SPEED;
  if (playery > entity->y)
    entity->y+=SPIDER_SPEED;
  else if (playery < entity->y)
    entity->y-=SPIDER_SPEED;
}

void spider_hit(EntityState* entity) {
  entity->entity = 9;
  damage(SPIDER_DAMAGE);
}

void spider_retreat(EntityState* entity) {
  entity->y += rand8()&7;
  entity->y -= rand8()&7;
  if (entity->x < playerx+TILE_SIZE*4)
    entity->x += rand8()&7;
  else
    entity->entity = 8;
}

void sign_read(EntityState*) {
  switch (area) {
    case 2:
      dialogue("Sign", "<-- Forest  ----  Turlin -->");
      break;
    case 3:
      dialogue("Welcome to Turlin!", "Population: 2");
      break;
    default:
      dialogue("Faded Sign", "The text is unreadable");
  };
  player_pushback();
}

void basic_sword_init(EntityState* entity) {
  if (weapon != 0)
    entity->entity = 0;
}

void basic_sword_collect(EntityState* entity) {
  weapon=1;
  entity->entity = 0;
}

void fountain_of_health(EntityState*) {
  switch(decision("Toss coin into fountain?")) {
    case YES:
      if (coins) {
      	dialogue("Plop.", "The coin lands in the water");
        update_coins(coins-1);
        MAX_PLAYER_HEALTH+=FOUNTAIN_HEALTH;
        dialogue("Max health increased", "You feel stronger somehow");
      }
      else
        dialogue("...", "You just tossed in some air");
      break;
    case NO:
      break;
    case MAYBE:
      dialogue("You stare at the fountain", "A few coins are inside.");
      break;
  }
  player_pushback();
}

void basic_die(EntityState* entity) {
  entity->entity = 0;
}

void basic_knockback(EntityState* entity) {
  if (dir == LEFT) entity->x-=TILE_SIZE*2;
  else entity->x+=TILE_SIZE*2;
}

void mayor_turt(EntityState*) {
  if (!(flags[TURLIN]&LE_INTRODUCTION)) {
  	flags[TURLIN] |= LE_INTRODUCTION;
  	dialogue("Turt Le", "Hello, I am Turt Le, ...");
    	dialogue("Mayor Le", "... Mayor of Turlin!");
    	wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
    wait_frame();
  }
  if (weapon == 0) {
        dialogue("Mayor Le", "You don't have a sword!");
        dialogue("Mayor Le", "It's dangerous to go alone.");
        dialogue("Mayor Le", "Take this.");
    	dialogue("Mayor Le", "Attack with \x13");
    	weapon = 4;
  }
  else if (!(flags[TURLIN]&LE_MONSTERS)) {
  	dialogue("Mayor Le", "Have you seen them?");
    	dialogue("Mayor Le", "The monsters.");
    	flags[TURLIN]|=LE_MONSTERS;
  }
  else {
    dialogue("Mayor Le", "Let me tell you a story.");
    dialogue("Mayor Le", "A few months ago,");
    dialogue("Mayor Le", "A monster came to Turlin.");
    dialogue("Mayor Le", "It tried to take a coin,");
    dialogue("Mayor Le", "One from the fountain.");
    dialogue("Mayor Le", "It just died on the spot.");
  dialogue("Mayor Le", "How strange ...");
    flags[TURLIN]|=LE_STORY;
  }
  player_pushback();
}

// Entities
const Entity entities[] = {
  // Null entity, nothing is rendered
  {0x1a,0,NULL,NULL},
  // Entities
  {0xe8, 0, horse_mount, NULL},	// 1: Horse
  // 2-5: Bobbert
  {0xfc, 0, bobbert_house, NULL}, // 2: Bobbert standstill
  {0xf8, 0, NULL, bobbert_walk}, // 3: Bobbert walk to house
  {0xf8, 0 | OAM_FLIP_H, NULL, bobbert_save, NULL, bobbert_init}, // 4: Bobbert saves the player
  {0xfc, 0 | OAM_FLIP_H, bobbert_soup, NULL}, // 5: Bobbert offers soup
  // 6: Soup
  {0xa8, 0, soup_pot, NULL},
  // 7-9: Spider
  {0xb0, 0, NULL, spider_wait, basic_die, NULL, basic_knockback},	// Spider wait
  {0xb0, 0, spider_hit, spider_attack, basic_die, NULL, basic_knockback},	// Spider attack
  {0xb0, 0, NULL, spider_retreat, basic_die, NULL, basic_knockback},	// Spider retreat
  // 10: Sign
  {0xb4, 1, sign_read, NULL, basic_die},
  // 11: Basic Sword
  {0x8c, 2, basic_sword_collect, NULL, NULL, basic_sword_init},
  // 12: Fountain of health
  {0xb8, 2, fountain_of_health, NULL},
  // 13: Mayor turt
  {0xbc, 2 | OAM_FLIP_H, mayor_turt, NULL},
  // 14: 
  
};


byte csong = 255;
void playsong(byte song) {
  if (song != csong)
    music_play(song);
  csong = song;
}

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x1A,0x19,0x29,0x00,	// background colors
  0x00,0x16,0x20,0x00,
  0x1A,0x17,0x2D,0x00,
  0x1C,0x21,0x2D,0x00,

  0x06,0x37,0x24,0x00,	// sprite colors
  0x16,0x36,0x34,0x00,
  0x0A,0x20,0x03,0x00,
  0x0F,0x27,0x2A
};

void wait_frame() {
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
  famitone_update();
}

void dialogue(char* name, char* text) {
  register byte counter, val;
  wait_frame();
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
    if (pad_poll(0)&PAD_B)
      break;
  }
  vrambuf_put(NTADR_B(2,2), text, val);
  counter = 0;
  do {
    counter++;
    if ((counter>>4)&1) vrambuf_put(NTADR_B(29,3), "A", 1);
    else vrambuf_put(NTADR_B(29,3), "\x3", 1);
    val = pad_poll(0);
    wait_frame();
  } while (!(val&PAD_A));
  vrambuf_put(NTADR_B(29,3), "\x3", 1);
  while (t_scroll > 0) {
    t_scroll -= t_scroll_speed;
    wait_frame();
  }
}

#define OPTION_TEXT "[  Yes      No       Maybe ]"

byte decision(char* title) {
  register byte counter, val;
  byte revali;
  byte answer = 0;
  wait_frame();
  vrambuf_put(NTADR_B(2,1), "                            ", 29);
  vrambuf_put(NTADR_B(2,1), title, strlen(title));
  vrambuf_put(NTADR_B(2,2), "                            ", 29);
  while (t_scroll < 256) {
    t_scroll += t_scroll_speed;
    wait_frame();
  } 
  val = strlen(OPTION_TEXT);
  for (counter = 0; counter < val*2; counter++) {
    vrambuf_put(NTADR_B(2+(counter>>1),2), OPTION_TEXT+(counter>>1), 1);
    wait_frame();
  }
  do {
    counter++;
    if ((counter>>4)&1)
      vrambuf_put(NTADR_B(29-5,3), answer == 0 ? "SELECT" : "\x3\x3\x3\x3\x3""A", 6);
    else if (answer == 0) vrambuf_put(NTADR_B(29-5,3), "\x3\x3\x3\x3\x3\x3", 6);
    else vrambuf_put(NTADR_B(29,3), " ", 1);
    revali = val;
    val = pad_poll(0);
    if (val&PAD_SELECT && (~revali)&PAD_SELECT) {
      if (answer != 0) vrambuf_put(NTADR_B(4+(answer-1)*9, 2), " ", 1);
    	answer++;
      if (answer > 3)
        answer = 1;
      vrambuf_put(NTADR_B(4+(answer-1)*9, 2), "\x3e", 1);
    }
    wait_frame();
  } while (answer == 0 || !(val&PAD_A));
  vrambuf_put(NTADR_B(29,3), "\x3", 1);
  while (t_scroll > 0) {
    t_scroll -= t_scroll_speed;
    wait_frame();
  }
  return answer;
}

void player_scroll() {
    newx_scroll = playerx-CENTER;
    // Limits
    if (newx_scroll > 0xfff)
      x_scroll = 0;
    else if (newx_scroll+0xff > TILE_SIZE*areas[area].width)
      x_scroll = areas[area].width*TILE_SIZE > 0xff ? (TILE_SIZE*areas[area].width-0xff) : 0;
    else
      x_scroll = newx_scroll;
}

void load_area(byte newarea, word x, byte y) {
  byte i;
  word final_scroll;
  wait_frame();
  area = newarea;
  playerx = x;
  playery = y;
  oldplayerx = x;
  oldplayery = y;
  player_scroll();
  final_scroll = x_scroll;
  x_scroll += 32*9;
  oam_hide_rest(4);
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
    if (entities[current_entities[i].entity].init != NULL)
      entities[current_entities[i].entity].init(&current_entities[i]);
  }
  playsong(areas[area].song);
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
      playery += 10;
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
  if (!horse && offroad || !flags[BOBBERT]&PLAYER_SAVED)
     playerspeed = PLAYER_WALK;
  if (attacktimer != 0)
    moving = false;
  if (moving) {
    moving = false;
    if (state&PAD_RIGHT) {
      playerx+=playerspeed;
      dir = RIGHT;
      if (playerx>>4>=areas[area].width-1) {
        playerx = areas[area].width-1<<4;
        if (areas[area].doors[RIGHT_AREA].area != NULL_AREA)
          load_area(areas[area].doors[RIGHT_AREA].area, areas[area].doors[RIGHT_AREA].x, areas[area].doors[RIGHT_AREA].y);
      
      }
      moving = true;
    }
    if (state&PAD_LEFT) {
      playerx-=playerspeed;
      dir = LEFT;
      if (playerx > 256<<4) {
        playerx = 0;
        if (areas[area].doors[LEFT_AREA].area != NULL_AREA)
          load_area(areas[area].doors[LEFT_AREA].area, areas[area].doors[LEFT_AREA].x, areas[area].doors[LEFT_AREA].y);
      }
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
    
    // Bobbert horse thief
    if (horse && !(flags[BOBBERT]&(HORSE_LENT|HORSE_STOLEN)) && (playerx > (28+HORSE_STEAL_DISTANCE)*TILE_SIZE || playerx < (28-HORSE_STEAL_DISTANCE)*TILE_SIZE)) {
      dialogue("Bobbert", "You stole my horse!");
      vrambuf_put(NTADR_A(26,2), "\x1f", 1);
      flags[BOBBERT] |= HORSE_STOLEN;
    }
  }
  
  // Attack
  if (state&PAD_A) {
  	if (!attacking && weapon != 0 && attacktimer == 0) {
    		attacktimer = ATTACK_TIME;
  	}
  }
  else if (attacking && attacktimer == 0) {
  	
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
  // Famitone
  famitone_init(music_data_test_song);
  //enable rendering
  ppu_on_all();
  
reset:
  // repeat forever
  load_area(INITIAL_AREA, 6*TILE_SIZE,6*TILE_SIZE-8);
  // Reset newx_scroll
  newx_scroll = x_scroll;
  renderx_scroll = x_scroll;
  // Initial health
  MAX_PLAYER_HEALTH = DEFAULT_MAX_HEALTH;
  playerhealth = DEFAULT_MAX_HEALTH-1;
  damage(255);
  damage_cooldown = 0;
  dir = RIGHT;
  // initial horse
  horse = false;
  weapon = INITIAL_WEAPON;
  // Initial Keys
  update_keys(keys);
  update_coins(coins);
  while(!(playerhealth == 0 && damage_cooldown == 0)) {
    if (attacktimer != 0) {
      attacktimer--;
      attacking = true;
    }
    else attacking = false;
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
  // reset oam_id
  oam_id = 4;
    
    // Update Game
    
    // Player walk
    
    if (attacktimer != 0) {
     sprite(playerx-4+(dir==RIGHT ? 16 : 0), playery+(attacktimer>ATTACK_TIME>>1 ? 9 : 2)-(horse?3:0), 0x83+weapon, 2 | (dir == RIGHT ? 0 : OAM_FLIP_H) | (attacktimer>ATTACK_TIME>>1 ? OAM_FLIP_V : 0));
    }
    if (horse) {
      metasprite((moving && anim&4) ? 0xec : 0xe4, ((damage_cooldown & 2) ? 1 : 0) | (dir == LEFT ? OAM_FLIP_H : 0), playerx, playery);
    }
    else {
      metasprite((moving) ? ((( (anim*(playerspeed == PLAYER_WALK ? PLAYER_WALK_ANIM_SPEED : PLAYER_RUN_ANIM_SPEED)) &8) ? (( (anim*(playerspeed == PLAYER_WALK ? PLAYER_WALK_ANIM_SPEED : PLAYER_RUN_ANIM_SPEED)) &4) ? 0xd8 : 0xdc) : (( (anim*(playerspeed == PLAYER_WALK ? PLAYER_WALK_ANIM_SPEED : PLAYER_RUN_ANIM_SPEED)) &4) ? 0xd8 : 0xe0))) : 0xd8, ((damage_cooldown & 2) ? 1 : 0) | (dir == LEFT ? OAM_FLIP_H : 0), playerx, playery);
    }
    
    // Render entities
    for (i = 0; i < MAX_ENTITIES + (area == horse_area ? 1 : 0); i++) {
      if (current_entities[i].entity != 0)
      // Render
      metasprite((entities[current_entities[i].entity].chr+current_entities[i].chr_offset), entities[current_entities[i].entity].attr, current_entities[i].x, current_entities[i].y);
    }
    
    // Hide unused sprites
    oam_hide_rest(oam_id);
    
    if (horse && !(flags[BOBBERT]&(HORSE_LENT|HORSE_STOLEN|HORSE_WARN))) {
    	dialogue("Bobbert", "That's Dave, my horse");
    	dialogue("Bobbert", "Just dismount with \x12");
      	flags[BOBBERT] |= HORSE_WARN;
    }
    
    // Process entities
    for (i = 0; i < MAX_ENTITIES + (area == horse_area ? 1 : 0); i++) {
      if (current_entities[i].entity == 0)
        continue;
      // Process
      if (attacktimer >= HIT_TIME && player_collision(current_entities[i].x, current_entities[i].y)) {
        current_entities[i].health-=weapon_damage[weapon];
        if (entities[current_entities[i].entity].hurt != NULL)
          entities[current_entities[i].entity].hurt(&current_entities[i]);
        if (current_entities[i].health > 200) {
          current_entities[i].health = 0;
          if (entities[current_entities[i].entity].die != NULL)
            entities[current_entities[i].entity].die(&current_entities[i]);
        }
      }
      else if (entities[current_entities[i].entity].collide != NULL && player_collision(current_entities[i].x, current_entities[i].y)) {
        entities[current_entities[i].entity].collide(&current_entities[i]);
      }
      else if (entities[current_entities[i].entity].tick != NULL) {
        entities[current_entities[i].entity].tick(&current_entities[i]);
      }
    }
    
    // Process tile
    i = tiles[collumns[areas[area].collumns[(playerx+8)>>4]].rows[(playery+8)>>4]].attr;
    if (i & FIRE)
      damage(1);
    if (i & SOLID) {
      player_pushback();
    }
    if (i & LOCKED)
      offroad = true;
    else
      offroad = false;
    if (i & DOOR ) {
      wait_frame();
      if (i&LOCKED && keys == 0) {
        dialogue("Door is locked", "It seems you need a key");
        playerx = oldplayerx;
        playery = oldplayery;
      }
      else if (horse && !(i&LARGE_DOOR) ) {
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
  for (i = 0; i < 16; i++)
    flags[i] = 0;
  dir = RIGHT;
  horse = false;
  horse_area = 0;
  current_entities[MAX_ENTITIES].entity = 0;
  goto reset;
}
