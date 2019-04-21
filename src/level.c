/*	For the interactive python interpreter	*/
#include <Python.h>

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h" // Can hopefully get rid of this soon
#include "spawn.h"
#include "deprecated_funcs.h"

#define SWORD_WIDTH 32
#define SWORD_HEIGHT 32 // Get rid of this, put it in animaions.cfg or something

//TODO: Layers functionality. Include spawnLayer() function (above or below)
/*	Global Variables	*/

int debugcount = 0; //delete


/* Time */

int level_init (struct status_struct status) {

	int rc; // Return code

	/*	Get some structs	*/

	struct graphics_struct *graphics = status.graphics;
	graphics->render_node_head = NULL;
	graphics->render_node_tail = NULL;
	graphics->num_images = 0;
	SDL_Renderer *renderer = graphics->renderer;


	/*	Some Variables	*/

	struct render_node *r_node;

	/* Status of the Level */

	struct time_struct *timing = status.timing;
	struct audio_struct *audio = status.audio;

	struct level_struct *level = status.level;
	*level = (struct level_struct) {0};
	level->currentlevel = 1;
	level->lanes.total = TOTAL_LANES;

	struct lane_struct *lanes = &level->lanes;

	struct std_list **object_list_stack_ptr = &level->object_list_stack;
	level->monster_list_stacks = malloc(sizeof(*level->monster_list_stacks) * level->lanes.total);
	for (int i = 0; i < level->lanes.total; i++) {
		level->monster_list_stacks[i] = NULL;
	}

	timing_init(timing);

	timing->pauselevel = &level->pauselevel;

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
	timing->pxperbeat = (float)confdub;
	printf("pxpb:		%f\n", timing->pxperbeat);
	config_setting_lookup_float(timing_setting, "bpmin", &confdub);
	timing->bps = (float)confdub/60.0;
	printf("bps:		%f\n", timing->bps);
	config_setting_lookup_float(timing_setting, "startbeat", &confdub);
	timing->startbeat = (float)confdub;


	config_setting_t *level_setting = config_setting_lookup(thislevel_setting, "level");
	double smd;

	if (level_setting == NULL) {
		printf("No settings found under 'level' in the config file\n");
		return -1;
	}
	if(!config_setting_lookup_float(level_setting, "speedmult", &smd))
		fprintf(stderr, "'speedmult' not found in config file.\n");
	level->speedmult = (float) smd;

	if(!config_setting_lookup_float(level_setting, "speedmultmon", &smd))
		fprintf(stderr, "'speedmultmon' not found in config file.\n");
	level->speedmultmon = (float) smd;

	/* Lanes */

	config_setting_lookup_int(level_setting, "totallanes", &lanes->total);

	int lanewidthnative;
	config_setting_lookup_int(level_setting, "lanewidthnative", &lanewidthnative);

	lanes->containers = malloc(sizeof(struct visual_container_struct) * lanes->total);
	struct visual_container_struct *container_level_play_area = malloc(sizeof(struct visual_container_struct));
	*container_level_play_area = (struct visual_container_struct) {
		.inherit = &graphics->screen,
		.rect_out_parent_scale = (struct float_rect) {.x = 0, .y = 0.3, .w = 1, .h = 0.7},
		.aspctr_lock = WH_INDEPENDENT,
	};

	lanes->lanewidth = 0.2; //Fractional scaling! :O
	lanes->laneheight = malloc(sizeof (float) * lanes->total);
	for (int i = 0; i < lanes->total; i++) {
		lanes->laneheight[i] = 0.2*i;
	}

	lanes->currentlane = lanes->total/2;

	double *remainder = calloc(lanes->total, sizeof(double));//Means that sub-frame movement is accounted for
	level->remainder = remainder;
	for (int lane = 0; lane < lanes->total; lane++)
		remainder[lane] = 0.0;

	/* Declare Textures */
	struct rects_struct *rects = malloc(sizeof(struct rects_struct));
	*rects = (struct rects_struct) {0};
	level->rects = rects;

	SDL_Texture *Spriteimg = NULL;
	SDL_Texture *Laserimg = NULL;
	SDL_Texture *Swordimg = NULL;
	SDL_Texture *Timg = NULL;
	SDL_Texture *Mon0img = NULL;
	SDL_Texture *Mon1img = NULL;
	SDL_Texture *Scoreimg = NULL;
	SDL_Texture *Beatimg = NULL;
	SDL_Texture *Itemimg = NULL;

	int w, h;

	/*		Sprite		*/

	graphics->num_images = 100; //TODO

	/* Initialise the image dictionary and generic animation dictionary.
	   Both are out-parameters here. */
	rc = dicts_populate(&level->generic_anim_dict, &graphics->image_dict, &status, graphics->renderer);
	if (rc == R_FAILURE) {
		return R_FAILURE;
	}
	struct dict_str_void *generic_anim_dict = level->generic_anim_dict;

	/* set sprite position */
	graphics->screen = (struct visual_container_struct) {
		.inherit = NULL,
		.rect_out_parent_scale = (struct float_rect) { .x = 1, .y = 1, .w = 1, .h = 1},
		.aspctr_lock = WH_INDEPENDENT,
	};

	for (int lane = 0; lane < lanes->total; lane++) {
		lanes->containers[lane] = (struct visual_container_struct) {
			.inherit = container_level_play_area,
			.rect_out_parent_scale = (struct float_rect) { .x = 0, .y = lanes->laneheight[lane], .w = 1, .h = lanes->lanewidth},
			.aspctr_lock = WH_INDEPENDENT,
		};
	}

	/* Status of the Player */

	config_setting_t *player_setting = config_setting_lookup(thislevel_setting, "player");

	if (player_setting == NULL) {
		printf("No settings found for 'player' in the config file\n");
		return R_FAILURE;
	}
	
	struct player_struct *player = status.player;
	*player = (struct player_struct) {0};

	struct visual_container_struct player_container = (struct visual_container_struct) {
		.inherit = &lanes->containers[lanes->currentlane],
		.rect_out_parent_scale = (struct float_rect) { .x = 0.4, .y = 0, .w = 0.1, .h = 1},
		.aspctr_lock = WH_INDEPENDENT,
	};

	spawn_player(object_list_stack_ptr, generic_anim_dict, graphics, player, &player_container, &status, player_setting, "player");
	player->living.invincibility_toggle = 1; //REMOVE


	/*		Tiles		*/

	// set tile position
	Timg = IMG_LoadTexture(renderer, TILE_PATH);
		
	SDL_QueryTexture(Timg, NULL, NULL, &w, &h); // get the width and height of the texture

	/* Create "film strip" of sequential background screens */
	level->maxscreens = 300;
	level->currentscreen = 0;

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

	struct ui_struct *ui = malloc(sizeof(struct ui_struct));
	graphics->ui = ui;

	struct visual_container_struct *container_level_ui_top = malloc(sizeof(struct visual_container_struct));
	*container_level_ui_top = (struct visual_container_struct) {
		.inherit = &graphics->screen,
		.rect_out_parent_scale = (struct float_rect) {.x = 0, .y = 0, .w = 1, .h = 0.3},
		.aspctr_lock = WH_INDEPENDENT,
	};

	/*	HP	*/

	struct visual_container_struct *hp_container = malloc(sizeof(struct visual_container_struct));
	*hp_container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect_out_parent_scale = (struct float_rect) { .x = 0.15, .y = 0.6, .w = 0.2, .h = 0.2},
		.aspctr_lock = WH_INDEPENDENT,
		.anchor_hook = (struct size_ratio_struct) {.w = 0.5, .h = 0.5},
	};

	ui->hp = spawn_ui_bar(object_list_stack_ptr, generic_anim_dict, graphics, &player->living.HP, &player->living.max_HP, hp_container, &status, "hp");

	/*	Power	*/

	struct visual_container_struct *power_container = malloc(sizeof(struct visual_container_struct));
	*power_container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect_out_parent_scale = (struct float_rect) { .x = 0.85, .y = 0.6, .w = 0.2, .h = 0.2},
		.aspctr_lock = WH_INDEPENDENT,
		.anchor_hook = (struct size_ratio_struct) {.w = 0.5, .h = 0.5},
	};

	ui->power = spawn_ui_bar(object_list_stack_ptr, generic_anim_dict, graphics, &player->living.power, &player->living.max_PP, power_container, &status, "power");

	/*	Score	*/

	level->score = 0;

	struct visual_container_struct *score_container = malloc(sizeof(struct visual_container_struct));
	*score_container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect_out_parent_scale = (struct float_rect) { .x = 0.15, .y = 0.2, .w = 0.2, .h = 0.2},
		.aspctr_lock = WH_INDEPENDENT,
		.anchor_hook = (struct size_ratio_struct) {.w = 0.5, .h = 0.5},
	};

	ui->score = spawn_ui_counter(object_list_stack_ptr, generic_anim_dict, graphics, &level->score, 5, score_container, &status, "score", &timing->currentbeat);

	/*	Beat Counter	*/

	struct visual_container_struct *beat_container = malloc(sizeof(struct visual_container_struct));
	*beat_container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect_out_parent_scale = (struct float_rect) { .x = 0.85, .y = 0.2, .w = 0.2, .h = 0.2},
		.aspctr_lock = WH_INDEPENDENT,
		.anchor_hook = (struct size_ratio_struct) {.w = 0.5, .h = 0.5},
	};

	ui->beat = spawn_ui_counter(object_list_stack_ptr, generic_anim_dict, graphics, &timing->currentbeat_int, 5, score_container, &status, "score", &timing->currentbeat);

	/* REDOING HOW OBJECTS ARE QUEUED */
	/* Generate Monster Linked Lists */

	struct object_spawn_array_struct *object_spawn_arrays = level_init_object_spawn_arrays(thislevel_setting, level->lanes.total);
	level->object_spawn_arrays = object_spawn_arrays;


	/*		Items		*/

	Itemimg = IMG_LoadTexture(renderer, "../art/items.png");
	SDL_QueryTexture(Itemimg, NULL, NULL, &w, &h); // get the width and height of the texture

	struct item potion;

	potion.itemnumber = 0;
	potion.int1 = &player->living.HP;
	potion.int2 = 50; //Amount of HP restored by potion
	potion.otherdata = (void *)&player;
	potion.functionptr = &restorehealth;
	memcpy(&potion.Src, &(int [2]){ 0, 0 }, sizeof potion.Src);
	memcpy(&potion.wh, &(int [2]){ 20,20 }, sizeof potion.wh);
	potion.image = &Itemimg;

	struct item laserjuice;

	laserjuice.itemnumber = 1;
	laserjuice.int1 = &player->living.power;
	laserjuice.int2 = 200; //Amount of HP restored by laserjuice
	laserjuice.otherdata = (void *)&player;
	laserjuice.functionptr = &restorepower;
	memcpy(&laserjuice.Src, &(int [2]){ 0, 20 }, sizeof laserjuice.Src);
	memcpy(&laserjuice.wh, &(int [2]){ 20,20 }, sizeof laserjuice.wh);
	laserjuice.image = &Itemimg;

	struct item fist;

	fist.itemnumber = 2;
	fist.int1 = &player->living.max_PP;
	fist.int2 = 20; //Amount of PP increased
	fist.functionptr = &PPup;
	memcpy(&fist.Src, &(int [2]){ 20, 0 }, sizeof fist.Src);
	memcpy(&fist.wh, &(int [2]){ 20,20 }, sizeof fist.wh);
	fist.image = &Itemimg;

	struct item *(*itempokedex)[10]	= &level->itempokedex;;
	(*itempokedex)[0] = &potion;
	(*itempokedex)[1] = &laserjuice;
	(*itempokedex)[2] = &fist;

	/* Item Functions */

	void (*itemfunctionarray[10])(int *int1ptr, int int2, void *otherdata);
	itemfunctionarray[0] = &restorehealth;
	itemfunctionarray[1] = &restorepower;
	//	(*itemfunctionarray[0])(&player->living.HP, 3);


	/*		Weapons		*/

	/* Laser Beam */

	struct laser_struct *laser = &level->laser;
	laser->power = 10;
	laser->count = 0;
	laser->on = 0;
	laser->turnon = 0;
	laser->turnoff = 0;

	Laserimg = IMG_LoadTexture(renderer, LASER_PATH);
	SDL_QueryTexture(Laserimg, NULL, NULL, &w, &h); // get the width and height of the texture

	/* Sword */

	// TODO: Finish making the sword just a regular object
	struct sword_struct *sword = &level->sword;
	//graphic_spawn(&sword->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){SWORD}, 1);
	sword->rect_in.w = SWORD_WIDTH;
	sword->rect_in.h = SWORD_HEIGHT;

	Swordimg = IMG_LoadTexture(renderer, SWORD_PATH);
	SDL_QueryTexture(Swordimg, NULL, NULL, &w, &h); // get the width and height of the texture

	/*		Monsters	*/

	Mon0img = IMG_LoadTexture(renderer, "../art/flyinghamster.png");
	Mon1img = IMG_LoadTexture(renderer, "../art/angrycircle.png");
	SDL_QueryTexture(Mon0img, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_QueryTexture(Mon1img, NULL, NULL, &w, &h); // get the width and height of the texture

	struct visual_container_struct hamster_container = {
		.inherit = &lanes->containers[lanes->currentlane],
		.rect_out_parent_scale = (struct float_rect) {.x = 0.9, .y = 0, .w = 1, .h = 1},
		.aspctr_lock = H_DOMINANT,
	};

	spawn_flying_hamster(&status, &hamster_container, lanes->currentlane);
	struct monster *flyinghamster = malloc(sizeof(struct monster));

	flyinghamster->health = 1;
	flyinghamster->attack = 10;
	flyinghamster->defence = 10;
	memcpy(&flyinghamster->Src, &(int [2]){ 0, 0 }, sizeof flyinghamster->Src);
	memcpy(&flyinghamster->wh, &(int [2]){ 20,20 }, sizeof flyinghamster->wh);
	flyinghamster->image = Mon0img;
	flyinghamster->generic_bank_index = 1;

	struct monster *angrycircle = malloc(sizeof(struct monster));

	angrycircle->health = 500;
	angrycircle->attack = 10;
	angrycircle->defence = 10;
	memcpy(&angrycircle->Src, &(int [2]){ 0, 0 }, sizeof angrycircle->Src);
	memcpy(&angrycircle->wh, &(int [2]){ 20,20 }, sizeof angrycircle->wh);
	angrycircle->image = Mon1img;

	struct monster *monsterpokedex[10];
	monsterpokedex[0] = flyinghamster;
	monsterpokedex[1] = angrycircle;
	struct monster *(*bestiary)[10] = &level->bestiary;
	(*bestiary)[0] = flyinghamster;
	(*bestiary)[1] = angrycircle;


	/* Generate Monster Linked Lists */

	const config_setting_t *arrays_setting;
	arrays_setting = config_setting_lookup(thislevel_setting, "arrays");
	if (arrays_setting == NULL) {
		printf("No settings found for 'arrays' in the config file\n");
		return R_FAILURE;
	}

	int count = config_setting_length(arrays_setting);
	config_setting_t *singlearray_setting;

	struct monster_node *ptr2mon;
	for (int lane = 0; lane < lanes->total; lane++) {
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
				return R_FAILURE;
			}
			linkptrs_end[lane] = linkptrs_start[lane];
			monsterlanenum[lane] = 0;
			ptr2mon = linkptrs_start[lane];

			for (int index = 0; index < count; index++) {
				ptr2mon->montype = 0;
				ptr2mon->status = 0;
				ptr2mon->health = (*bestiary)[ptr2mon->montype]->health;
				ptr2mon->speed = 1;
				ptr2mon->entrybeat = lanearray[index];
				ptr2mon->remainder = 0;
				ptr2mon->monster_rect.w = 32 * ZOOM_MULT;
				ptr2mon->monster_rect.h = 32 * ZOOM_MULT;
				ptr2mon->monster_rect.x = graphics->width;
				ptr2mon->monster_rect.y = lanes->laneheight[lane] - ptr2mon->monster_rect.h/2;
				ptr2mon->monster_src.x = (*bestiary)[ptr2mon->montype]->Src[0];
				ptr2mon->monster_src.y = (*bestiary)[ptr2mon->montype]->Src[1];
				ptr2mon->monster_src.w = (*bestiary)[ptr2mon->montype]->wh[0];
				ptr2mon->monster_src.h = (*bestiary)[ptr2mon->montype]->wh[1];
				if (index < count - 1) {
					ptr2mon->next = malloc( sizeof(struct monster_node) );
					if (ptr2mon->next == NULL) {
						return R_FAILURE;
					}
					ptr2mon = ptr2mon->next;
				}
			}
			ptr2mon->next = NULL;
		}
	}

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

	int (*screenstrip[level->maxscreens])[grid.x][grid.y][2];


	/*	Items	*/

	int sampleitemmap[lanes->total][MAX_ITEMS_PER_LANE_PER_SCREEN][2];
	/*          lane number ^         ^ order	*/

	//int sampleitemmap[grid.x][grid.y][2];

	for (int i = 0; i < lanes->total; i++) {
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

	int (*itemscreen[level->maxscreens])[lanes->total][MAX_ITEMS_PER_LANE_PER_SCREEN][2];
	int (*(*itemscreenstrip)[level->maxscreens])[lanes->total][MAX_ITEMS_PER_LANE_PER_SCREEN][2] = malloc(sizeof(itemscreen) * level->maxscreens);
	level->itemscreenstrip = *itemscreenstrip;


	/*	Monsters*/


	/* The same for items, except the only info is if it has been collected		*/

	int iteminfoarray0[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN] = {0};
	int iteminfoarray1[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN] = {0};
	int iteminfoarray2[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN] = {0};

	/* Array of pointers to each iteminfoarray */

	iteminfoptrs[0] = &iteminfoarray0;
	iteminfoptrs[1] = &iteminfoarray1;
	iteminfoptrs[2] = &iteminfoarray2;



	//	multidrop(); //	dropin( "house", sampletilemapmid, 20, 20);


	//refreshtiles(screenstrip, monsterscreenstrip, itemscreenstrip, level->currentscreen, grid, rects->rcTile, rects->rcTilemid, rects->rcTSrc, rects->rcTSrcmid, 0, lanes.laneheight, monsterpokedex, (*itempokedex));

	/*	Event Handling	*/

	struct level_var_struct *vars = malloc(sizeof(struct level_var_struct));
	*vars = (struct level_var_struct) {0};
	level->vars = vars;

	/* Play some music */

	soundstart();

	config_setting_lookup_int(thislevel_setting, "track", &audio->newtrack);

	struct musicstart_struct {
		int newtrack;
		int *pause;
	};
	struct musicstart_struct musicstart_struct = {
		.newtrack = audio->newtrack,
		.pause = &level->pauselevel
	};
	//TODO:	Fix audio segfaults!
	//rc = pthread_create(&threads, NULL, musicstart, (void*)&musicstart_struct);
	//if (rc) {
	//	exit(-1);
	//}


	//pthread_mutex_lock( &track_mutex );
	audio->track = audio->newtrack;
	//pthread_mutex_unlock( &track_mutex );

	/* Snazzy Effects */

	struct level_effects_struct *effects = malloc(sizeof(struct level_effects_struct));
	level->effects = effects;
	level->effects->angle = 0.0,
	level->effects->colournum = 0,
	level->effects->hue = 0;

	/* Create a Master Render Target */

	graphics->texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, graphics->width, graphics->height);
	SDL_SetRenderTarget(renderer, graphics->texTarget);
	SDL_RenderClear(graphics->renderer);
	SDL_SetRenderTarget(renderer, NULL);


	level->vars->soundstatus = 0;

	return R_LOOP_LEVEL;
}

