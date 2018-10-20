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
	struct float_rect rect = container->rect;

	while (container->inherit) {
		container = container->inherit;
		struct float_rect parent_rect = container->rect;
		rect.x = parent_rect.x + rect.x * parent_rect.w;
		rect.y = parent_rect.y + rect.y * parent_rect.h;
		rect.w *= parent_rect.w;
		rect.h *= parent_rect.h;
	}
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
