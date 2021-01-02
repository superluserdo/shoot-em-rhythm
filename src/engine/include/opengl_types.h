#ifndef _OPENGL_TYPES_H
#define _OPENGL_TYPES_H
#include <GL/glew.h>

struct int_rect {
	int x;
	int y;
	int w;
	int h;
};

struct framebuffer {
	unsigned int framebuffer;
	struct int_rect viewport;
	unsigned int texture;
};

//struct globject {
//	unsigned int texture;
//	unsigned int n_vertices;
//	//size_t vert_stride; //TODO
//	float *vertices;
//	unsigned int n_indices;
//	unsigned int *indices;
//	unsigned int shader;
//	void (*uniforms)(unsigned int shader);
//	//struct globject *next;
//};

//struct globject_list {
//	struct globject *head;
//	struct globject *tail;
//};

struct glrenderer {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	//struct globject_list object_list;
	float clear_colour[4];
	struct framebuffer *framebuffer;
	struct framebuffer *render_target;
	int do_wireframe;
};

typedef unsigned int texture_t;
typedef struct glrenderer *renderer_t;

struct shaders_struct {
	int simple;
	int white;
	int debug;
	int solid;
	int glow_behind;
	int glow;
};

#if defined _SDL_FUNCS_H || defined _SDL_TYPES_H
#error "Incompatible headers mixed together!"
#endif

#endif
