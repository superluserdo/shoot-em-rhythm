struct tr_sine_data {
	int rect_bitmask;
	float freq_perbeat;
	float ampl_pix;
	float offset;
};

void tr_sine(void *data, void *rect_trans);

