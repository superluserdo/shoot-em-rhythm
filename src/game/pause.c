#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include <SDL2/SDL_ttf.h>
#include "structdef.h"
#include "main.h"
#include "clock.h"
#include "audio.h"
//#include "level.h"

extern pthread_mutex_t display_mutex;
extern pthread_mutex_t clock_mutex;
extern pthread_cond_t display_cond;

// Put struct declarations in structdef:

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
	const char *text;
	void *data;
};

struct menu_struct {
	struct menu_struct *parent;
	int num_items;
	int cur_item;
	struct menu_item_struct *menu_items;
	struct tex_rect_struct **graphics;
};

struct tex_rect_struct {
	SDL_Texture *texture;
	SDL_Rect rect;
};

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

struct tex_rect_struct **menu_graphics(struct menu_struct *menu, TTF_Font *font, SDL_Color fg, SDL_Renderer *renderer) {

	SDL_Surface *textsurfaces[menu->num_items];
	struct tex_rect_struct **menu_gr = malloc(menu->num_items * sizeof(*menu_gr));

	for (int i = 0; i < menu->num_items; i++) {
		struct menu_item_struct item = menu->menu_items[i];

		if (item.menu_type == SLIDER) {
			menu_gr[i] = malloc(2 * sizeof(menu_gr[i]));
		} else {
			menu_gr[i] = malloc(1 * sizeof(menu_gr[i]));
		}
		const char *text = item.text;
		int text_length = strlen(text);
		menu_gr[i][0].rect.x = 500;
		menu_gr[i][0].rect.y = 300 + 80 * i;
		menu_gr[i][0].rect.w = 30 * text_length;
		menu_gr[i][0].rect.h = 50;

		textsurfaces[i] = TTF_RenderText_Solid(font, text, fg);
		if (textsurfaces[i] == NULL) {
			printf("darn\n");
		}

		menu_gr[i][0].texture = SDL_CreateTextureFromSurface(renderer, textsurfaces[i]);
		if (item.menu_type == SLIDER) {
			menu_gr[i][1].rect.x = 500 + 400;
			menu_gr[i][1].rect.y = 300 + 80 * i;
			menu_gr[i][1].rect.w = 30 * text_length;
			menu_gr[i][1].rect.h = 50;

			char snum[4];
			sprintf(snum, "%d", *((struct menu_slider_int_struct *)item.data)->cur);
			SDL_Surface *slider_surface = TTF_RenderText_Solid(font, snum, fg);
			menu_gr[i][1].texture = SDL_CreateTextureFromSurface(renderer, slider_surface);
		}
	}

	return menu_gr;
}

