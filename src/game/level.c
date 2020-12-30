/*	For the interactive python interpreter	*/
#include <Python.h>

#include "level.h"

#define SWORD_WIDTH 32
#define SWORD_HEIGHT 32 // Get rid of this, put it in animaions.cfg or something

//TODO: Layers functionality. Include spawnLayer() function (above or below)
/*	Global Variables	*/

//TODO: Fix this stuff
#define FIXME_SWITCH 0

/* Time */

int level_init (struct status_struct status) {

	int rc; // Return code

	/*	Get some structs	*/

	struct graphics_struct *master_graphics = status.master_graphics;

	/*	Some Variables	*/


	/* Status of the Level */

	struct time_struct *timing = status.timing;

	struct level_struct *level = status.level;
	*level = (struct level_struct) {0};

	level->stage = malloc(sizeof(*level->stage));
	*level->stage = (struct graphical_stage_child_struct) {0};
	spawn_graphical_stage_child(level->stage, master_graphics, level, "level");

	level->currentlevel = 1;
	level->lanes.total = TOTAL_LANES;

	struct lane_struct *lanes = &level->lanes;

	struct std_list **object_list_stack_ptr = &level->stage->graphics.object_list_stack;
	level->monster_list_stacks = malloc(sizeof(*level->monster_list_stacks) * level->lanes.total);
	for (int i = 0; i < level->lanes.total; i++) {
		level->monster_list_stacks[i] = NULL;
	}

	timing_init(timing);

	timing->pauselevel = &level->pauselevel;

	/* 	Initialisation	*/

	/* Setup Config File */
	config_t cfg;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "cfg/level.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
		FILEINFO
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
	config_setting_lookup_float(timing_setting, "container_width_per_beat", &confdub);
	timing->container_width_per_beat = (float)confdub;
	printf("container_width_per_beat:		%f\n", timing->container_width_per_beat);
	config_setting_lookup_float(timing_setting, "bpmin", &confdub);
	timing->bps = (float)confdub/60.0;
	printf("bps:		%f\n", timing->bps);
	config_setting_lookup_float(timing_setting, "startbeat", &confdub);
	timing->startbeat = (float)confdub;


	config_setting_t *level_setting = config_setting_lookup(thislevel_setting, "level");

	if (level_setting == NULL) {
		printf("No settings found under 'level' in the config file\n");
		return -1;
	}

	/* Lanes */

	config_setting_lookup_int(level_setting, "totallanes", &lanes->total);

	int lanewidthnative;
	config_setting_lookup_int(level_setting, "lanewidthnative", &lanewidthnative);

	lanes->containers = malloc(sizeof(struct visual_container_struct) * lanes->total);
	struct visual_container_struct *container_level_play_area = malloc(sizeof(struct visual_container_struct));
	*container_level_play_area = (struct visual_container_struct) {
		.inherit = &master_graphics->screen,
		.rect_out_parent_scale = (struct float_rect) {.x = 0, .y = 0.3, .w = 1, .h = 0.7},
		.aspctr_lock = WH_INDEPENDENT,
	};

	lanes->lanewidth = 0.2; //Fractional scaling! :O
	lanes->laneheight = malloc(sizeof (float) * lanes->total);
	for (int i = 0; i < lanes->total; i++) {
		lanes->laneheight[i] = 0.2*i;
	}

	lanes->currentlane = lanes->total/2;


	/*		Sprite		*/

	struct graphical_stage_struct *graphics = &level->stage->graphics;
	graphics->num_images = 100; //TODO

	/* Initialise the image dictionary and generic animation dictionary.
	   Both are out-parameters here. */
	rc = dicts_populate(&level->stage->graphics.generic_anim_dict, &level->stage->graphics.image_dict, &status, master_graphics->graphics.renderer);
	if (rc == R_FAILURE) {
		return R_FAILURE;
	}
	struct dict_void *generic_anim_dict = level->stage->graphics.generic_anim_dict;

	/* set sprite position */
	master_graphics->screen = (struct visual_container_struct) {
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
		.rect_out_parent_scale = (struct float_rect) { .x = 0.4, .y = 0, .w = 0.5, .h = 1},
		.aspctr_lock = H_DOMINANT,
	};

	spawn_player(object_list_stack_ptr, generic_anim_dict, graphics, player, &player_container, &status, player_setting, "player");


	/*		Tiles		*/

#if FIXME_SWITCH
	// set tile position
	int w, h;
	SDL_Texture *Timg = IMG_LoadTexture(master_graphics->renderer, TILE_PATH);
		
	SDL_QueryTexture(Timg, NULL, NULL, &w, &h); // get the width and height of the texture
#endif

	/* Create "film strip" of sequential background screens */
	level->currentscreen = 0;

	/*		HUD		*/

	struct ui_struct *ui = malloc(sizeof(struct ui_struct));
	level->ui = ui;

	struct visual_container_struct *container_level_ui_top = malloc(sizeof(struct visual_container_struct));
	*container_level_ui_top = (struct visual_container_struct) {
		.inherit = &master_graphics->screen,
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

	ui->beat = spawn_ui_counter(object_list_stack_ptr, generic_anim_dict, graphics, &timing->currentbeat_int, 5, beat_container, &status, "beat", &timing->currentbeat);

	/* REDOING HOW OBJECTS ARE QUEUED */
	/* Generate Monster Linked Lists */

	struct object_spawn_array_struct *object_spawn_arrays = level_init_object_spawn_arrays(thislevel_setting, level->lanes.total);
	level->object_spawn_arrays = object_spawn_arrays;


	//TODO: Redo items
#if 0
	/*		Items		*/

	SDL_Texture *Itemimg = IMG_LoadTexture(renderer, "../art/items.png");
	SDL_QueryTexture(Itemimg, NULL, NULL, &w, &h); // get the width and height of the texture

	struct item potion;

	potion.itemnumber = 0;
	potion.int1 = &player->living.HP;
	potion.int2 = 50; //Amount of HP restored by potion
	potion.otherdata = (void *)&player;
	potion.functionptr = NULL;//&restorehealth;
	memcpy(&potion.Src, &(int [2]){ 0, 0 }, sizeof potion.Src);
	memcpy(&potion.wh, &(int [2]){ 20,20 }, sizeof potion.wh);
	potion.image = &Itemimg;

	struct item laserjuice;

	laserjuice.itemnumber = 1;
	laserjuice.int1 = &player->living.power;
	laserjuice.int2 = 200; //Amount of HP restored by laserjuice
	laserjuice.otherdata = (void *)&player;
	laserjuice.functionptr = NULL;//&restorepower;
	memcpy(&laserjuice.Src, &(int [2]){ 0, 20 }, sizeof laserjuice.Src);
	memcpy(&laserjuice.wh, &(int [2]){ 20,20 }, sizeof laserjuice.wh);
	laserjuice.image = &Itemimg;

	struct item fist;

	fist.itemnumber = 2;
	fist.int1 = &player->living.max_PP;
	fist.int2 = 20; //Amount of PP increased
	fist.functionptr = NULL;//&PPup;
	memcpy(&fist.Src, &(int [2]){ 20, 0 }, sizeof fist.Src);
	memcpy(&fist.wh, &(int [2]){ 20,20 }, sizeof fist.wh);
	fist.image = &Itemimg;

	/* Item Functions */

	void (*itemfunctionarray[10])(int *int1ptr, int int2, void *otherdata);
	itemfunctionarray[0] = NULL;//&restorehealth;
	itemfunctionarray[1] = NULL;//&restorepower;
	//	(*itemfunctionarray[0])(&player->living.HP, 3);
#endif


	/*		Weapons		*/

	/* Laser Beam */

	struct laser_struct *laser = &level->laser;
	laser->power = 10;
	laser->count = 0;
	laser->on = 0;
	laser->turnon = 0;
	laser->turnoff = 0;

#if FIXME_SWITCH
	SDL_Texture *Laserimg = IMG_LoadTexture(master_graphics->renderer, LASER_PATH);
	SDL_QueryTexture(Laserimg, NULL, NULL, &w, &h); // get the width and height of the texture
#endif

	/* Sword */

	// TODO: Finish making the sword just a regular object
	struct visual_container_struct sword_container = (struct visual_container_struct) {
		.inherit = player->container,
		.rect_out_parent_scale = (struct float_rect) { .w = 1, .h = 1},
		.aspctr_lock = H_DOMINANT,
		.anchor_grabbed = &player->container->anchors_exposed[0],
		.anchor_hook = (struct size_ratio_struct) {.w = 0.05, .h = 0.8},
	};

	struct sword_struct *sword = &level->sword;
	//graphic_spawn(&sword->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){SWORD}, 1);
	spawn_sword(object_list_stack_ptr, generic_anim_dict, graphics, player, sword, &sword_container, &status, "sword");

#if FIXME_SWITCH
	SDL_Texture *Swordimg = IMG_LoadTexture(master_graphics->renderer, SWORD_PATH);
	SDL_QueryTexture(Swordimg, NULL, NULL, &w, &h); // get the width and height of the texture 
#endif

	/*		Maps & Mapstrips	*/

	/*	Tilemaps	*/

	/* Generate a sample background that is 1 screen large */
	//TODO?

	/*	Event Handling	*/

	struct level_var_struct *vars = malloc(sizeof(struct level_var_struct));
	*vars = (struct level_var_struct) {0};
	level->vars = vars;

	/* Play some music */

	//soundstart();

	const char *newtrack = NULL;
	config_setting_lookup_string(thislevel_setting, "track", &newtrack);
	if (!newtrack) {
		fprintf(stderr, "Couldn't find track for level\n");
		FILEINFO
	} else {
		playmusic(&status, newtrack, 0.2);
	}

	/* Snazzy Effects */

	struct level_effects_struct *effects = malloc(sizeof(struct level_effects_struct));
	level->effects = effects;
	level->effects->angle = 0.0,
	level->effects->colournum = 0,
	level->effects->hue = 0;



	level->vars->soundstatus = 0;

	/* Start beat counter */
	timing_zero(timing);
	update_time(timing);

	config_destroy(&cfg);
	return R_LOOP_LEVEL;
}

int level_loop(struct status_struct status) {

	/* Frame Loop */

	struct graphics_struct *master_graphics = status.master_graphics;
	struct audio_struct *audio = status.audio;
	struct level_struct *level = status.level;
	struct graphical_stage_struct *graphics = &level->stage->graphics;
	struct level_var_struct *vars = level->vars;
	struct player_struct *player = status.player;
	struct laser_struct *laser = &level->laser;
	struct sword_struct *sword = &level->sword;
	struct lane_struct *lanes = &level->lanes;
	//uint64_t cpustart, cpuend;
	struct time_struct *timing = status.timing;

	//cpustart = rdtsc();

	/* Hooks! */

	struct hooks_list_struct *hook = status.program->hooks.level_loop;
	while (hook) {
		hook->hookfunc(&status);
		hook = hook->next;
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

		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d) {
			*master_graphics->debug_anchors ^= 1;
			*master_graphics->debug_containers ^= 1;
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


		if ((e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT) ||
			(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN)) {
			if (lanes->currentlane < lanes->total - 1) {
				(lanes->currentlane)++;
			}
		}
		if ((e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT) ||
			(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP)) {
			if (lanes->currentlane > 0) {
				(lanes->currentlane)--;
			}
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
			if ( player->sword && sword->down ) {
				sword->swing = 1;
				sword->down = 0;
			}
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_f){
			if ( player->sword && !sword->down ) {
				sword->swing = 1;
				sword->down = 1;
			}
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

	/* Change music if necessary */

	//if ( audio->track != audio->newtrack ) {

	//	if (audio->noise) {
	//		musicstop();
	//	}
	//	else {
	//		rc = pthread_create(&threads, NULL, musicstart, (void*)&audio->newtrack);
	//		printf("NEW\n");
	//		if (rc) {
	//			printf("ya dun goofed. return code is %d\n.", rc);
	//			exit(-1);
	//		}
	//		audio->track = audio->newtrack;
	//	}
	//}

	/* The real business */

	/* Spawn in any objects that are ready */

	float currentbeat = timing->currentbeat;
	for (int lane = 0; lane < level->lanes.total; lane++) {
		struct object_spawn_array_struct *lane_array = &level->object_spawn_arrays[lane];

		while (lane_array->next_index < lane_array->num_objects) {
			struct object_spawn_elem_struct *lane_elem = &lane_array->objects[lane_array->next_index];
			float beatdiff = sword->dist_to_monster_spawn / timing->container_width_per_beat;
			float spawn_beat = lane_elem->spawn_beat - beatdiff;
			if ((currentbeat + 1) >= spawn_beat) { /* +1 to align with int beat */

				struct visual_container_struct hamster_container = {
					.inherit = &level->lanes.containers[lane],
					.rect_out_parent_scale = (struct float_rect) {.x = 0.9, .y = 0, .w = 1, .h = 1},
					.aspctr_lock = H_DOMINANT,
				};
				hamster_container.anchors_exposed = malloc(sizeof(hamster_container.anchors_exposed[0]));
				hamster_container.anchors_exposed[0] = (struct size_ratio_struct) {0.5, 0.5};

				lane_elem->ptr = spawn_flying_hamster(&status, &hamster_container, lane, spawn_beat);
				lane_array->next_index++;
			} else {
				break;
			}
		}
	}

	process_object_logics(graphics->object_list_stack);

	if (laser->on) {
		//laserfire(level, lanes->total, laser, player, rects->rcLaser, rects->rcLaserSrc, player->animation->rect_out, lanes->laneheight, lanes->currentlane, timing->framecount, level->currentscreen, level->effects->hue, audio->soundchecklist);
	}

	if ( player->sword )
		//swordfunc(level, sword, player->animation->rect_out, timing->framecount, linkptrs_start, *audio);

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
					FILEINFO
				}

				unpause_time(timing);

			} else {
				printf ("Some error with the function\n") ;
				FILEINFO
			}
		}
	}

	/* Clear screen */
