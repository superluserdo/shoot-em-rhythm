extern pthread_mutex_t display_mutex;
extern pthread_cond_t display_cond;

extern struct level_struct level_status;
extern struct audio_struct audio_status;

void *musicstart(void* ptr);
void *playsound(void* soundpath);
void *playmusic(void* soundpath);
void musicstop(void);
void soundstart(void);
void soundstop(void);

