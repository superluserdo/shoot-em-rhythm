#ifndef _GAME_TRANSFORM_H
#define _GAME_TRANSFORM_H
struct tr_resize_data {
	float w;
	float h;
	//struct xy_struct *centre;
	struct size_ratio_struct *centre;
};
void tr_resize(void *rect_trans, void *data);

struct tr_bump_data {
	int *score;
	float *currentbeat;
	float freq_perbeat;
	float ampl;
	float peak_offset;
	float bump_width;
	//struct xy_struct *centre;
	struct size_ratio_struct *centre;
};
void tr_bump(void *rect_trans, void *data);

struct cycle_struct {
	float ampl;
	float freq;
	float phase; /* Phase of cycle in radians */
};

struct tr_sine_data {
	float *currentbeat;
	int rect_bitmask;
	//struct cycle_struct cycle;
	float freq_perbeat;
	float ampl;
	float offset;
};

void tr_sine(void *rect_trans, void *data);

void tr_sine_sq(void *rect_trans, void *data);

void tr_sine_abs(void *rect_trans, void *data);

struct tr_blink_data {
	int *framecount;
	int frames_on;
	int frames_off;
};

void tr_blink(void *rect_trans, void *data);

void tr_orbit_xyz(void *animate_timing, void *data);

struct tr_orbit_xyz_data {
	struct cycle_struct x;
	struct cycle_struct y;
	struct cycle_struct z;
	float z_eqm; /* z value at equilibrium */
	float z_layer_ampl; /* Only for layer ordering */
	float *z_set; /* z value of the animation we're setting */
	float *currentbeat;
	//float size_multiplier_close;
	//float size_multiplier_far;
};

void tr_constmult(struct float_rect *rect, int rect_bitmask, float ampl, float mult);

int transform_add_check(struct animation_struct *animation, void *data, void (*func)());
int transform_rm(struct animation_struct *animation, void (*func)());

#endif
