#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "main.h"
#include "music.h"
#define SAMPLE_RATE 44100
#define BIT_DEPTH 32
#define CHANNELS 2
#define END_COUNT_MAX 8
#define START_COUNT_MAX 8

pthread_cond_t soundstatus_cond;
pthread_mutex_t soundstatus_mutex;
pthread_mutex_t track_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_end   = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_end2   = PTHREAD_COND_INITIALIZER;

struct audio_struct audio_status = {
		
	.track = 0,
	.newtrack = 0,
	.noise = 0
};

struct playmusic_struct {
	char *soundpath;
	int *pause;
};

void *playmusic(void* argvoid);
int offset_time = 87; //time in 100ths of a second

void music_Finished();
Mix_Music *music;
float loop_pos = 1.0;

// prototype for our audio callback
// see the implementation for more information
void my_audio_callback(void *userdata, Uint8 *stream, int len);

// variable declarations
static Uint8 *audio_pos; // global pointer to the audio buffer to be played
static Uint32 audio_len; // remaining length of the sample we have to play
static Uint32 loop_offset;
int vol = -128;
int ending = 0;
int endsound = 0;
int endingcount = END_COUNT_MAX;
int starting = 1;
int startingcount = START_COUNT_MAX;

int fade = 0;

void musicstop(void) {

	if (audio_status.noise) {
		if (!ending)
			ending = 1;
	}

}

void soundstart(void) {

	endsound = 0;

}

void soundstop(void) {

	endsound = 1;

}

void* musicstart(void* argvoid) {
	pthread_t pthread_self(void);
	int x = pthread_self();
	//printf("THREAD ... %d\n", x);
	struct musicstart_struct {
		int newtrack;
		int *pause;
	};
	struct musicstart_struct *arguments = (struct musicstart_struct *)argvoid;
	pthread_t soundthread;
	char *trackarray[5];
	trackarray[0] = "0";	
	trackarray[1] = "../music/Gator.ogg";//nugopen.wav";	
	trackarray[2] = "../music/rally2.ogg";//"../music/Chill.ogg";//nug.wav";	
	trackarray[3] = "../music/fastroot.wav";	
	trackarray[4] = "../music/rubyremix.wav";	
	
	pthread_mutex_lock( &track_mutex );
	int oldtrack = audio_status.track;
	pthread_mutex_unlock( &track_mutex );

	ending = 1;

//	endingcount = END_COUNT_MAX - startingcount;
	pthread_mutex_lock( &track_mutex );
	audio_status.track = arguments->newtrack;
	pthread_mutex_unlock( &track_mutex );


	if ( arguments->newtrack != 0 ) {
		printf("track ... %d\n\n", arguments->newtrack);

		audio_status.noise = 1;
		starting = 1;
		ending = 0;
		endingcount = END_COUNT_MAX;
		char mystr[40];
		strcpy(mystr, trackarray[arguments->newtrack]);
		struct playmusic_struct playmusic_struct = {
			.soundpath = mystr,
			.pause = arguments->pause
		};
		int rc = pthread_create(&soundthread, NULL, playmusic, (void*)&playmusic_struct);
		if (rc) {
			printf("ya dun goofed. return code is %d\n.", rc);
			exit(-1);
		}
	}
}
void *playsound(void* soundpath){

	extern pthread_cond_t soundstatus_cond;
	// start SDL with audio support
	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
	    printf("SDL_Init: %s\n", SDL_GetError());
	    exit(1);
	}
	// open 44.1KHz, signed 16bit, system byte order,
	//      stereo audio, using 1024 byte chunks
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
	    printf("Mix_OpenAudio: %s\n", Mix_GetError());
	    exit(2);
	}
	
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
//	    printf("opened=%d times  frequency=%dHz  format=%s  channels=%d\n", numtimesopened, frequency, format_str, channels);
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
		pthread_cond_wait(&display_cond, &display_mutex);
		pthread_mutex_unlock(&display_mutex);

		pthread_mutex_lock( &soundstatus_mutex);
		for (int i = 0; i < MAX_SOUNDS_LIST; i++ ) {
			soundchecklisttemp[i] = audio_status.soundchecklist[i];
		}
		pthread_cond_broadcast( &soundstatus_cond );
		pthread_mutex_unlock( &soundstatus_mutex);

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
	pthread_mutex_unlock( &soundstatus_mutex);

	Mix_CloseAudio();
	
