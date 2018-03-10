struct tr_resize_data {
	struct status_struct *status;
	float w;
	float h;
	struct xy_struct *centre;
};
void tr_resize(void *rect_trans, void *data);

struct tr_bump_data {
	struct status_struct *status;
	float freq_perbeat;
	float ampl;
	float peak_offset;
	float bump_width;
	struct xy_struct *centre;
};
void tr_bump(void *rect_trans, void *data);

struct tr_sine_data {
	struct status_struct *status;
	int rect_bitmask;
	float freq_perbeat;
	float ampl_pix;
	float offset;
};

void tr_sine(void *rect_trans, void *data);

void tr_sine_sq(void *rect_trans, void *data);

void tr_sine_abs(void *rect_trans, void *data);

struct tr_blink_data {
	struct status_struct *status;
	int frames_on;
	int frames_off;
};

void tr_blink(void *rect_trans, void *data);

void tr_constmult(SDL_Rect *rect, int rect_bitmask, float ampl_pix, float mult);

