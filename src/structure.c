#include <SDL2/SDL.h>
#include <assert.h>
enum visual_structure_name_e {SCREEN, LEVEL_UI_TOP, LEVEL_PLAY_AREA};

struct style_struct {
	enum visual_structure_name_e name;
	enum visual_structure_name_e inherit;
	SDL_Rect rect;
};

int declare_visual_structure() {

	int num_style_structs = 10;
	int i = 0;
	struct style_struct *visual_structure_list = malloc(sizeof(struct style_struct) * num_style_structs);

	struct style_struct style_struct = {
		.name = SCREEN,
		.inherit = SCREEN,
		.rect = {1, 1, 1, 1}
	};
	visual_structure_list[i++] = style_struct;

	assert(i+1 == num_style_structs);
	return 0;


}
