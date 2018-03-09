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

struct time_struct get_timing() {

	struct time_struct timing = {
		.ticks = 0,
		.countbeats = 0,
		.bps = 0,
		.startbeat = 0,
		.currentbeat = 0,
		.currentbeat_int = 0,
		.pxperbeat = 0,
		.framecount = 0,
		.fpsanim = 30,
		.fpsglobal = 60,
		.pauselevel = NULL,
		.pause_change = 0,
		.zerotime = 0,
		.pausetime = 0,
		.pausetime_completed = 0,
		.pausetime_ongoing = 0,
		.startpause = 0,
		.endpause = 0,
		.intervalanim = 0,
		.intervalglobal = 0,
	};

	return timing;
}

void *frametimer(void *timing_void) {

	extern pthread_mutex_t quit_mutex;
	extern int quitgame;
	int quitnow;

	struct time_struct *timing = (struct time_struct *)timing_void;

	timing->zerotime = SDL_GetTicks();
	timing->intervalanim = 1000.0/timing->fpsanim;
	timing->intervalglobal = 1000.0/(float)timing->fpsglobal;

	//int SDL_GetTicks(void);
	int ticktime;
	//beatstoplayer = 4.0;
	int pausetimelast = 0;
	int fpsframecount = 0;
	int fpsframecountlast = 0;
	int fpstimercount = 0;
	int fpstimer;
	int fpstimerlast = 0;
	float actualfps;

	while(1) {

		SDL_Delay(timing->intervalglobal);
	        pthread_mutex_lock( &clock_mutex );
		timing->ticks = SDL_GetTicks();
		ticktime = timing->ticks - timing->zerotime;

		if (fpstimercount >= 10) {
			fpsframecountlast = fpsframecount;
			fpsframecount = timing->framecount;
			fpstimerlast = fpstimer;
			fpstimer = ticktime;
			actualfps = (fpsframecount - fpsframecountlast)/((float)(fpstimer - fpstimerlast)/1000);
			printf("%6f\n", actualfps);
		}

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
