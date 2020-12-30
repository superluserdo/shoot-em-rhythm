#ifndef _OPENGL_FUNCS_H
#define _OPENGL_FUNCS_H
#include <GL/glew.h>
#include "opengl_types.h"
#include "structdef_engine.h"

//Starts up SDL, creates window, and initializes OpenGL
int init_sdl_opengl(struct graphics_struct *master_graphics);

//Initializes rendering program and clear color
void initGL(void);

void get_texture_size(unsigned int texture, int *w, int *h);

void checkCompileErrors(GLuint shader, const char *type);

struct globject *new_object(struct globject_list *object_list);

void insert_object_after(struct globject_list *object_list, struct globject *object, struct globject *target);

struct globject *new_quad(struct floatrect rect, unsigned int texture, 
		unsigned int prog_ID, struct globject_list *object_list, 
		int sdl_coordinates);

unsigned int texture_from_image(char *path);

void change_render_target(struct glrenderer *renderer);

//Renders quad to the screen
void render(struct graphics_struct *master_graphics);

#endif
