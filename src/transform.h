struct tr_resize_data {
	float w;
	float h;
};
void tr_resize(void *rect_trans, void *data);

struct tr_bump_data {
	float freq_perbeat;
	float ampl;
	float peak_offset;
	float bump_width;
};
void tr_bump(void *rect_trans, void *data);

struct tr_sine_data {
	int rect_bitmask;
	float freq_perbeat;
	float ampl_pix;
	float offset;
};

void tr_sine(void *rect_trans, void *data);

struct tr_blink_data {
	int frames_on;
	int frames_off;
};

void tr_blink(void *rect_trans, void *data);