int level_loop(struct status_struct status) {

	pthread_t threads;
	extern pthread_cond_t soundstatus_cond;
	extern pthread_mutex_t soundstatus_mutex;
	int rc; //Return code

	/* Frame Loop */

	struct graphics_struct *graphics = status.graphics;
	struct audio_struct *audio = status.audio;
	struct level_struct *level = status.level;
	struct level_var_struct *vars = level->vars;
	struct player_struct *player = status.player;
	struct laser_struct *laser = &level->laser;
	struct sword_struct *sword = &level->sword;
	struct rects_struct *rects = level->rects;
	struct monster *(*bestiary)[10] = &level->bestiary;
	struct item *(*itempokedex)[10]	= &level->itempokedex;;
	struct lane_struct *lanes = &level->lanes;
	SDL_Renderer *renderer = graphics->renderer;
	uint64_t cpustart, cpuend;
	struct time_struct *timing = status.timing;

	cpustart = rdtsc();

	/* Hooks! */

	struct hooks_list_struct *hook = status.program->hooks.level_loop;
	while (hook) {
		hook->hookfunc(&status);
		hook = hook->next;
	}
	
	if ( !debugcount ) {
		printf("%f\n", timing->currentbeat);
		debugcount++;
	}
	if (timing->currentbeat >= 81) {
		level->speedmult = 3;
		level->speedmultmon = 3;
	}


	if (level->levelover) {
		quitlevel(status);
		return R_STARTSCREEN;
	}
	//pthread_mutex_lock( &soundstatus_mutex);
	//while(!vars->soundstatus) {
	//	pthread_cond_wait( &soundstatus_cond, &soundstatus_mutex );
	//}
	vars->soundstatus = 0;
	//pthread_mutex_unlock( &soundstatus_mutex);
	for (int i = 0; i < MAX_SOUNDS_LIST; i++ ) {
		if ( audio->soundchecklist[i] == 1 )
			audio->soundchecklist[i] = 0;
	}

	/* Handle Events */

	int direction;

	SDL_Event e;
	while ( SDL_PollEvent(&e) ) {

		vars->histwrite++;
		if (vars->histwrite >= 4){
			vars->histwrite = 0;}

		if (e.type == SDL_QUIT) {
			level->levelover = 1;
			quitlevel(status);
			return R_QUIT_TO_DESKTOP;
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {

			return R_PAUSE_LEVEL;
		}

		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) {
			audio->music_mute ^= 1;
		}

		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_h){
			lanes->currentlane = 0;
		}

		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_j){
			lanes->currentlane = 1;
		}

		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_k){
			lanes->currentlane = 2;
		}

		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_l){
			lanes->currentlane = 3;
		}


		if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_RIGHT){
			vars->directionbuttonlist[1] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT){
			vars->directionbuttonlist[1] = 1;
			vars->history[vars->histwrite] = 1;
			direction = 1;
		}

		else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LEFT){
			vars->directionbuttonlist[3] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT){
			vars->directionbuttonlist[3] = 1;
			vars->history[vars->histwrite] = 3;
			direction = 3;
		}

		else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_DOWN){
			vars->directionbuttonlist[2] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN){
			vars->directionbuttonlist[2] = 1;
			vars->history[vars->histwrite] = 2;
			direction = 2;
		}

		else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_UP){
			vars->directionbuttonlist[0] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP){
			vars->directionbuttonlist[0] = 1;
			vars->history[vars->histwrite] = 0;
			direction = 0;
		}

		if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a){
			vars->actionbuttonlist[0] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a){
			vars->actionbuttonlist[0] = 1;
			vars->acthistory[vars->acthistwrite] = 0;
		}

		else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s){
			vars->actionbuttonlist[1] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s){
			vars->actionbuttonlist[1] = 1;
			vars->acthistory[vars->acthistwrite] = 0;
		}

		else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d){
			vars->actionbuttonlist[2] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d){
			vars->actionbuttonlist[2] = 1;
			vars->acthistory[vars->acthistwrite] = 0;
		}

		else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_f){
			vars->actionbuttonlist[3] = 0;}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_f){
			vars->actionbuttonlist[3] = 1;
			vars->acthistory[vars->acthistwrite] = 0;
		}

		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_p){
			level->partymode = !level->partymode;
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_i){
			status.program->python_interpreter_activate = 1;
		}


	}

	int donehere = 0;

	if ( player->living.power > 0 ) {

		for (int i = 0; i < 3; i++) {
			if (vars->actionbuttonlist[i] == 1) {
				level->effects->hue = i;
				laser->turnoff = 0;

				if (laser->count < 4) {
					laser->on = 1;
					laser->turnon = 1;
					if ( laser->count == 0 )
						audio->soundchecklist[1] = 1;
				}
				else {
					audio->soundchecklist[2] = 2;
				}
				donehere = 1;
				break;
			}
		}
		if ( !donehere ) {
			if (laser->on)
				//laser->turnon = 0;
				laser->turnoff = 1;
			audio->soundchecklist[2] = 0;
			if ( laser->count == 4 ){
				audio->soundchecklist[3] = 1;
			}
		}

	}
	else {
		/* In the case the power has run out */
		if (laser->on)
			//laser->turnon = 0;
			laser->turnoff = 1;
		audio->soundchecklist[2] = 0;
	}

	if ( player->sword ) {

		if ( vars->actionbuttonlist[3] == !sword->down ) {
			sword->swing = 1;
		}
		else
			sword->swing = 0;
	}

	/* Change music if necessary */

	if ( audio->track != audio->newtrack ) {

		if (audio->noise) {
			musicstop();
		}
		else {
			rc = pthread_create(&threads, NULL, musicstart, (void*)&audio->newtrack);
			printf("NEW\n");
			if (rc) {
				printf("ya dun goofed. return code is %d\n.", rc);
				exit(-1);
			}
			audio->track = audio->newtrack;
		}
	}

	/* The real business */

	/* Spawn in any objects that are ready */

	float currentbeat = timing->currentbeat;
	for (int lane = 0; lane < level->lanes.total; lane++) {
		struct object_spawn_array_struct *lane_array = &level->object_spawn_arrays[lane];

		while (lane_array->next_index < lane_array->num_objects) {
			struct object_spawn_elem_struct *lane_elem = &lane_array->objects[lane_array->next_index];
			if (currentbeat >= lane_elem->spawn_beat) {

				struct visual_container_struct hamster_container = {
					.inherit = &level->lanes.containers[lane],
					.rect_out_parent_scale = (struct float_rect) {.x = 0.9, .y = 0, .w = 1, .h = 1},
					.aspctr_lock = H_DOMINANT,
				};

				lane_elem->ptr = spawn_flying_hamster(&status, &hamster_container, lane);
				lane_array->next_index++;
			} else {
				break;
			}
		}
	}

	struct std_list *current_obj = level->object_list_stack;
	while (current_obj) {
		struct std_list *new_obj = current_obj->prev;
		if (current_obj->std->object_logic) {
			current_obj->std->object_logic(current_obj->std, current_obj->std->object_data);
		}
		current_obj = new_obj; /* Do this so an object can delete itself and 
								  this loop still works */
	}

	moveme(lanes, &direction, player->animation);


	//movemap(&level, player, grid, rects->rcTile, rects->rcTilemid, rects->rcTSrc, rects->rcTSrcmid, lanes->total, screenstrip, monsterscreenstrip, itemscreenstrip, &monsterpokedex, &itempokedex);

	//movemon(lanes->total, level->speedmultmon, *timing, linkptrs_start, linkptrs_end, monsterlanenum, level->remainder, player->animation->rect_out, sword->rect_out, graphics->width);
	if (laser->on) {
		//laserfire(level, lanes->total, laser, player, rects->rcLaser, rects->rcLaserSrc, player->animation->rect_out, lanes->laneheight, lanes->currentlane, timing->framecount, level->currentscreen, level->effects->hue, audio->soundchecklist);
	}

	if ( player->sword )
		//swordfunc(level, sword, player->animation->rect_out, timing->framecount, linkptrs_start, *audio);

	invinciblefunc(player);

	//amihurt(lanes->total, status, linkptrs_start, player->animation->rect_out, (*bestiary));

	//touchitem(lanes, lanes->currentlane, level->currentscreen, player->animation->rect_out, (*itempokedex), level->itemscreenstrip, &level->levelover, audio->soundchecklist);

	/*	Drop into python interpreter */

	if (status.program->python_interpreter_enable) {
		if (status.program->python_interpreter_activate) {
			if (status.program->python_helper_function && PyCallable_Check(status.program->python_helper_function)) {
			
				/* Pause the timer to resume correctly */
				pause_time(timing);

				PyObject *pResult = PyObject_CallMethod(status.program->python_helper_function_generator,"send", "s", NULL);
				//printf("Called function that yielded %p\n", pResult);
				if (!pResult) {
					fprintf(stderr, "Error in Python interpreter:	");
					PyErr_Print();
				}

				unpause_time(timing);

			} else {
				printf ("Some error with the function\n") ;
			}
		}
	}

	/* Clear screen */
	SDL_RenderClear(renderer);
	/* Copy textures to renderer	*/
	for (int i = 0; i < grid.x * 3; i++){
		for (int j = 0; j < grid.y; j++){
			//SDL_RenderCopy(renderer, imgs->Timg, &rects->rcTSrc[i][j], &rcTile[i][j]);
		}
	}
	for (int i = 0; i < grid.x * 3; i++){
		for (int j = 0; j < grid.y; j++){
			//SDL_RenderCopy(renderer, imgs->Timg, &rects->rcTSrcmid[i][j], &rcTilemid[i][j]);
		}
	}

	for (int lane = 0; lane < lanes->total; lane++) {
		struct monster_node *ptr2mon = linkptrs_start[lane];
		for (int i = 0; i < monsterlanenum[lane]; i++ ) {
			if ( ptr2mon->status != -1){
				SDL_RenderCopy(renderer, (*(*bestiary)[ptr2mon->montype]).image, &(ptr2mon->monster_src), &(ptr2mon->monster_rect));
			}
			ptr2mon = ptr2mon->next;
		}
	}
	for (int lane = 0; lane < lanes->total; lane++) {
		for (int screen = 0; screen < 3; screen++) {
			for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
				if ( (*iteminfoptrs[screen])[lane][i] != -1){
					SDL_RenderCopy(renderer, *(*(*itempokedex)[(*level->itemscreenstrip[level->currentscreen + screen])[lane][i][0]]).image, &rcItemSrc[screen][lane][i], &rcItem[screen][lane][i]);

				}
			}
		}
	}
	if (laser->on){
		for (int i = 0; i < 3; i++) {
			//SDL_RenderCopy(renderer, imgs->Laserimg, &rects->rcLaserSrc[i], &rects->rcLaser[i]);
		}
	}

	if ( player->sword ) {
		//SDL_RenderCopy(renderer, imgs->Swordimg, &sword->rect_in, &sword->rect_out);
	}

	struct render_node *node_ptr = graphics->render_node_head;

	if (status.level->partymode) {
		level->effects->angle ++;
		level->effects->colournum += level->speedmult*5;
		int r = level->effects->colournum%255;
		int g = (level->effects->colournum + 100)%255;
		int b = (level->effects->colournum + 200)%255;
		SDL_SetTextureColorMod(graphics->texTarget, r,g,b);
		struct rendercopyex_struct rendercopyex_data = {
			.renderer = renderer,
			.texture = graphics->texTarget,
			.srcrect = NULL,
			.dstrect = NULL,
			.angle = level->effects->angle * level->speedmult,
			.center = NULL,
			.flip = SDL_FLIP_NONE
		};
		graphics->rendercopyex_data = &rendercopyex_data;
	}
	else {
		graphics->rendercopyex_data = NULL;
	}

	render_process(level->object_list_stack, graphics, timing);

	cpuend = rdtsc();
	return R_LOOP_LEVEL; //loop
}


