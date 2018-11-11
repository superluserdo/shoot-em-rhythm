#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "newstruct.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h" // Can hopefully get rid of this soon

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
	level->object_list_stack = NULL;
	struct std_list **object_list_stack_ptr = &level->object_list_stack;
	level->gameover = 0;
	level->levelover = 0;
	level->pauselevel = 0;
	level->currentlevel = 1;
	level->totalnativedist = 0;
	level->partymode = 0;
	level->lanes.total = TOTAL_LANES;

	struct lane_struct *lanes = &level->lanes;

	timing_init(timing);

	timing->pauselevel = &level->pauselevel;

	/* Status of the Player */

	struct player_struct *player = status.player;

	player->invincibility = 0;
	player->invincibility_toggle = 0;
	player->sword = 1;
	player->direction = 4;
	player->flydir = 1;
	player->self = player;
	printf("%d\n", player->sword);

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
	printf("pxpb = %f\n", timing->pxperbeat);
	config_setting_lookup_float(timing_setting, "bpmin", &confdub);
	timing->bps = (float)confdub/60.0;
	printf("bps is %f\n", timing->bps);
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

	int *laneheightarray = malloc(sizeof (laneheightarray) * lanes->total);
	lanes->containers = malloc(sizeof(struct visual_container_struct) * lanes->total);
	struct visual_container_struct *container_level_play_area = malloc(sizeof(struct visual_container_struct));
	*container_level_play_area = (struct visual_container_struct) {
		.inherit = &graphics->screen,
		.rect = (struct float_rect) {.x = 0, .y = 0.3, .w = 1, .h = 0.7}
	};

	//lanes->lanewidth = lanewidthnative * ZOOM_MULT; //in pixels
	lanes->lanewidth = 0.2; //Fractional scaling! :O
	lanes->laneheight = laneheightarray;

	for (int i = 0; i < lanes->total; i++) {
		//lanes->laneheight[i] = graphics->height + lanes->lanewidth * (-(lanes->total - 1) + i - 0.5);
		lanes->laneheight[i] = 1 - lanes->lanewidth * ((lanes->total - 1) - i + 0.5);
	}

	//TODO: Currently all laneheights used in functions are ints (pixels). Change to floats for container format!


	lanes->currentlane = lanes->total/2;

	double *remainder = calloc(lanes->total, sizeof(double));//Means that sub-frame movement is accounted for
	level->remainder = remainder;
	for (int lane = 0; lane < lanes->total; lane++)
		remainder[lane] = 0.0;

	/* Declare Textures */
	struct rects_struct *rects = malloc(sizeof(struct rects_struct));
	*rects = new_rects_struct();
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

	struct texture_struct *imgs = malloc(sizeof(struct texture_struct));
	status.graphics->imgs = imgs;

	int w, h;

	/*		Sprite		*/

	graphics->num_images = 100; //TODO

	//enum {
	//	sprite, flyinghamster, HP1, HP2
	//} img_enum;

	graphics->image_bank = malloc(sizeof(SDL_Texture *) * graphics->num_images);
	/*	Initialise the image bank - a fixed array of (as of now hardcoded) pointers to SDL_Textures
		that is used to copulate the "img" pointers in the clip sections of each generic struct
	*/
	rc = image_bank_populate(graphics->image_bank, renderer);
	if (rc != R_SUCCESS)
		return R_FAILURE;

	struct animate_generic **generic_bank;
	rc = generic_bank_populate(&generic_bank, graphics->image_bank, &status); //EXPERIMENTAL
	if (rc != R_SUCCESS)
		return R_FAILURE;

	/* set sprite position */
	graphics->screen = (struct visual_container_struct) {
		.inherit = NULL,
		.rect = (struct float_rect) { .x = 0, .y = 0, .w = 1, .h = 1}
	};
	for (int lane = 0; lane < lanes->total; lane++) {
		lanes->containers[lane] = (struct visual_container_struct) {
			.inherit = &graphics->screen,
			.rect = (struct float_rect) { .x = 0.25, .y = lanes->laneheight[lane], .w = 1, .h = lanes->lanewidth}
		};
	}

	player->container = malloc(sizeof(struct visual_container_struct));
	*player->container = (struct visual_container_struct) {
		.inherit = &lanes->containers[lanes->currentlane],
		.rect = (struct float_rect) { .x = 0.4, .y = 0, .w = 0.1, .h = 1}
		//.rect = { .x = 0.2, .y = lanes->lanewidth/2, .w = 16, .h = 22}
	};
	player->name = "player";
	player->pos.x = 0;//0.2 * graphics->width;
	player->pos.y = 0;//lanes->laneheight[lanes->currentlane] - POKESPRITE_SIZEX*ZOOM_MULT*2;
	player->size_ratio.w = 1.0;
	player->size_ratio.h = 1.0;
	//graphic_spawn(&player->std, generic_bank, graphics, (enum graphic_type_e[]){PLAYER}, 1);
	graphic_spawn(&player->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){PLAYER2}, 1);
	player->animation->rules_list->data = (void *)&status;

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

	/*	HP	*/

	struct ui_struct *ui = malloc(sizeof(struct ui_struct));
	graphics->ui = ui;

	struct ui_bar *hp = &ui->hp;

	struct visual_container_struct *container_level_ui_top = malloc(sizeof(struct visual_container_struct));
	*container_level_ui_top = (struct visual_container_struct) {
		.inherit = &graphics->screen,
		.rect = (struct float_rect) {.x = 0, .y = 0, .w = 1, .h = 0.3}
	};
	hp->container = malloc(sizeof(struct visual_container_struct));
	*hp->container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect = (struct float_rect) { .x = 0.15, .y = 0.6, .w = 0.2, .h = 0.2}
	};
	hp->amount = &player->HP;
	hp->max = &player->max_HP;
	hp->name = "hp";
	hp->pos.x = 10 * ZOOM_MULT * 2;
	hp->pos.y = 10 * ZOOM_MULT * 2;
	hp->size_ratio.w = 1.0;
	hp->size_ratio.h = 1.0;
	hp->self = hp;
	graphic_spawn(&hp->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){HP, COLOURED_BAR}, 2);

	hp->animation->next->native_offset.x = 29;
	hp->animation->next->native_offset.y = 6;

	hp->animation->next->rules_list->data = hp->animation->next;
	struct tr_bump_data *tmp_data = malloc(sizeof(struct tr_bump_data));
	*tmp_data = *(struct tr_bump_data *)hp->animation->next->rules_list->next->data;
	tmp_data->centre = malloc(sizeof(struct xy_struct));
	tmp_data->centre->x = hp->animation->rect_out.x + hp->animation->rect_out.w/2;
	tmp_data->centre->y = hp->animation->rect_out.y + hp->animation->rect_out.h/2;
	hp->animation->next->rules_list->next->data = tmp_data;
	hp->animation->next->transform_list->data = tmp_data;


	config_setting_t *player_setting = config_setting_lookup(thislevel_setting, "player");

	if (player_setting == NULL) {
		printf("No settings found for 'player' in the config file\n");
		return R_FAILURE;
	}
	config_setting_lookup_int(player_setting, "max_HP", &player->max_HP);
	player->HP = player->max_HP;


	/*	Power	*/

	struct ui_bar *power = &ui->power;

	power->container = malloc(sizeof(struct visual_container_struct));
	*power->container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect = (struct float_rect) { .x = 0.85, .y = 0.6, .w = 0.2, .h = 0.2}
	};
	power->amount = &player->power;
	power->max = &player->max_PP;
	power->name = "power";
	power->pos.x = 10 * ZOOM_MULT * 2;
	power->pos.y = 10 * ZOOM_MULT * 2;
	power->size_ratio.w = 1.0;
	power->size_ratio.h = 1.0;
	power->self = power;
	graphic_spawn(&power->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){POWER, COLOURED_BAR}, 2);
	power->pos.x = (NATIVE_RES_X - 20) * ZOOM_MULT - power->animation->rect_out.w;
	//printf("%d\n", power.animation->rect_out.w);

	power->animation->next->native_offset.x = 48;
	power->animation->next->native_offset.y = 6;

	power->animation->next->rules_list->data = power->animation->next;
	tmp_data = malloc(sizeof(struct tr_bump_data));
	*tmp_data = *(struct tr_bump_data *)power->animation->next->rules_list->next->data;
	tmp_data->centre = malloc(sizeof(struct xy_struct));
	tmp_data->centre->x = power->pos.x + power->animation->rect_out.w/2;
	tmp_data->centre->y = power->pos.y + power->animation->rect_out.h/2;
	power->animation->next->rules_list->next->data = tmp_data;
	power->animation->next->transform_list->data = tmp_data;

	config_setting_lookup_int(player_setting, "max_PP", &player->max_PP);
	player->power = player->max_PP;

	/*	Score	*/

	Scoreimg = IMG_LoadTexture(renderer, "../art/numbers.png");
	SDL_QueryTexture(Scoreimg, NULL, NULL, &w, &h); // get the width and height of the texture

	//SDL_Rect rcScore[5], rcScoreSrc[5];

	for (int i = 0; i < 5; i++ ) {
		rects->rcScoreSrc[i].x = 5 * i;
		rects->rcScoreSrc[i].y = 0;
		rects->rcScoreSrc[i].w = 5;
		rects->rcScoreSrc[i].h = 8;

		rects->rcScore[i].x = (48 + rects->rcScoreSrc[i].w * i) * ZOOM_MULT * 2;
		rects->rcScore[i].y = 6 * ZOOM_MULT * 2;
		rects->rcScore[i].w = rects->rcScoreSrc[i].w * ZOOM_MULT * 2;
		rects->rcScore[i].h = rects->rcScoreSrc[i].h * ZOOM_MULT * 2;

	}

	level->score = 0;

	struct ui_counter *score = &ui->score;

	score->container = malloc(sizeof(struct visual_container_struct));
	*score->container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect = (struct float_rect) { .x = 0.15, .y = 0.2, .w = 0.2, .h = 0.2}
	};
	score->value = &level->score;
	score->digits = 5;
	score->array = calloc(5, sizeof(int));//(int[5]){0};
	score->name = "score";
	score->pos.x = 48 * ZOOM_MULT * 2;
	score->pos.y = 6 * ZOOM_MULT * 2;
	score->size_ratio.w = 1.0;
	score->size_ratio.h = 1.0;
	score->self = score;

	graphic_spawn(&score->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){NUMBERS,NUMBERS,NUMBERS,NUMBERS,NUMBERS}, 5);
	//printf("%d\n", power.animation->rect_out.w);

	score->animation->rules_list->next = malloc(sizeof(struct rule_node));
	tmp_data = malloc(sizeof(struct tr_bump_data));
	//*tmp_data = *(struct tr_bump_data *)score.animation->rules_list->next->data;
	tmp_data->freq_perbeat = 1;
	tmp_data->ampl = 2;
	tmp_data->peak_offset = 0.0;
	tmp_data->bump_width = 0.25;
	tmp_data->centre = malloc(sizeof(struct xy_struct));
	tmp_data->centre->x = score->pos.x + score->animation->rect_out.w * score->digits/2;
	tmp_data->centre->y = score->pos.y + score->animation->rect_out.h/2;
	tmp_data->status = &status;

	struct animate_specific *anim = score->animation;
	anim->rules_list->data = anim;
	for (int i = 0; i < score->digits; i++) {
		anim->native_offset.x = anim->generic->clips[anim->clip]->frames[anim->frame].rect.w * i;
		anim->native_offset.y = 0;
		anim->rules_list->data = tmp_data;
		anim->transform_list->data = tmp_data;
		anim = anim->next;
	}
	score->animation->rules_list->next->rule = rules_ui_counter;
	score->animation->rules_list->next->data = score->animation;
	score->animation->rules_list->next->next = NULL;

	/*	Beat Counter	*/

	struct ui_counter *beat = &ui->beat;

	beat->container = malloc(sizeof(struct visual_container_struct));
	*beat->container = (struct visual_container_struct) {
		.inherit = container_level_ui_top,
		.rect = (struct float_rect) { .x = 0.85, .y = 0.2, .w = 0.2, .h = 0.2}
	};
	beat->value = &timing->currentbeat_int;
	beat->digits = 5;
	beat->array = calloc(5, sizeof(int));//(int[5]){0};
	beat->name = "beat";
	beat->pos.x = 100 * ZOOM_MULT * 2;
	beat->pos.y = 6 * ZOOM_MULT * 2;
	beat->size_ratio.w = 2.0;
	beat->size_ratio.h = 2.0;
	beat->self = beat;

	graphic_spawn(&beat->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){NUMBERS,NUMBERS,NUMBERS,NUMBERS,NUMBERS}, 5);
	//printf("%d\n", power.animation->rect_out.w);

	beat->animation->rules_list->next = malloc(sizeof(struct rule_node));
	tmp_data = malloc(sizeof(struct tr_bump_data));
	//*tmp_data = *(struct tr_bump_data *)beat.animation->rules_list->next->data;
	tmp_data->freq_perbeat = 1;
	tmp_data->ampl = 2;
	tmp_data->peak_offset = 0.0;
	tmp_data->bump_width = 0.25;
	tmp_data->centre = malloc(sizeof(struct xy_struct));
	tmp_data->centre->x = beat->pos.x + beat->animation->rect_out.w * beat->digits/2;
	tmp_data->centre->y = beat->pos.y + beat->animation->rect_out.h/2;
	tmp_data->status = &status;

	anim = beat->animation;
	anim->rules_list->data = anim;
	for (int i = 0; i < beat->digits; i++) {
		anim->native_offset.x = anim->generic->clips[anim->clip]->frames[anim->frame].rect.w * beat->size_ratio.w * i;
		anim->native_offset.y = 0;
		anim->rules_list->data = tmp_data;
		anim->transform_list->data = tmp_data;
		anim = anim->next;
	}
	beat->animation->rules_list->next->rule = rules_ui_counter;
	beat->animation->rules_list->next->data = beat->animation;
	beat->animation->rules_list->next->next = NULL;

	Beatimg = IMG_LoadTexture(renderer, "../art/numbers.png");
	SDL_QueryTexture(Scoreimg, NULL, NULL, &w, &h); // get the width and height of the texture

	//SDL_Rect rcBeat[5], rcBeatSrc[5];

	for (int i = 0; i < 5; i++ ) {
		rects->rcBeatSrc[i].x = 5 * i;
		rects->rcBeatSrc[i].y = 0;
		rects->rcBeatSrc[i].w = 5;
		rects->rcBeatSrc[i].h = 8;

		rects->rcBeat[i].x = (50 + rects->rcBeatSrc[i].w * i) * ZOOM_MULT * 2 * 2;
		rects->rcBeat[i].y = 6 * ZOOM_MULT * 2;
		rects->rcBeat[i].w = rects->rcBeatSrc[i].w * ZOOM_MULT * 2 * 2;
		rects->rcBeat[i].h = rects->rcBeatSrc[i].h * ZOOM_MULT * 2 * 2;

	}

	/*		Items		*/

	Itemimg = IMG_LoadTexture(renderer, "../art/items.png");
	SDL_QueryTexture(Itemimg, NULL, NULL, &w, &h); // get the width and height of the texture

	struct item potion;

	potion.itemnumber = 0;
	potion.int1 = &player->HP;
	potion.int2 = 50; //Amount of HP restored by potion
	potion.otherdata = (void *)&player;
	potion.functionptr = &restorehealth;
	memcpy(&potion.Src, &(int [2]){ 0, 0 }, sizeof potion.Src);
	memcpy(&potion.wh, &(int [2]){ 20,20 }, sizeof potion.wh);
	potion.image = &Itemimg;

	struct item laserjuice;

	laserjuice.itemnumber = 1;
	laserjuice.int1 = &player->power;
	laserjuice.int2 = 200; //Amount of HP restored by laserjuice
	laserjuice.otherdata = (void *)&player;
	laserjuice.functionptr = &restorepower;
	memcpy(&laserjuice.Src, &(int [2]){ 0, 20 }, sizeof laserjuice.Src);
	memcpy(&laserjuice.wh, &(int [2]){ 20,20 }, sizeof laserjuice.wh);
	laserjuice.image = &Itemimg;

	struct item fist;

	fist.itemnumber = 2;
	fist.int1 = &player->max_PP;
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
	//	(*itemfunctionarray[0])(&player->HP, 3);


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
	//SDL_Rect rcTSrc[grid.x * 3][grid.y], rcTile[grid.x * 3][grid.y];
	//SDL_Rect rcTSrcmid[grid.x * 3][grid.y], rcTilemid[grid.x * 3][grid.y];
	//SDL_Rect rcLaser[3], rcLaserSrc[3];

	/* Sword */

	// TODO: Finish making the sword just a regular object
	struct sword_struct *sword = &level->sword;
	//graphic_spawn(&sword->std, generic_bank, graphics, (enum graphic_type_e[]){OBJECT}, 5);
	sword->count = 0;
	sword->down = 0;
	sword->swing = 0;
	sword->rect_in.w = SWORD_WIDTH;
	sword->rect_in.h = SWORD_HEIGHT;
	sword->rect_in.x = 0;
	sword->rect_in.y = 0;

	sword->rect_out.x = player->animation->rect_out.x + player->animation->rect_out.w - 2 * ZOOM_MULT; //set preliminary values for the sword's dimensions (otherwise the beat predictor gets it wrong the first time)
	sword->rect_out.y = lanes->laneheight[lanes->currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	sword->rect_out.w = sword->rect_in.w * ZOOM_MULT * 2;
	sword->rect_out.h = sword->rect_in.h * ZOOM_MULT * 2;

	Swordimg = IMG_LoadTexture(renderer, SWORD_PATH);
	SDL_QueryTexture(Swordimg, NULL, NULL, &w, &h); // get the width and height of the texture
	//SDL_Rect rcSword, rcSwordSrc;


	/*		Monsters	*/

	Mon0img = IMG_LoadTexture(renderer, "../art/flyinghamster.png");
	Mon1img = IMG_LoadTexture(renderer, "../art/angrycircle.png");
	SDL_QueryTexture(Mon0img, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_QueryTexture(Mon1img, NULL, NULL, &w, &h); // get the width and height of the texture

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
	//ptr2mon = linkptrs_start[2];
	imgs->Spriteimg = Spriteimg;
	imgs->Laserimg =  Laserimg;
	imgs->Swordimg =  Swordimg;
	imgs->Timg =      Timg;
	imgs->Mon0img =   Mon0img;
	imgs->Mon1img =   Mon1img;
	imgs->Scoreimg =  Scoreimg;
	imgs->Beatimg =   Beatimg;
	imgs->Itemimg =   Itemimg;

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

	int samplemonstermap[lanes->total][MAX_MONS_PER_LANE_PER_SCREEN][3];
	/*          lane number ^         ^ order	*/

	for (int i = 0; i < lanes->total; i++) {
		for (int j = 0; j < MAX_MONS_PER_LANE_PER_SCREEN; j++) {
			samplemonstermap[i][j][0] = -1;		//specifies the type of monster
			samplemonstermap[i][j][1] = 0;		//specifies monster position
			samplemonstermap[i][j][2] = 0;		//specifies monster HP
		}
	}

	int samplemonstermap2[lanes->total][MAX_MONS_PER_LANE_PER_SCREEN][3];

	for (int i = 0; i < lanes->total; i++) {
		for (int j = 0; j < MAX_MONS_PER_LANE_PER_SCREEN; j++) {
			samplemonstermap2[i][j][0] = -1;	//specifies the type on monster
			samplemonstermap2[i][j][1] = 0;		//specifies monster position
		}
	}

	int upperj = NATIVE_RES_X / PXPB;
	//for (int i = 1; i < lanes->total; i++) {
	for (int j = 0; j < upperj; j++) {

		samplemonstermap[3][j][0] = 0;
		samplemonstermap[3][j][1] = j * 83.24;
		samplemonstermap[3][j][2] = (*monsterpokedex[samplemonstermap[3][j][0]]).health;
	}
	//}

	samplemonstermap2[2][0][0] = 1;
	samplemonstermap2[2][0][1] = 0.5 * NATIVE_RES_X;
	samplemonstermap2[2][0][2] = (*monsterpokedex[samplemonstermap2[2][0][0]]).health;

	int (*monsterscreenstrip[level->maxscreens])[lanes->total][MAX_MONS_PER_LANE_PER_SCREEN][3];


	/*	Pointing Strip Entries to Maps	*/

	for (int i = 0; i < level->maxscreens; i++) {

		level->itemscreenstrip[i] = &sampleitemmap;
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


	//refreshtiles(screenstrip, monsterscreenstrip, itemscreenstrip, level->currentscreen, grid, rects->rcTile, rects->rcTilemid, rects->rcTSrc, rects->rcTSrcmid, 0, lanes.laneheight, monsterpokedex, (*itempokedex));

	/*	Event Handling	*/

	struct level_var_struct *vars = malloc(sizeof(struct level_var_struct));
	*vars = new_level_var_struct();
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

	//pthread_mutex_lock( &clock_mutex );
	//pthread_mutex_unlock( &clock_mutex );
	//pthread_mutex_lock(&display_mutex);
	//pthread_cond_wait(&display_cond, &display_mutex);
	//pthread_mutex_unlock(&display_mutex);

	/* Snazzy Effects */

	struct level_effects_struct *effects = malloc(sizeof(struct level_effects_struct));
	level->effects = effects;
	level->effects->angle = 0.0,
	level->effects->colournum = 0,
	level->effects->hue = 0;

	/* Create a Master Render Target */

	graphics->imgs->texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, graphics->width, graphics->height);
	SDL_SetRenderTarget(renderer, graphics->imgs->texTarget);
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
	struct texture_struct *imgs = graphics->imgs;
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
	//while (1) {

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
				timing->startpause = SDL_GetTicks();
				level->pauselevel = 1;
				timing->pause_change = 1;
				rc = pausefunc(renderer, graphics->imgs->texTarget, level->currentlevel, &status);
				timing->endpause = SDL_GetTicks();
				level->pauselevel = 0;
				timing->pause_change = 1;
				SDL_SetRenderTarget(renderer, NULL);

				if (rc != R_SUCCESS) {
					quitlevel(status);
					return rc;
				}
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
				player->direction = 1;
			}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LEFT){
				vars->directionbuttonlist[3] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT){
				vars->directionbuttonlist[3] = 1;
				vars->history[vars->histwrite] = 3;
				player->direction = 3;
			}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_DOWN){
				vars->directionbuttonlist[2] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN){
				vars->directionbuttonlist[2] = 1;
				vars->history[vars->histwrite] = 2;
				player->direction = 2;
			}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_UP){
				vars->directionbuttonlist[0] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP){
				vars->directionbuttonlist[0] = 1;
				vars->history[vars->histwrite] = 0;
				player->direction = 0;
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


		}

		int donehere = 0;

		if ( player->power > 0 ) {

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
		moveme(lanes, &player->direction, player->animation);


		//movemap(&level, player, grid, rects->rcTile, rects->rcTilemid, rects->rcTSrc, rects->rcTSrcmid, lanes->total, screenstrip, monsterscreenstrip, itemscreenstrip, &monsterpokedex, &itempokedex);

		movemon(lanes->total, level->speedmultmon, *timing, linkptrs_start, linkptrs_end, monsterlanenum, level->remainder, player->animation->rect_out, sword->rect_out, graphics->width);
		if (laser->on) {
			laserfire(level, lanes->total, laser, player, rects->rcLaser, rects->rcLaserSrc, player->animation->rect_out, lanes->laneheight, lanes->currentlane, timing->framecount, level->currentscreen, level->effects->hue, audio->soundchecklist);
		}

		if ( player->sword )
			swordfunc(level, sword, player->animation->rect_out, timing->framecount, linkptrs_start, *audio);
			//swordfunc(lanes->total, sword, &rects->rcSword, &rects->rcSwordSrc, player->animation->rect_out, lanes->laneheight, lanes->currentlane, timing->framecount, linkptrs_start, *audio);

		invinciblefunc(player);

		amihurt(lanes->total, status, linkptrs_start, player->animation->rect_out, (*bestiary));

		touchitem(lanes, lanes->currentlane, level->currentscreen, player->animation->rect_out, (*itempokedex), level->itemscreenstrip, &level->levelover, audio->soundchecklist);

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
				SDL_RenderCopy(renderer, imgs->Laserimg, &rects->rcLaserSrc[i], &rects->rcLaser[i]);
			}
		}

		if ( player->sword ) {
			SDL_RenderCopy(renderer, imgs->Swordimg, &sword->rect_in, &sword->rect_out);
		}

		int scorearray[SCORE_DIGITS];
		int2array(level->score, scorearray, SCORE_DIGITS);
		for ( int i = 0; i < 5; i++ ) {
			rects->rcScoreSrc[i].x = scorearray[i] * 5;
			//			SDL_RenderCopy(renderer, imgs->Scoreimg, &rcScoreSrc[i], &rcScore[i]);
		}

		int beatarray[SCORE_DIGITS];
		int2array(timing->currentbeat, beatarray, SCORE_DIGITS);
		for ( int i = 0; i < 5; i++ ) {
			rects->rcBeatSrc[i].x = beatarray[i] * 5;
			//			SDL_RenderCopy(renderer, imgs->Beatimg, &rcBeatSrc[i], &rcBeat[i]);
		}
		//advanceFrames(graphics->render_node_head, timing->currentbeat);
		//renderlist(graphics->render_node_head);
		struct render_node *node_ptr = graphics->render_node_head;
		//Detach the texture
		//SDL_SetRenderTarget(renderer, NULL);

		//SDL_RenderClear(renderer);

		if (status.level->partymode) {
			level->effects->angle ++;
			level->effects->colournum += level->speedmult*5;
			int r = level->effects->colournum%255;
			int g = (level->effects->colournum + 100)%255;
			int b = (level->effects->colournum + 200)%255;
			SDL_SetTextureColorMod(graphics->imgs->texTarget, r,g,b);
			struct rendercopyex_struct rendercopyex_data = {
				.renderer = renderer,
				.texture = graphics->imgs->texTarget,
				.srcrect = NULL,
				.dstrect = NULL,
				.angle = level->effects->angle * level->speedmult,
				.center = NULL,
				.flip = SDL_FLIP_NONE
			};
			graphics->rendercopyex_data = &rendercopyex_data;
			//SDL_RenderCopyEx(renderer, graphics->imgs->texTarget, NULL, NULL, level->effects->angle * level->speedmult, NULL, SDL_FLIP_NONE);
		}
		else {
			graphics->rendercopyex_data = NULL;
			//SDL_RenderCopy(renderer, graphics->imgs->texTarget, NULL, NULL);
		}

		//pthread_mutex_lock(&display_mutex);
		//pthread_cond_wait(&display_cond, &display_mutex);
		
		//int delay_time =wait_to_present(timing);
		//printf("Delaying by %d ms\n", delay_time);
		//SDL_Delay(delay_time);
		//SDL_RenderPresent(renderer);
		//update_time(timing);
		render_process(level->object_list_stack, graphics, timing);

		//pthread_mutex_unlock(&display_mutex);

		//SDL_SetRenderTarget(renderer, graphics->imgs->texTarget);
		cpuend = rdtsc();
		//printf("%d\n", cpuend-cpustart);
	//}
	return R_LOOP_LEVEL; //loop
}


