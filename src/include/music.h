struct playmusic_struct {
	char *soundpath;
	int *pause;
	struct time_struct *timing;
};

extern pthread_mutex_t display_mutex;
extern pthread_cond_t display_cond;

extern struct level_struct level_status;
extern struct audio_struct audio_status;

void my_audio_callback(void *userdata, Uint8 *stream, int len);

void *musicstart(void* ptr);
void *playsound(void* soundpath);
void *playmusic(void* argvoid);
void musicstop(void);
void music_Finished();
void soundstart(void);
void soundstop(void);

