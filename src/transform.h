struct tr_sine_data {
	int rect_bitmask;
	float freq_perbeat;
	float ampl_pix;
	float offset;
};

void tr_sine(void *data, void *rect_trans);

struct tr_blink_data {
	int frames_on;
	int frames_off;
};

void tr_blink(void *data, void *rect_trans);

