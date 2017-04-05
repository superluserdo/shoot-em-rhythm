#include <stdio.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <semaphore.h>
//#include "level.h"


pthread_mutex_t clock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t display_cond = PTHREAD_COND_INITIALIZER;

extern int local_grid_x;
extern int local_grid_y;
int framecount;
int countbeats;
float bps;
float startbeat;
float currentbeat;
float intervalanim;
float intervalglobal;
float pxperbeat;
//float beatstoplayer;
Uint32 zerotime;
Uint32 pausetime = 0;

void *frametimer(void *unused) {

	zerotime = SDL_GetTicks();
	Uint32 SDL_GetTicks(void);
	Uint32 ticktime;
	bps = 1.5185*2.0;
	pxperbeat = 140;
	//beatstoplayer = 4.0;
	//currentbeat;
	int fpsanim = 30;
	intervalanim = 1000.0/fpsanim;
	int fpsglobal = 60;
	intervalglobal = 1000.0/(float)fpsglobal;
	framecount = 0;
	extern int pauselevel;
	int startpause = 1;
	Uint32 ticks;
	Uint32 pausetimelast = 0;
	int fpsframecount = 0;
	int fpsframecountlast = 0;
	int fpstimercount = 0;
	int fpstimer;
	int fpstimerlast = 0;
	float actualfps;

	while(1) {

		SDL_Delay(intervalglobal);
	        pthread_mutex_lock( &clock_mutex );
		ticks = SDL_GetTicks();
		ticktime = ticks - zerotime;

		if (fpstimercount >= 10) {
			fpsframecountlast = fpsframecount;
			fpsframecount = framecount;
			fpstimerlast = fpstimer;
			fpstimer = ticktime;
			actualfps = (fpsframecount - fpsframecountlast)/((float)(fpstimer - fpstimerlast)/1000);
			printf("%6f\n", actualfps);
		}


		if (pauselevel) {
			if (startpause) {
				pausetimelast = ticks;
				startpause = 0;
			}
			pausetime += ticks - pausetimelast;
			pausetimelast = ticks;
		}
		else {
			startpause = 1;
		}
		currentbeat = (float)(ticktime - pausetime)* bps / 1000 + startbeat + 1;
	        pthread_mutex_unlock( &clock_mutex );
	pthread_cond_broadcast(&display_cond);
		framecount++;
	}
}
