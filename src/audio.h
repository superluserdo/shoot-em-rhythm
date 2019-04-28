int audio_init(struct audio_struct *audio);
void playmusic(struct status_struct *status, const char *track, float volume);
void pausemusic_toggle(struct audio_struct *audio, struct time_struct *timing);
void stopmusic(struct audio_struct *audio);
void queue_sound(struct audio_struct *audio, char *path, float vol);
void play_sounds(struct audio_struct *audio);
