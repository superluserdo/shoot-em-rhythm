#include <stdio.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <semaphore.h>
#include "structdef.h"
#include "clock.h"
#include "main.h"

void timing_init(struct time_struct *timing) {
	/* Sets important timing variables like the startup time.
	 * Right now intended to be run once in the whole program.
	 * May change this to once per level if needed. */

	float fpsanim, fpsglobal;

	/* Set fps to 60 if not already specified */
	if (timing->fpsanim == 0) {
		fpsanim = 60;
	} else {
		fpsanim = timing->fpsanim;
	}
	if (timing->fpsglobal == 0) {
		fpsglobal = 60;
	} else {
		fpsglobal = timing->fpsglobal;
	}

	struct time_struct newtime = {0};
	*timing = newtime;
	timing->fpsanim = fpsanim;
	timing->fpsglobal = fpsglobal;
	timing->zerotime = SDL_GetTicks();
	timing->ticks_last_frame = timing->zerotime;
	timing->ticks = timing->zerotime;
	timing->intervalanim = 1000.0/timing->fpsanim;
	timing->intervalglobal = 1000.0/(float)timing->fpsglobal;
}

void timing_zero(struct time_struct *timing) {
	timing->zerotime = SDL_GetTicks();
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

	timing->ticks_last_frame = timing->ticks;
	timing->ticks = SDL_GetTicks();
	int ticktime = timing->ticks - timing->zerotime;

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

	timing->currentbeat = (float)(ticktime - timing->pausetime)* timing->bps / 1000 + timing->startbeat;
	timing->currentbeat_int = (int) timing->currentbeat + 1;
	timing->framecount++;
}

void pause_time(struct time_struct *timing) {
	timing->startpause = SDL_GetTicks();
	*timing->pauselevel = 1;
	timing->pause_change = 1;
}
void unpause_time(struct time_struct *timing) {
	timing->endpause = SDL_GetTicks();
	*timing->pauselevel = 0;
	timing->pause_change = 1;
}
