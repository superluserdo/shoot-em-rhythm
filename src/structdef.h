#define MAX_SOUNDS_LIST 10
#define MAX_ITEMS_PER_LANE_PER_SCREEN 20
#define TOTAL_LANES 5

#define FILEINFO fprintf(stderr, "In %s, line %d\n", __FILE__, __LINE__);

/*	Main	*/

struct xy_struct {
	int x;
	int y;
} grid;

struct size_ratio_struct {
	float w;
	float h;
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
	SDL_Rect rect_out;
	SDL_Rect rect_in;
};
/* Status Struct */

struct status_struct {
	struct level_struct *level;
	struct player_struct *player;
	struct audio_struct *audio;
	struct time_struct *timing;
	struct graphics_struct *graphics;
	struct program_struct *program;

};

/* Status of the Level */

struct lane_struct {
	int total;
	int currentlane;
	int lanewidth;
	int *laneheight;
};
struct level_struct {
	int score;
	int gameover;
	int levelover;
	int pauselevel; //level.pauselevel changed by levelfunc()
	int currentlevel;
	struct xy_struct grid;
	int maxscreens;
	int totalnativedist;
	int partymode;
	float speedmult;
	float speedmultmon;
	int currentscreen;
	int *laneheight;
	struct lane_struct lanes;
	struct laser_struct laser;
	struct sword_struct sword;
	struct level_var_struct *vars;
	struct level_effects_struct *effects;
	struct rects_struct *rects;
	struct monster *bestiary[10];
	struct item *itempokedex[10];
	int (**itemscreenstrip)[TOTAL_LANES][MAX_ITEMS_PER_LANE_PER_SCREEN][2];
	double *remainder;
};

struct level_effects_struct {
	double angle;
	int colournum;
	int hue;
};

struct level_var_struct {
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

struct rects_struct {
	//SDL_Rect rcTSrc[grid.x * 3][grid.y], rcTile[grid.x * 3][grid.y];
	//SDL_Rect rcTSrcmid[grid.x * 3][grid.y], rcTilemid[grid.x * 3][grid.y];
	SDL_Rect rcLaser[3], rcLaserSrc[3];
	SDL_Rect rcScore[5];
	SDL_Rect rcScoreSrc[5];
	SDL_Rect rcBeat[5];
	SDL_Rect rcBeatSrc[5];
};
struct mutex_list_struct {
	pthread_mutex_t soundstatus_mutex;
};


/* Generic Struct For Grasphical Object */

#define STD_STRUCT_DEF \
	struct xy_struct pos;\
	struct size_ratio_struct size_ratio;\
	struct animate_specific *animation;\
	union {\
		struct player_struct *self_player;\
		struct ui_bar *self_ui_bar;\
		struct ui_counter *self_ui_counter;\
	};

//	Have to do it this crappy way because standard C11 doesn't allow unnamed structs within
//	other structs (without -fms-extensions). So I'm writing a macro to define the struct
//	that I can paste into other places as if it were a tagged unnamed struct.

struct std {
STD_STRUCT_DEF;
};

/* Status of the Player */

struct player_struct {

	int HP, power;
	int max_HP;
	int max_PP;
	int invincibility;
	int invincibility_toggle;
	int invinciblecounter[2];
	int sword;
	int direction;
	int flydir;
	union {
		struct {
			struct xy_struct pos;
			struct size_ratio_struct size_ratio;
			struct animate_specific *animation;
			struct player_struct *self;
		};
		struct std std;
	};
};

/* Plugin Stuff */

struct hooks_struct {
	int numhooks;
	void *(**hookfuncs)(struct status_struct *status);
};

struct hooks_list_struct {
	void *(*hookfunc)(struct status_struct *status);
	struct hooks_list_struct *next;
};

struct hooktypes_struct {
	struct hooks_list_struct *frame;
	struct hooks_list_struct *level_init;
	struct hooks_list_struct *level_loop;
};

/* Whole Program */

struct program_struct {
	struct hooktypes_struct hooks;
};

/* Audio */

struct audio_struct {

	int track;
	int newtrack;
	int noise;
	int soundchecklist[MAX_SOUNDS_LIST];
	int music_mute;
	float music_volume;

};

/* Timing */

struct time_struct {

