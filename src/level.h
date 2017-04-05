//#include "main.h"

#define PXPB 83.24
//#define SPRITE_PATH "../art/rubysprites2.png"
//#define TILE_PATH "../art/tileset2.png"
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
//#define WIDTH TILE_SIZE * (LOCAL_GRID_X - 2) * ZOOM_MULT
//#define HEIGHT TILE_SIZE * (LOCAL_GRID_Y - 2) * ZOOM_MULT
#define TOTAL_LANES 5
#define MAX_MONS_PER_LANE_PER_SCREEN 20
#define MAX_ITEMS_PER_LANE_PER_SCREEN 20
#define SCORE_DIGITS 5

/*	Global Variables	*/

/* Status of the Level */

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
extern int soundchecklist[MAX_SOUNDS_LIST];

/* Graphics */

int local_grid_x;
int local_grid_y;
int totallanes = TOTAL_LANES;
int maxscreens;
extern int framecount;
extern int width, height;
int totalnativedist = 0;

/* Timing */

extern int countbeats;
extern float bps;
extern float startbeat;
extern float currentbeat;
extern float intervalglobal;
extern float pxperbeat;
extern Uint32 zerotime;
extern Uint32 pausetime;

/* Objects */

/* Monsters */

int monsterlanenum[TOTAL_LANES];
SDL_Rect rcMonster[3][TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];
SDL_Rect rcMonsterSrc[3][TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];

typedef struct node {
	char montype;
	char status;
	int health;
	float speed;
	float entrybeat;
	float remainder;
	struct SDL_Rect monster_rect;
	struct SDL_Rect monster_src;
	struct node * next;
} node_t;


struct monster {
	int health;
	int attack;
	float defence;
	int Src[2];
	int wh[2];
	SDL_Texture **image;
	};

struct node *linkptrs_start[TOTAL_LANES];
struct node *linkptrs_end[TOTAL_LANES];

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
	void *functionptr;
	int Src[2];
	int wh[2];
	SDL_Texture **image;
	//void (*functionptr)(int int1, int int2);
	};

/* Threading */

extern pthread_mutex_t display_mutex;
extern pthread_mutex_t clock_mutex;
extern pthread_cond_t display_cond;
pthread_mutex_t track_mutex = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t cond_end;

/*	Function Prototypes	*/

/* Movement */

void moveme(int *currentlane, int totallanes, int *direction);

void movemap(int local_grid_x, int local_grid_y, int walkdir, SDL_Rect *rcSrc, SDL_Rect *rcSprite, SDL_Rect rcTile[local_grid_x][local_grid_y], int speedmult, SDL_Rect rcTilemid[local_grid_x][local_grid_y], SDL_Rect rcTSrc[local_grid_x][local_grid_y], SDL_Rect rcTSrcmid[local_grid_x][local_grid_y], int (*screenstrip[maxscreens]) [local_grid_x][local_grid_y][2], int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3], int (*itemscreenstrip[maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int *currentscreen, int maxscreens, int *levelover, int (*laneheight)[totallanes], struct monster *(*monsterpokedex)[10], struct item *(*itempokedex)[10]);

void movemon(float speedmultmon, struct node *linkptrs_start[TOTAL_LANES], struct node *linkptrs_end[TOTAL_LANES], int monsterlanenum[TOTAL_LANES], float bps, float currentbeat, float intervalglobal, float pxperbeat, float (*remainder)[totallanes], SDL_Rect rcSprite, SDL_Rect rcSword);

void mode2(int spritepos [2], int *walkdir, SDL_Rect rcTSrc[local_grid_x][local_grid_y], SDL_Rect rcTSrcmid[local_grid_x][local_grid_y], int sampletilemap[100][100][2], int sampletilemapmid[100][100][2], SDL_Rect rcTile[local_grid_x][local_grid_y], SDL_Rect rcTilemid[local_grid_x][local_grid_y]);

void refreshtiles(int (*screenstrip[maxscreens]) [local_grid_x][local_grid_y][2], int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3],  int (*itemscreenstrip[maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], int currentscreen, int local_grid_x, int local_grid_y, SDL_Rect rcTile[local_grid_x][local_grid_y], SDL_Rect rcTilemid[local_grid_x][local_grid_y], SDL_Rect rcTSrc[local_grid_x][local_grid_y], SDL_Rect rcTSrcmid[local_grid_x][local_grid_y], int frameoffset, int laneheight[totallanes], struct monster *monsterpokedex[10], struct item *itempokedex[10]);

/* Offence */

void laserfire(int *lasercountptr, int *laser_on, int *laser_turnon, int *laser_turnoff, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, int laneheight[totallanes], int currentlane, int framecount, int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3], int currentscreen, int hue);

//void swordfunc(int *lasercountptr, int *laser_on, int *laser_turnon, int *laser_turnoff, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, int laneheight[totallanes], int currentlane, int framecount, int (*monsterscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][3], int currentscreen, int hue);

void swordfunc(int *swordcountptr, int *sword_down, int *sword_swing, SDL_Rect *rcSword, SDL_Rect *rcSwordSrc, SDL_Rect rcSprite, int laneheight[totallanes], int currentlane, int framecount, struct node *linkptrs_start[TOTAL_LANES]);

void damage(int currentlane, struct node *ptr2mon, int laserpower);

/* Player Status */

void amihurt(int currentlane, struct node *linkptrs_start[totallanes], SDL_Rect rcSprite, struct monster *bestiary[10], int *levelover);

void touchitem(int currentlane, int currentscreen, SDL_Rect rcSprite, struct item *itempokedex[10], int (*itemscreenstrip[maxscreens])[totallanes][MAX_MONS_PER_LANE_PER_SCREEN][2], int *levelover);

int invinciblefunc();

void gethurt(int attack, int *levelover);

void restorehealth(int *HPptr, int restoreHPpts);

void restorepower(int *powerptr, int restorepowerpts);

void healthup(int *max_HPptr, int HPuppts);

void PPup(int *max_PPptr, int PPuppts);

/* Tools */

void multidrop();

void dropin( char arg[], int map[100][100][2], int posx, int posy);

void quitlevel(SDL_Texture *img, SDL_Texture *Timg, SDL_Texture *Laserimg, SDL_Texture *Swordimg, SDL_Renderer *renderer);

/* Helper Functions */

void int2array(int number, int (*array)[SCORE_DIGITS]);

void deleteList(struct node** head_ref);