#if FIXME_SWITCH
	if (status.level->partymode) {
		level->effects->angle ++;
		level->effects->colournum += 5;
		int r = level->effects->colournum%255;
		int g = (level->effects->colournum + 100)%255;
		int b = (level->effects->colournum + 200)%255;
		SDL_SetTextureColorMod(*graphics->tex_target_ptr, r,g,b);
		struct rendercopyex_struct rendercopyex_data = {
			.renderer = master_graphics->renderer,
			.texture = *graphics->tex_target_ptr,
			.srcrect = NULL,
			.dstrect = NULL,
			.angle = level->effects->angle,
			.center = NULL,
			.flip = SDL_FLIP_NONE,
		};
		graphics->rendercopyex_data = &rendercopyex_data;
	}
	else {
		graphics->rendercopyex_data = NULL;
	}
#endif

	render_process(graphics->object_list_stack, graphics, master_graphics, timing->currentbeat);

	//cpuend = rdtsc();
	return R_LOOP_LEVEL; //loop
}


/*Functions*/

void quitlevel(struct status_struct status) {
	struct graphics_struct *master_graphics = status.master_graphics;
	struct time_struct *timing = status.timing;
	printf("Level Over.\n");

	//SDL_RenderClear(master_graphics->renderer);
	//SDL_SetRenderTarget(master_graphics->renderer, *status.level->stage.graphics.tex_target_ptr);
	//SDL_RenderClear(master_graphics->renderer);
	//SDL_SetRenderTarget(master_graphics->renderer, NULL);
	stopmusic(status.audio);

	timing->countbeats = 0;
	timing->currentbeat = 0;

	struct graphical_stage_child_struct *stage = status.level->stage;
	exit_graphical_stage_child(stage);
	std_rm(&stage->std, &master_graphics->graphics.object_list_stack, &master_graphics->graphics, 1);
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
