#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include <SDL2/SDL_ttf.h>
#include "main.h"
//#include "level.h"

extern pthread_mutex_t display_mutex;
extern pthread_mutex_t clock_mutex;
extern pthread_cond_t display_cond;

int pausefunc(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel) {

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

	// Init SDL2 ...

	if ( TTF_Init( ) == -1 ) {

		fprintf(stderr, "Init TTF  failed\n");
		//return -1;
	} 
	TTF_Font *dejavu = TTF_OpenFont("DejaVuSans.ttf",300);
	SDL_Color fg;
	fg.r = 100;
	fg.g = 100;
	fg.b = 100;

	SDL_SetRenderTarget(renderer, NULL);

	SDL_Surface *textsurfaces[count];
	SDL_Texture *texttextures[count];

	for (int index = 0; index < count; index++) {
//		printf("%p\n", options[index]);
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

//	printf("%s\n", options[index].text);
//	printf("%d\n", options[index].length);
//	printf("size is %d\n", sizeof(options[index]));
	}
//	printf("size is %d\n", sizeof(options));

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

//	SDL_Texture *textbox = IMG_LoadTexture(renderer, "../art/textbox.png");
	SDL_Rect textbox_rect;
	textbox_rect.x = cursor.option_rect.x - 30;
	textbox_rect.y = options[0].option_rect.y - 30;
	textbox_rect.w = 800;
	textbox_rect.h = 400;
	while (1) {
		/* Handle Events */

		SDL_Event e;
		while ( SDL_PollEvent(&e) ) {

			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				return 0;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP) {
				//if (cursorpos > 0)
					cursorpos--;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN) {
				//if (cursorpos < count-1)
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
				printf("%d\n", cursorpos);
				cursorpos = count - 1;
		}
		
		cursorpos %= count;
		
		if (resume) {
			return 0;
		}
		else if (restart) {
			return 200 + currentlevel;
		}
		else if (quittostart) {
	TTF_CloseFont(dejavu);
	SDL_RenderClear(renderer);
	for (int i = 0; i < count; i++) {
		SDL_DestroyTexture(texttextures[i]);
	}
			return 101;
		}
		else if (quittodesktop) {
			return 102;
		}

		cursor.option_rect.y = options[cursorpos].option_rect.y;

		SDL_RenderCopy(renderer, levelcapture, NULL, NULL);
//		SDL_RenderCopy(renderer, textbox, NULL, &textbox_rect);
		for (int index = 0; index < count; index++) {
			SDL_RenderCopy(renderer, texttextures[index], NULL, &options[index].option_rect);
		}
		SDL_RenderCopy(renderer, cursortex, NULL, &cursor.option_rect);
		pthread_cond_wait(&display_cond, &display_mutex);
		SDL_RenderPresent(renderer);
		pthread_mutex_unlock(&display_mutex);

	}

}