/*Functions*/

void quitlevel(struct status_struct status) {
	struct graphics_struct *graphics = status.graphics;
	struct time_struct *timing = status.timing;
	printf("Level Over.\n");

	//	for ( int i = 0; i < MAX_SOUNDS_LIST; i++ )
	//		soundchecklist[i] = 0;
	SDL_RenderClear(graphics->renderer);
	SDL_SetRenderTarget(graphics->renderer, graphics->texTarget);
	SDL_RenderClear(graphics->renderer);
	SDL_SetRenderTarget(graphics->renderer, NULL);
	//musicstop();
	//soundstop();
	//pthread_mutex_lock( &track_mutex );
	//volatile extern int music_ended;
	//while(!music_ended) {
	//	pthread_cond_wait( &cond_end, &track_mutex);
	//}
	//pthread_mutex_unlock( &track_mutex );
	//	SDL_DestroyTexture(texTarget);
	timing->countbeats = 0;
	timing->currentbeat = 0;
	render_list_rm(&graphics->render_node_head);
}
struct object_spawn_array_struct *level_init_object_spawn_arrays(void *level_setting_void, int num_lanes) {

	config_setting_t *level_setting = level_setting_void;
	struct object_spawn_array_struct *object_spawn_arrays = malloc(sizeof(struct object_spawn_array_struct) * num_lanes);

