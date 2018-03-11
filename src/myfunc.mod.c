#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "structdef.h"
void mainfunc();

void myfunc(struct status_struct *status) {
	status->timing->bps *= 1.01;

//	/* Call function in main.c */
//	mainfunc();
}
