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

int monsterlanenum[TOTAL_LANES];
SDL_Rect rcMonster[3][TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];
SDL_Rect rcMonsterSrc[3][TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];

struct monster_node *linkptrs_start[TOTAL_LANES];
struct monster_node *linkptrs_end[TOTAL_LANES];

//int (*moninfoptrs[3])[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN][3];
//(Deprecated)

/* Items */

int itemlanenum[3][TOTAL_LANES];
SDL_Rect rcItem[3][TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN];
SDL_Rect rcItemSrc[3][TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN];

int (*iteminfoptrs[3])[TOTAL_LANES][MAX_MONS_PER_LANE_PER_SCREEN];

/* Threading */

extern pthread_mutex_t display_mutex;
extern pthread_mutex_t clock_mutex;
extern pthread_cond_t display_cond;
extern pthread_cond_t cond_end;
extern pthread_mutex_t track_mutex; 

/*	Function Prototypes	*/

/* Movement */

void moveme(struct lane_struct *lanes, int *direction, struct animate_specific *anim);

void movemap(struct level_struct *level, struct player_struct *player, struct xy_struct grid,
SDL_Rect rcTile[grid.x][grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y], 
SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], 
int totallanes,
int (*screenstrip[level->maxscreens]) [grid.x][grid.y][2], 
int (*itemscreenstrip[level->maxscreens])[level->lanes.total][MAX_ITEMS_PER_LANE_PER_SCREEN][2], 
struct monster *(*monsterpokedex)[10], struct item *(*itempokedex)[10]);

void movemon(int totallanes, float speedmultmon, struct time_struct timing, struct monster_node *linkptrs_start[TOTAL_LANES], struct monster_node *linkptrs_end[TOTAL_LANES], int monsterlanenum[TOTAL_LANES], double remainder[level->lanes.total], SDL_Rect player_out, SDL_Rect rcSword, int width);

void mode2(int spritepos [2], int *walkdir, SDL_Rect rcTSrc[level->grid.x][level->grid.y], 
SDL_Rect rcTSrcmid[level->grid.x][level->grid.y], int sampletilemap[100][100][2], 
int sampletilemapmid[100][100][2], SDL_Rect rcTile[level->grid.x][level->grid.y], 
SDL_Rect rcTilemid[level->grid.x][level->grid.y]);

void refreshtiles(int totallanes, int (*screenstrip[])[level->grid.x][level->grid.y][2], 
int (*itemscreenstrip[level->maxscreens])[totallanes][MAX_ITEMS_PER_LANE_PER_SCREEN][2], 
int currentscreen, struct xy_struct grid, SDL_Rect rcTile[level->grid.x][level->grid.y], 
SDL_Rect rcTilemid[grid.x][grid.y], 
SDL_Rect rcTSrc[grid.x][grid.y], 
SDL_Rect rcTSrcmid[grid.x][grid.y], int frameoffset, 
float laneheight[totallanes], struct monster *bestiary[10], 
struct item *itempokedex[10]);

/* Offence */

void laserfire(struct level_struct *level, int totallanes, struct laser_struct *laser_ptr, struct player_struct *player, SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect player_out, 
float laneheight[totallanes], int currentlane, int framecount, 
int currentscreen, int hue, int *soundchecklist);

/*void swordfunc(int *lasercountptr, int *laser_on, int *laser_turnon, int *laser_turnoff, 
SDL_Rect rcLaser[3], SDL_Rect rcLaserSrc[3], SDL_Rect rcSprite, 
int laneheight[level->lanes.total], int currentlane, int framecount, 
int (*monsterscreenstrip[level.maxscreens])[level->lanes.total][MAX_MONS_PER_LANE_PER_SCREEN][3], 
int currentscreen, int hue);
*/

//void swordfunc(int totallanes, struct sword_struct *sword, SDL_Rect *rcSword, SDL_Rect *rcSwordSrc, SDL_Rect player_out, int laneheight[totallanes], int currentlane, int framecount, struct monster_node *linkptrs_start[TOTAL_LANES], struct audio_struct audio);
void swordfunc(struct level_struct *level, struct sword_struct *sword, SDL_Rect player_out, int framecount, struct monster_node *linkptrs_start[TOTAL_LANES], struct audio_struct audio);

void damage(int currentlane, struct monster_node *ptr2mon, int power, int *soundchecklist, struct level_struct *level);

/* Player Status */

void amihurt(int totallanes, struct status_struct status, struct monster_node *linkptrs_start[level->lanes.total], SDL_Rect player_out, struct monster *bestiary[10]);

void touchitem(struct lane_struct *lanes, int currentlane, int currentscreen, SDL_Rect player_out, 
struct item *itempokedex[10], 
int (*itemscreenstrip[level->maxscreens])[lanes->total][MAX_MONS_PER_LANE_PER_SCREEN][2], 
int *levelover, int *soundchecklist);

int invinciblefunc(struct player_struct *player);

void gethurt(struct status_struct status, int attack);

void restorehealth(int *HPptr, int restoreHPpts, void *otherdata);

void restorepower(int *powerptr, int restorepowerpts, void *otherdata);

void healthup(int *max_HPptr, int HPuppts, void *nullptr);

void PPup(int *max_PPptr, int PPuppts, void *nullptr);

/* Tools */

void multidrop();

void dropin( char arg[], int map[100][100][2], int posx, int posy);

void quitlevel(struct status_struct status);

void emptyfunc();
