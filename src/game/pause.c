#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "structdef.h"
#include "main.h"
#include "animate.h"
#include "spawn.h"
#include "clock.h"
#include "audio.h"
#include "pause.h"

// Put struct declarations in structdef:

//struct pause_struct {
//	struct std std;
//	struct graphical_stage_struct *graphics;
//};
//
//struct pause_struct *pause = status.level;
//*pause = (struct pause_struct) {0};
//struct std *std = &pause->std;
//
//spawn_graphical_stage(std, master_graphics, &level->graphics, level, "level");

//struct tex_rect_struct {
//	SDL_Texture *texture;
//	SDL_Rect rect;
//};

struct menu_struct *create_menu_struct(struct status_struct *status) {

	//TODO: Have something that can parse the pause.cfg file, but
	// instead of just presenting the text, have different structs
	// that can do things like, set a particular value when selected,
	// or act as a slider, etc. Keep logic separate from graphics
	// representation.

	// Create structure:
	struct menu_struct *pause_menu = malloc(sizeof(struct menu_struct));
	*pause_menu = (struct menu_struct){0};
	pause_menu->num_items = 5;
	pause_menu->menu_items = malloc(pause_menu->num_items * sizeof(struct menu_item_struct));

	pause_menu->menu_items[0] = (struct menu_item_struct) {
		.menu_type = BUTTON,
		.text = "Resume",
		.data = malloc(sizeof(enum menu_var_e)),
	};
	*(enum menu_var_e *)pause_menu->menu_items[0].data = RESUME;

	pause_menu->menu_items[1] = (struct menu_item_struct) {
		.menu_type = BUTTON,
		.text = "Restart",
		.data = malloc(sizeof(enum menu_var_e)),
	};
	*(enum menu_var_e *)pause_menu->menu_items[1].data = RESTART;

	pause_menu->menu_items[2] = (struct menu_item_struct) {
		.menu_type = BUTTON,
		.text = "Quit To Start Screen",
		.data = malloc(sizeof(enum menu_var_e)),
	};
	*(enum menu_var_e *)pause_menu->menu_items[2].data = QUITTOSTART;

	pause_menu->menu_items[3] = (struct menu_item_struct) {
		.menu_type = BUTTON,
		.text = "Quit To Desktop",
		.data = malloc(sizeof(enum menu_var_e)),
	};
	*(enum menu_var_e *)pause_menu->menu_items[3].data = QUITTODESKTOP; 

	struct menu_struct *test_menu = malloc(sizeof(struct menu_struct));
	*test_menu = (struct menu_struct){0};
	test_menu->num_items = 3;
	test_menu->menu_items = malloc(test_menu->num_items * sizeof(struct menu_item_struct));

	test_menu->menu_items[0] = (struct menu_item_struct) {
		.menu_type = BUTTON,
		.text = "TEST resume",
		.data = malloc(sizeof(enum menu_var_e)),
	};
	*(enum menu_var_e *)test_menu->menu_items[0].data = RESUME;

	test_menu->menu_items[1] = (struct menu_item_struct) {
		.menu_type = BUTTON,
		.text = "TEST Restart",
		.data = malloc(sizeof(enum menu_var_e)),
	};
	*(enum menu_var_e *)test_menu->menu_items[1].data = RESTART;

	test_menu->menu_items[2] = (struct menu_item_struct) {
		.menu_type = SLIDER,
		.text = "TEST Slider",
		.data = malloc(sizeof(struct menu_slider_int_struct)),
	};
	int *testint = malloc(sizeof(*testint));
	*testint = 123;
	*(struct menu_slider_int_struct *)test_menu->menu_items[2].data = (struct menu_slider_int_struct) {
		.min = 0,
		.max = 100,
		.increment = 2,
		.cur = testint
	};
	*(enum menu_var_e *)test_menu->menu_items[1].data = RESTART;

	pause_menu->menu_items[4] = (struct menu_item_struct) {
		.menu_type = MENU,
		.text = "Test menu",
		.data = test_menu
	};
	

	return pause_menu;
}

void update_text_texture(char *text, SDL_Texture **texture_ptr, struct graphics_struct *master_graphics) {
		if (*texture_ptr) {
			SDL_DestroyTexture(*texture_ptr);
		}

		SDL_Surface *text_surface = TTF_RenderText_Solid(master_graphics->font, text, master_graphics->font_colour);
		if (text_surface == NULL) {
			fprintf(stderr, "Couldn't render text\n");
			FILEINFO
		}

		SDL_Texture *text_texture = SDL_CreateTextureFromSurface(master_graphics->renderer, text_surface);
		SDL_FreeSurface(text_surface);
		*texture_ptr = text_texture;
}

