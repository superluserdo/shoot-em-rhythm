#define ZOOM_MULT 2
//#define NATIVE_RES_X 640
#define NATIVE_RES_X 640
#define NATIVE_RES_Y 360
#define MAX_SOUNDS_LIST 10

void *frametimer(void *);
int pause(SDL_Renderer *renderer, SDL_Texture *levelcapture);



int level(SDL_Window *win, SDL_Renderer *renderer);
int startscreen(SDL_Window *win, SDL_Renderer *renderer);

void *musicstart(void* ptr);
void *playsound(void* soundpath);
void *playmusic(void* soundpath);
void musicstop(void);
void soundstart(void);
void soundstop(void);

void quitstart(SDL_Texture *startimgbg, SDL_Texture *startimgtext1, SDL_Texture *startimgtext2, SDL_Renderer *renderer);

/* Return Code definitions
	0:	Return normally
	1-100:	Return normally and again x many times
	101:	Quit to startscreen from wherever
	102:	Quit to desktop from wherever
*/
