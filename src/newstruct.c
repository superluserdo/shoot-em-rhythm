#include <SDL2/SDL.h>
#include "structdef.h"

/*	Main	*/

struct xy_struct new_xy_struct() {
	struct xy_struct new = {0};
	return new;
} 

struct size_ratio_struct new_size_ratio_struct() {
	struct size_ratio_struct new = {0};
	return new;
}

struct laser_struct new_laser_struct() {
	struct laser_struct new = {0};
	return new;
}

struct sword_struct new_sword_struct() {
	struct sword_struct new = {0};
	return new;
}
/* Status Struct */

struct status_struct new_status_struct() {
	struct status_struct new = {0};
	return new;
}

/* Status of the Level */

struct lane_struct new_lane_struct() {
	struct lane_struct new = {0};
	return new;
}

struct level_struct new_level_struct() {
	struct level_struct new = {0};
	return new;
}

struct level_effects_struct new_level_effects_struct() {
	struct level_effects_struct new = {0};
	return new;
}

struct level_var_struct new_level_var_struct() {
	struct level_var_struct new = {0};
	return new;
}

struct rects_struct new_rects_struct() {
	struct rects_struct new = {0};
	return new;
}

struct mutex_list_struct new_mutex_list_struct() {
	struct mutex_list_struct new = {0};
	return new;
}

struct std new_std() {
	struct std new = {0};
	return new;
}

/* Status of the Player */

struct player_struct new_player_struct() {
	struct player_struct new = {0};
	return new;
}

/* Whole Program */

struct program_struct new_program_struct() {
	struct program_struct new = {0};
	return new;
}

/* Audio */

struct audio_struct new_audio_struct() {
	struct audio_struct new = {0};
	return new;
}

/* Timing */

struct time_struct new_time_struct() {
	struct time_struct new = {0};
	return new;
}

struct graphics_struct new_graphics_struct() {
	struct graphics_struct new = {0};
	return new;
}

struct texture_struct new_texture_struct() {
	struct texture_struct new = {0};
	return new;
}

struct render_node new_render_node() {
	struct render_node new = {0};
	return new;
}

/*	Level	*/

struct monster_node new_monster_node() {
	struct monster_node new = {0};
	return new;
}


struct monster new_monster() {
	struct monster new = {0};
	return new;
}

struct item new_item() {
	struct item new = {0};
	return new;
}

struct ui_bar new_ui_bar() {
	struct ui_bar new = {0};
	return new;
}

struct ui_counter new_ui_counter() {
	struct ui_counter new = {0};
	return new;
}

struct ui_struct new_ui_struct() {
	struct ui_struct new = {0};
	return new;
}

/* ANIMATION */

	/* Structs */

struct frame new_frame() {
	struct frame new = {0};
	return new;
}

struct clip new_clip() {
	struct clip new = {0};
	return new;
}

struct animate_generic new_animate_generic() {
	struct animate_generic new = {0};
	return new;
}

struct animate_specific new_animate_specific() {
	struct animate_specific new = {0};
	return new;
}

struct rule_node new_rule_node() {
	struct rule_node new = {0};
	return new;
}

struct func_node new_func_node() {
	struct func_node new = {0};
	return new;
}
