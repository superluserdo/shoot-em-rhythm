#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "main.h"
#include "level.h"
#include "music.h"
#include "helpers.h"

/*	Global Variables	*/

/* Status of the Level */

//TODO: Convert into level.gameover, level.pause, etc...
int gameover = 0;
int pauselevel = 0;
int currentlevel = 1;
int partymode = 0;

/* Status of the Player */

int HP, power, score;
int max_HP;
int max_PP;
int invincibility = 0;
int invinciblecounter[2];
int sword = 1;

/* Audio */

int track = 0;
int newtrack = 0;
int noise = 0;

/* Graphics */

int local_grid_x;
int local_grid_y;
int totallanes = TOTAL_LANES;
int maxscreens;
int totalnativedist = 0;

int level (SDL_Window *win, SDL_Renderer *renderer) {//(int argc, char *argv[]) {

	/*	Some Variables	*/

	int levelover = 0;
	float speedmult;
	float speedmultmon;

	int direction = 4;
	int flydir = 1;


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

	double smd;

	if(!config_lookup_float(&cfg, "speedmult", &smd))
		fprintf(stderr, "'speedmult' not found in config file.\n");

	speedmult = (float) smd;

	if(!config_lookup_float(&cfg, "speedmultmon", &smd))
		fprintf(stderr, "'speedmultmon' not found in config file.\n");

	speedmultmon = (float) smd;

	/* Lanes */

	int currentlane;
	int lanewidthnative;
	config_lookup_int(&cfg, "lanewidthnative", &lanewidthnative);
	int lanewidth = lanewidthnative * ZOOM_MULT; //in pixels

	int laneheight[totallanes];

	for (int i = 0; i < totallanes; i++) {
		laneheight[i] = height + lanewidth * (-(totallanes - 1) + i - 0.5);
	}


	if (totallanes%2 == 0)
		currentlane = totallanes/2;
	else
		currentlane = (totallanes-1)/2;

	float remainder[totallanes];	//Means that sub-frame movement is accounted for
	for (int lane = 0; lane < totallanes; lane++)
		remainder[lane] = 0.0;


	/* Declare Textures */

	SDL_Texture *img = NULL;
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

	img = IMG_LoadTexture(renderer, SPRITE_PATH);

	SDL_QueryTexture(img, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_Rect rcSrc, rcSprite;

	/* set animation frame */
	rcSrc.x = 0;
	rcSrc.y = 0;
	rcSrc.w = POKESPRITE_SIZEX;
	rcSrc.h = POKESPRITE_SIZEY;

	/*		Tiles		*/

	// set tile position
	Timg = IMG_LoadTexture(renderer, TILE_PATH);
	SDL_QueryTexture(Timg, NULL, NULL, &w, &h); // get the width and height of the texture

	/* Create "film strip" of sequential background screens */
	maxscreens = 300;
	int currentscreen = 0;

	if (NATIVE_RES_X % TILE_SIZE == 0) {
		local_grid_x = NATIVE_RES_X / TILE_SIZE;
	}
	else {
		local_grid_x = NATIVE_RES_X / TILE_SIZE + 1;
	}
	if (NATIVE_RES_Y % TILE_SIZE == 0) {
		local_grid_y = NATIVE_RES_Y / TILE_SIZE;
	}
	else {
		local_grid_y = NATIVE_RES_Y / TILE_SIZE + 1;
	}

	int sampletilemap[local_grid_x][local_grid_y][2];
	int sampletilemap2[local_grid_x][local_grid_y][2];
	int sampletilemapmid[local_grid_x][local_grid_y][2];
	int sampletilemaptop[local_grid_x][local_grid_y][2];

	/*		HUD		*/

	/*	HP	*/

	HPimg0 = IMG_LoadTexture(renderer, "../art/hp.png");
	SDL_QueryTexture(HPimg0, NULL, NULL, &w, &h); // get the width and height of the texture

	HPimg1 = IMG_LoadTexture(renderer, "../art/colourstrip.png");
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

	config_lookup_int(&cfg, "max_HP", &max_HP);
	HP = max_HP;

	/*	Power	*/

	Powerimg0 = IMG_LoadTexture(renderer, "../art/power.png");
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

	config_lookup_int(&cfg, "max_PP", &max_PP);
	power = max_PP;

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

	score = 0;

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
	potion.int1 = &HP;
	potion.int2 = 50; //Amount of HP restored by potion
	potion.functionptr = &restorehealth;	
	memcpy(&potion.Src, &(int [2]){ 0, 0 }, sizeof potion.Src);
	memcpy(&potion.wh, &(int [2]){ 20,20 }, sizeof potion.wh);
	potion.image = &Itemimg;

	struct item laserjuice;

	laserjuice.itemnumber = 1;
	laserjuice.int1 = &power;
	laserjuice.int2 = 200; //Amount of HP restored by laserjuice
	laserjuice.functionptr = &restorepower;	
	memcpy(&laserjuice.Src, &(int [2]){ 0, 20 }, sizeof laserjuice.Src);
	memcpy(&laserjuice.wh, &(int [2]){ 20,20 }, sizeof laserjuice.wh);
	laserjuice.image = &Itemimg;

	struct item fist;

	fist.itemnumber = 2;
	fist.int1 = &max_PP;
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

	void (*itemfunctionarray[10])(int *int1ptr, int int2);
	itemfunctionarray[0] = &restorehealth;
	itemfunctionarray[1] = &restorepower;
	(*itemfunctionarray[0])(&HP, 3);


	/*		Weapons		*/

	/* Laser Beam */

	int lasercount = 0;
	int laser_on = 0;
	int laser_turnon = 0;
	int laser_turnoff = 0;

	Laserimg = IMG_LoadTexture(renderer, LASER_PATH);
	SDL_QueryTexture(Laserimg, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_Rect rcTSrc[local_grid_x * 3][local_grid_y], rcTile[local_grid_x * 3][local_grid_y];
	SDL_Rect rcTSrcmid[local_grid_x * 3][local_grid_y], rcTilemid[local_grid_x * 3][local_grid_y];
	SDL_Rect rcLaser[3], rcLaserSrc[3];

	/* Sword */

	int swordcount = 0;
	int sword_down = 0;
	int sword_swing = 0;

	Swordimg = IMG_LoadTexture(renderer, SWORD_PATH);
	SDL_QueryTexture(Swordimg, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_Rect rcSword, rcSwordSrc;

	rcSwordSrc.w = SWORD_WIDTH;
	rcSwordSrc.h = SWORD_HEIGHT;
	rcSwordSrc.x = 0;
	rcSwordSrc.y = 0;

	rcSword.x = rcSprite.x + rcSprite.w - 2 * ZOOM_MULT; //set preliminary values for the sword's dimensions (otherwise the beat predictor gets it wrong the first time)
	rcSword.y = laneheight[currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	rcSword.w = rcSwordSrc.w * ZOOM_MULT * 2;
	rcSword.h = rcSwordSrc.h * ZOOM_MULT * 2;

	/*		Monsters	*/

	Mon0img = IMG_LoadTexture(renderer, "../art/flyinghamster.png");
	Mon1img = IMG_LoadTexture(renderer, "../art/angrycircle.png");
	SDL_QueryTexture(Mon0img, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_QueryTexture(Mon1img, NULL, NULL, &w, &h); // get the width and height of the texture

	struct monster flyinghamster;

	flyinghamster.health = 70;
	flyinghamster.attack = 10;
	flyinghamster.defence = 10;
	memcpy(&flyinghamster.Src, &(int [2]){ 0, 0 }, sizeof flyinghamster.Src);
	memcpy(&flyinghamster.wh, &(int [2]){ 20,20 }, sizeof flyinghamster.wh);
	flyinghamster.image = &Mon0img;

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

	int testarray[3][2] = {{1,1}, {2,2}, {2,3}};

	const config_setting_t *arraysetting;
	arraysetting = config_lookup(&cfg, "array");
	int count = config_setting_length(arraysetting);

	int lanearray[count];
			/*   ^{entrybeat, montype}	*/
	for (int index = 0; index < count; index++) {
		int element = config_setting_get_int_elem(arraysetting, index);
//		if (index%2 == 0) {
			lanearray[index] = element;//[0] = element;
//		}
//		else {
//			lanearray[index][1] = element;
//		}
	}


	struct node *ptr2mon;
	for (int lane = 0; lane < totallanes; lane++) {
		linkptrs_start[lane] = malloc( sizeof(struct node));
		if (linkptrs_start[lane] == NULL) {
			return 1;
		}
		linkptrs_end[lane] = linkptrs_start[lane];
		monsterlanenum[lane] = 0;
		ptr2mon = linkptrs_start[lane];

		if (lane != 2) {
			linkptrs_start[lane] = NULL;
		}

		if (lane == 2){
		for (int index = 0; index < count; index++) {
			ptr2mon->montype = 1;
			ptr2mon->status = 0;
			ptr2mon->health = bestiary[ptr2mon->montype]->health;
			ptr2mon->speed = 1;
			ptr2mon->entrybeat = (float)lanearray[index];
			ptr2mon->remainder = 0;
			ptr2mon->monster_rect.w = 32 * ZOOM_MULT;
			ptr2mon->monster_rect.h = 32 * ZOOM_MULT;
			ptr2mon->monster_rect.x = width;
			ptr2mon->monster_rect.y = laneheight[lane] - ptr2mon->monster_rect.h/2;
			ptr2mon->monster_src.x = bestiary[ptr2mon->montype]->Src[0];
			ptr2mon->monster_src.y = bestiary[ptr2mon->montype]->Src[1];
			ptr2mon->monster_src.w = bestiary[ptr2mon->montype]->wh[0];
			ptr2mon->monster_src.h = bestiary[ptr2mon->montype]->wh[1];
			if (index < count - 1) {
				ptr2mon->next = malloc( sizeof(struct node) );
				if (ptr2mon->next == NULL) {
					return 1;
				}
				ptr2mon = ptr2mon->next;
			}
		}
		ptr2mon->next = NULL;

		}
	}
	ptr2mon = linkptrs_start[2];


	/*		Maps & Mapstrips	*/

	/*	Tilemaps	*/

	/* Generate a sample background that is 1 screen large */
	for (int i = 0; i < local_grid_x; i++){
		for (int j = 0; j < local_grid_y; j++){
			sampletilemap2[i][j][0] = 0;
			sampletilemap2[i][j][1] = 1;

			if ((j == local_grid_y - 2 || j == 0) && i%3 == 0){
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

	int (*screenstrip[maxscreens])[local_grid_x][local_grid_y][2];


	/*	Items	*/

	int sampleitemmap[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2];
	/*          lane number ^         ^ order	*/

	//int sampleitemmap[local_grid_x][local_grid_y][2];

	for (int i = 0; i < totallanes; i++) {
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

	int (*itemscreenstrip[maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2];


	/*	Monsters*/

	int samplemonstermap[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3];
	/*          lane number ^         ^ order	*/

	for (int i = 0; i < totallanes; i++) {
		for (int j = 0; j < MAX_MONS_PER_LANE_PER_SCREEN; j++) {
			samplemonstermap[i][j][0] = -1;		//specifies the type of monster
			samplemonstermap[i][j][1] = 0;		//specifies monster position
			samplemonstermap[i][j][2] = 0;		//specifies monster HP
		}
	}

	int samplemonstermap2[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3];

	for (int i = 0; i < totallanes; i++) {
		for (int j = 0; j < MAX_MONS_PER_LANE_PER_SCREEN; j++) {
			samplemonstermap2[i][j][0] = -1;	//specifies the type on monster
			samplemonstermap2[i][j][1] = 0;		//specifies monster position
		}
	}

	int upperj = NATIVE_RES_X / PXPB;
	//for (int i = 1; i < totallanes; i++) {
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

	int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3];


	/*	Pointing Strip Entries to Maps	*/

	for (int i = 0; i < maxscreens; i++) {

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


	refreshtiles(screenstrip, monsterscreenstrip, itemscreenstrip, currentscreen, local_grid_x, local_grid_y, rcTile, rcTilemid, rcTSrc, rcTSrcmid, 0, laneheight, monsterpokedex, itempokedex);

	/*	Event Handling	*/

	int buttonlist [4] = {0, 0, 0, 0};
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
	newtrack = 2;
	rc = pthread_create(&threads, NULL, musicstart, (void*)&newtrack);
	if (rc) {
		printf("ya dun goofed. return code is %d\n.", rc);
		exit(-1);
	}


	pthread_mutex_lock( &track_mutex );
	track = newtrack;
	pthread_mutex_unlock( &track_mutex );

	pthread_mutex_lock( &clock_mutex );
	pauselevel = 0;
	pausetime = 0;
	zerotime = SDL_GetTicks();
	startbeat = -8;
	countbeats = 1;
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

	SDL_Texture *texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);




	/* Frame Loop */

	while (1) {

		if (currentbeat >= 81) {
			speedmult = 3;
			speedmultmon = 3;
		}


		if (levelover) {
			quitlevel(img, Timg, Laserimg, Swordimg, renderer);
			return 0;
		}

		//pthread_mutex_unlock( &soundstatus_mutex);
		pthread_cond_wait( &soundstatus_cond, &soundstatus_mutex );
		//pthread_mutex_lock( &soundstatus_mutex);
		for (int i = 0; i < MAX_SOUNDS_LIST; i++ ) {
			if ( soundchecklist[i] == 1 )
				soundchecklist[i] = 0;
		}

		/* Handle Events */

		SDL_Event e;
		while ( SDL_PollEvent(&e) ) {

			histwrite++;
			if (histwrite >= 4){
				histwrite = 0;}

			if (e.type == SDL_QUIT) {
				levelover = 1;
				quitlevel(img, Timg, Laserimg, Swordimg, renderer);
				return 102; /* Quit to desktop" code */
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_1) {
				levelover = 1;
				quitlevel(img, Timg, Laserimg, Swordimg, renderer);
				return 0; /* Quit up one level */
			}

			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				pauselevel = 1;
				int rc = pause(renderer, texTarget);
				pauselevel = 0;
				SDL_SetRenderTarget(renderer, NULL);

				if (rc >= 100 && rc < 200) {
					quitlevel(img, Timg, Laserimg, Swordimg, renderer);
					return rc;
				}
				if (rc >= 200) {
					quitlevel(img, Timg, Laserimg, Swordimg, renderer);
					return rc;
				}
			}


			if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_RIGHT){
				buttonlist[1] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT){
				buttonlist[1] = 1;
				history[histwrite] = 1;
				direction = 1;
				}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LEFT){
				buttonlist[3] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT){
				buttonlist[3] = 1;
				history[histwrite] = 3;
				direction = 3;
				}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_DOWN){
				buttonlist[2] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN){
				buttonlist[2] = 1;
				history[histwrite] = 2;
				direction = 2;
				}

			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_UP){
				buttonlist[0] = 0;}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP){
				buttonlist[0] = 1;
				history[histwrite] = 0;
				direction = 0;
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

		if ( power > 0 ) {

			for (int i = 0; i < 3; i++) {
				if (actionbuttonlist[i] == 1) {
					hue = i;
					laser_turnoff = 0;

					if (lasercount < 4) {
						laser_on = 1;
						laser_turnon = 1;
						if  ( lasercount == 0 )
							soundchecklist[1] = 1;
					}
					else {
						soundchecklist[2] = 2;
					}
					donehere = 1;
					break;
				}
			}
			if ( !donehere ) {
				if (laser_on)
					//laser_turnon = 0;
					laser_turnoff = 1;
					soundchecklist[2] = 0;
					if ( lasercount == 4 ){
						soundchecklist[3] = 1;
					}
			}

		}
		else {
		/* In the case the power has run out */
			if (laser_on)
				//laser_turnon = 0;
				laser_turnoff = 1;
				soundchecklist[2] = 0;
		}

		if ( sword ) {

			if ( actionbuttonlist[3] == !sword_down ) {
				sword_swing = 1;
			}
			else
				sword_swing = 0;
		}

		/* Change music if necessary */

		if ( track != newtrack ) {

			if (noise) {
				musicstop();
			}
			else {
				rc = pthread_create(&threads, NULL, musicstart, (void*)&newtrack);
				printf("NEW\n");
				if (rc) {
					printf("ya dun goofed. return code is %d\n.", rc);
					exit(-1);
				}
				track = newtrack;
			}
		}

		/* The real business */
		moveme(&currentlane, totallanes, &direction);

		/* set sprite position */
		rcSprite.x = 0.2 * width;
		rcSprite.y = laneheight[currentlane] - POKESPRITE_SIZEX*ZOOM_MULT*2;
		rcSprite.w = POKESPRITE_SIZEX*ZOOM_MULT*2;
		rcSprite.h = POKESPRITE_SIZEY*ZOOM_MULT*2;


		movemap(local_grid_x, local_grid_y, flydir, &rcSrc, &rcSprite, rcTile, speedmult, rcTilemid, rcTSrc, rcTSrcmid, screenstrip, monsterscreenstrip, itemscreenstrip, &currentscreen, maxscreens, &levelover, &laneheight, &monsterpokedex, &itempokedex);

		movemon(speedmultmon, linkptrs_start, linkptrs_end, monsterlanenum, bps, currentbeat, intervalglobal, pxperbeat, &remainder, rcSprite, rcSword);
		if (laser_on) {
			laserfire(&lasercount, &laser_on, &laser_turnon, &laser_turnoff, rcLaser, rcLaserSrc, rcSprite, laneheight, currentlane, framecount, monsterscreenstrip, currentscreen, hue);
		}

		if ( sword )
			swordfunc(&swordcount, &sword_down, &sword_swing, &rcSword, &rcSwordSrc, rcSprite, laneheight, currentlane, framecount, linkptrs_start);

		invinciblefunc();

		amihurt(currentlane, linkptrs_start, rcSprite, bestiary, &levelover);

		touchitem(currentlane, currentscreen, rcSprite, itempokedex, itemscreenstrip, &levelover);


		/* Clear screen */
//		SDL_RenderClear(renderer);

	//Now render to the texture
	SDL_RenderClear(renderer);
		/* Copy textures to renderer	*/
		for (int i = 0; i < local_grid_x * 3; i++){
	                for (int j = 0; j < local_grid_y; j++){
				SDL_RenderCopy(renderer, Timg, &rcTSrc[i][j], &rcTile[i][j]);
			}
		}
		for (int i = 0; i < local_grid_x * 3; i++){
	                for (int j = 0; j < local_grid_y; j++){
				SDL_RenderCopy(renderer, Timg, &rcTSrcmid[i][j], &rcTilemid[i][j]);
			}
		}

		if ( !(invincibility == 1 && ( invinciblecounter[1]%8 <= 3 ) ) )
			SDL_RenderCopy(renderer, img, &rcSrc,  &rcSprite);
		//for (int lane =  0; lane < totallanes; lane++) {
			//for (int screen = 0; screen < 3; screen++) {
				//for (int i = 0; i < monsterlanenum[screen][lane]; i++ ) {
					//if ( (*moninfoptrs[screen])[lane][i][2] != -1){
						//SDL_RenderCopy(renderer, *(*monsterpokedex[(*monsterscreenstrip[currentscreen + screen])[lane][i][0]]).image, &rcMonsterSrc[screen][lane][i], &rcMonster[screen][lane][i]);
					//}
				//}
			//}
		//}


		for (int lane = 0; lane < totallanes; lane++) {
			struct node *ptr2mon = linkptrs_start[lane];
			for (int i = 0; i < monsterlanenum[lane]; i++ ) {
				if ( ptr2mon->status != -1){
					SDL_RenderCopy(renderer, *(*bestiary[ptr2mon->montype]).image, &(ptr2mon->monster_src), &(ptr2mon->monster_rect));
				}
				ptr2mon = ptr2mon->next;
			}
		}
		for (int lane =  0; lane < totallanes; lane++) {
			for (int screen = 0; screen < 3; screen++) {
				for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
					if ( (*iteminfoptrs[screen])[lane][i] != -1){
						SDL_RenderCopy(renderer, *(*itempokedex[(*itemscreenstrip[currentscreen + screen])[lane][i][0]]).image, &rcItemSrc[screen][lane][i],  &rcItem[screen][lane][i]);

					}
				}
			}
		}
		if (laser_on){
			for (int i = 0; i < 3; i++) {
				SDL_RenderCopy(renderer, Laserimg, &rcLaserSrc[i], &rcLaser[i]);
			}
		}

		if ( sword ) {
				SDL_RenderCopy(renderer, Swordimg, &rcSwordSrc, &rcSword);

		}

		if ( HP <= 50 ) {
			if ( HP <= 20 ) {
				rcHP1Src.y = 16;
			}
			else {
				rcHP1Src.y = 8;
			}
		}
		else {
			rcHP1Src.y = 0;
		}

		rcHP1.w = 0.5 * HP * ZOOM_MULT * 2;
		SDL_RenderCopy(renderer, HPimg0, &rcHP0Src, &rcHP0);
		SDL_RenderCopy(renderer, HPimg1, &rcHP1Src, &rcHP1);

		if ( power <= 250 ) {
			if ( power <= 100 ) {
				rcPower1Src.y = 16;
			}
			else {
				rcPower1Src.y = 8;
			}
		}
		else {
			rcPower1Src.y = 0;
		}

		rcPower1.w = 0.1 * power * ZOOM_MULT * 2;
		SDL_RenderCopy(renderer, Powerimg0, &rcPower0Src, &rcPower0);
		SDL_RenderCopy(renderer, Powerimg1, &rcPower1Src, &rcPower1);

		int scorearray[SCORE_DIGITS];
		int2array(score, &scorearray);
		for ( int i = 0; i < 5; i++ ) {
			rcScoreSrc[i].x = scorearray[i] * 5;
			SDL_RenderCopy(renderer, Scoreimg, &rcScoreSrc[i], &rcScore[i]);
		}

		int beatarray[SCORE_DIGITS];
		int2array(currentbeat, &beatarray);
		for ( int i = 0; i < 5; i++ ) {
			rcBeatSrc[i].x = beatarray[i] * 5;
			SDL_RenderCopy(renderer, Beatimg, &rcBeatSrc[i], &rcBeat[i]);
		}

		//Detach the texture
		SDL_SetRenderTarget(renderer, NULL);

		SDL_RenderClear(renderer);

		if (partymode) {
			angle ++;
			colournum += speedmult*5;
			int r = colournum%255;
			int g = (colournum + 100)%255;
			int b = (colournum + 200)%255;
			SDL_SetTextureColorMod(texTarget, r,g,b);
			SDL_RenderCopyEx(renderer, texTarget, NULL, NULL, angle * speedmult, NULL, SDL_FLIP_NONE);
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

	quitlevel(img, Timg, Laserimg, Swordimg, renderer);



	return 0;
}


/*Functions*/



void moveme(int *currentlane, int totallanes, int *direction) {

if (*direction < 4) {
	if (*direction == 0 || *direction == 3) {
		if (*currentlane > 0) {
			(*currentlane)--;
		}
	}
	if (*direction == 2 || *direction == 1) {
		if (*currentlane < totallanes - 1) {
			(*currentlane)++;
		}
	}
	*direction = 4;
}
}

void movemap(int local_grid_x, int local_grid_y, int flydir, SDL_Rect *rcSrc, SDL_Rect *rcSprite, SDL_Rect rcTile[local_grid_x * 3][local_grid_y], int speedmult, SDL_Rect rcTilemid[local_grid_x * 3][local_grid_y], SDL_Rect rcTSrc[local_grid_x][local_grid_y], SDL_Rect rcTSrcmid[local_grid_x][local_grid_y], int (*screenstrip [maxscreens]) [local_grid_x * 3][local_grid_y][2], int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3], int (*itemscreenstrip[maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int *currentscreen, int maxscreens, int *levelover, int (*laneheight)[totallanes], struct monster *(*bestiary)[10], struct item *(*itempokedex)[10]){
	if (flydir == 0){
		for (int i = 0; i < local_grid_x * 3; i++){
			for(int j = 0; j < local_grid_y; j++){
				rcTile[i][j].y += 4 * ZOOM_MULT * speedmult;
				rcTilemid[i][j].y += 4 * ZOOM_MULT *speedmult;
			}
		}
	}
	if (flydir == 1 ){
		if (rcTile[local_grid_x][0].x <= 0 ) {
			int frameoffset = rcTile[local_grid_x][0].x;
			(*currentscreen)++;
			if (*currentscreen >= maxscreens - 3){
				*levelover = 1;
			}
			//else {
				refreshtiles(screenstrip, monsterscreenstrip, itemscreenstrip, *currentscreen, local_grid_x, local_grid_y, rcTile, rcTilemid, rcTSrc, rcTSrcmid, frameoffset, *laneheight, *bestiary, *itempokedex);
			//}
		}

		if ( !(*levelover) ) {
			for (int i = 0; i < local_grid_x * 3; i++){
				for(int j = 0; j < local_grid_y; j++){
					rcTile[i][j].x -= 4 * ZOOM_MULT * speedmult;
					rcTilemid[i][j].x -= 4 * ZOOM_MULT *speedmult;
					totalnativedist += 4 * speedmult;
				}
			}
			for ( int screen = 0; screen < 3; screen++ ) {
				for (int lane = 0; lane < totallanes; lane++ ) {
					//for (int i = 0; i < monsterlanenum[screen][lane]; i++ ) {
					//		rcMonster[screen][lane][i].x -= 4 * ZOOM_MULT * speedmult;
					//}
					for (int i = 0; i < itemlanenum[screen][lane]; i++ ) {
							rcItem[screen][lane][i].x -= 4 * ZOOM_MULT * speedmult;
					}
				}
			}
		}
	}

	if (flydir == 2){
		for (int i = 0; i < local_grid_x * 3; i++){
			for(int j = 0; j < local_grid_y; j++){
				rcTile[i][j].y -= 4 * ZOOM_MULT * speedmult;
				rcTilemid[i][j].y -= 4 * ZOOM_MULT *speedmult;
			}
		}
	}

	if (flydir == 3){
		for (int i = 0; i < local_grid_x * 3; i++){
			for(int j = 0; j < local_grid_y; j++){
				rcTile[i][j].x += 4 * ZOOM_MULT * speedmult;
				rcTilemid[i][j].x += 4 * ZOOM_MULT *speedmult;
			}
		}
	}


}


void movemon(float speedmultmon, struct node *linkptrs_start[TOTAL_LANES], struct node *linkptrs_end[TOTAL_LANES], int monsterlanenum[TOTAL_LANES], float bps, float currentbeat, float intervalglobal, float pxperbeat, float (*remainder)[totallanes], SDL_Rect rcSprite, SDL_Rect rcSword) {
	struct node *ptr2mon;
	float transspeed;

	for (int lane = 0; lane < TOTAL_LANES; lane++) {
		if (linkptrs_start[lane] != NULL) {
			ptr2mon = linkptrs_start[lane];
			for (int i = 0; i < monsterlanenum[lane]; i++) {
				transspeed = bps * linkptrs_start[lane]->speed * ZOOM_MULT * speedmultmon * pxperbeat + ptr2mon->remainder;
				float translation = transspeed * intervalglobal/1000.0;
				int transint = (int)translation;
				ptr2mon->remainder = translation - transint;
				ptr2mon->monster_rect.x -= transint;
				ptr2mon = ptr2mon->next;
			}
			while (linkptrs_start[lane]->monster_rect.x  + linkptrs_start[lane]->monster_rect.w < 0) {
				struct node *tmp = linkptrs_start[lane];
				linkptrs_start[lane] = linkptrs_start[lane]->next;
				monsterlanenum[lane]--;
				free(tmp);
				if (linkptrs_start[lane] == NULL)
					break;
			}
			float Dt;
			while (linkptrs_start[lane] != NULL && linkptrs_end[lane] != NULL) {
				transspeed = bps * linkptrs_start[lane]->speed * ZOOM_MULT * speedmultmon * pxperbeat + ptr2mon->remainder;
				pthread_mutex_lock( &clock_mutex );
				float extra = (((float)(width - (rcSprite.x + rcSprite.w + rcSword.w/2)))/transspeed - 4*intervalglobal/1000.0) * bps;
				Dt = currentbeat - linkptrs_end[lane]->entrybeat + extra;
				pthread_mutex_unlock( &clock_mutex );
				if (Dt >= 0) {
					float Dx = Dt / bps * speedmultmon * linkptrs_end[lane]->speed;
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


void refreshtiles(int (*screenstrip[maxscreens]) [local_grid_x][local_grid_y][2], int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3], int (*itemscreenstrip[maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int currentscreen, int local_grid_x, int local_grid_y, SDL_Rect rcTile[local_grid_x][local_grid_y], SDL_Rect rcTilemid[local_grid_x][local_grid_y], SDL_Rect rcTSrc[local_grid_x][local_grid_y], SDL_Rect rcTSrcmid[local_grid_x][local_grid_y], int frameoffset, int laneheight[totallanes], struct monster *bestiary[10], struct item *itempokedex[10]) {

	for (int k = 0; k <= 2; k++) {
	for (int i = 0; i < local_grid_x; i++){
		for (int j = 0; j < local_grid_y; j++){

			rcTile[i + local_grid_x * k][j].x = (i + local_grid_x * k) * ZOOM_MULT * TILE_SIZE + frameoffset;
			rcTile[i + local_grid_x * k][j].y = (j) * ZOOM_MULT * TILE_SIZE;
			rcTile[i + local_grid_x * k][j].w = TILE_SIZE*ZOOM_MULT * 2;
			rcTile[i + local_grid_x * k][j].h = TILE_SIZE*ZOOM_MULT * 2;
			rcTSrc[i + local_grid_x * k][j].x = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][1]/2;
			rcTSrc[i + local_grid_x * k][j].y = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][0]/2;
			rcTSrc[i + local_grid_x * k][j].w = TILE_SIZE;
			rcTSrc[i + local_grid_x * k][j].h = TILE_SIZE;
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
				rcItem[screen][lane][itemcounterperscreen].x = ( (*itemscreenstrip[currentscreen + screen])[lane][j][1] + local_grid_x * screen * TILE_SIZE) * ZOOM_MULT + frameoffset;
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
//	for (int i = 0; i < local_grid_x; i++){
//		for (int j = 0; j < local_grid_y; j++){
//	
//			rcTilemid[i + local_grid_x * k][j].x = (i + local_grid_x * k) * ZOOM_MULT * TILE_SIZE + frameoffset;
//			rcTilemid[i + local_grid_x * k][j].y = (j) * ZOOM_MULT * TILE_SIZE;
//			rcTilemid[i + local_grid_x * k][j].w = TILE_SIZE*ZOOM_MULT;
//			rcTilemid[i + local_grid_x * k][j].h = TILE_SIZE*ZOOM_MULT;
//			rcTSrcmid[i + local_grid_x * k][j].x = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][1];
//			rcTSrcmid[i + local_grid_x * k][j].y = TILE_SIZE*(*screenstrip[currentscreen + k])[i][j][0];
//			rcTSrcmid[i + local_grid_x * k][j].w = TILE_SIZE;
//			rcTSrcmid[i + local_grid_x * k][j].h = TILE_SIZE;
//		}
//	}
//	}

}



void laserfire(int *lasercountptr, int *laser_on, int *laser_turnon, int *laser_turnoff, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, int laneheight[totallanes], int currentlane, int framecount, int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3], int currentscreen, int hue) {
	int laserpower = 10;
	if (*laser_turnon) {
		(*lasercountptr)++;

		if (*lasercountptr >= 4) {
			*laser_turnon = 0;
		}
	}

	else if (*laser_turnoff) {
//		if ( *laser_turnon )
			(*lasercountptr)--;

		if (*lasercountptr <= 0) {
			*laser_on = 0;
			*laser_turnoff = 0;
			return;
		}
	}



	int lasersrcpos;
	lasersrcpos = (*lasercountptr);
	if (*lasercountptr >= 4 && framecount%4 == 0)
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

	int laserlength = NATIVE_RES_X * ZOOM_MULT - rcSprite.x - rcSprite.w + rcLaser[2].w;

	int done = 0;
	struct node *ptr2mon = linkptrs_start[currentlane];
	for ( int i = 0; i < monsterlanenum[currentlane]; i++ ) {
		if ( ptr2mon->monster_rect.x > ( rcSprite.x + rcSprite.w ) && (ptr2mon->status != -1)){
			if ( ptr2mon->monster_rect.x <= NATIVE_RES_X * ZOOM_MULT ) {
				laserlength = - rcSprite.x - rcSprite.w + ptr2mon->monster_rect.x;
				damage(currentlane, ptr2mon, laserpower);
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

	power--;
}

void swordfunc(int *swordcountptr, int *sword_down, int *sword_swing, SDL_Rect *rcSword, SDL_Rect *rcSwordSrc, SDL_Rect rcSprite, int laneheight[totallanes], int currentlane, int framecount, struct node *linkptrs_start[TOTAL_LANES]) {

	int swordpower = 30000;

	rcSword->x = rcSprite.x + rcSprite.w - 2 * ZOOM_MULT;
	rcSword->y = laneheight[currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	rcSword->w = rcSwordSrc->w * ZOOM_MULT * 2;
	rcSword->h = rcSwordSrc->h * ZOOM_MULT * 2;

	if ( *sword_swing ) {

		if ( *sword_down ) {
			(*swordcountptr)--;
			if ( *swordcountptr <= 0 ) {
				*sword_down = 0;
			}
		}

		else if ( !(*sword_down) ) {

			if ( *swordcountptr == 0 ) {
				soundchecklist[8] = 1;
			}

			(*swordcountptr)++;

			if ( *swordcountptr >= 2 ) {
				*sword_down = 1;

				//int done = 0;
				struct node *ptr2mon = linkptrs_start[currentlane];
				for ( int i = 0; i < monsterlanenum[currentlane]; i++ ) {
					if ( ptr2mon->monster_rect.x > ( rcSprite.x + rcSprite.w ) && (ptr2mon->status != -1)){
						if ( ptr2mon->monster_rect.x <= rcSword->x + rcSword->w ) {
							damage(currentlane, ptr2mon, swordpower);
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



	rcSwordSrc->y = SWORD_HEIGHT * *swordcountptr;

}

void quitlevel(SDL_Texture *img, SDL_Texture *Timg, SDL_Texture *Laserimg, SDL_Texture *Swordimg, SDL_Renderer *renderer) {
	printf("Level Over.\n");

//	for ( int i = 0; i < MAX_SOUNDS_LIST; i++ )
//		soundchecklist[i] = 0;
	SDL_RenderClear(renderer);
	musicstop();
	soundstop();
	pthread_cond_wait( &cond_end, &track_mutex);
	pthread_mutex_unlock( &track_mutex );
	SDL_DestroyTexture(img);
	SDL_DestroyTexture(Timg);
	SDL_DestroyTexture(Laserimg);
	SDL_DestroyTexture(Swordimg);
//	SDL_DestroyTexture(texTarget);
	countbeats = 0;
	currentbeat = 0;
}


void damage(int currentlane, struct node *ptr2mon, int power) {

	ptr2mon->health -= power;
	if (ptr2mon->health <= 0){
		ptr2mon->status = -1;
		soundchecklist[7] = 1;
		score += 10;
//		(*monsterscreenstrip[currentscreen])[currentlane][((*moninfoptrs[screen])[currentlane][order][0])][0] = -1;
	}
}

void amihurt(int currentlane, struct node *linkptrs_start[totallanes], SDL_Rect rcSprite, struct monster *bestiary[10], int *levelover) {

	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	//int done = 0;

	struct node *ptr2mon = linkptrs_start[currentlane];

	for ( int i = 0; i < monsterlanenum[currentlane]; i++ ) {

		if ( ( ptr2mon->monster_rect.x > rcSprite.x ) && ( ptr2mon->monster_rect.x < ( rcSprite.x + rcSprite.w ) ) ) {

			/* Is the monster dead? */
			if ( ptr2mon->status != -1) {

	//			int order = (*moninfoptrs[screen])[currentlane][i][0];
	//			int order = i;
	//			int monstertype = samplemonstermap[currentlane][order][0];
				int monstertype = ptr2mon->montype;
				gethurt((*bestiary[monstertype]).attack, levelover);
			}
			//done = 1;
			break;
		}
		ptr2mon = ptr2mon->next;
	}
	//if ( done )
	//	break;

}


void touchitem(int currentlane, int currentscreen, SDL_Rect rcSprite, struct item *itempokedex[10], int (*itemscreenstrip[maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int *levelover) {
	/* Check screen 0, current lane for monster sprites encroaching on the player sprite */

	int done = 0;
	for ( int screen = 0; screen < 3; screen++ ) {

		for ( int i = 0; i < itemlanenum[screen][currentlane]; i++ ) {

			if ( ( rcItem[screen][currentlane][i].x > rcSprite.x ) && ( rcItem[screen][currentlane][i].x < ( rcSprite.x + rcSprite.w ) ) ) {

				/* Is the item "dead"? */
				if ( (*iteminfoptrs[screen])[currentlane][i] != -1) {

	//				int order = (*iteminfoptrs[screen])[currentlane][i][0];
					int order = i;
	//				int itemtype = sampleitemmap[currentlane][order][0];
					int itemtype = (*itemscreenstrip[currentscreen + screen])[currentlane][i][0];
					struct item thisitem = *itempokedex[itemtype];
					void (*functionptr)(int *int1, int int2) = thisitem.functionptr;
					(*functionptr)(thisitem.int1, thisitem.int2);
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


int invinciblefunc() {

	if ( invincibility != 0 ) {
		invinciblecounter[invincibility]--;
		if ( invinciblecounter[invincibility] <= 0 ) {
			invincibility = 0;
		}
	}

	return invincibility;
}

void gethurt(int attack, int *levelover) {

	if (invincibility == 0 ) {
		int damage = attack;
		HP -= damage;
		if ( HP <= 0 ) {
			soundchecklist[5] = 1;
			*levelover = 1;
		}
		else {
			soundchecklist[4] = 1;
			invincibility = 1;
			invinciblecounter[1] = 40;
		}
	}
}

void restorehealth(int *HPptr, int restoreHPpts) {
	if ( max_HP - *HPptr <= restoreHPpts ) {
		*HPptr = max_HP;
	}
	else
		*HPptr += restoreHPpts;

}

void restorepower(int *powerptr, int restorepowerpts) {

	if ( max_PP - *powerptr <= restorepowerpts ) {
		*powerptr = max_PP;
	}
	else
		*powerptr += restorepowerpts;

}


void healthup(int *max_HPptr, int HPuppts) {

		*max_HPptr += HPuppts;
}

void PPup(int *max_PPptr, int PPuppts) {

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
//void deleteList(struct node** head_ref) {
//   /* deref head_ref to get the real head */
//   struct node* current = *head_ref;
//   struct node* next;
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

