#define ZOOM_MULT 2
//#define NATIVE_RES_X 640
#define NATIVE_RES_X 640
#define NATIVE_RES_Y 360

int pausefunc(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel, struct status_struct *status);

int level_init(struct status_struct status);
int level_loop(struct status_struct status);
int startscreen(SDL_Window *win, SDL_Renderer *renderer, struct status_struct *status);

void quitstart(SDL_Texture *startimgbg, SDL_Texture *startimgtext1, SDL_Texture *startimgtext2, SDL_Renderer *renderer);

uint64_t rdtsc();
/* Return Code definitions
	0:	Return normally
	1-100:	Return normally and again x many times
	101:	Quit to startscreen from wherever
	102:	Quit to desktop from wherever
*/
