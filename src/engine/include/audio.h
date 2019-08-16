#ifndef _ENGINE_AUDIO_H
#define _ENGINE_AUDIO_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "structdef_game.h"
#include "dict.h"

void audio_func(struct audio_struct *audio);
void audio_mute_toggle(struct audio_struct *audio);
int audio_init(struct audio_struct *audio);
void playmusic(struct status_struct *status, const char *track, float volume);
void pausemusic_toggle(struct audio_struct *audio, struct time_struct *timing);
void stopmusic(struct audio_struct *audio);
void queue_sound(struct audio_struct *audio, char *path, float vol);
void play_sounds(struct audio_struct *audio);
#endif
