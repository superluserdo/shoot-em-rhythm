#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h" // Can hopefully get rid of this soon


//TODO: Layers functionality. Include spawnLayer() function (above or below)
/*	Global Variables	*/
struct status_struct status;

/* Time */

struct lane_struct lanes = {
	.total = TOTAL_LANES
};

int levelfunc (SDL_Window *win, SDL_Renderer *renderer) {//(int argc, char *argv[]) {

	/*	Some Variables	*/

	struct render_node *r_node;
//		/* Copy textures to renderer	*/ for (int i = 0; i < grid.x * 3; i++){
//			for (int j = 0; j < grid.y; j++){
//				SDL_RenderCopy(renderer, Timg, &rcTSrc[i][j], &rcTile[i][j]);
//			}
//		}
//		for (int i = 0; i < grid.x * 3; i++){
//			for (int j = 0; j < grid.y; j++){
//				SDL_RenderCopy(renderer, Timg, &rcTSrcmid[i][j], &rcTilemid[i][j]);
//			}
//		}
//
//		if ( !(player.invincibility == 1 && ( player.invinciblecounter[1]%8 <= 3 ) ) )
//			SDL_RenderCopy(renderer, Spriteimg, &rcSrc, &rcSprite);
//
//		for (int lane = 0; lane < lanes.total; lane++) {
//			struct monster_node *ptr2mon = linkptrs_start[lane];
//			for (int i = 0; i < monsterlanenum[lane]; i++ ) {
//				if ( ptr2mon->status != -1){
//					SDL_RenderCopy(renderer, *(*bestiary[ptr2mon->montype]).image, &(ptr2mon->monster_src), &(ptr2mon->monster_rect));
//				}
//				ptr2mon = ptr2mon->next;
//			}
//		}
//		for (int lane = 0; lane < lanes.total; lane++) {
//			for (int screen = 0; screen < 3; screen++) {
//				for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
//					if ( (*iteminfoptrs[screen])[lane][i] != -1){
//						SDL_RenderCopy(renderer, *(*itempokedex[(*itemscreenstrip[level.currentscreen + screen])[lane][i][0]]).image, &rcItemSrc[screen][lane][i], &rcItem[screen][lane][i]);
//
//					}
//				}
//			}
//		}
//		if (laser.on){
//			for (int i = 0; i < 3; i++) {
//				SDL_RenderCopy(renderer, Laserimg, &rcLaserSrc[i], &rcLaser[i]);
//			}
//		}
//
//		if ( player.sword ) {
//				SDL_RenderCopy(renderer, Swordimg, &rcSwordSrc, &rcSword);
//
//		}
//
//		if ( player.HP <= 50 ) {
//			if ( player.HP <= 20 ) {
//				rcHP1Src.y = 16;
//			}
//			else {
//				rcHP1Src.y = 8;
//			}
//		}
//		else {
//			rcHP1Src.y = 0;
//		}
//
//		rcHP1.w = 0.5 * player.HP * ZOOM_MULT * 2;
//		SDL_RenderCopy(renderer, HPimg0, &rcHP0Src, &rcHP0);
//		SDL_RenderCopy(renderer, HPimg1, &rcHP1Src, &rcHP1);
//
//		if ( player.power <= 250 ) {
//			if ( player.power <= 100 ) {
//				rcPower1Src.y = 16;
//			}
//			else {
//				rcPower1Src.y = 8;
//			}
//		}
//		else {
//			rcPower1Src.y = 0;
//		}
//
//		rcPower1.w = 0.1 * player.power * ZOOM_MULT * 2;
//		SDL_RenderCopy(renderer, Powerimg0, &rcPower0Src, &rcPower0);
//		SDL_RenderCopy(renderer, Powerimg1, &rcPower1Src, &rcPower1);
//
//		int scorearray[SCORE_DIGITS];
//		int2array(program.score, &scorearray);
//		for ( int i = 0; i < 5; i++ ) {
//			rcScoreSrc[i].x = scorearray[i] * 5;
//			SDL_RenderCopy(renderer, Scoreimg, &rcScoreSrc[i], &rcScore[i]);
//		}
//
//		int beatarray[SCORE_DIGITS];
//		int2array(timing.currentbeat, &beatarray);
//		for ( int i = 0; i < 5; i++ ) {
//			rcBeatSrc[i].x = beatarray[i] * 5;
//			SDL_RenderCopy(renderer, Beatimg, &rcBeatSrc[i], &rcBeat[i]);
//		}



	/* Status of the Level */

	struct level_struct level = {
	.gameover = 0,
	.levelover = 0,
	.pauselevel = 0,
	.currentlevel = 1,
	.totalnativedist = 0,
	.partymode = 0,
	.lanes = &lanes
	};

	timing.pauselevel = &level.pauselevel;

	/* Status of the Player */

	struct player_struct player = {
	
		.invincibility = 0,
		.sword = 1,
		.direction = 4,
		.flydir = 1,
	};


	/* All Status */

	status.level = &level;
	status.player = &player;
	status.audio = &audio;
	status.timing = &timing;


	/* 	Initialisation	*/

	/* Setup Config File */
	config_t cfg;
	config_setting_t *setting;
	const char *str;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "level.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}

	config_setting_t *all_setting = config_lookup(&cfg, "all");
	config_setting_t *thislevel_setting = config_setting_get_elem(all_setting, 0);
	config_setting_t *timing_setting = config_setting_lookup(thislevel_setting, "timing");
	if (timing_setting == NULL) {
		printf("No settings found under 'timing' in the config file\n");
		return -1;
	}

	double confdub;
	config_setting_lookup_float(timing_setting, "pxperbeat", &confdub);
	timing.pxperbeat = (float)confdub;
	printf("pxpb = %f\n", timing.pxperbeat);
	config_setting_lookup_float(timing_setting, "bpmin", &confdub);
	timing.bps = (float)confdub/60.0;
	printf("bps is %f\n", timing.bps);
	config_setting_lookup_float(timing_setting, "startbeat", &confdub);
	timing.startbeat = (float)confdub;


	config_setting_t *level_setting = config_setting_lookup(thislevel_setting, "level");
	double smd;

	if (level_setting == NULL) {
			printf("No settings found under 'level' in the config file\n");
			return -1;
	}
	if(!config_setting_lookup_float(level_setting, "speedmult", &smd))
		fprintf(stderr, "'speedmult' not found in config file.\n");
	level.speedmult = (float) smd;
	
	if(!config_setting_lookup_float(level_setting, "speedmultmon", &smd))
		fprintf(stderr, "'speedmultmon' not found in config file.\n");
	level.speedmultmon = (float) smd;
	
	/* Lanes */
	
	config_setting_lookup_int(level_setting, "totallanes", &lanes.total);

	int lanewidthnative;
	config_setting_lookup_int(level_setting, "lanewidthnative", &lanewidthnative);
	
	int laneheightarray[lanes.total];

	lanes.lanewidth = lanewidthnative * ZOOM_MULT; //in pixels
	lanes.laneheight = laneheightarray;
	
	for (int i = 0; i < lanes.total; i++) {
		lanes.laneheight[i] = program.height + lanes.lanewidth * (-(lanes.total - 1) + i - 0.5);
	}


	lanes.currentlane = lanes.total/2;

	float remainder[lanes.total];	//Means that sub-frame movement is accounted for
	for (int lane = 0; lane < lanes.total; lane++)
		remainder[lane] = 0.0;

	/* Declare Textures */

	SDL_Texture *Spriteimg = NULL;
	SDL_Texture *Laserimg = NULL;
	SDL_Texture *Swordimg = NULL;
	SDL_Texture *Timg = NULL;
	SDL_Texture *HPimg0 = NULL;
	SDL_Texture *HPimg1 = NULL;
	SDL_Texture *Powerimg0 = NULL;
	SDL_Texture *Powerimg1 = NULL;
	SDL_Texture *Mon0img = NULL;
	SDL_Texture *Mon1img = NULL;
	SDL_Texture *Scoreimg = NULL;
	SDL_Texture *Beatimg = NULL;
	SDL_Texture *Itemimg = NULL;

	int w, h;

	/*		Sprite		*/
	
	int num_images = 100; //TODO

	//enum {
	//	sprite, flyinghamster, HP1, HP2
	//} img_enum;

	SDL_Texture *image_bank[num_images];
	/*	Initialise the image bank - a fixed array of (as of now hardcoded) pointers to SDL_Textures
	 *	that is used to populate the "img" pointers in the clip sections of each generic struct
	 */
	image_bank_populate(image_bank, renderer);

	struct animate_generic **generic_bank;
	generic_bank_populate(&generic_bank, image_bank); //EXPERIMENTAL

	/* set sprite position */
	player.pos.x = 0.2 * program.width;
	player.pos.y = lanes.laneheight[lanes.currentlane] - POKESPRITE_SIZEX*ZOOM_MULT*2;
	player.size_ratio.w = 1.0;
	player.size_ratio.h = 1.0;
	graphic_spawn(&player.std, generic_bank, renderer, (int[]){3}, 1);
	player.animation->rules_list->data = (void *)&player;

	/*		Tiles		*/

	// set tile position
	Timg = IMG_LoadTexture(renderer, TILE_PATH);
	SDL_QueryTexture(Timg, NULL, NULL, &w, &h); // get the width and height of the texture

	/* Create "film strip" of sequential background screens */
	level.maxscreens = 300;
	level.currentscreen = 0;

	if (NATIVE_RES_X % TILE_SIZE == 0) {
		grid.x = NATIVE_RES_X / TILE_SIZE;
	}
	else {
		grid.x = NATIVE_RES_X / TILE_SIZE + 1;
	}
	if (NATIVE_RES_Y % TILE_SIZE == 0) {
		grid.y = NATIVE_RES_Y / TILE_SIZE;
	}
	else {
		grid.y = NATIVE_RES_Y / TILE_SIZE + 1;
	}

	int sampletilemap[grid.x][grid.y][2];
	int sampletilemap2[grid.x][grid.y][2];
	int sampletilemapmid[grid.x][grid.y][2];
	int sampletilemaptop[grid.x][grid.y][2];

	/*		HUD		*/

	/*	HP	*/

	struct animate_specific *HP0animation;
	struct xy_struct HP0pos = {
		.x = 10 * ZOOM_MULT * 2,
		.y = 10 * ZOOM_MULT * 2
	};
	struct size_ratio_struct HP0ratio = {
		.w = 1.0,
		.h = 1.0
	};
//	graphic_spawn(&HP0animation, HP0pos, HP0ratio, generic_bank, renderer, 2);
	struct ui_bar hp;
	hp.amount = &player.HP;
	hp.max = &player.max_HP;
	hp.pos.x = 10 * ZOOM_MULT * 2;
	hp.pos.y = 10 * ZOOM_MULT * 2;
	hp.size_ratio.w = 1.0;
	hp.size_ratio.h = 1.0;
	graphic_spawn(&hp.std, generic_bank, renderer, (int[]){2,4}, 2);

	struct animate_specific *HP1animation;
	struct xy_struct HP1pos = {
		.x = 29 * ZOOM_MULT * 2 + HP0animation->parent_pos->x,
		.y = 6 * ZOOM_MULT * 2 + HP0animation->parent_pos->x
	};
	struct size_ratio_struct HP1ratio = {
		.w = 1.01,
		.h = 1.01
	};
	HP0animation->next = HP1animation;
//	graphic_spawn(&HP1animation, HP1pos, HP1ratio, generic_bank, renderer, 4);
	HP1animation->rules_list->data = HP1animation;
	struct tr_bump_data *tmp_data = (struct tr_bump_data *)HP1animation->rules_list->next->data;
	tmp_data->centre = malloc(sizeof(struct xy_struct));
	tmp_data->centre->x = HP0animation->rect_out.x + HP0animation->rect_out.w/2;
	tmp_data->centre->y = HP0animation->rect_out.y + HP0animation->rect_out.h/2;
	//HP0animation->rect_out.w = HP0animation->rect_out.w * ZOOM_MULT * 2;
	//HP0animation->rect_out.h = HP0animation->rect_out.h * ZOOM_MULT * 2;

	HPimg0 = NULL;//IMG_LoadTexture(renderer, "../art/hp.png");
	SDL_QueryTexture(HPimg0, NULL, NULL, &w, &h); // get the width and height of the texture

	HPimg1 = NULL;//IMG_LoadTexture(renderer, "../art/colourstrip.png");
	SDL_QueryTexture(HPimg1, NULL, NULL, &w, &h); // get the width and height of the texture

	SDL_Rect rcHP0, rcHP0Src, rcHP1, rcHP1Src;

	rcHP0Src.x = 0;
	rcHP0Src.y = 0;
	rcHP0Src.w = 100;
	rcHP0Src.h = 20;

	rcHP0.x = 10 * ZOOM_MULT * 2;
	rcHP0.y = 10 * ZOOM_MULT * 2;
	rcHP0.w = rcHP0Src.w * ZOOM_MULT * 2;
	rcHP0.h = rcHP0Src.h * ZOOM_MULT * 2;

	rcHP1Src.x = 0;
	rcHP1Src.y = 0;
	rcHP1Src.w = 1;
	rcHP1Src.h = 8;

	rcHP1.x = 29 * ZOOM_MULT * 2 + rcHP0.x;
	rcHP1.y = 6 * ZOOM_MULT * 2 + rcHP0.y;
	rcHP1.w = 50 * ZOOM_MULT * 2;
	rcHP1.h = rcHP1Src.h * ZOOM_MULT * 2;


	config_setting_t *player_setting = config_setting_lookup(thislevel_setting, "player");

	if (player_setting == NULL) {
			printf("No settings found for 'player' in the config file\n");
			return -1;
	}
	config_setting_lookup_int(player_setting, "max_HP", &player.max_HP);
	player.HP = player.max_HP;
	

	/*	Power	*/

	struct animate_specific *Power0animation;
	struct xy_struct Power0pos;
	struct size_ratio_struct Power0ratio = {
		.w = 1.0,
		.h = 1.0
	};
	//graphic_spawn(&Power0animation, Power0pos, Power0ratio, generic_bank, renderer, 3);
	Power0pos.x = (NATIVE_RES_X - 20) * ZOOM_MULT - Power0animation->rect_out.w;
	Power0pos.y = 10 * ZOOM_MULT * 2;

	Powerimg0 = NULL;//IMG_LoadTexture(renderer, "../art/power.png");
	SDL_QueryTexture(Powerimg0, NULL, NULL, &w, &h); // get the width and height of the texture

	Powerimg1 = IMG_LoadTexture(renderer, "../art/colourstrip.png");
	SDL_QueryTexture(Powerimg1, NULL, NULL, &w, &h); // get the width and height of the texture

	SDL_Rect rcPower0, rcPower0Src, rcPower1, rcPower1Src;

	rcPower0Src.x = 0;
	rcPower0Src.y = 0;
	rcPower0Src.w = 100;
	rcPower0Src.h = 20;

	rcPower0.x = (NATIVE_RES_X - rcPower0Src.w * 2 - 20) * ZOOM_MULT ;
	rcPower0.y = 10 * ZOOM_MULT * 2;
	rcPower0.w = rcPower0Src.w * ZOOM_MULT * 2;
	rcPower0.h = rcPower0Src.h * ZOOM_MULT * 2;

	rcPower1Src.x = 0;
	rcPower1Src.y = 0;
	rcPower1Src.w = 1;
	rcPower1Src.h = 8;

	rcPower1.x = 48 * ZOOM_MULT * 2 + rcPower0.x;
	rcPower1.y = 6 * ZOOM_MULT * 2 + rcPower0.y;
	rcPower1.w = 50 * ZOOM_MULT * 2;
	rcPower1.h = rcPower1Src.h * ZOOM_MULT * 2;

	config_setting_lookup_int(player_setting, "max_PP", &player.max_PP);
	player.power = player.max_PP;

	/*	Score	*/

	Scoreimg = IMG_LoadTexture(renderer, "../art/numbers.png");
	SDL_QueryTexture(Scoreimg, NULL, NULL, &w, &h); // get the width and height of the texture

	SDL_Rect rcScore[5], rcScoreSrc[5];

	for (int i = 0; i < 5; i++ ) {
	rcScoreSrc[i].x = 5 * i;
	rcScoreSrc[i].y = 0;
	rcScoreSrc[i].w = 5;
	rcScoreSrc[i].h = 8;

	rcScore[i].x = (48 + rcScoreSrc[i].w * i) * ZOOM_MULT * 2;
	rcScore[i].y = 6 * ZOOM_MULT * 2;
	rcScore[i].w = rcScoreSrc[i].w * ZOOM_MULT * 2;
	rcScore[i].h = rcScoreSrc[i].h * ZOOM_MULT * 2;

	}

	program.score = 0;

	/*	Beat Counter	*/

	Beatimg = IMG_LoadTexture(renderer, "../art/numbers.png");
	SDL_QueryTexture(Scoreimg, NULL, NULL, &w, &h); // get the width and height of the texture

	SDL_Rect rcBeat[5], rcBeatSrc[5];

	for (int i = 0; i < 5; i++ ) {
	rcBeatSrc[i].x = 5 * i;
	rcBeatSrc[i].y = 0;
	rcBeatSrc[i].w = 5;
	rcBeatSrc[i].h = 8;

	rcBeat[i].x = (50 + rcBeatSrc[i].w * i) * ZOOM_MULT * 2 * 2;
	rcBeat[i].y = 6 * ZOOM_MULT * 2;
	rcBeat[i].w = rcBeatSrc[i].w * ZOOM_MULT * 2 * 2;
	rcBeat[i].h = rcBeatSrc[i].h * ZOOM_MULT * 2 * 2;

	}

	/*		Items		*/

	Itemimg = IMG_LoadTexture(renderer, "../art/items.png");
	SDL_QueryTexture(Itemimg, NULL, NULL, &w, &h); // get the width and height of the texture

	struct item potion;

	potion.itemnumber = 0;
	potion.int1 = &player.HP;
	potion.int2 = 50; //Amount of HP restored by potion
	potion.otherdata = (void *)&player;
	potion.functionptr = &restorehealth;	
	memcpy(&potion.Src, &(int [2]){ 0, 0 }, sizeof potion.Src);
	memcpy(&potion.wh, &(int [2]){ 20,20 }, sizeof potion.wh);
	potion.image = &Itemimg;

	struct item laserjuice;

	laserjuice.itemnumber = 1;
	laserjuice.int1 = &player.power;
	laserjuice.int2 = 200; //Amount of HP restored by laserjuice
	laserjuice.otherdata = (void *)&player;
	laserjuice.functionptr = &restorepower;	
	memcpy(&laserjuice.Src, &(int [2]){ 0, 20 }, sizeof laserjuice.Src);
	memcpy(&laserjuice.wh, &(int [2]){ 20,20 }, sizeof laserjuice.wh);
	laserjuice.image = &Itemimg;

	struct item fist;

	fist.itemnumber = 2;
	fist.int1 = &player.max_PP;
	fist.int2 = 20; //Amount of PP increased
	fist.functionptr = &PPup;	
	memcpy(&fist.Src, &(int [2]){ 20, 0 }, sizeof fist.Src);
	memcpy(&fist.wh, &(int [2]){ 20,20 }, sizeof fist.wh);
	fist.image = &Itemimg;

	struct item *itempokedex[10];
	itempokedex[0] = &potion;
	itempokedex[1] = &laserjuice;
	itempokedex[2] = &fist;

	/* Item Functions */

	void (*itemfunctionarray[10])(int *int1ptr, int int2, void *otherdata);
	itemfunctionarray[0] = &restorehealth;
	itemfunctionarray[1] = &restorepower;
//	(*itemfunctionarray[0])(&player.HP, 3);


	/*		Weapons		*/

	/* Laser Beam */

	struct laser_struct laser = {
		.power = 10,
		.count = 0,
		.on = 0,
		.turnon = 0,
		.turnoff = 0
	};
	Laserimg = IMG_LoadTexture(renderer, LASER_PATH);
	SDL_QueryTexture(Laserimg, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_Rect rcTSrc[grid.x * 3][grid.y], rcTile[grid.x * 3][grid.y];
	SDL_Rect rcTSrcmid[grid.x * 3][grid.y], rcTilemid[grid.x * 3][grid.y];
	SDL_Rect rcLaser[3], rcLaserSrc[3];

	/* Sword */

	struct sword_struct sword = {
	.count = 0,
	.down = 0,
	.swing = 0
	};

	Swordimg = IMG_LoadTexture(renderer, SWORD_PATH);
	SDL_QueryTexture(Swordimg, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_Rect rcSword, rcSwordSrc;

	rcSwordSrc.w = SWORD_WIDTH;
	rcSwordSrc.h = SWORD_HEIGHT;
	rcSwordSrc.x = 0;
	rcSwordSrc.y = 0;

	rcSword.x = player.animation->rect_out.x + player.animation->rect_out.w - 2 * ZOOM_MULT; //set preliminary values for the sword's dimensions (otherwise the beat predictor gets it wrong the first time)
	rcSword.y = lanes.laneheight[lanes.currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	rcSword.w = rcSwordSrc.w * ZOOM_MULT * 2;
	rcSword.h = rcSwordSrc.h * ZOOM_MULT * 2;

	/*		Monsters	*/

	Mon0img = IMG_LoadTexture(renderer, "../art/flyinghamster.png");
	Mon1img = IMG_LoadTexture(renderer, "../art/angrycircle.png");
	SDL_QueryTexture(Mon0img, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_QueryTexture(Mon1img, NULL, NULL, &w, &h); // get the width and height of the texture

	struct monster flyinghamster;

	flyinghamster.health = 1;
	flyinghamster.attack = 10;
	flyinghamster.defence = 10;
	memcpy(&flyinghamster.Src, &(int [2]){ 0, 0 }, sizeof flyinghamster.Src);
	memcpy(&flyinghamster.wh, &(int [2]){ 20,20 }, sizeof flyinghamster.wh);
	flyinghamster.image = &Mon0img;
	flyinghamster.generic_bank_index = 1;

	struct monster angrycircle;

	angrycircle.health = 500;
	angrycircle.attack = 10;
	angrycircle.defence = 10;
	memcpy(&angrycircle.Src, &(int [2]){ 0, 0 }, sizeof angrycircle.Src);
	memcpy(&angrycircle.wh, &(int [2]){ 20,20 }, sizeof angrycircle.wh);
	angrycircle.image = &Mon1img;

	struct monster *monsterpokedex[10];
	monsterpokedex[0] = &flyinghamster;
	monsterpokedex[1] = &angrycircle;
	struct monster *bestiary[10];
	bestiary[0] = &flyinghamster;
	bestiary[1] = &angrycircle;


	/* Generate Monster Linked Lists */

	const config_setting_t *arrays_setting;
	arrays_setting = config_setting_lookup(thislevel_setting, "arrays");
	if (arrays_setting == NULL) {
			printf("No settings found for 'arrays' in the config file\n");
			return -1;
	}

	int count = config_setting_length(arrays_setting); 
	config_setting_t *singlearray_setting;

	struct monster_node *ptr2mon;
	for (int lane = 0; lane < lanes.total; lane++) {
		singlearray_setting = config_setting_get_elem(arrays_setting, lane);
		if (singlearray_setting == NULL) {
			linkptrs_start[lane] = NULL;
		}
		else if (config_setting_length(singlearray_setting) <= 0) {
			linkptrs_start[lane] = NULL;
		}

		else {
			int count = config_setting_length(singlearray_setting);
		
			float lanearray[count];
					/*   ^{entrybeat, montype}	*/
			for (int index = 0; index < count; index++) {
				lanearray[index] = config_setting_get_float_elem(singlearray_setting, index);
			}

			linkptrs_start[lane] = malloc( sizeof(struct monster_node));
			if (linkptrs_start[lane] == NULL) {
				return 1;
			}
			linkptrs_end[lane] = linkptrs_start[lane];
			monsterlanenum[lane] = 0;
			ptr2mon = linkptrs_start[lane];
	
			for (int index = 0; index < count; index++) {
				ptr2mon->montype = 0;
				ptr2mon->status = 0;
				ptr2mon->health = bestiary[ptr2mon->montype]->health;
				ptr2mon->speed = 1;
				ptr2mon->entrybeat = lanearray[index];
				ptr2mon->remainder = 0;
				ptr2mon->monster_rect.w = 32 * ZOOM_MULT;
				ptr2mon->monster_rect.h = 32 * ZOOM_MULT;
				ptr2mon->monster_rect.x = program.width;
				ptr2mon->monster_rect.y = lanes.laneheight[lane] - ptr2mon->monster_rect.h/2;
				ptr2mon->monster_src.x = bestiary[ptr2mon->montype]->Src[0];
				ptr2mon->monster_src.y = bestiary[ptr2mon->montype]->Src[1];
				ptr2mon->monster_src.w = bestiary[ptr2mon->montype]->wh[0];
				ptr2mon->monster_src.h = bestiary[ptr2mon->montype]->wh[1];
				if (index < count - 1) {
					ptr2mon->next = malloc( sizeof(struct monster_node) );
					if (ptr2mon->next == NULL) {
						return 1;
					}
					ptr2mon = ptr2mon->next;
				}
			}
			ptr2mon->next = NULL;
		}
	}
	//ptr2mon = linkptrs_start[2];

	/*		Maps & Mapstrips	*/

	/*	Tilemaps	*/

	/* Generate a sample background that is 1 screen large */
	for (int i = 0; i < grid.x; i++){
		for (int j = 0; j < grid.y; j++){
			sampletilemap2[i][j][0] = 0;
			sampletilemap2[i][j][1] = 1;

			if ((j == grid.y - 2 || j == 0) && i%3 == 0){
				sampletilemap[i][j][0] = 0;
				sampletilemap[i][j][1] = 1;
			}
			else{
				sampletilemap[i][j][0] = 0;
				sampletilemap[i][j][1] = 0;
			}
			if (j == 4 && i== 4){
				sampletilemap[i][j][0] = 1;
				sampletilemap[i][j][1] = 3;
			}
			sampletilemapmid[i][j][0] = 0;
			sampletilemapmid[i][j][1] = 0;

		}
	}

	/* "Film strip" of items */

	int (*screenstrip[level.maxscreens])[grid.x][grid.y][2];


	/*	Items	*/

	int sampleitemmap[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2];
	/*          lane number ^         ^ order	*/

	//int sampleitemmap[grid.x][grid.y][2];

	for (int i = 0; i < lanes.total; i++) {
		for (int j = 0; j < MAX_ITEMS_PER_LANE_PER_SCREEN; j++) {
			sampleitemmap[i][j][0] = -1;		//specifies the type of item
			sampleitemmap[i][j][1] = 0;		//specifies item position
		}
	}
	sampleitemmap[0][0][0] = 0;
	sampleitemmap[0][0][1] = 0 * NATIVE_RES_X;

	sampleitemmap[0][1][0] = 1;
	sampleitemmap[0][1][1] = 0.5 * NATIVE_RES_X;

	sampleitemmap[3][0][0] = 2;
	sampleitemmap[3][0][1] = 0.3 * NATIVE_RES_X;

	int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2];


	/*	Monsters*/

	int samplemonstermap[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3];
	/*          lane number ^         ^ order	*/

	for (int i = 0; i < lanes.total; i++) {
		for (int j = 0; j < MAX_MONS_PER_LANE_PER_SCREEN; j++) {
			samplemonstermap[i][j][0] = -1;		//specifies the type of monster
			samplemonstermap[i][j][1] = 0;		//specifies monster position
			samplemonstermap[i][j][2] = 0;		//specifies monster HP
		}
	}

	int samplemonstermap2[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3];

	for (int i = 0; i < lanes.total; i++) {
		for (int j = 0; j < MAX_MONS_PER_LANE_PER_SCREEN; j++) {
			samplemonstermap2[i][j][0] = -1;	//specifies the type on monster
			samplemonstermap2[i][j][1] = 0;		//specifies monster position
		}
	}

	int upperj = NATIVE_RES_X / PXPB;
	//for (int i = 1; i < lanes.total; i++) {
		for (int j = 0; j < upperj; j++) {

			samplemonstermap[3][j][0] = 0;
			samplemonstermap[3][j][1] = j * 83.24;
			samplemonstermap[3][j][2] = (*monsterpokedex[samplemonstermap[3][j][0]]).health;
		}
	//}

	samplemonstermap2[2][0][0] = 1;
	samplemonstermap2[2][0][1] = 0.5 * NATIVE_RES_X;
	samplemonstermap2[2][0][2] = (*monsterpokedex[samplemonstermap2[2][0][0]]).health;
//	samplemonstermap2[2][0][2] = 100;

	int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3];


	/*	Pointing Strip Entries to Maps	*/

	for (int i = 0; i < level.maxscreens; i++) {

		itemscreenstrip[i] = &sampleitemmap;
		if ( i%2 == 0 ){
			screenstrip[i] = &sampletilemap;
			monsterscreenstrip[i] = &samplemonstermap;
		}
		else{
			screenstrip[i] = &sampletilemap2;
			monsterscreenstrip[i] = &samplemonstermap;
		}
	}


	/*		Info		*/

	/*		MONINFO IS OUTDATED -- WAS REPLACED BY LINKED LIST		*/

	/* Here we have the arrays of information regarding the monsters onscreen. 	*/

	/* The array is filled from 0 in order of the monsters' x positions in their	*/
	/* respective lanes. moninfoarray0 deals with the 1st screen, etc.		*/

	/* moninfoarrayx[monster's lane][monster's order from left][3]			*/

	/* monsternum[lane] specifies how many monsters have info in moninfoarray[lane]	*/

	/* moninfoarrayx[x][x][0] == monster's number in samplemonstermap.		*/
	/* moninfoarrayx[x][x][1] == monster's **current** health.			*/
	/* moninfoarrayx[x][x][2] == monster's status (0 is default, -1 is dead).	*/

	//int moninfoarray0[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN][3] = {0};
	//int moninfoarray1[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN][3] = {0};
	//int moninfoarray2[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN][3] = {0};

	/* Array of pointers to each moninfoarray */

	//moninfoptrs[0] = &moninfoarray0;
	//moninfoptrs[1] = &moninfoarray1;
	//moninfoptrs[2] = &moninfoarray2;

	/* The same for items, except the only info is if it has been collected		*/

	int iteminfoarray0[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN] = {0};
	int iteminfoarray1[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN] = {0};
	int iteminfoarray2[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN] = {0};

	/* Array of pointers to each iteminfoarray */

	iteminfoptrs[0] = &iteminfoarray0;
	iteminfoptrs[1] = &iteminfoarray1;
	iteminfoptrs[2] = &iteminfoarray2;



//	multidrop(); //	dropin( "house", sampletilemapmid, 20, 20);


	refreshtiles(screenstrip, monsterscreenstrip, itemscreenstrip, level.currentscreen, grid, rcTile, rcTilemid, rcTSrc, rcTSrcmid, 0, lanes.laneheight, monsterpokedex, itempokedex);

	/*	Event Handling	*/

	int directionbuttonlist [4] = {0, 0, 0, 0};
	int history [4] = {0, 0, 0, 0};
	int histwrite = 0;
	int histread = 0;
	int actionbuttonlist [4] = {0, 0, 0, 0}; //a, b, start, select
	int acthistory [4] = {0, 0, 0, 0};
	int acthistwrite = 0;
	int acthistread = 0;

	/* Play some music */

	soundstart();

	pthread_t threads;
	int rc;
	config_setting_lookup_int(thislevel_setting, "track", &audio.newtrack);

	struct musicstart_struct {
		int newtrack;
		int *pause;
	};
	struct musicstart_struct musicstart_struct = {
		.newtrack = audio.newtrack,
		.pause = &level.pauselevel
	};
	rc = pthread_create(&threads, NULL, musicstart, (void*)&musicstart_struct);
	if (rc) {
		exit(-1);
	}


	pthread_mutex_lock( &track_mutex );
	audio.track = audio.newtrack;
	pthread_mutex_unlock( &track_mutex );

	pthread_mutex_lock( &clock_mutex );
	level.pauselevel = 0;
	timing.pausetime = 0;
	timing.zerotime = SDL_GetTicks();
	timing.countbeats = 1;
	pthread_mutex_unlock( &clock_mutex );
	pthread_mutex_lock( &clock_mutex );
	pthread_mutex_unlock( &clock_mutex );
	pthread_cond_wait(&display_cond, &display_mutex);
	pthread_mutex_unlock(&display_mutex);

	extern pthread_cond_t soundstatus_cond;
	extern pthread_mutex_t soundstatus_mutex;

	/* Snazzy Effects */

	double angle = 0.0;
	int colournum = 0;
	int hue;

	/* Create a Master Render Target */

	SDL_Texture *texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, program.width, program.height);




	/* Frame Loop */

	while (1) {

		if (timing.currentbeat >= 81) {
			level.speedmult = 3;
			level.speedmultmon = 3;
		}


		if (level.levelover) {
			quitlevel(Spriteimg, Timg, Laserimg, Swordimg, renderer);
			return 0;
		}

		//pthread_mutex_unlock( &soundstatus_mutex);
		pthread_cond_wait( &soundstatus_cond, &soundstatus_mutex );
		//pthread_mutex_lock( &soundstatus_mutex);
		for (int i = 0; i < MAX_SOUNDS_LIST; i++ ) {
			if ( audio.soundchecklist[i] == 1 )
				audio.soundchecklist[i] = 0;
		}

		/* Handle Events */

		SDL_Event e;
		while ( SDL_PollEvent(&e) ) {

			histwrite++;
			if (histwrite >= 4){
				histwrite = 0;}

			if (e.type == SDL_QUIT) {
				level.levelover = 1;
				quitlevel(Spriteimg, Timg, Laserimg, Swordimg, renderer);
				return 102; /* Quit to desktop" code */
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_1) {
				level.levelover = 1;
				quitlevel(Spriteimg, Timg, Laserimg, Swordimg, renderer);
				return 0; /* Quit up one level */
			}

			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				timing.startpause = SDL_GetTicks();
				level.pauselevel = 1;
				timing.pause_change = 1;
				int rc = pausefunc(renderer, texTarget, level.currentlevel);
				timing.endpause = SDL_GetTicks();
				level.pauselevel = 0;
				timing.pause_change = 1;
				SDL_SetRenderTarget(renderer, NULL);

				if (rc >= 100 && rc < 200) {
					quitlevel(Spriteimg, Timg, Laserimg, Swordimg, renderer);
					return rc;
				}
				if (rc >= 200) {
					quitlevel(Spriteimg, Timg, Laserimg, Swordimg, renderer);
					return rc;
				}
			}


			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_h){
				lanes.currentlane = 0;
				}

			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_j){
				lanes.currentlane = 1;
				}

			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_k){
				lanes.currentlane = 2;
				}

			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_l){
				lanes.currentlane = 3;
				}


			if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_RIGHT){
				directionbuttonlist[1] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT){
				directionbuttonlist[1] = 1;
				history[histwrite] = 1;
				player.direction = 1;
				}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LEFT){
				directionbuttonlist[3] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT){
				directionbuttonlist[3] = 1;
				history[histwrite] = 3;
				player.direction = 3;
				}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_DOWN){
				directionbuttonlist[2] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN){
				directionbuttonlist[2] = 1;
				history[histwrite] = 2;
				player.direction = 2;
				}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_UP){
				directionbuttonlist[0] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP){
				directionbuttonlist[0] = 1;
				history[histwrite] = 0;
				player.direction = 0;
				}

			if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a){
				actionbuttonlist[0] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a){
				actionbuttonlist[0] = 1;
				acthistory[acthistwrite] = 0;
				}
			
			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s){
				actionbuttonlist[1] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s){
				actionbuttonlist[1] = 1;
				acthistory[acthistwrite] = 0;
				}
			
			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d){
				actionbuttonlist[2] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d){
				actionbuttonlist[2] = 1;
				acthistory[acthistwrite] = 0;
				}
			
			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_f){
				actionbuttonlist[3] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_f){
				actionbuttonlist[3] = 1;
				acthistory[acthistwrite] = 0;
				}


		}

		int donehere = 0;

		if ( player.power > 0 ) {

			for (int i = 0; i < 3; i++) {
				if (actionbuttonlist[i] == 1) {
					hue = i;
					laser.turnoff = 0;

					if (laser.count < 4) {
						laser.on = 1;
						laser.turnon = 1;
						if ( laser.count == 0 )
							audio.soundchecklist[1] = 1;
					}
					else {
						audio.soundchecklist[2] = 2;
					}
					donehere = 1;
					break;
				}
			}
			if ( !donehere ) {
				if (laser.on)
					//laser.turnon = 0;
					laser.turnoff = 1;
					audio.soundchecklist[2] = 0;
					if ( laser.count == 4 ){
						audio.soundchecklist[3] = 1;
					}
			}

		}
		else {
		/* In the case the power has run out */
			if (laser.on)
				//laser.turnon = 0;
				laser.turnoff = 1;
				audio.soundchecklist[2] = 0;
		}

		if ( player.sword ) {

			if ( actionbuttonlist[3] == !sword.down ) {
				sword.swing = 1;
			}
			else
				sword.swing = 0;
		}

		/* Change music if necessary */

		if ( audio.track != audio.newtrack ) {

			if (audio.noise) {
				musicstop();
			}
			else {
				rc = pthread_create(&threads, NULL, musicstart, (void*)&audio.newtrack);
				printf("NEW\n");
				if (rc) {
					printf("ya dun goofed. return code is %d\n.", rc);
					exit(-1);
				}
				audio.track = audio.newtrack;
			}
		}

		/* The real business */
		moveme(&lanes.currentlane, lanes.total, &player.direction, player.animation);


		movemap(&level, &player, grid, rcTile, rcTilemid, rcTSrc, rcTSrcmid, screenstrip, monsterscreenstrip, itemscreenstrip, &monsterpokedex, &itempokedex);

		movemon(level.speedmultmon, timing, linkptrs_start, linkptrs_end, monsterlanenum, &remainder, player.animation->rect_out, rcSword);
		if (laser.on) {
			laserfire(&laser, &player, rcLaser, rcLaserSrc, player.animation->rect_out, lanes.laneheight, lanes.currentlane, timing.framecount, monsterscreenstrip, level.currentscreen, hue);
		}

		if ( player.sword )
			swordfunc(&sword, &rcSword, &rcSwordSrc, player.animation->rect_out, lanes.laneheight, lanes.currentlane, timing.framecount, linkptrs_start);

		invinciblefunc(&player);

		amihurt(status, linkptrs_start, player.animation->rect_out, bestiary);

		touchitem(lanes.currentlane, level.currentscreen, player.animation->rect_out, itempokedex, itemscreenstrip, &level.levelover);

		/* Clear screen */
