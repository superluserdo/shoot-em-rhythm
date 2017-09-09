#include <stdio.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <semaphore.h>
#include "clock.h"
#include "main.h"


pthread_mutex_t clock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t display_cond = PTHREAD_COND_INITIALIZER;

extern int local_grid_x;
extern int local_grid_y;

struct time_struct timing = {
	.pxperbeat = 0,
	.bps = 0,
	.framecount = 0,
	.pauselevel = NULL,
	.fpsanim = 30,
	.fpsglobal = 60,
	.pausetime = 0
};


void *frametimer(void *unused) {

		timing.zerotime = SDL_GetTicks();
		timing.intervalanim = 1000.0/timing.fpsanim;
		timing.intervalglobal = 1000.0/(float)timing.fpsglobal;

	//int SDL_GetTicks(void);
	int ticktime;
	//beatstoplayer = 4.0;
	int startpause = 1;
	int pausetimelast = 0;
	int fpsframecount = 0;
	int fpsframecountlast = 0;
	int fpstimercount = 0;
	int fpstimer;
	int fpstimerlast = 0;
	float actualfps;

	while(1) {

		SDL_Delay(timing.intervalglobal);
	        pthread_mutex_lock( &clock_mutex );
		timing.ticks = SDL_GetTicks();
		ticktime = timing.ticks - timing.zerotime;

		if (fpstimercount >= 10) {
			fpsframecountlast = fpsframecount;
			fpsframecount = timing.framecount;
			fpstimerlast = fpstimer;
			fpstimer = ticktime;
			actualfps = (fpsframecount - fpsframecountlast)/((float)(fpstimer - fpstimerlast)/1000);
			printf("%6f\n", actualfps);
		}

		if (timing.pause_change) {
			timing.pause_change = 0;
			if (timing.pauselevel) {
				if (!(*timing.pauselevel)) {
					timing.pausetime_completed += (timing.endpause - timing.startpause);
					timing.pausetime_ongoing = 0;
					timing.pausetime = timing.pausetime_completed;
					*timing.pauselevel = 0;
				}
				else {
					*timing.pauselevel = 1;
				}
			}
			else {
				printf("Er, timing.pauselevel is still a null pointer. That shouldn't be right.\n");
			}
		}

		if (timing.pauselevel) {
			if (*timing.pauselevel) {
				printf("Pause!\n");
				timing.pausetime_ongoing = timing.ticks - timing.startpause;
				timing.pausetime = timing.pausetime_completed + timing.pausetime_ongoing;
			}
		}

//		if (timing.pauselevel) {
//			if (*timing.pauselevel) {
//				if (startpause) {
//					pausetimelast = timing.ticks;
//					startpause = 0;
//				}
//				timing.pausetime += timing.ticks - pausetimelast;
//				pausetimelast = timing.ticks;
//			}
//		}
//		else {
//			startpause = 1;
//		}
		timing.currentbeat = (float)(ticktime - timing.pausetime)* timing.bps / 1000 + timing.startbeat + 1;
	        pthread_mutex_unlock( &clock_mutex );
	pthread_cond_broadcast(&display_cond);
		timing.framecount++;
	}
}
