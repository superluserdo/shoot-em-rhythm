#ifndef _GAME_LEVEL_H
#define _GAME_LEVEL_H
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef_game.h"
#include "main.h"
#include "audio.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h" // Can hopefully get rid of this soon
#include "spawn.h"
#include "object_logic.h"
//#include "deprecated_funcs.h"

#define PXPB 83.24
#define SPRITE_PATH "../art/selfsprite.png"
#define TILE_PATH "../art/selftile.png"
#define LASER_PATH "../art/lasers.png"
#define SWORD_PATH "../art/swords.png"
#define LASER_HEIGHT 20
#define LASER_SEPARATOR_X 38
#define LASER_SEPARATOR_Y 20
#define POKESPRITE_SIZEX    16
#define POKESPRITE_SIZEY    22
#define SPRITE_SIZE    32
#define TILE_SIZE     32
#define VISIBLE_GRID_X 16
#define VISIBLE_GRID_Y 10
#define LOCAL_GRID_X VISIBLE_GRID_X + 2
#define LOCAL_GRID_Y VISIBLE_GRID_Y + 2
#define TOTAL_LANES 5
#define MAX_MONS_PER_LANE_PER_SCREEN 20
#define MAX_ITEMS_PER_LANE_PER_SCREEN 20
#define SCORE_DIGITS 5

/*	Global Variables	*/

extern struct level_struct *level;

/* Objects */

/* Monsters */

struct object_spawn_array_struct *level_init_object_spawn_arrays(void *level_setting_void, int num_lanes);

void quitlevel(struct status_struct status);
#endif