//		SDL_RenderClear(renderer);

		//Now render to the texture
		SDL_RenderClear(renderer);
		/* Copy textures to renderer	*/
		for (int i = 0; i < grid.x * 3; i++){
			for (int j = 0; j < grid.y; j++){
				SDL_RenderCopy(renderer, Timg, &rcTSrc[i][j], &rcTile[i][j]);
			}
		}
		for (int i = 0; i < grid.x * 3; i++){
			for (int j = 0; j < grid.y; j++){
				SDL_RenderCopy(renderer, Timg, &rcTSrcmid[i][j], &rcTilemid[i][j]);
			}
		}

	//	if ( !(player.invincibility == 1 && ( player.invinciblecounter[1]%8 <= 3 ) ) )
	//		SDL_RenderCopy(renderer, Spriteimg, &rcSrc, &rcSprite);
		//for (int lane = 0; lane < lanes.total; lane++) {
			//for (int screen = 0; screen < 3; screen++) {
				//for (int i = 0; i < monsterlanenum[screen][lane]; i++ ) {
					//if ( (*moninfoptrs[screen])[lane][i][2] != -1){
						//SDL_RenderCopy(renderer, *(*monsterpokedex[(*monsterscreenstrip[currentscreen + screen])[lane][i][0]]).image, &rcMonsterSrc[screen][lane][i], &rcMonster[screen][lane][i]);
					//}
				//}
			//}
		//}

		for (int lane = 0; lane < lanes.total; lane++) {
			struct monster_node *ptr2mon = linkptrs_start[lane];
			for (int i = 0; i < monsterlanenum[lane]; i++ ) {
				if ( ptr2mon->status != -1){
					SDL_RenderCopy(renderer, *(*bestiary[ptr2mon->montype]).image, &(ptr2mon->monster_src), &(ptr2mon->monster_rect));
				}
				ptr2mon = ptr2mon->next;
			}
		}
		for (int lane = 0; lane < lanes.total; lane++) {
			for (int screen = 0; screen < 3; screen++) {
				for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
					if ( (*iteminfoptrs[screen])[lane][i] != -1){
						SDL_RenderCopy(renderer, *(*itempokedex[(*itemscreenstrip[level.currentscreen + screen])[lane][i][0]]).image, &rcItemSrc[screen][lane][i], &rcItem[screen][lane][i]);

					}
				}
			}
		}
		if (laser.on){
			for (int i = 0; i < 3; i++) {
				SDL_RenderCopy(renderer, Laserimg, &rcLaserSrc[i], &rcLaser[i]);
			}
		}

		if ( player.sword ) {
				SDL_RenderCopy(renderer, Swordimg, &rcSwordSrc, &rcSword);

		}

		if ( player.HP <= 50 ) {
			if ( player.HP <= 20 ) {
				rcHP1Src.y = 16;
			}
			else {
				rcHP1Src.y = 8;
			}
		}
		else {
			rcHP1Src.y = 0;
		}

		rcHP1.w = 0.5 * player.HP * ZOOM_MULT * 2;
		SDL_RenderCopy(renderer, HPimg0, &rcHP0Src, &rcHP0);
		SDL_RenderCopy(renderer, HPimg1, &rcHP1Src, &rcHP1);

		if ( player.power <= 250 ) {
			if ( player.power <= 100 ) {
				rcPower1Src.y = 16;
			}
			else {
				rcPower1Src.y = 8;
			}
		}
		else {
			rcPower1Src.y = 0;
		}

		rcPower1.w = 0.1 * player.power * ZOOM_MULT * 2;
		SDL_RenderCopy(renderer, Powerimg0, &rcPower0Src, &rcPower0);
		SDL_RenderCopy(renderer, Powerimg1, &rcPower1Src, &rcPower1);

		int scorearray[SCORE_DIGITS];
		int2array(program.score, &scorearray);
		for ( int i = 0; i < 5; i++ ) {
			rcScoreSrc[i].x = scorearray[i] * 5;
			SDL_RenderCopy(renderer, Scoreimg, &rcScoreSrc[i], &rcScore[i]);
		}

		int beatarray[SCORE_DIGITS];
		int2array(timing.currentbeat, &beatarray);
		for ( int i = 0; i < 5; i++ ) {
			rcBeatSrc[i].x = beatarray[i] * 5;
			SDL_RenderCopy(renderer, Beatimg, &rcBeatSrc[i], &rcBeat[i]);
		}
		advanceFrames(render_node_head);
		renderlist(render_node_head);
		struct render_node *node_ptr = render_node_head;
		//Detach the texture
		SDL_SetRenderTarget(renderer, NULL);

		SDL_RenderClear(renderer);

		if (status.level->partymode) {
			angle ++;
			colournum += level.speedmult*5;
			int r = colournum%255;
			int g = (colournum + 100)%255;
			int b = (colournum + 200)%255;
			SDL_SetTextureColorMod(texTarget, r,g,b);
			SDL_RenderCopyEx(renderer, texTarget, NULL, NULL, angle * level.speedmult, NULL, SDL_FLIP_NONE);
		}
		else {
			SDL_RenderCopy(renderer, texTarget, NULL, NULL);
		}

		pthread_cond_wait(&display_cond, &display_mutex);
		SDL_RenderPresent(renderer);
		pthread_mutex_unlock(&display_mutex);

		SDL_SetRenderTarget(renderer, texTarget);
	}

	//	SDL_DestroyRenderer(renderer);

	quitlevel(Spriteimg, Timg, Laserimg, Swordimg, renderer);



	return 0;
}


