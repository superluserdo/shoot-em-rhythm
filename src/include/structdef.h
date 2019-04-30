#define MAX_SOUNDS_LIST 10
#define MAX_ITEMS_PER_LANE_PER_SCREEN 20
#define TOTAL_LANES 5
#define PI 3.14159265

#define FILEINFO fprintf(stderr, "In %s, line %d\n", __FILE__, __LINE__);
#define VEC_RESIZE_MULTIPLIER 2


/*	Main	*/

struct xy_struct {
	int x;
	int y;
} grid;

struct size_ratio_struct {
	float w;
	float h;
};

/* Generic Struct For Graphical Object */


#define STD_MEMBERS \
	const char *name; \
	struct visual_container_struct *container; /* Container structure of the entire object. Use container.pos_relative to control the object's position relative to parent at runtime. */\
	struct animation_struct *animation; \
	int (*object_logic)(struct std *std, void *data); \
	void *object_data; \
	struct std_list *object_stack_location; \
	void *self; \

/*	Have to do it this crappy way because standard C11 doesn't allow unnamed structs within
 *	other structs (without -fms-extensions). So I'm writing a macro to define the struct
 *	that I can paste into other places as if it were a tagged unnamed struct.
 */

struct std {
	STD_MEMBERS;
};

/* Generic Struct For Living Object */

struct living {
	int alive;
	int HP, power;
	float defence;
	int max_HP;
	int max_PP;
	int invincibility; /* 0 = Normal, 1 = hurt, 2 = invincible */
	float invincible_since;
	float iframes_duration;
	int invincibility_toggle;
	void *self;
};



struct std_list {
	struct std *std;
	struct std_list *next;
	struct std_list *prev;
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
	int swing_up_duration;
	int swing_down_duration;
	float dist_to_monster_spawn;
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
};
/* Status Struct */

struct status_struct {
	struct level_struct *level;
	struct player_struct *player;
	struct audio_struct *audio;
	struct time_struct *timing;
	struct graphics_struct *master_graphics;
	struct program_struct *program;

};

/* Status of the Level */

struct lane_struct {
	int total;
	int currentlane;
	float lanewidth;
	float *laneheight;	//TODO: Make float
	struct visual_container_struct *containers;
};

struct graphical_stage_struct {
	/* Something that has its own render target, animations, render nodes.
	   E.g. level, startscreen, pause screen, etc. ``graphics`` has master. */
	struct render_node *render_node_head;
	struct render_node *render_node_tail;
	struct rendercopyex_struct *rendercopyex_data;
	struct dict_void *generic_anim_dict;
	struct dict_void *image_dict;
	struct std_list *object_list_stack;
	int num_images;
	SDL_Texture **tex_target_ptr;
	struct graphics_struct *master_graphics;
};

struct graphical_stage_child_struct {
	struct std std;
	struct graphical_stage_struct graphics;
};

struct level_struct {
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	int score;
	int gameover;
	int levelover;
	int pauselevel; //level.pauselevel changed by levelfunc()
	int currentlevel;
	int partymode;
	struct lane_struct lanes;
	struct std_list **monster_list_stacks;
	struct std_list *monster_list_stack_end;//TODO: TMP
	struct laser_struct laser;
	struct sword_struct sword;
	struct level_var_struct *vars;
	struct level_effects_struct *effects;
	struct object_spawn_array_struct *object_spawn_arrays;
	struct ui_struct *ui;

	struct graphical_stage_child_struct stage;

	struct xy_struct grid;
	int currentscreen;
	float *laneheight;
};

struct level_effects_struct {
	double angle;
	int colournum;
	int hue;
};

struct level_var_struct {

	/* Below here are not used anymore: */
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

struct mutex_list_struct {
	pthread_mutex_t soundstatus_mutex;
};


/* Status of the Player */

struct player_struct {

	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	int invinciblecounter[2];
	int sword;
	int flydir;
	struct living living;
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

/* Debug Stuff */

struct debug_struct {
	int show_anchors;
	int show_containers;
	int test_render_list_robustness;
};

/* Whole Program */

struct program_struct {
	struct debug_struct debug;
	struct hooktypes_struct hooks;
	void *python_helper_function;
	void *python_helper_function_generator;
	void *status_python_capsule;
	int python_interpreter_activate;
	int python_interpreter_enable;
};

/* Audio */

struct audio_struct {

	/* Music */
	int trackmaxlen;
	char track[50];
	int soundchecklist[MAX_SOUNDS_LIST];
	int music_paused;
	int music_mute;
	float music_volume;

