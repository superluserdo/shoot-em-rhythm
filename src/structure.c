#include <SDL2/SDL.h>
#include "structdef.h"
#include "structure.h"
#include <assert.h>

////int declare_visual_structure(enum visual_structure_name_e name, enum visual_structure_name_e inherit, struct float_rect rect, struct visual_container_struct *visual_structure_list) {
//int declare_visual_structure(char *name, struct visual_container_struct inherit, struct float_rect rect, struct visual_container_struct *visual_structure_list) {
//
//	struct visual_container_struct *container = malloc(sizeof(*container));
//
//	container->name = name;
//	container->inherit = inherit;
//	container->rect = rect;
//
//	//if (visual_structure_list) {
//
//
//	int num_visual_container_structs = 10;
//	int i = 0;
//	//struct visual_container_struct *visual_structure_list = malloc(sizeof(*visual_structure_list) * num_visual_container_structs);
//
//	struct visual_container_struct container2 = {
//		.name = SCREEN,
//		.inherit = SCREEN,
//		.rect = {0, 0, 1, 1}
//	};
//	visual_structure_list[i++] = container2;
//
//	//container2.name = LEVEL_UI_TOP;
//	//container2.inherit = SCREEN;
//	//container2.rect = {0, 0, 1, 0.2};
//	//visual_structure_list[i++] = container2;
//
//	//container2.name = LEVEL_PLAY_AREA;
//	//container2.inherit = SCREEN;
//	//container2.rect = {1, 0.2, 1, 0.8};
//	//visual_structure_list[i++] = container2;
//
//	assert(i+1 == num_visual_container_structs);
//
//	return 0;
//}

struct float_rect decascade_visual_container(struct visual_container_struct *container) {
	if (!container->inherit) {
		return (struct float_rect) {.x=0, .y=0, .w=1, .h=1};
	} else if (container->screen_scale_uptodate) {
		return container->rect_out_screen_scale;
	}
	struct float_rect rect = container->rect_out_parent_scale;
	struct float_rect parent_rect = decascade_visual_container(container->inherit);

	/* Set width & height */
	enum aspctr_lock_e aspctr_lock = container->aspctr_lock;

	if (aspctr_lock == WH_INDEPENDENT) {
		rect.w *= parent_rect.w;
		rect.h *= parent_rect.h;
	} else if (aspctr_lock == W_DOMINANT) {
		float aspctr = rect.w/rect.h;
		rect.w *= parent_rect.w;
		rect.h *= parent_rect.w / aspctr;
	} else if (aspctr_lock == H_DOMINANT) {
		float aspctr = rect.w/rect.h;
		rect.w *= parent_rect.h;
		rect.h *= parent_rect.h * aspctr;
	}

	struct size_ratio_struct anchor_grabbed;
	if (container->anchor_grabbed) {
		anchor_grabbed = *container->anchor_grabbed;
	} else {
		anchor_grabbed = (struct size_ratio_struct) {0};
	}

	struct size_ratio_struct anchor_hook_pos = {
		.w = container->anchor_hook.w * rect.w,
		.h = container->anchor_hook.h * rect.h,
	};

	/* Set x and y position wrt whole window */
	rect.x = parent_rect.x + rect.x * parent_rect.w + parent_rect.w * anchor_grabbed.w - anchor_hook_pos.w;
	rect.y = parent_rect.y + rect.y * parent_rect.h + parent_rect.h * anchor_grabbed.h - anchor_hook_pos.h;

	container->rect_out_screen_scale = rect;
	//container->screen_scale_uptodate = 1;

	return rect;
}

SDL_Rect float_rect_to_pixels(struct float_rect *float_rect, struct xy_struct screen_size) {
	SDL_Rect pix_rect = {
		.x = (int)(float_rect->x * screen_size.x),
		.y = (int)(float_rect->y * screen_size.y),
		.w = (int)(float_rect->w * screen_size.x),
		.h = (int)(float_rect->h * screen_size.y),
	};
	return pix_rect;
}