void spawn_menu_graphics(struct menu_struct *menu, struct visual_container_struct *menu_container, TTF_Font *font, SDL_Color font_colour, struct graphical_stage_child_struct *stage, SDL_Renderer *renderer) {

	int num_animations = 1; /* Menu cursor */

	/* Get number of animations to spawn */
	for (int i = 0; i < menu->num_items; i++) {
		struct menu_item_struct item = menu->menu_items[i];
		if (item.menu_type == SLIDER) {
			num_animations += 2;
		} else {
			num_animations++;
		}
	}

	const char *animation_type_array[num_animations];
	for (int i = 0; i < num_animations; i++) {
		animation_type_array[i] = "ser_texture";
	}

	/* Spawn animations */
	graphic_spawn(&menu->std, &stage->graphics.object_list_stack, NULL, &stage->graphics, animation_type_array, num_animations);

	struct animation_struct *animation = menu->animation;

	/* Do the cursor first */
	float container_w_pad = 0.1;
	float container_cursor_w = 0.2;

	update_text_texture(">", &animation->img, stage->graphics.master_graphics);

	struct visual_container_struct item_text_container = {
		.inherit = menu_container,
		.rect_out_parent_scale = (struct float_rect) { 
			.x = - (container_w_pad + container_cursor_w),
			.y = 0,
			.w = container_cursor_w,
			.h = 1,
		},
		.aspctr_lock = WH_INDEPENDENT,
	};

	animation->container = item_text_container;

	animation = animation->next;

	/* Now the items in the menu */
	for (int i = 0; i < menu->num_items; i++) {

		struct menu_item_struct *item = &menu->menu_items[i];

		char *text = item->text;

		update_text_texture(text, &animation->img, stage->graphics.master_graphics);

		struct visual_container_struct item_text_container = {
			.inherit = menu_container,
			.rect_out_parent_scale = (struct float_rect) { 
				.x = 0,//container_w_pad + container_cursor_w, 
				.y = (float)i/menu->num_items, 
				.w = strlen(text) * 0.05, 
				.h = 0.75 / menu->num_items,
			},
			.aspctr_lock = WH_INDEPENDENT,
		};

		animation->container = item_text_container;
		item->animation = animation;

		animation = animation->next;

		if (item->menu_type == SLIDER) {
			float slider_ideal_w = 0.5;
			float container_w_remainder = 1 - (item_text_container.rect_out_parent_scale.x + item_text_container.rect_out_parent_scale.w + container_w_pad);
			float slider_w;
			if (container_w_remainder < 0) {
				slider_w = 0;
			} else if (container_w_remainder > slider_ideal_w) {
				slider_w = slider_ideal_w;
			} else {
				slider_w = container_w_remainder;
			}
			struct visual_container_struct slider_container = {
				.inherit = menu_container,
				.rect_out_parent_scale = (struct float_rect) { 
					.x = 1 - slider_w,
					.y = (float)i/menu->num_items, 
					.w = slider_w,
					.h = 0.75 / menu->num_items,
				},
				.aspctr_lock = WH_INDEPENDENT,
			};
			animation->container = slider_container;

			char snum[4];
			sprintf(snum, "%d", *((struct menu_slider_int_struct *)item->data)->cur);

			SDL_Surface *text_surface = TTF_RenderText_Solid(font, snum, font_colour);
			if (text_surface == NULL) {
				fprintf(stderr, "Couldn't render text\n");
				FILEINFO
			}

			SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
			SDL_FreeSurface(text_surface);
			animation->img = text_texture;

			animation = animation->next;
		}
	}

	/* Set the cursor to the first item */
	menu->animation->container.inherit = &menu->menu_items[0].animation->container;
}

struct menu_struct *pause_init(struct graphical_stage_child_struct **pause_stage_ptr, SDL_Texture *levelcapture, struct status_struct *status) {

	pause_time(status->timing);
	pausemusic_toggle(status->audio, status->timing);

	*pause_stage_ptr = malloc(sizeof(struct graphical_stage_child_struct));
	struct graphical_stage_child_struct *stage = *pause_stage_ptr;
	struct graphical_stage_struct *graphics = &stage->graphics;
	*stage = (struct graphical_stage_child_struct) {0};

	struct graphics_struct *master_graphics = status->master_graphics;
	spawn_graphical_stage_child(stage, master_graphics, &stage->std, "pausemenu");

	/* Set the background texture as an object */

	spawn_bg(graphics, master_graphics, levelcapture);

	// Init Font ...


