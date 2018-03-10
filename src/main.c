#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "structdef.h"
#include "newstruct.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include <libconfig.h>

struct program_struct program = {
};
int soundchecklist[MAX_SOUNDS_LIST] = {0};

SDL_Renderer *renderer;
SDL_Window *win;
pthread_mutex_t quit_mutex;
int quitgame = 0;

int main() {


	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	//TODO:	Fix audio segfaults!
	// start SDL with audio support
	//if(SDL_Init(SDL_INIT_AUDIO)==-1) {
	//    printf("SDL_Init: %s\n", SDL_GetError());
	//    exit(1);
	//}
	//if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
	//    printf("Mix_OpenAudio: %s\n", Mix_GetError());
	//    exit(2);
	//}
    //    pthread_t soundeffects;
    //    rc = pthread_create(&soundeffects, NULL, playsound, (void*)NULL);
    //    if (rc) {
    //            printf("ya dun goofed. return code is %d\n.", rc);
    //            exit(-1);
    //    }

	/*	Initialise the basic game state struct	*/

	struct level_struct level = {0};
	struct player_struct player = {0};
	struct audio_struct audio = {0};
	struct time_struct timing = {0};
	struct graphics_struct graphics = {0};
	
	struct status_struct status = {
		.level = &level,
		.player = &player,
		.audio = &audio,
		.timing = &timing,
		.graphics = &graphics
	};
	timing.fpsanim = 30;
	timing.fpsglobal = 60;
	timing_init(&timing);
	graphics.width = NATIVE_RES_X * ZOOM_MULT;
	graphics.height = NATIVE_RES_Y * ZOOM_MULT;

	/* NOTE: I've moved timing out of its own thread into the main thread.
	 * frametimer() is no longer used */
	//pthread_t framethread;
	//int rc = pthread_create(&framethread, NULL, frametimer, (void *)&timing);
	//if (rc) {
	//	printf("ya dun goofed. return code is %d\n.", rc);
	//	exit(-1);
	//}

	win = SDL_CreateWindow("TOM'S SUPER COOL GAME", 100, 100, graphics.width, graphics.height, 0);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("SDL_CreateRenderer failed. Error message: '%s\n'", SDL_GetError());
		
		/* Print driver info if renderer creation fails */
		SDL_RendererInfo info;
		int num = SDL_GetNumRenderDrivers();
		printf("There are %d usable render drivers\n", num);
		printf("Driver  SDL_RendererFlags\n");
		for (int i = 0; i < num; i++) {
	   	if (SDL_GetRenderDriverInfo(i,&info) == 0)
	   	printf("%s  %d\n", info.name, info.flags&2);
		}
	}
	graphics.renderer = renderer;

	enum  return_codes_e returncode = startscreen(win, renderer, &status);

	/*	The Main Game Loop */
	//	NOTE: The stack smashing error seemed to occur when the loop just kept looping because it wasn'tr intercepting a particular return code!
	while (1) {

		if ( returncode == R_SUCCESS ) {
			returncode = level_init(status);
		}
		else if ( returncode == R_FAILURE ) {
			returncode = level_loop(status);
		}
		else if ( returncode == R_STARTSCREEN ) {
			returncode = startscreen(win, renderer, &status);
		}
		else if ( returncode == R_LOOP_LEVEL) {
			returncode = level_loop(status);
		}
		else if ( returncode == R_RESTART_LEVEL) {
			returncode = level_init(status);
		}
		else if ( returncode == R_QUIT_TO_DESKTOP) {
			break;
		}
		else {
			break;
		}

	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	printf("Game Over.\n");
	printf("%d\n", timing.framecount);
	//pthread_mutex_lock(&quit_mutex);
	//quitgame = 1;
	//pthread_mutex_unlock(&quit_mutex);
	//pthread_join(framethread, NULL);
	return 0;
}