int pausefunc(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel, struct status_struct *status) {

	pause_time(status->timing);
	pausemusic_toggle(status->audio, status->timing);

	// Init Font ...

	if ( TTF_Init( ) == -1 ) {

		fprintf(stderr, "Init TTF  failed\n");
		//return -1;
	} 
	TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf",300);
	SDL_Color fg;
	fg.r = 150;
	fg.g = 250;
	fg.b = 250;

	struct menu_struct *menu = create_menu_struct(status);

	if (!menu->graphics) {
	menu->graphics = menu_graphics(menu, font, fg, renderer);
	}

	struct tex_rect_struct cursor;
	const char *cursor_text = ">";
	int cursor_length = strlen(cursor_text);
	cursor.rect.x = menu->graphics[menu->cur_item][0].rect.x - 30;
	cursor.rect.y = menu->graphics[menu->cur_item][0].rect.y;
	cursor.rect.w = 30;
	cursor.rect.h = 50;

	SDL_Surface *cursorsurf = TTF_RenderText_Solid(font, cursor_text, fg);
	if (cursorsurf == NULL) {
		printf("darn\n");
	}
	SDL_Texture *cursortex = SDL_CreateTextureFromSurface(renderer, cursorsurf);

	SDL_Rect textbox_rect;
	textbox_rect.x = cursor.rect.x - 30;
	textbox_rect.y = menu->graphics[0][0].rect.y - 30;
	textbox_rect.w = 800;
	textbox_rect.h = 400;

	SDL_Texture *texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, status->graphics->width, status->graphics->height);

	enum menu_var_e action;

	while (1) {

		SDL_SetRenderTarget(renderer, texTarget);
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
					SDL_Surface *slider_surface = TTF_RenderText_Solid(font, snum, fg);
					menu->graphics[menu->cur_item][1].texture = SDL_CreateTextureFromSurface(renderer, slider_surface);
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT) {
				if (item.menu_type == SLIDER) {
					(*((struct menu_slider_int_struct *)item.data)->cur)++;
					char snum[4];
					sprintf(snum, "%d", *((struct menu_slider_int_struct *)item.data)->cur);
					SDL_Surface *slider_surface = TTF_RenderText_Solid(font, snum, fg);
					menu->graphics[menu->cur_item][1].texture = SDL_CreateTextureFromSurface(renderer, slider_surface);
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
				if (item.menu_type == BUTTON) {
					action = *(enum menu_var_e *)item.data;
				} else if (item.menu_type == MENU) {
					struct menu_struct *menu_new = (struct menu_struct *)item.data;
					menu_new->parent = menu;
					menu = menu_new;
					menu->graphics = menu_graphics(menu, font, fg, renderer);
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
			stopmusic(status->audio, status->timing);
			return R_RESTART_LEVEL;
		}
		else if (action == QUITTOSTART) {
			TTF_CloseFont(font);
			SDL_RenderClear(renderer);
			for (int i = 0; i < menu->num_items; i++) {
				SDL_DestroyTexture(menu->graphics[i][0].texture);
			}
			unpause_time(status->timing);
			stopmusic(status->audio, status->timing);
			return R_STARTSCREEN;
		}
		else if (action == QUITTODESKTOP) {
			unpause_time(status->timing);
			stopmusic(status->audio, status->timing);
			return R_QUIT_TO_DESKTOP;
		}

		cursor.rect.y = menu->graphics[menu->cur_item][0].rect.y;

		SDL_RenderCopy(renderer, levelcapture, NULL, NULL);
		for (int index = 0; index < menu->num_items; index++) {
			SDL_RenderCopy(renderer, menu->graphics[index][0].texture, NULL, &menu->graphics[index][0].rect);
			if (menu->menu_items[index].menu_type == SLIDER) {
				SDL_RenderCopy(renderer, menu->graphics[index][1].texture, NULL, &menu->graphics[index][1].rect);
			}
		}
		SDL_RenderCopy(renderer, cursortex, NULL, &cursor.rect);

		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopy(renderer, texTarget, NULL, NULL);

		wait_to_present(status->timing);
		//pthread_cond_wait(&display_cond, &display_mutex);
		SDL_RenderPresent(renderer);
		//pthread_mutex_unlock(&display_mutex);

	}

}

