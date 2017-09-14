#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "main.h"
#include "music.h"
#define START_PATH0 "../art/startimgbg.png"
#define START_PATH1 "../art/startimgtext1.png"
#define START_PATH2 "../art/startimgtext2.png"

extern pthread_mutex_t track_mutex;
extern pthread_mutex_t display_mutex;
extern pthread_cond_t display_cond;
extern pthread_cond_t cond_end;
extern struct audio_struct audio;
extern struct program_struct program;

int startscreen(SDL_Window *win, SDL_Renderer *renderer) {

	pthread_t soundthread;
	audio.newtrack = 1;
	struct musicstart_struct {
		int newtrack;
		int *pause;
	};
	int dummypause = 0;
	struct musicstart_struct musicstart_struct = {
		.newtrack = audio.newtrack,
		.pause = &dummypause
	};
	int rc = pthread_create(&soundthread, NULL, musicstart, (void*)&musicstart_struct);
	if (rc) {
	printf("ya dun goofed. return code is %d\n.", rc);
	exit(-1);
	}


        pthread_mutex_lock( &track_mutex );
        audio.track = 1;
        pthread_mutex_unlock( &track_mutex );

	SDL_Texture * startimgbg = NULL;
	SDL_Texture * startimgtext1 = NULL;
	SDL_Texture * startimgtext2 = NULL;
	startimgbg = IMG_LoadTexture(renderer, START_PATH0);
	startimgtext1 = IMG_LoadTexture(renderer, START_PATH1);
	startimgtext2 = IMG_LoadTexture(renderer, START_PATH2);
	int w, h;
	SDL_QueryTexture(startimgbg, NULL, NULL, &w, &h);
	SDL_QueryTexture(startimgtext1, NULL, NULL, &w, &h);
	SDL_QueryTexture(startimgtext2, NULL, NULL, &w, &h);

	SDL_Rect rcStartbg, rcStartbgSrc;
	rcStartbg.x = 0;
	rcStartbg.y = 0;
	rcStartbg.w = 640 * ZOOM_MULT;
	rcStartbg.h = 360 * ZOOM_MULT;
	rcStartbgSrc.x = 0;
	rcStartbgSrc.y = 0;
	rcStartbgSrc.w = 640;
	rcStartbgSrc.h = 360;

	SDL_Rect rcStartt1, rcStartt1Src;
	rcStartt1.x = 0.1 * program.width;
	rcStartt1.y = 0;
	rcStartt1.w = 444 * ZOOM_MULT;
	rcStartt1.h = 58 * ZOOM_MULT;
	rcStartt1Src.x = 0;
	rcStartt1Src.y = 0;
	rcStartt1Src.w = 444;
	rcStartt1Src.h = 58;

	SDL_Rect rcStartt2, rcStartt2Src;
	rcStartt2.x = 0.3 * program.width;
	rcStartt2.y = 0.6 * program.height;
	rcStartt2.w = 356 * ZOOM_MULT;
	rcStartt2.h = 27 * ZOOM_MULT;
	rcStartt2Src.x = 0;
	rcStartt2Src.y = 0;
	rcStartt2Src.w = 356;
	rcStartt2Src.h = 27;

	int counter;
	while (1) {
		counter++;
		SDL_Event e;
		while ( SDL_PollEvent(&e) ) {
			
			if (e.type == SDL_QUIT) {
				quitstart(startimgbg, startimgtext1, startimgtext2, renderer);
				return 1;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
				quitstart(startimgbg, startimgtext1, startimgtext2, renderer);
				return 0;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				quitstart(startimgbg, startimgtext1, startimgtext2, renderer);
				return 1;
			}
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, startimgbg, &rcStartbgSrc,  &rcStartbg);
		SDL_RenderCopy(renderer, startimgtext1, &rcStartt1Src,  &rcStartt1);
		SDL_RenderCopy(renderer, startimgtext2, &rcStartt2Src,  &rcStartt2);
		pthread_cond_wait(&display_cond, &display_mutex);
		SDL_RenderPresent(renderer);
		pthread_mutex_unlock(&display_mutex);
	}

return 0;
}

void quitstart(SDL_Texture *startimgbg, SDL_Texture *startimgtext1, SDL_Texture *startimgtext2, SDL_Renderer *renderer) {
	SDL_RenderClear(renderer);
	musicstop();

	pthread_cond_wait( &cond_end, &track_mutex);
	pthread_mutex_unlock( &track_mutex );
	SDL_DestroyTexture(startimgbg);
	SDL_DestroyTexture(startimgtext1);
	SDL_DestroyTexture(startimgtext2);
}