	int ticks;
	int ticks_last_frame;
	int countbeats;
	float bps;
	float startbeat;
	float currentbeat;
	int currentbeat_int;
	float pxperbeat;
	int framecount;
	int fpsanim;
	int fpsglobal;
	int *pauselevel; //changed inside frametimer(). Points to level.pauselevel
	int pause_change;
	int zerotime;
	int pausetime;
	int pausetime_completed;
	int pausetime_ongoing;
	int startpause;
	int endpause;
	float intervalanim;
	float intervalglobal;
};

struct graphics_struct {
	int width, height;
	SDL_Renderer *renderer;
	struct render_node *render_node_head;
	struct render_node *render_node_tail;
	struct ui_struct *ui;
	struct texture_struct *imgs; //temporary
	int num_images;
	struct rendercopyex_struct *rendercopyex_data;
	SDL_Texture *image_bank[];
};

struct texture_struct {
	SDL_Texture *Spriteimg;
	SDL_Texture *Laserimg;
	SDL_Texture *Swordimg;
	SDL_Texture *Timg;
	SDL_Texture *Mon0img;
	SDL_Texture *Mon1img;
	SDL_Texture *Scoreimg;
	SDL_Texture *Beatimg;
	SDL_Texture *Itemimg;
	SDL_Texture *texTarget;
};

/* RENDERING */

/* Custom Rendering (SDL_RenderCopyEx()) arguments struct */

struct rendercopyex_struct {
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Rect *srcrect;
	SDL_Rect *dstrect;
	double angle;
	SDL_Point *center;
	SDL_RendererFlip flip;
};

struct render_node {

	struct render_node *prev;
	struct render_node *next;
	SDL_Rect *rect_in;
	SDL_Rect rect_out;//*rect_out;
	SDL_Texture *img;
	int (*customRenderFunc)(void*);
	void *customRenderArgs;
	SDL_Renderer *renderer;
	struct animate_specific *animation;
	struct func_node *transform_list;
	float z; // Player defined as z = 0. +z defined as out of screen towards human.
};

	/* z	Reserved for
	 * -1	background
	 *  0	player
	 *  1	enemies
	 */

/*	Level	*/

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
	SDL_Texture *image;
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

struct ui_struct {
	struct ui_bar power;
	struct ui_bar hp;
	struct ui_counter score;
	struct ui_counter beat;
};

/* ANIMATION */

	/* Structs */

struct frame {
	SDL_Rect rect;
	float duration;
};

struct clip {
	SDL_Texture *img;
	int num_frames;
	struct frame *frames;
};

struct animate_generic {
	int num_clips;
	struct clip **clips;
	struct animate_specific *default_specific;
};

struct animate_specific {
	struct animate_generic *generic;

	struct rule_node *rules_list;
	int clip;
	int frame;
	float speed;
	int loops;
	int return_clip;
	float lastFrameBeat;
	struct xy_struct native_offset;
	struct std *parent;
	//struct xy_struct *parent_pos;	/*	Adjust object position and size ratio in these two structs.	*/
	//struct size_ratio_struct *parent_size_ratio;	/*	They're put into rect_out each frame.		*/
	//struct animate_specific *list_head;
	struct func_node *transform_list;

	struct render_node *render_node;
	SDL_Rect rect_out;
	struct animate_specific *next;
};

struct rule_node {
	void (*rule)(void *);
	void *data;
	struct rule_node *next;
};

struct func_node {
	void *data;
	void (*func)(void *rect_trans, void *data);
	struct func_node *next;
};

enum graphic_cat_e {CHARACTER, UI, UI_BAR, UI_COUNTER};
enum graphic_type_e {PLAYER, FLYING_HAMSTER, HP, POWER, COLOURED_BAR, NUMBERS, PLAYER2};

enum return_codes_e { R_SUCCESS, R_FAILURE, R_RESTART_LEVEL, R_LOOP_LEVEL, R_QUIT_TO_DESKTOP, R_CASCADE_UP=100, R_CASCADE_UP_MAX=199, R_STARTSCREEN=200, R_LEVELS=201 };

enum hook_type_e {FRAME, LEVEL_INIT, LEVEL_LOOP};

struct float_rect { // Floating-point analogue to the (int) pixel-based SDL_Rect
	double x;
	double y;
	double w;
	double h;
};
enum visual_structure_name_e {SCREEN, LEVEL_UI_TOP, LEVEL_PLAY_AREA};

struct style_struct {
	enum visual_structure_name_e name;
	enum visual_structure_name_e inherit;
	struct float_rect rect;
};

enum vector_e { ELEM_SIZE=-3, LEN=-2, USED=-1, DATA=0};