/*Functions*/



void moveme(int *currentlane, int totallanes, int *direction, struct animate_specific *anim) {

	if (*direction < 4) {
		if (*direction == 0 || *direction == 3) {
			if (*currentlane > 0) {
				(*currentlane)--;
			}
		}
		if (*direction == 2 || *direction == 1) {
			if (*currentlane < lanes.total - 1) {
				(*currentlane)++;
			}
		}
		*direction = 4;
	}
	/* set sprite position */
	anim->parent_pos->y = lanes.laneheight[lanes.currentlane] - POKESPRITE_SIZEX*ZOOM_MULT*2;

}

void movemap(struct level_struct *level_ptr, struct player_struct *player_ptr, struct xy_struct grid, SDL_Rect rcTile[grid.x * 3][grid.y], SDL_Rect rcTilemid[grid.x * 3][grid.y], SDL_Rect rcTSrc[grid.x][grid.y], SDL_Rect rcTSrcmid[grid.x][grid.y], int (*screenstrip [level.maxscreens]) [grid.x * 3][grid.y][2], int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], struct monster *(*bestiary)[10], struct item *(*itempokedex)[10]){
	if (player_ptr->flydir == 0){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].y += 4 * ZOOM_MULT * level_ptr->speedmult;
				rcTilemid[i][j].y += 4 * ZOOM_MULT * level_ptr->speedmult;
			}
		}
	}
	if (player_ptr->flydir == 1 ){
		if (rcTile[grid.x][0].x <= 0 ) {
			int frameoffset = rcTile[grid.x][0].x;
			(level_ptr->currentscreen)++;
			if (level_ptr->currentscreen >= level_ptr->maxscreens - 3){
				level_ptr->levelover = 1;
			}
			//else {
				refreshtiles(screenstrip, monsterscreenstrip, itemscreenstrip, level_ptr->currentscreen, grid, rcTile, rcTilemid, rcTSrc, rcTSrcmid, frameoffset, level_ptr->lanes->laneheight, *bestiary, *itempokedex);
			//}
		}

		if ( !(level_ptr->levelover) ) {
			for (int i = 0; i < grid.x * 3; i++){
				for(int j = 0; j < grid.y; j++){
					rcTile[i][j].x -= 4 * ZOOM_MULT * level_ptr->speedmult;
					rcTilemid[i][j].x -= 4 * ZOOM_MULT * level_ptr->speedmult;
					level_ptr->totalnativedist += 4 * level_ptr->speedmult;
				}
			}
			for ( int screen = 0; screen < 3; screen++ ) {
				for (int lane = 0; lane < lanes.total; lane++ ) {
					//for (int i = 0; i < monsterlanenum[screen][lane]; i++ ) {
					//		rcMonster[screen][lane][i].x -= 4 * ZOOM_MULT * level_ptr->speedmult;
					//}
					for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
							rcItem[screen][lane][i].x -= 4 * ZOOM_MULT * level_ptr->speedmult;
					}
				}
			}
		}
	}

	if (player_ptr->flydir == 2){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].y -= 4 * ZOOM_MULT * level_ptr->speedmult;
				rcTilemid[i][j].y -= 4 * ZOOM_MULT * level_ptr->speedmult;
			}
		}
	}

	if (player_ptr->flydir == 3){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].x += 4 * ZOOM_MULT * level_ptr->speedmult;
				rcTilemid[i][j].x += 4 * ZOOM_MULT * level_ptr->speedmult;
			}
		}
	}


}


