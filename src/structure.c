#include <SDL2/SDL.h>
#include "structdef.h"
#include "structure.h"
#include <assert.h>

int declare_visual_structure(enum visual_structure_name_e name, enum visual_structure_name_e inherit, struct float_rect rect, struct style_struct *visual_structure_list) {

	struct style_struct *style = malloc(sizeof(*style));

	style->name = name;
	style->inherit = inherit;
	style->rect = rect;

	//if (visual_structure_list) {


	int num_style_structs = 10;
	int i = 0;
	//struct style_struct *visual_structure_list = malloc(sizeof(*visual_structure_list) * num_style_structs);

	struct style_struct style2 = {
		.name = SCREEN,
		.inherit = SCREEN,
		.rect = {0, 0, 1, 1}
	};
	visual_structure_list[i++] = style2;

	//style2.name = LEVEL_UI_TOP;
	//style2.inherit = SCREEN;
	//style2.rect = {0, 0, 1, 0.2};
	//visual_structure_list[i++] = style2;

	//style2.name = LEVEL_PLAY_AREA;
	//style2.inherit = SCREEN;
	//style2.rect = {1, 0.2, 1, 0.8};
	//visual_structure_list[i++] = style2;

	assert(i+1 == num_style_structs);

	return 0;
}

struct style_struct *decascade_visual_structure(struct style_struct *style, struct style_struct *visual_structure_list) {
	struct float_rect rect = style->rect;
	if (style->inherit == SCREEN) {
		return style;
	} else {
		struct style_struct *parent = decascade_visual_structure(&visual_structure_list[style->inherit], visual_structure_list);
		struct float_rect parent_rect = parent->rect;
		rect.x = parent_rect.x + rect.x * parent_rect.w;
		rect.y = parent_rect.y + rect.y * parent_rect.h;
		rect.w *= parent_rect.w;
		rect.h *= parent_rect.h;
	}
	style->rect = rect;
	return style;
}

SDL_Rect float_rect_to_pixels(struct float_rect *float_rect, struct xy_struct *screen_size) {
	SDL_Rect pix_rect = {
		.x = (int)(float_rect->x * screen_size->x),
		.y = (int)(float_rect->y * screen_size->y),
		.w = (int)(float_rect->w * screen_size->x),
		.h = (int)(float_rect->h * screen_size->y),
	};
	return pix_rect;
}

SDL_Rect visual_structure_to_pixels(struct style_struct *relative_style, struct style_struct *visual_structure_list, struct xy_struct *screen_size) {
	struct style_struct *absolute_style = decascade_visual_structure(relative_style, visual_structure_list);
	struct float_rect *float_rect = &absolute_style->rect;
	return float_rect_to_pixels(float_rect, screen_size);
}
