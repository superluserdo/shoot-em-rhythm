#ifndef _SDL_TYPES_H
#define _SDL_TYPES_H
#include <SDL2/SDL.h>

typedef SDL_Texture *texture_t;
typedef SDL_Renderer *renderer_t;

#if defined _OPENGL_FUNCS_H || defined _OPENGL_TYPES_H
#error "Incompatible headers mixed together!"
#endif

#endif