void movemon(float speedmultmon, struct time_struct timing, struct monster_node *linkptrs_start[TOTAL_LANES], struct monster_node *linkptrs_end[TOTAL_LANES], int monsterlanenum[TOTAL_LANES], float (*remainder)[lanes.total], SDL_Rect player_out, SDL_Rect rcSword) {
	struct monster_node *ptr2mon;
	float transspeed;
	for (int lane = 0; lane < TOTAL_LANES; lane++) {
		if (linkptrs_start[lane] != NULL) {
			ptr2mon = linkptrs_start[lane];
			for (int i = 0; i < monsterlanenum[lane]; i++) {
				transspeed = timing.bps * linkptrs_start[lane]->speed * ZOOM_MULT * speedmultmon * timing.pxperbeat + ptr2mon->remainder;
				float translation = transspeed * timing.intervalglobal/1000.0;
				int transint = (int)translation;
				ptr2mon->remainder = translation - transint;
				ptr2mon->monster_rect.x -= transint;
				ptr2mon = ptr2mon->next;
			}
			while (linkptrs_start[lane]->monster_rect.x + linkptrs_start[lane]->monster_rect.w < 0) {
				struct monster_node *tmp = linkptrs_start[lane];
				linkptrs_start[lane] = linkptrs_start[lane]->next;
				monsterlanenum[lane]--;
				free(tmp);
				if (linkptrs_start[lane] == NULL)
					break;
			}
			float Dt;
			while (linkptrs_start[lane] != NULL && linkptrs_end[lane] != NULL) {
				transspeed = timing.bps * linkptrs_start[lane]->speed * ZOOM_MULT * speedmultmon * timing.pxperbeat + ptr2mon->remainder;
				pthread_mutex_lock( &clock_mutex );
				float extra = (((float)(program.width - (player_out.x + player_out.w + rcSword.w/2)))/transspeed - 4*timing.intervalglobal/1000.0) * timing.bps;
				Dt = timing.currentbeat - linkptrs_end[lane]->entrybeat + extra;
				pthread_mutex_unlock( &clock_mutex );
				if (Dt >= 0) {
					float Dx = Dt / timing.bps * speedmultmon * linkptrs_end[lane]->speed;
					linkptrs_end[lane]->monster_rect.x = program.width - Dx;
					linkptrs_end[lane] = linkptrs_end[lane]->next;
					monsterlanenum[lane]++;
				}
				else
					break;
			}
		}
	}

}

