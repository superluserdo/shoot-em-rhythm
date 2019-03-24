#define ZOOM_MULT 2
//#define NATIVE_RES_X 640
#define NATIVE_RES_X 640
#define NATIVE_RES_Y 360

int pausefunc(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel, struct status_struct *status);
int pausefunc_bkp(SDL_Renderer *renderer, SDL_Texture *levelcapture, int currentlevel, struct status_struct *status);

int level_init(struct status_struct status);
int level_loop(struct status_struct status);
int startscreen(SDL_Window *win, SDL_Renderer *renderer, struct status_struct *status);

void quitstart(SDL_Texture *startimgbg, SDL_Texture *startimgtext1, SDL_Texture *startimgtext2, SDL_Renderer *renderer);

uint64_t rdtsc();

/* Module stuff */
void *(*funcptr)(struct status_struct *status);
#define NUM_HOOK_LOCATIONS 3
enum return_codes_e hooks_setup(struct program_struct *program);
