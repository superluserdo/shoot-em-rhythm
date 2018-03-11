#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "structdef.h"
void mainfunc();

void myfunc(struct status_struct *status) {
	printf("Ya printed something!\n");
	printf("sqrt(9) = %f\n", sqrt(9));
	status->timing->bps *= 1.01;
	printf("bps = %f\n", status->timing->bps);
	printf("currentbeat = %f\n", status->timing->currentbeat);

//	/* Call function in main.c */
//	mainfunc();
}
