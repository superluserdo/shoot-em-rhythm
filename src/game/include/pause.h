#ifndef _GAME_PAUSE_H
#define _GAME_PAUSE_H
enum menu_type_e {MENU, BUTTON, SLIDER};

struct option {
	enum menu_type_e menu_type;
	void *data;
	const char *text;
	int length;
	SDL_Rect option_rect;
};

enum menu_var_e {RESUME, RESTART, QUITTODESKTOP, QUITTOSTART};

struct option_button {
	enum menu_var_e var;
	int target_val;
};

struct menu_slider_int_struct {
	int min;
	int max;
	int increment;
	int *cur;
};

struct menu_slider_float_struct {
	float min;
	float max;
	float increment;
	float *cur;
};

struct menu_button_struct {
	 int *var;
	 int target_val;
};

struct menu_item_struct {
	enum menu_type_e menu_type;
	char *text;
	void *data;
	struct animation_struct *animation;
};

struct menu_struct {
	struct menu_struct *parent;
	int num_items;
	int cur_item;
	struct menu_item_struct *menu_items;
	union {
		struct {
			STD_MEMBERS
		};
		struct std std;
	};
	//struct tex_rect_struct **graphics;
};

struct menu_stage_struct {
	//Don't know if I need these
	//union {
	//	struct {
	//		STD_MEMBERS
	//	};
	//	struct std std;
	//};
	struct menu_struct *menu;
	struct graphical_stage_child_struct *stage;
};


struct menu_struct *pause_init(struct graphical_stage_child_struct **pause_stage_ptr, struct status_struct *status);
int pausefunc(SDL_Renderer *renderer, struct menu_stage_struct *pause_stage, struct status_struct *status);
//int pausefunc(SDL_Renderer *renderer, struct menu_stage_struct *pause_stage, SDL_Texture *levelcapture, struct status_struct *status);
//int pausefunc_bkp(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel, struct status_struct *status);

void update_text_texture(char *text, SDL_Texture **texture_ptr, struct graphics_struct *master_graphics);
void spawn_menu_graphics(struct menu_struct *menu, struct visual_container_struct *menu_container, TTF_Font *font, SDL_Color font_colour, struct graphical_stage_child_struct *stage, SDL_Renderer *renderer);

void quit_pause_menu(struct status_struct *status, struct menu_stage_struct *pause_stage);
#endif