SDL_Rect visual_container_to_pixels(struct visual_container_struct *relative_container, struct xy_struct screen_size) {
	struct float_rect float_rect = decascade_visual_container(relative_container);
	return float_rect_to_pixels(&float_rect, screen_size);
}

int container_test_overlap_x(struct visual_container_struct *container_1, struct visual_container_struct *container_2) {
	struct float_rect decascade_1 = decascade_visual_container(container_1);
	struct float_rect decascade_2 = decascade_visual_container(container_2);
	if (
		((decascade_1.x > decascade_2.x) && (decascade_1.x < decascade_2.x + decascade_2.w)) ||
		((decascade_2.x > decascade_1.x) && (decascade_2.x < decascade_1.x + decascade_1.w))
	   ) {
		return 1;
	} else {
		return 0;
	}
}

int container_test_overlap_y(struct visual_container_struct *container_1, struct visual_container_struct *container_2) {
	struct float_rect decascade_1 = decascade_visual_container(container_1);
	struct float_rect decascade_2 = decascade_visual_container(container_2);
	if (
		((decascade_1.y > decascade_2.y) && (decascade_1.y < decascade_2.y + decascade_2.h)) ||
		((decascade_2.y > decascade_1.y) && (decascade_2.y < decascade_1.y + decascade_1.h))
	   ) {
		return 1;
	} else {
		return 0;
	}
}

int container_test_overlap(struct visual_container_struct *container_1, struct visual_container_struct *container_2) {
	/* Compute whether there is any overlap between to containers.
	   Only works for non-rotated containers */

	if (
		(container_test_overlap_x(container_1, container_2)) && 
		(container_test_overlap_y(container_1, container_2))
		) {
		return 1;
		} else {
		return 0;
	}
}

struct size_ratio_struct pos_at_custom_anchor_hook(struct visual_container_struct *container, float x, float y) {
	struct size_ratio_struct custom_anchor_hook = {.w = x, .h = y};
	struct size_ratio_struct old_anchor_hook = container->anchor_hook;

	/* Preserve the real position: */
	struct size_ratio_struct old_pos = {container->rect_out_parent_scale.x, container->rect_out_parent_scale.y};

	struct size_ratio_struct custom_pos;

	/* Finding new position is trivial for independent axes: */
	if (container->aspctr_lock == WH_INDEPENDENT) {
		custom_pos.w = old_pos.w + (custom_anchor_hook.w - old_anchor_hook.w) * container->rect_out_parent_scale.w;
		custom_pos.h = old_pos.h + (custom_anchor_hook.h - old_anchor_hook.h) * container->rect_out_parent_scale.h;
	} else {
		/* Requires knowing the absolute dimensions when you have a fixed aspect ratio: */

		/* Get up-to-date aspect ratios */
		if (!container->screen_scale_uptodate) {
			decascade_visual_container(container);
		}
		float aspctr = container->rect_out_screen_scale.w / container->rect_out_screen_scale.h;

		if (!container->inherit->screen_scale_uptodate) {
			decascade_visual_container(container->inherit);
		}
		float aspctr_inherit = container->inherit->rect_out_screen_scale.w / container->inherit->rect_out_screen_scale.h;

		if (container->aspctr_lock == W_DOMINANT) {
			custom_pos.w = old_pos.w + (custom_anchor_hook.w - old_anchor_hook.w) 
				* container->rect_out_parent_scale.w;
			custom_pos.h = old_pos.h + (custom_anchor_hook.h - old_anchor_hook.h) 
				* container->rect_out_parent_scale.w / aspctr * aspctr_inherit;
		} else if (container->aspctr_lock == H_DOMINANT) {
			custom_pos.w = old_pos.w + (custom_anchor_hook.w - old_anchor_hook.w) 
				* container->rect_out_parent_scale.h * aspctr / aspctr_inherit;
			custom_pos.h = old_pos.h + (custom_anchor_hook.h - old_anchor_hook.h) 
				* container->rect_out_parent_scale.h;
		}
	}

	return custom_pos;
}

