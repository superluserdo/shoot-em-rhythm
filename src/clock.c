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

struct time_struct time_status = {
	.pxperbeat = 140,
	.bps = 1.5185*2.0,
	.framecount = 0,
	.fpsanim = 30,
	.fpsglobal = 60,
	.pausetime = 0
};


void *frametimer(void *unused) {

		time_status.zerotime = SDL_GetTicks();
		time_status.intervalanim = 1000.0/time_status.fpsanim;
		time_status.intervalglobal = 1000.0/(float)time_status.fpsglobal;

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

		SDL_Delay(time_status.intervalglobal);
	        pthread_mutex_lock( &clock_mutex );
		time_status.ticks = SDL_GetTicks();
		ticktime = time_status.ticks - time_status.zerotime;

		if (fpstimercount >= 10) {
			fpsframecountlast = fpsframecount;
			fpsframecount = time_status.framecount;
			fpstimerlast = fpstimer;
			fpstimer = ticktime;
			actualfps = (fpsframecount - fpsframecountlast)/((float)(fpstimer - fpstimerlast)/1000);
			printf("%6f\n", actualfps);
		}


		if (time_status.pauselevel) {
			if (startpause) {
				pausetimelast = time_status.ticks;
				startpause = 0;
			}
			time_status.pausetime += time_status.ticks - pausetimelast;
			pausetimelast = time_status.ticks;
		}
		else {
			startpause = 1;
		}
		time_status.currentbeat = (float)(ticktime - time_status.pausetime)* time_status.bps / 1000 + time_status.startbeat + 1;
	        pthread_mutex_unlock( &clock_mutex );
	pthread_cond_broadcast(&display_cond);
		time_status.framecount++;
	}
}
