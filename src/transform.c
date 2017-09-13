#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "main.h"
#include "level.h"
#include "music.h"
#include "transform.h"
#define PI 3.14159265


void tr_resize(void *rect_trans, void *data) {
	struct tr_resize_data str = *(struct tr_resize_data *)data;
	SDL_Rect rect = *(SDL_Rect*)rect_trans;
	struct xy_struct local_centre = {
		.x = rect.x + rect.w/2,
		.y = rect.y + rect.h/2
	};
	rect.w *= str.w;
	rect.h *= str.h;
	if (str.centre) {
		local_centre.x += (local_centre.x - str.centre->x) * (str.w - 1);
		local_centre.y += (local_centre.y - str.centre->y) * (str.h - 1);
	//	rect.x = str.centre->x + (local_centre.x - str.centre->x)*(str.w) - rect.w/2;
	//	rect.y = str.centre->y + (local_centre.y - str.centre->y)*(str.h) - rect.h/2;
	}
		rect.x  = local_centre.x - rect.w/2;
		rect.y  = local_centre.y - rect.h/2;
	*(SDL_Rect *)rect_trans = rect;
}
void tr_bump(void *rect_trans, void *data) {
	/*	Resizes object from centre in periodic triangle-shaped "bump" pattern	*/
	struct tr_bump_data str = *(struct tr_bump_data *)data;
	str.peak_offset = str.peak_offset - (int)str.peak_offset;
	if (str.bump_width > 1.0) {
		str.bump_width = 1.0;
	}

	SDL_Rect rect = *(SDL_Rect*)rect_trans;
	float dec = timing.currentbeat - (int)timing.currentbeat;
	float peak_start = str.peak_offset - str.bump_width/2;
	float peak_end = str.peak_offset + str.bump_width/2;
	if ((dec >= peak_start && dec <= peak_end) || (dec >= peak_start + 1) || (dec <= peak_end - 1)) {
		float diff = str.peak_offset - dec;
		if (diff < 0) {
			diff *= -1;//++;
		}
		if (diff > str.bump_width / 2) {
			diff = 1 - diff;
		}
		struct tr_resize_data resize_data = {
			.w = 1 + (str.ampl -1) * (1 - 2*diff/str.bump_width),
			.h = resize_data.w,
			.centre = str.centre
		};
		tr_resize(rect_trans, (void *)&resize_data);
	}
}

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

void tr_blink(void *rect_trans, void *data) {
	SDL_Rect rect = *(SDL_Rect*)rect_trans;
	struct tr_blink_data str = *(struct tr_blink_data *)data;
	int on = str.frames_on;	
	int off = str.frames_off;	

	if (timing.framecount%(on + off) >= on) {
		rect.w = 0;
		rect.h = 0;
	}
	else {
	}
	*(SDL_Rect *)rect_trans = rect;
}