	/* Initially set all to NULL */
	for (int lane = 0; lane < num_lanes; lane++) {
		object_spawn_arrays[lane] = (struct object_spawn_array_struct) {0};
	}

	const config_setting_t *arrays_setting = config_setting_lookup(level_setting, "arrays");
	
	if (arrays_setting == NULL) {
		printf("No settings found for 'arrays' in the config file\n");
		return NULL;
	}

	int count = config_setting_length(arrays_setting);
	config_setting_t *singlearray_setting;

	for (int lane = 0; lane < count; lane++) {
		singlearray_setting = config_setting_get_elem(arrays_setting, lane);
		if ((singlearray_setting == NULL) || (config_setting_length(singlearray_setting) <= 0)) {
			object_spawn_arrays[lane] = (struct object_spawn_array_struct) {0};
		}
		else {
			int lane_count = config_setting_length(singlearray_setting);
			object_spawn_arrays[lane] = (struct object_spawn_array_struct) {
				.next_index = 0, /* Index of next item to load by spawn_beat */
				.num_objects = lane_count,
				.objects = malloc(sizeof(struct object_spawn_elem_struct) * lane_count),
			};

			struct object_spawn_elem_struct *lane_array = object_spawn_arrays[lane].objects;

			for (int index = 0; index < lane_count; index++) {
				lane_array[index] = (struct object_spawn_elem_struct) {
					.spawn_beat = config_setting_get_float_elem(singlearray_setting, index),
					.ptr = NULL,
				};
			}
		}
	}
	return object_spawn_arrays;
}
