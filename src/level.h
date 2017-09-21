#define PXPB 83.24
#define SPRITE_PATH "../art/selfsprite.png"
#define TILE_PATH "../art/selftile.png"
#define LASER_PATH "../art/lasers.png"
#define SWORD_PATH "../art/swords.png"
#define SWORD_WIDTH 32
#define SWORD_HEIGHT 32
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

extern struct program_struct program;
extern struct time_struct timing;
extern struct audio_struct audio;

extern struct level_struct level;
extern struct lane_struct lanes;
extern struct xy_struct grid;
extern struct laser_struct laser;

/* Objects */

/* Monsters */

int monsterlanenum[TOTAL_LANES];
SDL_Rect rcMonster[3][TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];
SDL_Rect rcMonsterSrc[3][TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];

struct monster_node {
	char montype;
	char status;
	int health;
	float speed;
	float entrybeat;
	float remainder;
	struct SDL_Rect monster_rect;
	struct SDL_Rect monster_src;
	struct animate_specific *animation;
	struct monster_node * next;
};


struct monster {
	int health;
	int attack;
	float defence;
	int Src[2];
	int wh[2];
	int generic_bank_index;
	SDL_Texture **image;
	};

struct monster_node *linkptrs_start[TOTAL_LANES];
struct monster_node *linkptrs_end[TOTAL_LANES];

//int (*moninfoptrs[3])[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN][3];
//(Deprecated)

/* Items */

int itemlanenum[3][TOTAL_LANES];
SDL_Rect rcItem[3][TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN];
SDL_Rect rcItemSrc[3][TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN];

int (*iteminfoptrs[3])[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];

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
			struct xy_struct pos;
			struct size_ratio_struct size_ratio;
			struct animate_specific *animation;
			struct ui_bar *self;
		};
		struct std std;
	};
};
struct ui_counter {
	int *value;
	int digits;
	int *array;
	union {
		struct {
			struct xy_struct pos;
			struct size_ratio_struct size_ratio;
			struct animate_specific *animation;
			struct ui_counter *self;
		};
		struct std std;
	};
};


/* Threading */

extern pthread_mutex_t display_mutex;
extern pthread_mutex_t clock_mutex;
extern pthread_cond_t display_cond;
extern pthread_cond_t cond_end;
extern pthread_mutex_t track_mutex; 

/*	Function Prototypes	*/

/* Movement */

void moveme(int *currentlane, int totallanes, int *direction, struct animate_specific *anim);

void movemap(struct level_struct *level_ptr, struct player_struct *player, struct xy_struct grid,
SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y], 
SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], 
int (*screenstrip[level.maxscreens]) [grid.x][grid.y][2], 
int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], 
struct monster *(*monsterpokedex)[10], struct item *(*itempokedex)[10]);

void movemon(float speedmultmon, struct time_struct timer, struct monster_node *linkptrs_start[TOTAL_LANES], struct monster_node *linkptrs_end[TOTAL_LANES], int monsterlanenum[TOTAL_LANES], float (*remainder)[lanes.total], SDL_Rect player_out, SDL_Rect rcSword);

void mode2(int spritepos [2], int *walkdir, SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], int sampletilemap[100][100][2], 
int sampletilemapmid[100][100][2], SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y]);

void refreshtiles(int (*screenstrip[level.maxscreens])[grid.x][grid.y][2], 
int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3],  
int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], 
int currentscreen, struct xy_struct grid, SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y], 
SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], int frameoffset, 
int laneheight[lanes.total], struct monster *monsterpokedex[10], 
struct item *itempokedex[10]);

/* Offence */

void laserfire(struct laser_struct *laser_ptr, struct player_struct *player, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect player_out, 
int laneheight[lanes.total], int currentlane, int framecount, 
int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int currentscreen, int hue);

/*void swordfunc(int *lasercountptr, int *laser_on, int *laser_turnon, int *laser_turnoff, 
SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, 
int laneheight[lanes.total], int currentlane, int framecount, 
int (*monsterscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int currentscreen, int hue);
*/

void swordfunc(struct sword_struct *sword, SDL_Rect *rcSword, SDL_Rect *rcSwordSrc, SDL_Rect player_out, int laneheight[lanes.total], int currentlane, int framecount, struct monster_node *linkptrs_start[TOTAL_LANES]);

void damage(int currentlane, struct monster_node *ptr2mon, int power);

/* Player Status */

void amihurt(struct status_struct status, struct monster_node *linkptrs_start[lanes.total], SDL_Rect player_out, struct monster *bestiary[10]);

void touchitem(int currentlane, int currentscreen, SDL_Rect player_out, 
struct item *itempokedex[10], 
int (*itemscreenstrip[level.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][2], 
int *levelover);

int invinciblefunc(struct player_struct *player);

void gethurt(struct status_struct status, int attack);

void restorehealth(int *HPptr, int restoreHPpts, void *otherdata);

void restorepower(int *powerptr, int restorepowerpts, void *otherdata);

void healthup(int *max_HPptr, int HPuppts, void *nullptr);

void PPup(int *max_PPptr, int PPuppts, void *nullptr);

/* Tools */

void multidrop();

void dropin( char arg[], int map[100][100][2], int posx, int posy);

void quitlevel(SDL_Texture *img, SDL_Texture *Timg, SDL_Texture *Laserimg, 
SDL_Texture *Swordimg, SDL_Renderer *renderer);

void emptyfunc();
