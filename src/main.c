#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "main.h"
#include "music.h"
#include <libconfig.h>

extern struct time_struct timing;
struct program_struct program = {
	.width = NATIVE_RES_X * ZOOM_MULT,
	.height = NATIVE_RES_Y * ZOOM_MULT
};
int soundchecklist[MAX_SOUNDS_LIST] = {0};

SDL_Renderer *renderer;
SDL_Window *win;

int main() {

	pthread_t framethread;
	int rc = pthread_create(&framethread, NULL, frametimer, NULL);
	if (rc) {
		printf("ya dun goofed. return code is %d\n.", rc);
		exit(-1);
	}


	SDL_Window *win = NULL;
        SDL_Renderer *renderer = NULL;
	// Initialize SDL.
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
                        return 1;

	// start SDL with audio support
	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
	    printf("SDL_Init: %s\n", SDL_GetError());
	    exit(1);
	}
        pthread_t soundeffects;
        rc = pthread_create(&soundeffects, NULL, playsound, (void*)NULL);
        if (rc) {
                printf("ya dun goofed. return code is %d\n.", rc);
                exit(-1);
        }


        // create the window and renderer
        // note that the renderer is accelerated
	//printf("main says %d\n", width);
        win = SDL_CreateWindow("TOM'S SUPER COOL GAME", 100, 100, program.width, program.height, 0);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	int returncode = startscreen(win, renderer);

	while (1) {

		if ( returncode == 0 ) {
			returncode = levelfunc(win, renderer);
		}
		else if ( returncode > 0 && returncode < 100 ) {
			break;
		}
		else if ( returncode == 101 ) {
			returncode = startscreen(win, renderer);
		}
		else if ( returncode == 102 ) {
			break;
		}
		else if ( returncode >= 200 ) {
			returncode = levelfunc(win, renderer);
		}
		else {
			break;
		}

	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	printf("Game Over.\n");
	printf("%d\n", timing.framecount);
	return 0;
}
