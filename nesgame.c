
/*
	Another game for the nes
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
#include "music_songlist.inc"
extern byte music_data_nes_game_music[];

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

// BCD
#include "bcd.h"
//#link "bcd.c"

#define INITIAL_AREA 0

#define INITIAL_WEAPON 0
#define INITIAL_COMPASS false
#define INITIAL_HORSE false
const byte weapon_damage[] = {3, 2, 4, 1};
#define DEFAULT_MAX_HEALTH 12
#define FOUNTAIN_HEALTH 4
// Global Variables
#define INITIAL_COINS 0
#define INITIAL_KEYS 0

word deaths = 0;
void death() {
  char counter[4] = "\x26   ";
  deaths = bcd_add(deaths, 1);
  counter[1] = 0x30+((deaths>>8)&0xf);
  counter[deaths > 0x99 ? 2 : 1] = 0x30+((deaths>>4)&0xf);
  counter[deaths > 9 ? (deaths > 0x99 ? 3 : 2) : 1] = 0x30+(deaths&0xf);
  vrambuf_put(NTADR_A(16, 2), counter, 4);
}
char* death_messages[10] = {
	"Just a mild inconvenience",
  	"Pretend that didn't happen",
  	"Time travel saves the day",
  	"Maybe swing earlier?",
 	"This counter goes to 999",
  	"Running is strategic",
  	"It could have been worse",
  	"Timing is critical.",
  	"It saves while loading.",
  	"There are 10 of these",
};

byte coins;
bool doorinhibitor = false;
bool compass = false;
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
#define GREEN_SPIDER_DAMAGE 4
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
#define PLAYER_KNOCKBACK 6
#define WEAK_PLAYER_KNOCKBACK 3
#define WEAK_HIT_HP 1
byte damage_cooldown = 0;
bool knockback_flip = false;
void damage(byte dmg) {
  byte i, j;
  
  if (knockback_flip) dir = dir==RIGHT? LEFT:RIGHT;
  if (dmg > WEAK_HIT_HP)
    j = PLAYER_KNOCKBACK;
  else
    j = WEAK_PLAYER_KNOCKBACK;
  playerx = oldplayerx+(dir == RIGHT? -j : j);
  playery = oldplayery;
    if ( dir == RIGHT && oldplayerx < playerx || dir == LEFT && oldplayerx > playerx)
      playerx = oldplayerx;
    
  dir = dir==RIGHT? LEFT:RIGHT;
  knockback_flip = true;
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
  doorinhibitor = true;
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
    if (player_collision(entity->x, entity->y)) {
    	entity->x-=TILE_SIZE;
    }
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
      entity->x-=4;
    else if (entity->x < x)
      entity->x+=4;
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
      dialogue("Bobbert", "You can dismount with \x12");
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
    entity->entity = (entity->entity == 7) ? 8 : 21;
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
  byte i;
  switch (area) {
    case 2:
      dialogue("Sign", "<-- Forest ------ Turlin -->");
      break;
    case 3:
      dialogue("Welcome to Turlin!", "Population: 2");
      break;
    case 5:
      dialogue("End of the road.", "Continue if you dare");
      break;
    case 9:
      dialogue("Deep within the forest", "The great sword lies");
      break;
    case 11:
      dialogue("Look behind you", "Monsters.");
      for (i = 1; i <= 3; i++) {
      	current_entities[i].entity = 7;
        current_entities[i].x = TILE_SIZE*32;
        current_entities[i].y = (TILE_SIZE*3)+(TILE_SIZE*3-8)*(i-1);
        current_entities[i].health = 3;
      }
      for (i = 4; i <= 5; i++) {
      	current_entities[i].entity = 17;
        current_entities[i].x = TILE_SIZE*i*5;
        current_entities[i].y = TILE_SIZE*6-8;
      }
      break;
    case 20:
      dialogue("Position the turtles.", "The paths show the way.");
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
  load_area(14, TILE_SIZE*38, TILE_SIZE*5);
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
  if (dir == LEFT) entity->x-=TILE_SIZE*2+8;
  else entity->x+=TILE_SIZE*2+8;
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

void chest_collect(EntityState* entity) {
  switch(area) {
    case 8:
      compass = true;
      break;
    case 12:
      update_coins(coins+1);
      flags[FOREST] |= COINS_1;
      break;
    case 10:
      update_keys(keys+1);
      flags[FOREST] |= KEYS_1;
      break;
    case 14:
      update_coins(coins+1);
      flags[FOREST] |= COINS_2;
      break;
    case 19:
      update_keys(keys+1);
      flags[FOREST] |= KEYS_2;
      break;
    case 16:
      update_coins(coins+2);
      flags[FOREST] |= COINS_3;
      break;
  }
  entity->entity = 0;
}

void chest_init(EntityState* entity) {
  bool empty = false;
  switch (area) {
    case 8:
  if (compass)
    empty = true;
  	break;
    case 12:
      if (flags[FOREST]&COINS_1) empty = true;
      break;
    case 10:
      if (flags[FOREST]&KEYS_1) empty = true;
      break;
    case 14:
      if (flags[FOREST]&COINS_2) empty = true;
      break;
    case 19:
      if (flags[FOREST]&KEYS_2) empty = true;
      break;
    case 16:
      if (flags[FOREST]&COINS_3) empty = true;
      break;
      
      
  }
  if (empty)
    entity->entity = 0;
}

void solid_entity(EntityState*) {
	player_pushback();
}

void bat_fly(EntityState* entity) {
  entity->chr_offset = (anim&4) ? 4 : 0;
  if (entity->y+TILE_SIZE*2 > playery)
    entity->y--;
  else if (entity->x+TILE_SIZE*2 > playerx && entity->x < playerx+TILE_SIZE*2)
    entity->entity = 18;
  else
    entity->y += rand8()&1 ? -1 : 1;
  
  if (playerx > entity->x)
    entity->x++;
  else if (playerx < entity->x)
    entity->x--;
  else
    entity->x+= rand8()&1 ? -2 : 2;
  
}

void bat_attack(EntityState* entity) {
  if (playerx > entity->x)
    entity->x+=3;
  else if (playerx < entity->x)
    entity->x-=3;
  if (playery > entity->y)
    entity->y+=3;
  else if (playery < entity->y)
    entity->y-=3;
}

void bat_hit(EntityState* entity) {
  damage(1);
  entity->entity = 17;
}

void tunnel_door_inhibitor(EntityState* entity) {
  doorinhibitor = true;
  if (entity->x > TILE_SIZE*11 || entity->x < TILE_SIZE*9
	|| entity->y < TILE_SIZE*5 || entity->y > TILE_SIZE*7)
    doorinhibitor = false;
} 

void strong_spider_hit(EntityState* entity) {
  damage(GREEN_SPIDER_DAMAGE);
  entity->entity = 22;
}

void strong_spider_attack(EntityState* entity) {
  byte sign = 0;
  if (entity->memory < 10) entity->memory++;
  if (playerx > entity->x)
    sign += 1;
  else 
    sign -= 1;
  if (entity->x > playerx) {
    entity->x -= entity->memory;
  }
  else if (entity->x < playerx) {
    entity->x += entity->memory;
  }
  if (playerx > entity->x)
    sign += 1;
  else 
    sign -= 1;
  if (sign == 0)
    entity->x = playerx;
  
  if (entity->y > playery) {
  	entity->y -= entity->memory;
  }
  else if (entity->y < playery) {
  	entity->y += entity->memory;
  }
}

void strong_spider_retreat(EntityState* entity) {
  entity->memory = 0;
  entity->x += 12;
  entity->y += rand8()&7;
  entity->y -= rand8()&7;
  if (entity->y > playery)
    entity->y += 1;
  if (entity->y < playery)
    entity->y -= 1;
  
  if (entity->x > playerx+TILE_SIZE*4)
    entity->entity = 21;
}

void maze_puzzle_solved(EntityState* entity) {
  bool solved = true;
  doorinhibitor = true;
  if (current_entities[0].y != TILE_SIZE*8)
    solved = false;
  
  if (current_entities[1].y != TILE_SIZE*8)
    solved = false;
  
  if (current_entities[2].y != TILE_SIZE*3)
    solved = false;
    
  if (solved) {
    doorinhibitor = false;
    entity->entity = 0;
  }
}

void puzzle_pushable(EntityState* entity) {
  entity->y += playery-oldplayery;
  playerx = oldplayerx;
  if (entity->y < TILE_SIZE*3) {
    entity->y = TILE_SIZE*3;
    playery = oldplayery;
  }
  if (entity->y > TILE_SIZE*8) {
    entity->y = TILE_SIZE*8;
    playery = oldplayery;
  }
}

void ball_hit(EntityState* entity) {
  entity->entity = 0;
  damage(1);
}

void ball_tick(EntityState* entity) {
  entity->x-=(entity->memory&0b10000000) ? -2 : 2;
  if (entity->memory&0b01000000)
    entity->y -= entity->memory&0xf;
  else
    entity->y += entity->memory&0xf;
  if (entity->x < TILE_SIZE*1-8 || entity->x > TILE_SIZE*15-8) {
    entity->memory^=0b10000000;
  }
  if (entity->y < TILE_SIZE*2-8 || entity->y > TILE_SIZE*10-8) {
    entity->memory^=0b01000000;
  }
}

void ball_whack(EntityState* entity) {
  entity->entity = 26;
}

void ball_flyback(EntityState* entity) {
  entity->x += 8;
  
  // Course Correction
  if (entity->y > TILE_SIZE*6-8)
    entity->y-=2;
  if (entity->y < TILE_SIZE*6-8)
    entity->y+=2;
  
  if (collision(entity->x, entity->y, current_entities[MAX_ENTITIES-1].x, current_entities[MAX_ENTITIES-1].y)) {
    entity->entity = 0;
    current_entities[MAX_ENTITIES-2].memory--;
  }
  if (entity->x > TILE_SIZE*16) {
  	entity->entity = 0;
  }
}

void boss_tick(EntityState* entity) {
  bool summon = true;
  byte i = 0;
  for (i = 0; i < 4; i++) {
  	if (current_entities[i].entity != 0)
          summon = false;
  }
  if (summon && entity->memory == 0)
    current_entities[MAX_ENTITIES-1].memory++;
  entity->chr_offset = 4;
  if (summon) switch (current_entities[MAX_ENTITIES-1].memory) {
    case 1:
      current_entities[0].entity = 25;
      current_entities[0].x = TILE_SIZE*14;
      current_entities[0].y = TILE_SIZE*6-8;
      current_entities[0].memory = 0;
      current_entities[0].chr_offset = 0;
      entity->memory = 1;
      break;
    case 2:
      current_entities[0].entity = 25;
      current_entities[0].x = TILE_SIZE*14;
      current_entities[0].y = TILE_SIZE*6-8;
      current_entities[0].memory = 1;
      current_entities[0].chr_offset = 0;
      current_entities[1].entity = 25;
      current_entities[1].x = TILE_SIZE*14;
      current_entities[1].y = TILE_SIZE*6-8;
      current_entities[1].memory = 0b01000010;
      current_entities[1].chr_offset = 0;
      entity->memory = 2;
      break;
    case 3:
      current_entities[0].entity = 25;
      current_entities[0].x = TILE_SIZE*14;
      current_entities[0].y = TILE_SIZE*6-8;
      current_entities[0].memory = 5;
      current_entities[0].chr_offset = 0;
      current_entities[1].entity = 25;
      current_entities[1].x = TILE_SIZE*14;
      current_entities[1].y = TILE_SIZE*6-8;
      current_entities[1].memory = 0b01000100;
      current_entities[1].chr_offset = 0;
      entity->memory = 2;
      break;
    case 13:
      current_entities[0].entity = 25;
      current_entities[0].x = TILE_SIZE*14;
      current_entities[0].y = TILE_SIZE*6-8;
      current_entities[0].memory = 1;
      current_entities[0].chr_offset = 0;
      entity->memory = 1;
      current_entities[1].entity = 17;
      current_entities[1].x = TILE_SIZE*14;
      current_entities[1].y = TILE_SIZE*3;
      break;
    case 34:
      
      current_entities[0].entity = 25;
      current_entities[0].x = TILE_SIZE*14;
      current_entities[0].y = TILE_SIZE*6-8;
      current_entities[0].memory = 4;
      current_entities[0].chr_offset = 0;
      current_entities[1].entity = 25;
      current_entities[1].x = TILE_SIZE*14;
      current_entities[1].y = TILE_SIZE*6-8;
      current_entities[1].memory = 0b01000100;
      current_entities[1].chr_offset = 0;
      entity->memory = 3;  
      current_entities[2].entity = 25;
      current_entities[2].x = TILE_SIZE*14;
      current_entities[2].y = TILE_SIZE*6-8;
      current_entities[2].memory = 0;
      current_entities[2].chr_offset = 0;
      break;
    case 100:
      
      current_entities[0].entity = 8;
      current_entities[0].x = TILE_SIZE*14;
      current_entities[0].y = TILE_SIZE*8;
      current_entities[0].chr_offset = 0;
      current_entities[1].entity = 8;
      current_entities[1].x = TILE_SIZE*14;
      current_entities[1].y = TILE_SIZE*3;
      current_entities[1].chr_offset = 0;
      break;
      
    case 255:
      // Melee
      entity->entity = 30;
      current_entities[MAX_ENTITIES-1].entity = 0;
      break;
  }
  else entity->chr_offset = 0;
}

void sword_lowering(EntityState* entity) {
  entity->y+=2;
  if (entity->y == TILE_SIZE*6-8) {
  	entity->y = TILE_SIZE*6-8;
    	entity->entity = 11;
  }
}

void boss_melee_fly(EntityState* entity) {
  entity->chr_offset = (anim&4) ? 4 : 0;
  if (entity->y+TILE_SIZE*2 > playery)
    entity->y--;
  else if (entity->x+TILE_SIZE*2 > playerx && entity->x < playerx+TILE_SIZE*2)
    entity->entity = 31;
  else
    entity->y += rand8()&1 ? -1 : 1;
  
  if (playerx > entity->x)
    entity->x+=4;
  else if (playerx < entity->x)
    entity->x-=4;
  else
    entity->x+= rand8()&1 ? -2 : 2;
}

void boss_melee_attack(EntityState* entity) {
  if (playerx > entity->x) {
    entity->x+=6;
  }
  else if (playerx < entity->x) {
    entity->x-=6;
  }
  if (playery > entity->y) {
    entity->y+=4;
  }
  else if (playery < entity->y) {
    entity->y-=3;
  }
  
}

void boss_retreat(EntityState* entity) {
  entity->x+=8;
  if (entity->x >= TILE_SIZE*15)
    entity->entity = 30;
}

void boss_hit(EntityState* entity) {
  damage(2);
  entity->entity = 32;
}

void boss_die(EntityState* entity) {
  entity->entity = 0;
  
  current_entities[0].entity = 29;
  current_entities[0].x = TILE_SIZE*8-8;
  current_entities[0].y = -TILE_SIZE;
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
  // 14: Chest
  {0xc8, 0, chest_collect, NULL, NULL, chest_init, NULL},
  // 15: Turtle Statue right facing
  {0xbc, 0, solid_entity},
  // 16: Turtle Statue left facing
  {0xbc, 0 | OAM_FLIP_H, solid_entity},
  // 17: Bat eye fly
  {0xf0, 2, NULL, bat_fly, basic_die, NULL, basic_knockback},
  // 18: Bat eye attack
  {0xf0, 2, bat_hit, bat_attack, basic_die, NULL, basic_knockback},
  // 19: Pushable Grass Cover
  {0x90, 2, pushable, tunnel_door_inhibitor},
  // 20: Stronger Spider wait
  {0xb0, 2, NULL, spider_wait},
  // 21: Stonger Spider attack
  {0xb0, 2, strong_spider_hit, strong_spider_attack, basic_die, NULL, basic_knockback},
  // 22: Stronger Spider retreat
  {0xb0, 2, NULL, strong_spider_retreat, basic_die, NULL, basic_knockback},
  // 23: Pushable Brick
  {0xbc, 0, puzzle_pushable, NULL},
  // 24: Puzzle solver door inhibitor
  {0x90, 1, NULL, maze_puzzle_solved},
  // 25: Boss1 Projectile
  {0xc, 2, ball_hit, ball_tick, ball_whack, NULL, NULL},
  // 26: Boss1 Projectile FLyback
  {0xc, 2, NULL, ball_flyback, NULL, NULL, NULL},
  // 27: Boss1 Legs (Spider)
  {0xb0, 2},
  // 28: Boss1 Head (Bat)
  {0xf0, 0, NULL, boss_tick},
  // 29: Sword lowering
  {0x8c, 2, NULL, sword_lowering},
  // 30: Boss1 Melee fly
  {0xf0, 0, NULL, boss_melee_fly, boss_die},
  // 31: Boss1 Melee attack
  {0xf0, 0, boss_hit, boss_melee_attack, boss_die},
  // 32: Boss1 Melee retreat
  {0xf0, 0, NULL, boss_retreat, boss_die},
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

// Save data
byte save_coins;
bool save_compass;
bool save_horse;
EntityState save_horse_pos;
byte save_coins;
byte save_health;
byte save_max_health;
byte save_weapon;
byte save_keys;
word save_x;
byte save_y;
bool save_bow;
byte save_dir;
byte save_flags[16];

void load_area(byte newarea, word x, byte y) {
  byte i;
  word final_scroll;
  wait_frame();
  area = newarea;
  playerx = x;
  playery = y;
  oldplayerx = x;
  oldplayery = y;
  save_x = x;
  save_y = y;
  save_keys = keys;
  save_weapon = weapon;
  save_health = playerhealth;
  save_max_health = MAX_PLAYER_HEALTH;
  save_horse = horse;
  save_coins = coins;
  save_compass = compass;
  save_bow = bow;
  save_dir = dir;
  save_horse_pos = current_entities[HORSE_INDEX];
  for (i = 0; i < 16; i++) {
  	save_flags[i] = flags[i];
  }
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
  oam_spr(0, 30, 1, 0, 0);
  // vrambuf initialization
  vrambuf_clear();
  set_vram_update(updbuf);
  // display
  nmi_set_callback(start_frame);
  // Famitone
  famitone_init(music_data_nes_game_music);
  //enable rendering
  ppu_on_all();
  
  knockback_flip = false;
  vrambuf_put(NTADR_A(0,2), "                                 ", 32);
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
  dir = LEFT;
  // initial horse
  horse = INITIAL_HORSE;
  compass = INITIAL_COMPASS;
  weapon = INITIAL_WEAPON;
  // Initial Keys
  keys = INITIAL_KEYS;
  coins = INITIAL_COINS;
  update_keys(keys);
  update_coins(coins);
  
  save_loaded:
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
    
    if (knockback_flip) {
      	dir = (dir == LEFT) ? RIGHT : LEFT;
    	knockback_flip = false;
    }
    // Controls
    controls();
    anim++;
    
    // Draw Compass
    if (compass) {
    i = areas[area].dir;
    if (areas[area].dir == UNKNOWN)
      i = (anim>>2)&0b11;
    i += 0x8;
    vrambuf_put(NTADR_A(25,2), &i, 1);
      }
    
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
    doorinhibitor = false;
    for (i = 0; i < MAX_ENTITIES + (area == horse_area ? 1 : 0); i++) {
      if (current_entities[i].entity == 0)
        continue;
      // Process
      if (attacktimer >= HIT_TIME && player_collision(current_entities[i].x, current_entities[i].y)) {
        current_entities[i].health-=weapon_damage[weapon-1];
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
    if (i & OFFROAD)
      offroad = true;
    else
      offroad = false;
    if (i & DOOR && !doorinhibitor) {
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
  dialogue("GAME OVER", death_messages[deaths&0xf]);
  vrambuf_put(NTADR_A(0,2), "                                 ", 32);
  death();
  for (i = 0; i < 16; i++)
    flags[i] = save_flags[i];
  horse = save_horse;
  update_coins(save_coins);
  update_keys(save_keys);
  playerhealth = save_health;
  damage(0);
  knockback_flip = false;
  damage_cooldown = 0;
  MAX_PLAYER_HEALTH = save_max_health;
  horse = save_horse;
  weapon = save_weapon;
  compass = save_compass;
  bow = save_bow;
  dir = save_dir;
  
  i = ' ';
  if (!compass)
    vrambuf_put(NTADR_A(25,2), &i, 1);
  if (!flags[BOBBERT]&HORSE_STOLEN && !flags[BOBBERT]&HORSE_LENT)
    vrambuf_put(NTADR_A(26,2), &i, 1);
  
  current_entities[HORSE_INDEX] = save_horse_pos;
  
  
  load_area(area, save_x, save_y);
  goto save_loaded;
}