/*Functions*/



void moveme(struct lane_struct *lanes, int *direction, struct animate_specific *anim) {

	if (*direction < 4) {
		if (*direction == 0 || *direction == 3) {
			if (lanes->currentlane > 0) {
				(lanes->currentlane)--;
			}
		}
		if (*direction == 2 || *direction == 1) {
			if (lanes->currentlane < lanes->total - 1) {
				(lanes->currentlane)++;
			}
		}
		*direction = 4;
	}
	/* set sprite position */
	//anim->parent->pos.y = lanes->laneheight[lanes->currentlane] - POKESPRITE_SIZEX*ZOOM_MULT*2;
	anim->parent->container->inherit = &lanes->containers[lanes->currentlane];

}

void movemap(struct level_struct *level, struct player_struct *player_ptr, struct xy_struct grid, SDL_Rect rcTile[grid.x * 3][grid.y], SDL_Rect rcTilemid[grid.x * 3][grid.y], SDL_Rect rcTSrc[grid.x][grid.y], SDL_Rect rcTSrcmid[grid.x][grid.y], int totallanes, int (*screenstrip [level->maxscreens]) [grid.x * 3][grid.y][2], int (*itemscreenstrip[level->maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], struct monster *(*bestiary)[10], struct item *(*itempokedex)[10]){
	if (player_ptr->flydir == 0){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].y += 4 * ZOOM_MULT * level->speedmult;
				rcTilemid[i][j].y += 4 * ZOOM_MULT * level->speedmult;
			}
		}
	}
	if (player_ptr->flydir == 1 ){
		if (rcTile[grid.x][0].x <= 0 ) {
			int frameoffset = rcTile[grid.x][0].x;
			(level->currentscreen)++;
			if (level->currentscreen >= level->maxscreens - 3){
				level->levelover = 1;
			}
			refreshtiles(totallanes, screenstrip, itemscreenstrip, level->currentscreen, grid, rcTile, rcTilemid, rcTSrc, rcTSrcmid, frameoffset, level->lanes.laneheight, *bestiary, *itempokedex);
		}

		if ( !(level->levelover) ) {
			for (int i = 0; i < grid.x * 3; i++){
				for(int j = 0; j < grid.y; j++){
					rcTile[i][j].x -= 4 * ZOOM_MULT * level->speedmult;
					rcTilemid[i][j].x -= 4 * ZOOM_MULT * level->speedmult;
					level->totalnativedist += 4 * level->speedmult;
				}
			}
			for ( int screen = 0; screen < 3; screen++ ) {
				for (int lane = 0; lane < level->lanes.total; lane++ ) {
					for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
						rcItem[screen][lane][i].x -= 4 * ZOOM_MULT * level->speedmult;
					}
				}
			}
		}
	}

	if (player_ptr->flydir == 2){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].y -= 4 * ZOOM_MULT * level->speedmult;
				rcTilemid[i][j].y -= 4 * ZOOM_MULT * level->speedmult;
			}
		}
	}

	if (player_ptr->flydir == 3){
		for (int i = 0; i < grid.x * 3; i++){
			for(int j = 0; j < grid.y; j++){
				rcTile[i][j].x += 4 * ZOOM_MULT * level->speedmult;
				rcTilemid[i][j].x += 4 * ZOOM_MULT * level->speedmult;
			}
		}
	}


}


