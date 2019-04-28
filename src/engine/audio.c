#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "structdef.h"
#include "audio.h"
#include "dict.h"

struct sound_val_struct {
	/* For use as a value to a sound file-path key */
	Mix_Chunk *sound;
	float play_vol;
};

int audio_init(struct audio_struct *audio) {

	/*	Music	*/

	strcpy(audio->track, "");
	audio->trackmaxlen = 50;
	
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
	    printf("Mix_OpenAudio: %s\n", Mix_GetError());

		/* Print music decoders */
		printf("There are %d music decoders available\n", Mix_GetNumMusicDecoders());

		int i,max=Mix_GetNumMusicDecoders();
		for(i=0; i<max; ++i)
			printf("Music decoder %d is for %s\n", i, Mix_GetMusicDecoder(i));
	    return R_FAILURE;
	}

	/*	Sounds	*/

	audio->max_soundchannels = 16;
	Mix_AllocateChannels(audio->max_soundchannels);
	audio->sound_dict = malloc(sizeof(struct dict_void));
	*audio->sound_dict = (struct dict_void) {0};
	

}

void queue_sound(struct audio_struct *audio, char *path, float vol) {
	/* Check if sound is loaded and in the dict */
	struct sound_val_struct *sound_val = dict_void_get_val(audio->sound_dict, path);

	/* If not, try to load it */
	if (sound_val) {
		sound_val->play_vol += vol;
	} else {
		sound_val = malloc(sizeof(*sound_val));
		*sound_val = (struct sound_val_struct) {
			.sound=Mix_LoadWAV(path),
			.play_vol = vol,
		};
		if(!sound_val->sound) {
			printf("Mix_LoadWAV: %s\n", Mix_GetError());
			FILEINFO
		}
		dict_void_add_keyval(audio->sound_dict, path, sound_val);
	}
}

void play_sounds(struct audio_struct *audio) {
	int channels_playing[audio->max_soundchannels];
	for (int i = 0; i < audio->max_soundchannels; i++) {
		channels_playing[i] = -1;//Mix_Playing(i);
	}

	for (int i = 0; i < audio->sound_dict->num_entries; i++) {

		struct sound_val_struct *sound_val = audio->sound_dict->entries[i].val;

		/* Get volume */
		float play_vol = sound_val->play_vol;
		if (play_vol != 0) {
			play_vol = play_vol > 1 ? 1: play_vol;
			sound_val->play_vol = 0;

		if ((!sound_val->sound) || (play_vol == 0)) {
			continue;
		}

			/* Find a channel */
			int channel_found = 0;
			for (int j = 0; j < audio->max_soundchannels; j++) {
				if (channels_playing[j] == -1) {
					channels_playing[j] = Mix_Playing(j);
				}

				if (channels_playing[j] == 0) {
					if(Mix_PlayChannel(j, sound_val->sound, 0) == -1) {
						printf("Mix_PlayChannel: %s\n",Mix_GetError());
					}
					channel_found = 1;
					break;
				}
			}
			if (!channel_found) {
				Mix_HaltChannel(audio->max_soundchannels - 1);
				/* Kick sound off furthest-right channel */
				if(Mix_PlayChannel(audio->max_soundchannels - 1, sound_val->sound, 1) == 0) {
					printf("Mix_PlayChannel: %s\n",Mix_GetError());
				}
			}
		}
	}
}