	if ( TTF_Init() == -1 ) {

		fprintf(stderr, "Init TTF  failed\n");
		//return R_FAILURE;
	} 
	if (!master_graphics->font) {
		master_graphics->font = (TTF_Font *) TTF_OpenFont("data/DejaVuSans.ttf",300);
	}
	if (!master_graphics->font) {
		fprintf(stderr, "Couldn't open font\n");
		FILEINFO
		abort();
	}
	master_graphics->font_colour = (SDL_Color) {
		.r = 150,
		.g = 250,
		.b = 250,
	};
	TTF_Font *font = master_graphics->font;

	struct menu_struct *menu = create_menu_struct(status);

	struct visual_container_struct *menu_container = malloc(sizeof(*menu_container));
	*menu_container = (struct visual_container_struct) {
		.inherit = &master_graphics->screen,
		.rect_out_parent_scale = (struct float_rect) { 
			.x = 0.3,
			.y = 0.3,
			.w = 0.4,
			.h = 0.4,
		},
		.aspctr_lock = WH_INDEPENDENT,
	};
	spawn_menu_graphics(menu, menu_container, font, master_graphics->font_colour, stage, master_graphics->renderer);

	return menu;

}

int pausefunc(SDL_Renderer *renderer, struct menu_stage_struct *pause_stage, SDL_Texture *levelcapture, struct status_struct *status) {

	struct graphical_stage_child_struct **pause_stage_ptr = &pause_stage->stage;
	if (!*pause_stage_ptr) {
		pause_stage->menu = pause_init(pause_stage_ptr, levelcapture, status);
	}

	enum menu_var_e action;
	struct graphical_stage_child_struct *stage = *pause_stage_ptr;
	struct graphics_struct *master_graphics = stage->graphics.master_graphics;

	struct menu_struct *menu = pause_stage->menu;

	//SDL_SetRenderTarget(renderer, texTarget);
	action = -1;

	struct menu_item_struct item = menu->menu_items[menu->cur_item];

	/* Handle Events */

	SDL_Event e;
	while ( SDL_PollEvent(&e) ) {

		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
			if (menu->parent) {
				menu = menu->parent;
			} else {
				unpause_time(status->timing);
				pausemusic_toggle(status->audio, status->timing);
				return R_LOOP_LEVEL;
			}
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP) {
			menu->cur_item--;
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN) {
			menu->cur_item++;
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT) {
			if (item.menu_type == SLIDER) {
				(*((struct menu_slider_int_struct *)item.data)->cur)--;
				char snum[4];
				sprintf(snum, "%d", *((struct menu_slider_int_struct *)item.data)->cur);
				update_text_texture(snum, &item.animation->next->img, master_graphics);
			}
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT) {
			if (item.menu_type == SLIDER) {
				(*((struct menu_slider_int_struct *)item.data)->cur)++;
				char snum[4];
				sprintf(snum, "%d", *((struct menu_slider_int_struct *)item.data)->cur);
				update_text_texture(snum, &item.animation->next->img, master_graphics);
			}
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
			if (item.menu_type == BUTTON) {
				action = *(enum menu_var_e *)item.data;
			} else if (item.menu_type == MENU) {
				struct menu_struct *menu_new = (struct menu_struct *)item.data;
				menu_new->parent = menu;
				menu = menu_new;
				//TODO
				//spawn_menu_graphics(menu, container, font, master_graphics->font_colour, *pause_stage_ptr, renderer);
			}
		}
	}

	if (menu->cur_item < 0) {
			menu->cur_item = menu->num_items - 1;
	}
	
	menu->cur_item %= menu->num_items;
	
	if (action == RESUME) {
		unpause_time(status->timing);
		pausemusic_toggle(status->audio, status->timing);
		return R_LOOP_LEVEL;
	}
	else if (action == RESTART) {
		unpause_time(status->timing);
		stopmusic(status->audio);
		return R_RESTART_LEVEL;
	}
	else if (action == QUITTOSTART) {
		TTF_CloseFont(master_graphics->font);
		SDL_RenderClear(renderer);
		for (int i = 0; i < menu->num_items; i++) {
			//SDL_DestroyTexture(menu->graphics[i][0].texture);
		}
		unpause_time(status->timing);
		stopmusic(status->audio);
		return R_STARTSCREEN;
	}
	else if (action == QUITTODESKTOP) {
		unpause_time(status->timing);
		stopmusic(status->audio);
		return R_QUIT_TO_DESKTOP;
	}

	/* Set the cursor to the highlighted item */
	menu->animation->container.inherit = &menu->menu_items[menu->cur_item].animation->container;

	render_process(stage->graphics.object_list_stack, &stage->graphics, master_graphics, stage->graphics.tex_target_ptr, 0);
	return R_PAUSE_LEVEL;

}
