#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "structdef.h"
void mainfunc();

void myfunc(struct status_struct *status) {
	printf("Ya printed something!\n");
	printf("sqrt(9) = %f\n", sqrt(9));
	printf("Variable = %f\n", status->level->speedmult++);

//	/* Call function in main.c */
//	mainfunc();
}