//	Mix_Quit();
	
}



void my_audio_callback(void *userdata, Uint8 *stream, int len) {
	
	if (audio_len ==0)
		return;
	
	len = ( len > audio_len ? audio_len : len );
	SDL_memcpy (stream, audio_pos, len);
	// simply copy from one buffer into the other
	SDL_MixAudio(stream, audio_pos, len, vol);// mix from one buffer into another
//	printf("volume %d\n", vol);

	//volume was SDL_MIX_MAXVOLUME
	audio_pos += len;
	audio_len -= len;
}


void *playmusic(void* argvoid){

	struct playmusic_struct *arguments = (struct playmusic_struct *)argvoid;
	// open 44.1KHz, signed 16bit, system byte order,
	//      stereo audio, using 1024 byte chunks
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
	    printf("Mix_OpenAudio: %s\n", Mix_GetError());
	    exit(2);
	}

	// print the number of music decoders available
	//printf("There are %d music deocoders available\n", Mix_GetNumMusicDecoders());



	// print music decoders available
	//int i,max=Mix_GetNumMusicDecoders();
	//for(i=0; i<max; ++i)
	//      printf("Music decoder %d is for %s",Mix_GetMusicDecoder(i));

	const char *path = arguments->soundpath;

	//load the MP3 file "music.mp3" to play as music
	music=Mix_LoadMUS(path);
	if(!music) {
	    printf("Mix_LoadMUS(\"music.mp3\"): %s\n", Mix_GetError());
	    // this might be a critical error...
	}



	// use musicFinished for when music stops
	//Mix_HookMusicFinished(music_Finished);

	if (fade <= 0) {
		if(Mix_FadeInMusicPos(music, 1, 0, 0)==-1) {
		    printf("Mix_FadeInMusic: %s\n", Mix_GetError());
		    // well, there's no music, but most games don't break without music...
		}
	}
	else {
		if(Mix_FadeInMusicPos(music, 1, 1000, 0)==-1) {
		    printf("Mix_FadeInMusic: %s\n", Mix_GetError());
		    // well, there's no music, but most games don't break without music...
		}
	}

	// play music forever, fading in over 2 seconds
	// Mix_Music *music; // I assume this has been loaded already

	while(1) {

	        if (ending) {
			if (fade <= 0) {
				Mix_HaltMusic();
			}
			else {

	                // fade out music to finish 3 seconds from now
		                while(Mix_FadeOutMusic(1000) && Mix_PlayingMusic()) {
					// wait for any fades to complete
					SDL_Delay(100);
		                }
			}
		        break;
	        }
		else if (*arguments->pause) {
			Mix_PauseMusic();
			while (1) {
				if (ending) {
					Mix_HaltMusic();
					break;
				}
				else if (!*arguments->pause) {
					Mix_ResumeMusic();
					break;
				}
				SDL_Delay(10);
			}
		}
	SDL_Delay(100);
	}
	audio_status.noise = 0;
	pthread_cond_signal( &cond_end);

	Mix_Quit();

}

void (music_Finished)() {

	if (!ending) {
		printf("Another one\n");
		
		
		if(Mix_FadeInMusicPos(music, 1, 0, 0)==-1) {
		    printf("Mix_FadeInMusic: %s\n", Mix_GetError());
		    // well, there's no music, but most games don't break without music...
		}
		
		// skip one minute into the song, from the start
		// this assumes you are playing an MP3
		if(Mix_SetMusicPosition(loop_pos)==-1) {
		    printf("Mix_SetMusicPosition: %s\n", Mix_GetError());
		}
	}
}


