#ifndef _GAME_STRUCTDEF_H
#define _GAME_STRUCTDEF_H
#include "structdef_engine.h"


/* Generic Struct For Living Object */

struct living {
	int alive;
	int HP, power;
	float defence;
	int max_HP;
	int max_PP;
	int invincibility; /* 0 = Normal, 1 = hurt, 2 = invincible */
	float invincible_since;
	float iframes_duration;
	int invincibility_toggle;
	void *self;
};


struct laser_struct {
	int power;
	int count;
	int on;
	int turnon;
	int turnoff;
};

struct sword_struct {
	int power;
	int count;
	int down;
	int swing;
	int swing_up_duration;
	int swing_down_duration;
	float dist_to_monster_spawn;
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
};

/* Status of the Level */

struct lane_struct {
	int total;
	int currentlane;
	float lanewidth;
	float *laneheight;	//TODO: Make float
	struct visual_container_struct *containers;
};

struct level_struct {
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	int score;
	int gameover;
	int levelover;
	int pauselevel; //level.pauselevel changed by levelfunc()
	int currentlevel;
	int partymode;
	struct lane_struct lanes;
	struct std_list **monster_list_stacks;
	struct std_list *monster_list_stack_end;//TODO: TMP
	struct laser_struct laser;
	struct sword_struct sword;
	struct level_var_struct *vars;
	struct level_effects_struct *effects;
	struct object_spawn_array_struct *object_spawn_arrays;
	struct ui_struct *ui;

	struct graphical_stage_child_struct *stage;

	struct xy_struct grid;
	int currentscreen;
	float *laneheight;
};

struct level_var_struct {

	/* Below here are not used anymore: */
	struct mutex_list_struct *mutexes;
	int soundstatus;
	/* Event handling */
	int directionbuttonlist[4];
	int history [4];
	int histwrite;
	int histread;
	int actionbuttonlist [4]; //a, b, start, select
	int acthistory [4];
	int acthistwrite;
	int acthistread;
};


/* Status of the Player */

struct player_struct {

	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	int invinciblecounter[2];
	int sword;
	int flydir;
	struct living living;
};

struct monster_struct {
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	struct living living;
	struct std_list *monster_stack_location;
	struct std_list **monster_list_stack_ptr;
	float entrybeat; //TODO: TMP
};


struct item {
	int itemnumber;
	int *int1;
	int int2;
	void *otherdata;
	void *functionptr;
	int Src[2];
	int wh[2];
	SDL_Texture **image;
	//void (*functionptr)(int int1, int int2);
};

struct ui_bar {
	int *amount;
	int *max;
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
};

/* Needed for death explosion animation: */
struct explosion_data_struct {
	struct animation_struct *animation;
	struct living *living;
};

enum return_codes_e { R_SUCCESS, R_FAILURE, R_RESTART_LEVEL, R_LOOP_LEVEL, R_PAUSE_LEVEL, R_QUIT_TO_DESKTOP, R_CASCADE_UP=100, R_CASCADE_UP_MAX=199, R_STARTSCREEN=200, R_LEVELS=201 };

enum hook_type_e {FRAME, LEVEL_INIT, LEVEL_LOOP};
#endif
