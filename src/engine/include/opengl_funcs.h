#ifndef _OPENGL_FUNCS_H
#define _OPENGL_FUNCS_H
#include <GL/glew.h>
#include "opengl_types.h"
#include "structdef_engine.h"
#include "backend_funcs.h"

//Starts up SDL, creates window, and initializes OpenGL
int init_sdl_opengl(struct graphics_struct *master_graphics);

//Initializes rendering program and clear color
int initGL(void);

void get_texture_size(unsigned int texture, int *w, int *h);

void checkCompileErrors(GLuint shader, const char *type);

struct render_node *new_quad(struct float_rect rect_in, 
		struct float_rect rect_out,
		unsigned int texture, unsigned int texture_shader, int sdl_coords);

unsigned int texture_from_image(char *path);

void change_render_target(struct glrenderer *renderer);

//Renders quad to the screen
void render(struct graphics_struct *master_graphics);

void update_render_node(struct animation_struct *animation);
#endif