//queue sound:
//	sound_dict[soundfile_string].vol_mult++
//	if not soundfile_string in sound_dict
//		try to load sound
//play sound:
//	for sound in sound_dict
//		get free sound channel
//		play sound (vol=vol_mult)
//		loop?
#if 0
void *playsound(void* soundpath){

	int audio_rate = 44100;
	int audio_channels = 2;
	int audio_buffers = 2;
	Uint16 audio_format = MIX_DEFAULT_FORMAT;
		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
		printf("Opened audio at %d Hz %d bit %s (%s), %d bytes audio buffer\n", audio_rate,
		(audio_format&0xFF),
		(audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo" : "mono",
		(audio_format&0x1000) ? "BE" : "LE",
		audio_buffers );
	//}
	
	endsound = 0;

	int loopinglist[MAX_SOUNDS_LIST];
	
	for ( int i = 0; i < MAX_SOUNDS_LIST; i++ )
		loopinglist[i] = -1;
	
	// get and print the audio format in use
	int numtimesopened, frequency, channels;
	Uint16 format;
	numtimesopened=Mix_QuerySpec(&frequency, &format, &channels);
	if(!numtimesopened) {
	    printf("Mix_QuerySpec: %s\n",Mix_GetError());
	}
	else {
	    char *format_str="Unknown";
	    switch(format) {
	        case AUDIO_U8: format_str="U8"; break;
	        case AUDIO_S8: format_str="S8"; break;
	        case AUDIO_U16LSB: format_str="U16LSB"; break;
	        case AUDIO_S16LSB: format_str="S16LSB"; break;
	        case AUDIO_U16MSB: format_str="U16MSB"; break;
	        case AUDIO_S16MSB: format_str="S16MSB"; break;
	    }
	}
	
	
	Mix_Chunk *samplelist[MAX_SOUNDS_LIST];
	
	// load sample.wav in to sample

	samplelist[1]=Mix_LoadWAV("../sounds/pew.wav");
	if(!samplelist[1]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	
	samplelist[2]=Mix_LoadWAV("../sounds/pew2.wav");
	if(!samplelist[2]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	
	samplelist[3]=Mix_LoadWAV("../sounds/pew3.wav");
	if(!samplelist[3]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	
	samplelist[4]=Mix_LoadWAV("../sounds/hurtsound.wav");
	if(!samplelist[4]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	samplelist[5]=Mix_LoadWAV("../sounds/diesound.wav");
	if(!samplelist[5]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	
	samplelist[6]=Mix_LoadWAV("../sounds/itemsound.wav");
	if(!samplelist[6]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	
	samplelist[7]=Mix_LoadWAV("../sounds/killsound.wav");
	if(!samplelist[7]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}

	samplelist[8]=Mix_LoadWAV("../sounds/whoosh.wav");
	if(!samplelist[7]) {
	    printf("Mix_LoadWAV: %s\n", Mix_GetError());
	    // handle error
	}
	
	// allocate 16 mixing channels
	Mix_AllocateChannels(16);
	int soundchecklisttemp[MAX_SOUNDS_LIST];	

	while(1) {
		pthread_mutex_lock(&display_mutex);
		pthread_cond_wait(&display_cond, &display_mutex);
		pthread_mutex_unlock(&display_mutex);

		pthread_mutex_lock( &soundstatus_mutex);
		for (int i = 0; i < MAX_SOUNDS_LIST; i++ ) {
			soundchecklisttemp[i] = audio.soundchecklist[i];
		}

		soundstatus = 1;
		pthread_mutex_unlock( &soundstatus_mutex);
		pthread_cond_broadcast( &soundstatus_cond );

		for (int i = 0; i < MAX_SOUNDS_LIST; i++ ) {
			if ( soundchecklisttemp[i] == 1 ) {
				// play sample on first free unreserved channel
				// play it exactly once through
				// Mix_Chunk *sample; //previously loaded
				if(Mix_PlayChannel(-1, samplelist[i], 0)==-1) {
				    printf("Mix_PlayChannel: %s\n",Mix_GetError());
				    // may be critical error, or maybe just no channels were free.
				    // you could allocated another channel in that case...
				}
			}
			
			if ( loopinglist[i] == -1 ) {
				if ( soundchecklisttemp[i] == 2 ) {
					for ( int j = 0; j < MAX_SOUNDS_LIST; j++ ) {
						if ( Mix_Playing(j) == 0 ) {
							// play sample on first free unreserved channel
							// play it exactly once through
							// Mix_Chunk *sample; //previously loaded
							if(Mix_PlayChannel(j, samplelist[i], -1)==-1) {
								printf("Mix_PlayChannel: %s\n",Mix_GetError());
							    // may be critical error, or maybe just no channels were free.
							    // you could allocated another channel in that case...
							}
							loopinglist[i] = j;
							break;
						}
					}
	
				}
			}

			else if ( loopinglist[i] != -1 ) {
				if ( soundchecklisttemp[i] != 2 ) {

					Mix_HaltChannel(loopinglist[i]);
					loopinglist[i] = -1;

				}

			}
		}
	Mix_Volume( -1, MIX_MAX_VOLUME/2 );

	if ( endsound ){
		for ( int i = 0; i < MAX_SOUNDS_LIST; i++ )
			loopinglist[i] = -1;
	}
//		break;
	}	
	
	pthread_cond_broadcast( &soundstatus_cond );

	Mix_CloseAudio();
	
//	Mix_Quit();
	
}
#endif

void pausemusic_toggle(struct audio_struct *audio, struct time_struct *timing) {
	if (!audio->music_paused) {
		Mix_PauseMusic();
		//Mix_SetMusicPosition((float)(timing->startpause - timing->pausetime) / 1000);
		audio->music_paused = 1;
	} else {
		Mix_ResumeMusic();
		audio->music_paused = 0;
	}
}

void stopmusic(struct audio_struct *audio) {
	Mix_HaltMusic();
	audio->music_paused = 0;
}

void playmusic(struct status_struct *status, const char *track, float volume){

	int starting = 0;
	int ending = 0;
	struct audio_struct *audio = status->audio;
	Mix_VolumeMusic(volume * 128);
	audio->music_volume = volume;
	if (strcmp(track, audio->track) != 0) {
		strncpy(audio->track, track, audio->trackmaxlen+1);
		printf("Track: %s\n", track);

		starting = 1;
		ending = 0;
	}

	struct time_struct timing = *status->timing;

	//load the MP3 file "music.mp3" to play as music
	Mix_Music *music=Mix_LoadMUS(audio->track);
	if(!music) {
	    printf("Mix_LoadMUS(\"music.mp3\"): %s\n", Mix_GetError());
	    // this might be a critical error...
	}

	// use musicFinished for when music stops
	//Mix_HookMusicFinished(music_Finished);
	int fade = 0;

	//if(Mix_PlayMusic(music, 1)==-1) {
	if(Mix_FadeInMusicPos(music, 1, 0, 0)==-1) {
		printf("Mix_FadeInMusic: %s\n", Mix_GetError());
	}
	//if (fade <= 0) {
	//	if(Mix_FadeInMusicPos(music, 1, 0, 0)==-1) {
	//	    printf("Mix_FadeInMusic: %s\n", Mix_GetError());
	//	}
	//} else {
	//	if(Mix_FadeInMusicPos(music, 1, 1000, 0)==-1) {
	//	    printf("Mix_FadeInMusic: %s\n", Mix_GetError());
	//	}
	//}

	int muted = 0;


	//if (ending) {
	//	if (fade <= 0) {
	//		Mix_HaltMusic();
	//	}
	//	else {
	//
	//			// fade out music to finish 3 seconds from now
	//				while(Mix_FadeOutMusic(1000) && Mix_PlayingMusic()) {
	//				}
	//	}
	//}
	//else if (audio->music_paused) {
	//Mix_SetMusicPosition((float)(timing.startpause - timing.pausetime) / 1000);
	//	while (1) {
	//		if (ending) {
	//			Mix_HaltMusic();
	//			break;
	//		}
	//		else if (!audio->music_paused) {
	//			Mix_ResumeMusic();
	//			break;
	//		}
	//	}
	//}
	//
	//else if (audio.music_mute != muted) {
	//	muted = audio.music_mute;
	//	if (audio.music_mute) {
	//		Mix_VolumeMusic(0);
	//	}
	//	else {
	//		Mix_VolumeMusic(audio.music_volume * 128);
	//	}
	//}
	//audio.noise = 0;
	//music_ended = 1;

	//Mix_Quit();
}

