#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "main.h"
#include "level.h"
#include "music.h"
#include "transform.h"
#define PI 3.14159265

void tr_sine(void *rect_trans, void *data) {
	struct tr_sine_data str = *(struct tr_sine_data *)data;
	SDL_Rect rect = *(SDL_Rect*)rect_trans;
	float dec = timing.currentbeat - (int)timing.currentbeat;
	if (str.rect_bitmask & 1) {
		rect.x += (int)str.ampl_pix*sin(2*PI*str.freq_perbeat*(dec+str.offset));
	}
	if (str.rect_bitmask & 2) {
		rect.y += (int)str.ampl_pix*sin(2*PI*str.freq_perbeat*(dec+str.offset));
	}
	if (str.rect_bitmask & 4) {
		rect.w += (int)str.ampl_pix*sin(2*PI*str.freq_perbeat*(dec+str.offset));
	}
	if (str.rect_bitmask & 8) {
		rect.h += (int)str.ampl_pix*sin(2*PI*str.freq_perbeat*(dec+str.offset));
	}
	*(SDL_Rect *)rect_trans = rect;
}
