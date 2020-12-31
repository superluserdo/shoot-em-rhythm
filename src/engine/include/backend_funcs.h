#ifndef _BACKEND_FUNCS_H
#define _BACKEND_FUNCS_H
#include "backend_types.h"
#if USE_OPENGL
#include "opengl_funcs.h"
#endif
#include "structdef_engine.h"

int graphics_init(struct graphics_struct *master_graphics);
void clear_render_target(renderer_t renderer);
void query_resize(struct graphics_struct *master_graphics);
int texture_from_path(unsigned int *texture, int *w, int *h, char *path);
int render_copy(struct render_node *node_ptr, renderer_t renderer);
texture_t create_texture(renderer_t renderer, int width, int height);
void present_screen(struct time_struct timing, struct graphics_struct *master_graphics);
void graphics_deinit(struct graphics_struct *master_graphics);
void render_list(struct graphics_struct *master_graphics);

struct glrenderer *make_renderer(struct framebuffer *render_target, float clear_colour[4],
		int own_framebuffer, struct int_rect viewport);
void update_quad_vertices_opengl(struct float_rect rect_in, struct float_rect rect_out, struct render_node *node);
void update_quad_vertices_sdl(struct float_rect rect_in, struct float_rect rect_out, struct render_node *node);
void change_renderer(struct glrenderer *renderer);
#endif