int pausefunc_bkp(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel, struct status_struct *status) {

	pause_time(status->timing);
	int resume = 0;
	int restart = 0;
	int quittostart = 0;
	int quittodesktop = 0;

	/* Setup Config File */
	config_t cfg2;
	config_setting_t *setting;
	char *str;

	config_init(&cfg2);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg2, "pause.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg2),
		config_error_line(&cfg2), config_error_text(&cfg2));
		config_destroy(&cfg2);
		//return(EXIT_FAILURE);
	}

	struct option {
		const char *text;
		int length;
		SDL_Rect option_rect;
	};

	config_setting_t *optionssetting;
	optionssetting = config_lookup(&cfg2, "options");
	int count = config_setting_length(optionssetting);
	struct option options[count];

	int cursorpos = 0;

	// Init Font ...

	if ( TTF_Init( ) == -1 ) {

		fprintf(stderr, "Init TTF  failed\n");
		//return -1;
	} 
	TTF_Font *dejavu = TTF_OpenFont("DejaVuSans.ttf",300);
	SDL_Color fg;
	fg.r = 150;
	fg.g = 250;
	fg.b = 250;


	SDL_Surface *textsurfaces[count];
	SDL_Texture *texttextures[count];

	for (int index = 0; index < count; index++) {
		const char *element = config_setting_get_string_elem(optionssetting, index);
		options[index].text = element;
		options[index].length = strlen(element);
		options[index].option_rect.x = 500;
		options[index].option_rect.y = 300 + 80 * index;
		options[index].option_rect.w = 30 * options[index].length;
		options[index].option_rect.h= 50;

		textsurfaces[index] = TTF_RenderText_Solid(dejavu, options[index].text, fg);
		if (textsurfaces[index] == NULL) {
			printf("darn\n");
		}

		texttextures[index] = SDL_CreateTextureFromSurface(renderer, textsurfaces[index]);
	}

	struct option cursor;
	cursor.text = ">";
	cursor.length = strlen(cursor.text);
	cursor.option_rect.x = options[cursorpos].option_rect.x - 30;
	cursor.option_rect.y = options[cursorpos].option_rect.y;
	cursor.option_rect.w = 30;
	cursor.option_rect.h = 50;

	SDL_Surface *cursorsurf = TTF_RenderText_Solid(dejavu, cursor.text, fg);
	if (cursorsurf == NULL) {
		printf("darn\n");
	}
	SDL_Texture *cursortex = SDL_CreateTextureFromSurface(renderer, cursorsurf);

	SDL_Rect textbox_rect;
	textbox_rect.x = cursor.option_rect.x - 30;
	textbox_rect.y = options[0].option_rect.y - 30;
	textbox_rect.w = 800;
	textbox_rect.h = 400;

	SDL_Texture *texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, status->graphics->width, status->graphics->height);

	while (1) {

		SDL_SetRenderTarget(renderer, texTarget);
		/* Handle Events */

		SDL_Event e;
		while ( SDL_PollEvent(&e) ) {

			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				unpause_time(status->timing);
				return R_LOOP_LEVEL;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP) {
				cursorpos--;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN) {
				cursorpos++;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
				if (cursorpos == 0) {
					resume = 1;
				}
				else if (cursorpos == 1) {
					restart = 1;
				}
				else if (cursorpos == 2) {
					quittostart = 1;
				}
				else if (cursorpos == 3) {
					quittodesktop = 1;
				}
			}
		}

		if (cursorpos < 0) {
				cursorpos = count - 1;
		}
		
		cursorpos %= count;
		
		if (resume) {
			unpause_time(status->timing);
			return R_LOOP_LEVEL;
		}
		else if (restart) {
			unpause_time(status->timing);
			return R_RESTART_LEVEL;
		}
		else if (quittostart) {
			TTF_CloseFont(dejavu);
			SDL_RenderClear(renderer);
			for (int i = 0; i < count; i++) {
				SDL_DestroyTexture(texttextures[i]);
			}
			unpause_time(status->timing);
			return R_STARTSCREEN;
		}
		else if (quittodesktop) {
			unpause_time(status->timing);
			return R_QUIT_TO_DESKTOP;
		}

		cursor.option_rect.y = options[cursorpos].option_rect.y;

		SDL_RenderCopy(renderer, levelcapture, NULL, NULL);
		for (int index = 0; index < count; index++) {
			SDL_RenderCopy(renderer, texttextures[index], NULL, &options[index].option_rect);
		}
		SDL_RenderCopy(renderer, cursortex, NULL, &cursor.option_rect);

		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopy(renderer, texTarget, NULL, NULL);

		wait_to_present(status->timing);
		//pthread_cond_wait(&display_cond, &display_mutex);
		SDL_RenderPresent(renderer);
		//pthread_mutex_unlock(&display_mutex);

	}

}