void dropin( char arg[], int map[100][100][2], int posx, int posy){

	if (arg == "house"){
		for (int i = 0; i < 5; i++){
			for (int j = 0; j < 5; j++){

				map[i + posx][j + posy][0] = j + 4;
				map[i + posx][j + posy][1] = i;
			}
		}
	}

	if (arg == "treenobg"){
		for (int i = 0; i < 2; i++){
			for (int j = 0; j < 3; j++){

				map[i + posx][j + posy][0] = j + 1;
				map[i + posx][j + posy][1] = i + 4;
			}
		}
	}
	if (arg == "treebg"){
		for (int i = 0; i < 2; i++){
			for (int j = 0; j < 2; j++){

				map[i + posx][j + posy][0] = j + 1;
				map[i + posx][j + posy][1] = i + 6;
			}
		}
	}

	if (arg == "sand"){
		map[posx][posy][0] = 1;
		map[posx][posy][1] = 2;
	}
}

//void multidrop(){
//
//	for (int k = 0; k <= 28; k += 7){
//		dropin( "house", sampletilemapmid, 45, (45 + k));
//		dropin( "house", sampletilemapmid, 51, (45 + k));
//		dropin( "house", sampletilemapmid, 57, (45 + k));
//		dropin( "house", sampletilemapmid, 63, (45 + k));
//		for (int i = 0; i < 23; i++){
//			dropin( "sand", sampletilemapmid, (45 + i), (50 + k));
//		}
//	}
//	dropin( "house", sampletilemapmid, 20, 20);
//
//}