void movemon(int totallanes, float speedmultmon, struct time_struct timing, struct monster_node *linkptrs_start[totallanes], struct monster_node *linkptrs_end[totallanes], int monsterlanenum[totallanes], double remainder[totallanes], SDL_Rect player_out, SDL_Rect rcSword, int width) {

	struct monster_node *ptr2mon;
	float transspeed;
	for (int lane = 0; lane < totallanes; lane++) {
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
				//pthread_mutex_lock( &clock_mutex );
				float extra = (((float)(width - (player_out.x + player_out.w + rcSword.w/2)))/transspeed - 4*timing.intervalglobal/1000.0) * timing.bps;
				Dt = timing.currentbeat - linkptrs_end[lane]->entrybeat + extra;
				//printf("%f\n", extra);
				//pthread_mutex_unlock( &clock_mutex );
				if (Dt >= 0) {
					float Dx = Dt / timing.bps * speedmultmon * linkptrs_end[lane]->speed;
					linkptrs_end[lane]->monster_rect.x = width - Dx;
					linkptrs_end[lane] = linkptrs_end[lane]->next;
					monsterlanenum[lane]++;
				}
				else
					break;
			}
		}
	}

}


void refreshtiles(int totallanes, int (*screenstrip[]) [grid.x][grid.y][2], int (*itemscreenstrip[])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int currentscreen, struct xy_struct grid, SDL_Rect rcTile[grid.x][grid.y], SDL_Rect rcTilemid[grid.x][grid.y], SDL_Rect rcTSrc[grid.x][grid.y], SDL_Rect rcTSrcmid[grid.x][grid.y], int frameoffset, int laneheight[totallanes], struct monster *bestiary[10], struct item *itempokedex[10]) {

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
for (int lane = 0; lane < totallanes; lane++){
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
}

void laserfire(struct level_struct *level, int totallanes, struct laser_struct *laser, struct player_struct *player, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect player_out, int laneheight[totallanes], int currentlane, int framecount, int currentscreen, int hue, int *soundchecklist) {
	if (laser->turnon) {
		(laser->count)++;

		if (laser->count >= 4) {
			laser->turnon = 0;
		}
	}

	else if (laser->turnoff) {
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
				damage(currentlane, ptr2mon, laser->power, soundchecklist, level);
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

void swordfunc(struct level_struct *level, struct sword_struct *sword, SDL_Rect player_out, int framecount, struct monster_node *linkptrs_start[TOTAL_LANES], struct audio_struct audio) {

	struct lane_struct *lanes = &level->lanes;
	
	sword->power = 30000;

	sword->rect_out.x = player_out.x + player_out.w - 2 * ZOOM_MULT;
	sword->rect_out.y = lanes->laneheight[lanes->currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	sword->rect_out.w = sword->rect_in.w * ZOOM_MULT * 2;
	sword->rect_out.h = sword->rect_in.h * ZOOM_MULT * 2;

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

				struct monster_node *ptr2mon = linkptrs_start[lanes->currentlane];
				for ( int i = 0; i < monsterlanenum[lanes->currentlane]; i++ ) {
					if ( ptr2mon->monster_rect.x > ( player_out.x + player_out.w ) && (ptr2mon->status != -1)){
						if ( ptr2mon->monster_rect.x <= sword->rect_out.x + sword->rect_out.w ) {
							damage(lanes->currentlane, ptr2mon, sword->power, audio.soundchecklist, level);
						}
						else {
							break;
						}
					}
					ptr2mon = ptr2mon->next;
				}

			}
		}
	}



	sword->rect_in.y = SWORD_HEIGHT * sword->count;
}

void quitlevel(struct status_struct status) {
	struct graphics_struct *graphics = status.graphics;
	struct time_struct *timing = status.timing;
	printf("Level Over.\n");

	//	for ( int i = 0; i < MAX_SOUNDS_LIST; i++ )
	//		soundchecklist[i] = 0;
	SDL_RenderClear(graphics->renderer);
	SDL_SetRenderTarget(graphics->renderer, graphics->imgs->texTarget);
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
	SDL_DestroyTexture(graphics->imgs->Spriteimg);
	SDL_DestroyTexture(graphics->imgs->Timg);
	SDL_DestroyTexture(graphics->imgs->Laserimg);
	SDL_DestroyTexture(graphics->imgs->Swordimg);
	//	SDL_DestroyTexture(texTarget);
	timing->countbeats = 0;
	timing->currentbeat = 0;
	list_rm(graphics->render_node_head);
}


void damage(int currentlane, struct monster_node *ptr2mon, int power, int *soundchecklist, struct level_struct *level) {

	ptr2mon->health -= power;
	if (ptr2mon->health <= 0){
		ptr2mon->status = -1;
		soundchecklist[7] = 1;
		level->score += 10;
		//		(*monsterscreenstrip[currentscreen])[currentlane][((*moninfoptrs[screen])[currentlane][order][0])][0] = -1;
	}
}

void amihurt(int totallanes, struct status_struct status, struct monster_node *linkptrs_start[totallanes], SDL_Rect player_out, struct monster *bestiary[10]) {

	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	struct monster_node *ptr2mon = linkptrs_start[status.level->lanes.currentlane];

	for ( int i = 0; i < monsterlanenum[status.level->lanes.currentlane]; i++ ) {

		if ( ( ptr2mon->monster_rect.x > player_out.x ) && ( ptr2mon->monster_rect.x < ( player_out.x + player_out.w ) ) ) {

			/* Is the monster dead? */
			if ( ptr2mon->status != -1) {

				int monstertype = ptr2mon->montype;
				gethurt(status, (*bestiary[monstertype]).attack);
			}
			break;
		}
		ptr2mon = ptr2mon->next;
	}

}


void touchitem(struct lane_struct *lanes, int currentlane, int currentscreen, SDL_Rect player_out, struct item *itempokedex[10], int (*itemscreenstrip[])[lanes->total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int *levelover, int *soundchecklist) {
	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	int done = 0;
	for ( int screen = 0; screen < 3; screen++ ) {

		for ( int i = 0; i < itemlanenum[screen][currentlane]; i++ ) {

			if ( ( rcItem[screen][currentlane][i].x > player_out.x ) && ( rcItem[screen][currentlane][i].x < ( player_out.x + player_out.w ) ) ) {

				/* Is the item "dead"? */
				if ( (*iteminfoptrs[screen])[currentlane][i] != -1) {

					int order = i;
					int itemtype = (*itemscreenstrip[currentscreen + screen])[currentlane][i][0];
					struct item thisitem = *itempokedex[itemtype];
					void (*functionptr)(int *int1, int int2, void *otherdata) = thisitem.functionptr;
					(*functionptr)(thisitem.int1, thisitem.int2, thisitem.otherdata);
					(*iteminfoptrs[screen])[currentlane][i] = -1;
					soundchecklist[6] = 1;
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

void emptyfunc() {
}
