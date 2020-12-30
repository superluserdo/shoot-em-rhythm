#ifndef _OPENGL_TYPES_H
#define _OPENGL_TYPES_H
#include <GL/glew.h>

struct floatrect {
	float x;
	float y;
	float w;
	float h;
};

struct intrect {
	int x;
	int y;
	int w;
	int h;
};

struct framebuffer {
	unsigned int framebuffer;
	struct intrect viewport;
	unsigned int texture;
};

struct glrenderer *make_renderer(char *texture_path, 
		struct framebuffer *render_target, float clear_colour[4],
		int own_framebuffer, struct intrect viewport);

struct globject {
	unsigned int texture;
	unsigned int n_vertices;
	//size_t vert_stride; //TODO
	float *vertices;
	unsigned int n_indices;
	unsigned int *indices;
	unsigned int texture_shader;
	void (*uniforms)(unsigned int texture_shader);
	//struct globject *next;
};

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

#if defined _SDL_FUNCS_H || defined _SDL_TYPES_H
#error "Incompatible headers mixed together!"
#endif

#endif