void refreshtiles(int (*screenstrip[level.maxscreens]) [grid.x][grid.y][2], int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int currentscreen, struct xy_struct grid, SDL_Rect rcTile[grid.x][grid.y], SDL_Rect rcTilemid[grid.x][grid.y], SDL_Rect rcTSrc[grid.x][grid.y], SDL_Rect rcTSrcmid[grid.x][grid.y], int frameoffset, int laneheight[lanes.total], struct monster *bestiary[10], struct item *itempokedex[10]) {

	for (int k = 0; k <= 2; k++) {
	for (int i = 0; i < grid.x; i++){
		for (int j = 0; j < grid.y; j++){

			rcTile[i + grid.x * k][j].x = (i + grid.x * k) * ZOOM_MULT * TILE_SIZE + frameoffset;
			rcTile[i + grid.x * k][j].y = (j) * ZOOM_MULT * TILE_SIZE;
			rcTile[i + grid.x * k][j].w = TILE_SIZE*ZOOM_MULT * 2;
			rcTile[i + grid.x * k][j].h = TILE_SIZE*ZOOM_MULT * 2;
			rcTSrc[i + grid.x * k][j].x = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][1]/2;
			rcTSrc[i + grid.x * k][j].y = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][0]/2;
			rcTSrc[i + grid.x * k][j].w = TILE_SIZE;
			rcTSrc[i + grid.x * k][j].h = TILE_SIZE;
		}
	}
	}

	//}

	if ( currentscreen != 0 ) {
		void *ptrtemp = iteminfoptrs[0];
		iteminfoptrs[0] = iteminfoptrs[1];
		iteminfoptrs[1] = iteminfoptrs[2];
		iteminfoptrs[2] = ptrtemp;
	}


	int itemcounter;
	int itemcounterperscreen;
	for (int lane = 0; lane < lanes.total; lane++){
	itemcounter = 0;
	for (int screen = 0; screen <= 2; screen++) {
		itemcounterperscreen = 0;
		for (int j = 0; j < MAX_ITEMS_PER_LANE_PER_SCREEN; j++){
			if ((*itemscreenstrip[currentscreen + screen])[lane][j][0] != -1 ) {
				rcItemSrc[screen][lane][itemcounterperscreen].x = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).Src[0];// * ZOOM_MULT;
				rcItemSrc[screen][lane][itemcounterperscreen].y = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).Src[1];// * ZOOM_MULT;
				rcItemSrc[screen][lane][itemcounterperscreen].w = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[0];// * ZOOM_MULT;
				rcItemSrc[screen][lane][itemcounterperscreen].h = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[1];// * ZOOM_MULT;

				rcItem[screen][lane][itemcounterperscreen].w = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[0] * ZOOM_MULT * 2;
				rcItem[screen][lane][itemcounterperscreen].h = (*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][j][0]]).wh[1] * ZOOM_MULT * 2;
				rcItem[screen][lane][itemcounterperscreen].x = ( (*itemscreenstrip[currentscreen + screen])[lane][j][1] + grid.x * screen * TILE_SIZE) * ZOOM_MULT + frameoffset;
				rcItem[screen][lane][itemcounterperscreen].y = laneheight[lane] - rcItem[screen][lane][itemcounterperscreen].h / 2;

				if ( currentscreen == 0 || screen == 2 ) {
					(*iteminfoptrs[screen])[lane][itemcounterperscreen] = 0;
				}

				itemcounter++;
				itemcounterperscreen++;
			}
		}
		itemlanenum[screen][lane] = itemcounterperscreen;
	}
	}





