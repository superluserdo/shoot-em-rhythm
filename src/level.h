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

extern struct level_struct level_status;
extern struct lane_struct lanes;
extern struct grid_struct grid;
extern struct laser_struct laser;

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
	void *otherdata;
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
extern pthread_cond_t cond_end;
extern pthread_mutex_t track_mutex; 

/*	Function Prototypes	*/

/* Movement */

void moveme(int *currentlane, int totallanes, int *direction);

void movemap(struct level_struct *level_status_ptr, struct player_struct *player_status, struct grid_struct grid, SDL_Rect *rcSrc, 
SDL_Rect *rcSprite, SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y], 
SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], 
int (*screenstrip[level_status.maxscreens]) [grid.x][grid.y][2], 
int (*monsterscreenstrip[level_status.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int (*itemscreenstrip[level_status.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], 
struct monster *(*monsterpokedex)[10], struct item *(*itempokedex)[10]);

void movemon(float speedmultmon, struct node *linkptrs_start[TOTAL_LANES], 
struct node *linkptrs_end[TOTAL_LANES], int monsterlanenum[TOTAL_LANES], float bps, 
float currentbeat, float intervalglobal, float pxperbeat, float (*remainder)[lanes.total],
SDL_Rect rcSprite, SDL_Rect rcSword);

void mode2(int spritepos [2], int *walkdir, SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], int sampletilemap[100][100][2], 
int sampletilemapmid[100][100][2], SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y]);

void refreshtiles(int (*screenstrip[level_status.maxscreens])[grid.x][grid.y][2], 
int (*monsterscreenstrip[level_status.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3],  
int (*itemscreenstrip[level_status.maxscreens])[lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], 
int currentscreen, struct grid_struct grid, SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y], 
SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], int frameoffset, 
int laneheight[lanes.total], struct monster *monsterpokedex[10], 
struct item *itempokedex[10]);

/* Offence */

void laserfire(struct laser_struct *laser_ptr, struct player_struct *player_status, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, 
int laneheight[lanes.total], int currentlane, int framecount, 
int (*monsterscreenstrip[level_status.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int currentscreen, int hue);

/*void swordfunc(int *lasercountptr, int *laser_on, int *laser_turnon, int *laser_turnoff, 
SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, 
int laneheight[lanes.total], int currentlane, int framecount, 
int (*monsterscreenstrip[level_status.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int currentscreen, int hue);
*/

void swordfunc(int *swordcountptr, int *sword_down, int *sword_swing, SDL_Rect *rcSword, 
SDL_Rect *rcSwordSrc, SDL_Rect rcSprite, int laneheight[lanes.total], int currentlane, 
int framecount, struct node *linkptrs_start[TOTAL_LANES]);

void damage(int currentlane, struct node *ptr2mon, int power);

/* Player Status */

void amihurt(struct status_struct status, struct node *linkptrs_start[lanes.total], SDL_Rect rcSprite, struct monster *bestiary[10]);

void touchitem(int currentlane, int currentscreen, SDL_Rect rcSprite, 
struct item *itempokedex[10], 
int (*itemscreenstrip[level_status.maxscreens])[lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][2], 
int *levelover);

int invinciblefunc(struct player_struct *player_status);

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