	/* Sounds */
	struct dict_void *sound_dict;
	int max_soundchannels;

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
	float container_width_per_beat;
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

struct float_rect { // Floating-point analogue to the (int) pixel-based SDL_Rect
	double x;
	double y;
	double w;
	double h;
};
enum visual_structure_name_e {SCREEN, LEVEL_UI_TOP, LEVEL_PLAY_AREA};

enum aspctr_lock_e {WH_INDEPENDENT, W_DOMINANT, H_DOMINANT};

struct visual_container_struct {
	struct visual_container_struct *inherit;
	int num_anchors_exposed;
	struct size_ratio_struct *anchor_grabbed; /* Pointer to the container-scale anchor 
												 this animation is locked to. Can be
												 defined by the animation, or point to
												 another animation's exposed anchor */
	struct size_ratio_struct *anchors_exposed; /* List of "exposed anchors" for a single texture/layer
										  animation. Other animations can lock onto these */
	struct size_ratio_struct anchor_hook; /* Sprite-relative position of anchor hook.
											 Default is the one specified by generic animation */
	enum aspctr_lock_e aspctr_lock; /* Should both w and h be inherited, or just one (locked aspect ratio) */
	struct float_rect rect_out_parent_scale;
	struct float_rect rect_out_screen_scale;
	int screen_scale_uptodate;
};

struct visual_container_struct_old {
	//enum visual_structure_name_e name;
	//enum visual_structure_name_e inherit;
	struct visual_container_struct_old *inherit;
	struct float_rect rect;
	float aspctr; /* Aspect ratio of container = w/h
				   * Don't use if aspctr_lock == WH_INDEPENDENT */
	enum aspctr_lock_e aspctr_lock; /* Should both w and h be inherited, or just one (locked aspect ratio) */
};

enum vector_e { START=-3, ELEM_SIZE=-3, LEN=-2, USED=-1, DATA=0};

struct graphics_struct {
	SDL_Window *window;
	int width, height;
	struct visual_container_struct screen;
	SDL_Renderer *renderer;
	int *debug_anchors;
	int *debug_containers;
	int *debug_test_render_list_robustness;
	struct std_list *object_list_stack;
	struct graphical_stage_struct graphics;
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
	struct animation_struct *animation;
	struct func_node *transform_list;
	float z; // Player defined as z = 0. +z defined as out of screen towards human.
};

	/* z	Reserved for
	 * -1	background
	 *  0	player
	 *  1	enemies
	 */

/*	Level	*/

/* New way to queue monsters and items: */

struct object_spawn_elem_struct {
	float spawn_beat;
	void *ptr;
};

struct object_spawn_array_struct {
	int next_index;
	int num_objects;
	struct object_spawn_elem_struct *objects;
};

struct object_n_lanes_arrays_struct {
	int num_lanes;
	struct object_spawn_array_struct *arrays;
};

struct monster_struct {
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	struct living living;
	struct std_list *monster_stack_location;
	struct std_list **monster_list_stack_ptr;
	float entrybeat; //TODO: TMP
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
			STD_MEMBERS
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
			STD_MEMBERS
		};
		struct std std;
	};
};

struct ui_struct {
	struct ui_bar *power;
	struct ui_bar *hp;
	struct ui_counter *score;
	struct ui_counter *beat;
};

/* ANIMATION */

	/* Structs */

enum layer_mode_e { TIGHT, GLOBAL };

struct frame {
	SDL_Rect rect;
	struct xy_struct anchor_hook; /* The position of the texture that "hooks" onto anchors */
	float duration;
};

struct clip {
	SDL_Texture *img;
	int num_frames;
	struct frame *frames;
	float container_scale_factor;
	enum aspctr_lock_e aspctr_lock;
};

struct animate_generic {
	int num_clips;
	struct clip **clips;
};

struct anchor_struct {
	struct size_ratio_struct pos_anim_internal;
	struct animation_struct *anim;
};
	
enum animate_mode_e {
	GENERIC,	/* Controls a full generic animation with frames */
	TEXTURE,	/* Transform-based animations of a static 
				   or externally-changed texture */
	CONTAINER,	/* Transform-based animations of a container */
};

struct animate_control {

	/* Control of generic animation */
	struct animate_generic *generic;
	int clip;
	int frame;
	float speed;
	int loops;
	int backwards;
	int return_clip;
	float lastFrameBeat;
};

struct animation_struct {

	/* Parent object the animation belongs to */
	struct std *object;

	/* Animation mode */
	enum animate_mode_e animate_mode;

	/* GENERIC mode only */
	struct animate_control *control;

	/* GENERIC and TEXTURE modes only */
	SDL_Texture *img;

	/* Container-related */
	struct visual_container_struct container;
	enum layer_mode_e layer_mode; // Not used yet -- whether z describes placement within one
								  // or all animations (I think? Can't remember)
	float z;
	struct rule_node *rules_list;
	struct func_node *transform_list;
	struct render_node *render_node;
	struct animation_struct *next;
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

//enum graphic_cat_e {CHARACTER, UI, UI_BAR, UI_COUNTER};
//enum graphic_type_e {PLAYER, FLYING_HAMSTER, HP, POWER, COLOURED_BAR, NUMBERS, PLAYER2, SWORD, SMILEY};

/* Needed for death explosion animation: */
struct explosion_data_struct {
	struct animation_struct *animation;
	struct living *living;
};


enum return_codes_e { R_SUCCESS, R_FAILURE, R_RESTART_LEVEL, R_LOOP_LEVEL, R_PAUSE_LEVEL, R_QUIT_TO_DESKTOP, R_CASCADE_UP=100, R_CASCADE_UP_MAX=199, R_STARTSCREEN=200, R_LEVELS=201 };

enum hook_type_e {FRAME, LEVEL_INIT, LEVEL_LOOP};