//	for (int k = 0; k <= 2; k++) {
//	for (int i = 0; i < grid.x; i++){
//		for (int j = 0; j < grid.y; j++){
//	
//			rcTilemid[i + grid.x * k][j].x = (i + grid.x * k) * ZOOM_MULT * TILE_SIZE + frameoffset;
//			rcTilemid[i + grid.x * k][j].y = (j) * ZOOM_MULT * TILE_SIZE;
//			rcTilemid[i + grid.x * k][j].w = TILE_SIZE*ZOOM_MULT;
//			rcTilemid[i + grid.x * k][j].h = TILE_SIZE*ZOOM_MULT;
//			rcTSrcmid[i + grid.x * k][j].x = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][1];
//			rcTSrcmid[i + grid.x * k][j].y = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][0];
//			rcTSrcmid[i + grid.x * k][j].w = TILE_SIZE;
//			rcTSrcmid[i + grid.x * k][j].h = TILE_SIZE;
//		}
//	}
//	}

}



void laserfire(struct laser_struct *laser, struct player_struct *player, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect player_out, int laneheight[lanes.total], int currentlane, int framecount, int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], int currentscreen, int hue) {
	if (laser->turnon) {
		(laser->count)++;

		if (laser->count >= 4) {
			laser->turnon = 0;
		}
	}

	else if (laser->turnoff) {
//		if ( laser->turnon )
			(laser->count)--;

		if (laser->count <= 0) {
			laser->on = 0;
			laser->turnoff = 0;
			return;
		}
	}



	int lasersrcpos;
	lasersrcpos = (laser->count);
	if (laser->count >= 4 && framecount%4 == 0)
		lasersrcpos --;
	rcLaserSrc[0].x = LASER_SEPARATOR_X * 0 + 120 * hue;
	rcLaserSrc[0].y = LASER_SEPARATOR_Y * lasersrcpos;
	rcLaserSrc[0].w = LASER_SEPARATOR_X - 1;
	rcLaserSrc[0].h = LASER_HEIGHT;

	rcLaserSrc[1].x = LASER_SEPARATOR_X * 1 + 17 + 120 * hue;
	rcLaserSrc[1].y = LASER_SEPARATOR_Y * lasersrcpos;
	rcLaserSrc[1].w = 1;
	rcLaserSrc[1].h = LASER_HEIGHT;

	rcLaserSrc[2].x = LASER_SEPARATOR_X * 2 + 120 * hue;
	rcLaserSrc[2].y = LASER_SEPARATOR_Y * lasersrcpos;
	rcLaserSrc[2].w = LASER_SEPARATOR_X;
	rcLaserSrc[2].h = LASER_HEIGHT;

	int laserlength = NATIVE_RES_X * ZOOM_MULT - player_out.x - player_out.w + rcLaser[2].w;

	int done = 0;
	struct monster_node *ptr2mon = linkptrs_start[currentlane];
	for ( int i = 0; i < monsterlanenum[currentlane]; i++ ) {
		if ( ptr2mon->monster_rect.x > ( player_out.x + player_out.w ) && (ptr2mon->status != -1)){
			if ( ptr2mon->monster_rect.x <= NATIVE_RES_X * ZOOM_MULT ) {
				laserlength = - player_out.x - player_out.w + ptr2mon->monster_rect.x;
				damage(currentlane, ptr2mon, laser->power);
			}
			break;
		}
		ptr2mon = ptr2mon->next;
	}

	rcLaser[0].x = 0.24 * NATIVE_RES_X * ZOOM_MULT;
	rcLaser[0].y = laneheight[currentlane] - LASER_HEIGHT * ZOOM_MULT/2;
	rcLaser[0].w = LASER_SEPARATOR_X * ZOOM_MULT;
	rcLaser[0].h = LASER_HEIGHT * ZOOM_MULT;

	rcLaser[1].x = rcLaser[0].x + rcLaser[0].w;
	rcLaser[1].y = laneheight[currentlane] - LASER_HEIGHT * ZOOM_MULT/2;
	rcLaser[1].w = laserlength - rcLaser[2].w;
	rcLaser[1].h = LASER_HEIGHT * ZOOM_MULT;

	rcLaser[2].x = rcLaser[1].x + rcLaser[1].w;
	rcLaser[2].y = laneheight[currentlane] - LASER_HEIGHT * ZOOM_MULT/2;
	rcLaser[2].w = LASER_SEPARATOR_X * ZOOM_MULT;
	rcLaser[2].h = LASER_HEIGHT * ZOOM_MULT;

	player->power--;
}

