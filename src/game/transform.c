#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "structdef.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "transform.h"


void tr_resize(void *rect_trans, void *data) {
	struct tr_resize_data str = *(struct tr_resize_data *)data;
	//SDL_Rect rect = *(SDL_Rect*)rect_trans;
	struct float_rect rect = *(struct float_rect*)rect_trans;
	struct size_ratio_struct local_centre = {
		.w = rect.w + rect.w/2,
		.h = rect.h + rect.h/2
	};
	rect.w *= str.w;
	rect.h *= str.h;
	if (str.centre) {
		local_centre.w += (local_centre.w - str.centre->w) * (str.w - 1);
		local_centre.h += (local_centre.h - str.centre->h) * (str.h - 1);
	}
		rect.w  = local_centre.w - rect.w/2;
		rect.h  = local_centre.h - rect.h/2;
	//*(SDL_Rect *)rect_trans = rect;
	*(struct float_rect *)rect_trans = rect;
}
void tr_bump(void *rect_trans, void *data) {
	/*	Resizes object from centre in periodic triangle-shaped "bump" pattern	*/
	struct tr_bump_data str = *(struct tr_bump_data *)data;
	str.peak_offset = str.peak_offset - (int)str.peak_offset;
	if (str.bump_width > 1.0) {
		str.bump_width = 1.0;
	}

	struct float_rect rect = *(struct float_rect*)rect_trans;
	float dec = *str.currentbeat - (int)*str.currentbeat;
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
		if (str.centre)
			printf("%f, %f\n", resize_data.centre->w, resize_data.centre->h);
		tr_resize(rect_trans, (void *)&resize_data);
	}
}

void tr_sine(void *rect_trans, void *data) {
	struct tr_sine_data str = *(struct tr_sine_data *)data;
	float dec = *str.currentbeat - (int)*str.currentbeat;
	float sin_mult = sin(2*PI*str.freq_perbeat*(dec+str.offset));
	tr_constmult((struct float_rect *)rect_trans, str.rect_bitmask, str.ampl, sin_mult);
}

void tr_sine_sq(void *rect_trans, void *data) {
	struct tr_sine_data str = *(struct tr_sine_data *)data;
	float dec = *str.currentbeat - (int)*str.currentbeat;
	float sin_mult = sin(2*PI*str.freq_perbeat*(dec+str.offset));
	sin_mult *= sin_mult;
	tr_constmult((struct float_rect *)rect_trans, str.rect_bitmask, str.ampl, sin_mult);
}

void tr_sine_abs(void *rect_trans, void *data) {
	struct tr_sine_data str = *(struct tr_sine_data *)data;
	float dec = *str.currentbeat - (int)*str.currentbeat;
	float sin_mult = sin(2*PI*str.freq_perbeat*(dec+str.offset));
	sin_mult = -fabs(sin_mult);
	tr_constmult((struct float_rect *)rect_trans, str.rect_bitmask, str.ampl, sin_mult);
}

void tr_constmult(struct float_rect *rect, int rect_bitmask, float ampl, float mult) {
	if (rect_bitmask & 1) {
		rect->x += ampl*mult;
	}
	if (rect_bitmask & 2) {
		rect->y += ampl*mult;
	}
	if (rect_bitmask & 4) {
		rect->w += ampl*mult;
	}
	if (rect_bitmask & 8) {
		rect->h += ampl*mult;
	}
}

void tr_blink(void *rect_trans, void *data) {
	//SDL_Rect rect = *(SDL_Rect*)rect_trans;
	struct float_rect rect = *(struct float_rect*)rect_trans;
	struct tr_blink_data str = *(struct tr_blink_data *)data;
	int on = str.frames_on;	
	int off = str.frames_off;	

	if (*str.framecount%(on + off) >= on) {
		rect.w = 0;
		rect.h = 0;
	}
	else {
	}
	//*(SDL_Rect *)rect_trans = rect;
	*(struct float_rect *)rect_trans = rect;
}

void tr_orbit_xyz(void *rect_trans, void *data) {
	//SDL_Rect *rect = (SDL_Rect*)rect_trans;
	struct float_rect *rect = (struct float_rect*)rect_trans;
	
	struct tr_orbit_xyz_data str = *(struct tr_orbit_xyz_data *) data;
	float beat = *str.currentbeat;
	rect->x += str.x.ampl*sin(2*PI*(str.x.freq*beat + str.x.phase));
	rect->y += str.y.ampl*sin(2*PI*(str.y.freq*beat + str.y.phase));

	//SDL_Rect pre_scaling = *rect;
	struct float_rect pre_scaling = *rect;
	rect->w *= 1 + str.z.ampl*sin(2*PI*(str.z.freq*beat) + str.z.phase);
	rect->h *= 1 + str.z.ampl*sin(2*PI*(str.z.freq*beat) + str.z.phase);

	///* Enlarge about centre so move back to correct position: */
	//rect->x -= (rect->w - pre_scaling.w)/2;
	//rect->y -= (rect->h - pre_scaling.h)/2;
	*str.z_set = str.z_eqm + str.z_layer_ampl*sin(2*PI*(str.z.freq*beat) + str.z.phase);

	//*str.z_set = ((int)beat%4)/2 ? 1 : -1;
	//*str.z_set = 1;
}

int transform_add_check(struct animate_specific *animation, void *data, void (*func)()) {
	struct func_node *node = animation->transform_list;
	int do_add = 1;
	while (node) {
		if (node->func == func) {
			do_add = 0;
			break;
		}
		node = node->next;
	}
	if (do_add) {
		node = malloc(sizeof(struct func_node));
		node->data = data;
		node->func = func;
		node->next = animation->transform_list;
		animation->transform_list = node;
	}
	return 0;
}
	

int transform_rm(struct animate_specific *animation, void (*func)()) {
	struct func_node *node = animation->transform_list;
	struct func_node *prev = NULL;
	while (node) {
		
		if (node->func == func) {
			if (prev) {
				prev->next = node->next;
			}
			else {
				animation->transform_list = node->next;
			}
			free(node->data);
			free(node);
			break;
		}
		prev = node;
		node = node->next;
	}

	return 0;
}

