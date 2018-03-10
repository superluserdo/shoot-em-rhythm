#include <stdio.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <semaphore.h>
#include "structdef.h"
#include "clock.h"
#include "main.h"


pthread_mutex_t clock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t display_cond = PTHREAD_COND_INITIALIZER;

extern int local_grid_x;
extern int local_grid_y;

void *frametimer(void *timing_void) {

	extern pthread_mutex_t quit_mutex;
	extern int quitgame;
	int quitnow;

	struct time_struct *timing = (struct time_struct *)timing_void;

	timing->zerotime = SDL_GetTicks();
	timing->intervalanim = 1000.0/timing->fpsanim;
	timing->intervalglobal = 1000.0/(float)timing->fpsglobal;

	int ticktime;
	float actualfps;

	while(1) {

		SDL_Delay(timing->intervalglobal);
	        pthread_mutex_lock( &clock_mutex );
		timing->ticks = SDL_GetTicks();
		ticktime = timing->ticks - timing->zerotime;

		//if (fpstimercount >= 10) {
		//	fpsframecountlast = fpsframecount;
		//	fpsframecount = timing->framecount;
		//	fpstimerlast = fpstimer;
		//	fpstimer = ticktime;
		//	actualfps = (fpsframecount - fpsframecountlast)/((float)(fpstimer - fpstimerlast)/1000);
		//	printf("%6f\n", actualfps);
		//}

		if (timing->pause_change) {
			timing->pause_change = 0;
			if (timing->pauselevel) {
				if (!(*timing->pauselevel)) {
					timing->pausetime_completed += (timing->endpause - timing->startpause);
					timing->pausetime_ongoing = 0;
					timing->pausetime = timing->pausetime_completed;
					*timing->pauselevel = 0;
				}
				else {
					*timing->pauselevel = 1;
				}
			}
			else {
				printf("Er, timing->pauselevel is still a null pointer. That shouldn't be right.\n");
			}
		}

		if (timing->pauselevel) {
			if (*timing->pauselevel) {
				timing->pausetime_ongoing = timing->ticks - timing->startpause;
				timing->pausetime = timing->pausetime_completed + timing->pausetime_ongoing;
			}
		}
		timing->currentbeat = (float)(ticktime - timing->pausetime)* timing->bps / 1000 + timing->startbeat + 1;
		timing->currentbeat_int = (int) timing->currentbeat;
		pthread_mutex_unlock( &clock_mutex );
		pthread_cond_broadcast(&display_cond);
		timing->framecount++;

		pthread_mutex_lock(&quit_mutex);
		quitnow = quitgame;
		pthread_mutex_unlock(&quit_mutex);
		if (quitnow) {
			return 0;
		}
	}
}

void timing_init(struct time_struct *timing) {
	/* Sets important timing variables like the startup time.
	 * Right now intended to be run once in the whole program.
	 * May change this to once per level if needed. */

	timing->zerotime = SDL_GetTicks();
	/* Set fps to 60 if not already specified */
	if (!timing->fpsanim) {
		timing->fpsanim = 60;
	}
	if (!timing->fpsglobal) {
		timing->fpsglobal = 60;
	}
	timing->intervalanim = 1000.0/timing->fpsanim;
	timing->intervalglobal = 1000.0/(float)timing->fpsglobal;
}

int wait_to_present(struct time_struct *timing) {
	timing->ticks = SDL_GetTicks();
	int ticks_since_last = timing->ticks - timing->ticks_last_frame;
	int ticks_to_wait = timing->intervalglobal - ticks_since_last;

	if (ticks_since_last < 0 || ticks_to_wait < 0) {
		ticks_to_wait = 0;
	}
	return ticks_to_wait;
}

void update_time(struct time_struct *timing) {

	timing->ticks = SDL_GetTicks();
	timing->ticks_last_frame = timing->ticks;
	int ticktime = timing->ticks - timing->zerotime;
	timing->currentbeat = (float)(ticktime - timing->pausetime)* timing->bps / 1000 + timing->startbeat + 1;
	timing->currentbeat_int = (int) timing->currentbeat;
	timing->framecount++;
	if (timing->pause_change) {
		timing->pause_change = 0;
		if (timing->pauselevel) {
			if (!(*timing->pauselevel)) {
				timing->pausetime_completed += (timing->endpause - timing->startpause);
				timing->pausetime_ongoing = 0;
				timing->pausetime = timing->pausetime_completed;
				*timing->pauselevel = 0;
			}
			else {
				*timing->pauselevel = 1;
			}
		}
		else {
			printf("Er, timing->pauselevel is still a null pointer. That shouldn't be right.\n");
		}
	}

	if (timing->pauselevel) {
		if (*timing->pauselevel) {
			timing->pausetime_ongoing = timing->ticks - timing->startpause;
			timing->pausetime = timing->pausetime_completed + timing->pausetime_ongoing;
		}
	}
}