void swordfunc(struct sword_struct *sword, SDL_Rect *rcSword, SDL_Rect *rcSwordSrc, SDL_Rect player_out, int laneheight[lanes.total], int currentlane, int framecount, struct monster_node *linkptrs_start[TOTAL_LANES]) {

	sword->power = 30000;

	rcSword->x = player_out.x + player_out.w - 2 * ZOOM_MULT;
	rcSword->y = laneheight[currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	rcSword->w = rcSwordSrc->w * ZOOM_MULT * 2;
	rcSword->h = rcSwordSrc->h * ZOOM_MULT * 2;

	if ( sword->swing ) {

		if ( sword->down ) {
			(sword->count)--;
			if ( sword->count <= 0 ) {
				sword->down = 0;
			}
		}

		else if ( !(sword->down) ) {

			if ( sword->count == 0 ) {
				audio.soundchecklist[8] = 1;
			}

			(sword->count)++;

			if ( sword->count >= 2 ) {
				sword->down = 1;

				//int done = 0;
				struct monster_node *ptr2mon = linkptrs_start[currentlane];
				for ( int i = 0; i < monsterlanenum[currentlane]; i++ ) {
					if ( ptr2mon->monster_rect.x > ( player_out.x + player_out.w ) && (ptr2mon->status != -1)){
						if ( ptr2mon->monster_rect.x <= rcSword->x + rcSword->w ) {
							damage(currentlane, ptr2mon, sword->power);
						}
						else {
							//done = 1;
							break;
						}
					}
					ptr2mon = ptr2mon->next;
				}
				//if ( done )
				//	break;

			}
		}
	}



	rcSwordSrc->y = SWORD_HEIGHT * sword->count;

}

void quitlevel(SDL_Texture *Spriteimg, SDL_Texture *Timg, SDL_Texture *Laserimg, SDL_Texture *Swordimg, SDL_Renderer *renderer) {
	printf("Level Over.\n");

//	for ( int i = 0; i < MAX_SOUNDS_LIST; i++ )
//		soundchecklist[i] = 0;
	SDL_RenderClear(renderer);
	musicstop();
	soundstop();
	pthread_cond_wait( &cond_end, &track_mutex);
	pthread_mutex_unlock( &track_mutex );
	SDL_DestroyTexture(Spriteimg);
	SDL_DestroyTexture(Timg);
	SDL_DestroyTexture(Laserimg);
	SDL_DestroyTexture(Swordimg);
//	SDL_DestroyTexture(texTarget);
	timing.countbeats = 0;
	timing.currentbeat = 0;
	list_rm(render_node_head);
}


void damage(int currentlane, struct monster_node *ptr2mon, int power) {

	ptr2mon->health -= power;
	if (ptr2mon->health <= 0){
		ptr2mon->status = -1;
		audio.soundchecklist[7] = 1;
		program.score += 10;
//		(*monsterscreenstrip[currentscreen])[currentlane][((*moninfoptrs[screen])[currentlane][order][0])][0] = -1;
	}
}

void amihurt(struct status_struct status, struct monster_node *linkptrs_start[lanes.total], SDL_Rect player_out, struct monster *bestiary[10]) {

	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	//int done = 0;

	struct monster_node *ptr2mon = linkptrs_start[status.level->lanes->currentlane];

	for ( int i = 0; i < monsterlanenum[status.level->lanes->currentlane]; i++ ) {

		if ( ( ptr2mon->monster_rect.x > player_out.x ) && ( ptr2mon->monster_rect.x < ( player_out.x + player_out.w ) ) ) {

			/* Is the monster dead? */
			if ( ptr2mon->status != -1) {

	//			int order = (*moninfoptrs[screen])[currentlane][i][0];
	//			int order = i;
	//			int monstertype = samplemonstermap[currentlane][order][0];
				int monstertype = ptr2mon->montype;
				gethurt(status, (*bestiary[monstertype]).attack);
			}
			//done = 1;
			break;
		}
		ptr2mon = ptr2mon->next;
	}
	//if ( done )
	//	break;

}


void touchitem(int currentlane, int currentscreen, SDL_Rect player_out, struct item *itempokedex[10], int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int *levelover) {
	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	int done = 0;
	for ( int screen = 0; screen < 3; screen++ ) {

		for ( int i = 0; i < itemlanenum[screen][currentlane]; i++ ) {

			if ( ( rcItem[screen][currentlane][i].x > player_out.x ) && ( rcItem[screen][currentlane][i].x < ( player_out.x + player_out.w ) ) ) {

				/* Is the item "dead"? */
				if ( (*iteminfoptrs[screen])[currentlane][i] != -1) {

	//				int order = (*iteminfoptrs[screen])[currentlane][i][0];
					int order = i;
	//				int itemtype = sampleitemmap[currentlane][order][0];
					int itemtype = (*itemscreenstrip[currentscreen + screen])[currentlane][i][0];
					struct item thisitem = *itempokedex[itemtype];
					void (*functionptr)(int *int1, int int2, void *otherdata) = thisitem.functionptr;
					(*functionptr)(thisitem.int1, thisitem.int2, thisitem.otherdata);
					(*iteminfoptrs[screen])[currentlane][i] = -1;
					audio.soundchecklist[6] = 1;
				}
				done = 1;
				break;
			}
		}
		if ( done )
			break;
	}
}


int invinciblefunc(struct player_struct *player) {

	if ( player->invincibility != 0 ) {
		player->invinciblecounter[player->invincibility]--;
		if ( player->invinciblecounter[player->invincibility] <= 0 ) {
			player->invincibility = 0;
			player->invincibility_toggle = 1;
		}
	}

	return player->invincibility;
}

void gethurt(struct status_struct status, int attack) {

	if (status.player->invincibility == 0 ) {
		int damage = attack;
		status.player->HP -= damage;
		if ( status.player->HP <= 0 ) {
			status.audio->soundchecklist[5] = 1;
			status.level->levelover = 1;
		}
		else {
			status.audio->soundchecklist[4] = 1;
			status.player->invincibility = 1;
			status.player->invincibility_toggle = 1;
			status.player->invinciblecounter[1] = 40;
		}
	}
}

void restorehealth(int *HPptr, int restoreHPpts, void *otherdata) {
	struct player_struct *player = (struct player_struct *) otherdata; 
	if ( player->max_HP - *HPptr <= restoreHPpts ) {
		*HPptr = player->max_HP;
	}
	else
		*HPptr += restoreHPpts;

}

void restorepower(int *powerptr, int restorepowerpts, void *otherdata) { 

	struct player_struct *player = (struct player_struct *)otherdata;
	if ( player->max_PP - *powerptr <= restorepowerpts ) {
		*powerptr = player->max_PP;
	}
	else
		*powerptr += restorepowerpts;

}


void healthup(int *max_HPptr, int HPuppts, void *nullptr) {

		*max_HPptr += HPuppts;
}

void PPup(int *max_PPptr, int PPuppts, void *nullptr) {

		*max_PPptr += PPuppts;
}


//void int2array(int number, int (*array)[SCORE_DIGITS]) {
//        int digits = 1;
//        int numchecker = number;
//        while (numchecker /= 10)
//                digits++;
//
//        for (int i = 0; i < SCORE_DIGITS; i++ )
//                (*array)[i] = 0;
//
//        int diff = SCORE_DIGITS - digits;
//        for (int i = 0; i < SCORE_DIGITS; i++) {
//                if ( i < diff ) {
//                        (*array)[i] = 0;
//                }
//                else {
//                        (*array)[SCORE_DIGITS - (i - diff) - 1] = number%10;
//                        number /= 10;
//                }
//        }
//
//}
//
//
///* Function to delete the entire linked list [geeksforgeeks.org]*/
//void deleteList(struct monster_node** head_ref) {
//   /* deref head_ref to get the real head */
//   struct monster_node* current = *head_ref;
//   struct monster_node* next;
//
//   while (current != NULL) 
//   {
//       next = current->next;
//       free(current);
//       current = next;
//   }
//
//   /* deref head_ref to affect the real head back
//      in the caller. */
//   *head_ref = NULL;
//}
//
//


void emptyfunc() {
}
