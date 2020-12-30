#ifndef _BACKEND_FUNCS_H
#define _BACKEND_FUNCS_H
#include "backend_types.h"
#if USE_OPENGL
#include "opengl_funcs.h"
#endif
#include "structdef_engine.h"

int graphics_init(struct graphics_struct *master_graphics);
void clear_render_target(struct graphics_struct *master_graphics, struct graphical_stage_struct *graphics);
void query_resize(struct graphics_struct *master_graphics);
int texture_from_path(texture_t *texture, renderer_t renderer, char *path);
int render_copy(struct render_node *node_ptr, renderer_t renderer);
texture_t create_texture(renderer_t renderer, int width, int height);
void present_screen(struct time_struct timing, struct graphics_struct *master_graphics);
void graphics_deinit(struct graphics_struct *master_graphics);
void render_list(struct graphics_struct *master_graphics);

struct glrenderer *make_renderer(struct framebuffer *render_target, float clear_colour[4],
		int own_framebuffer, struct int_rect viewport);
void update_quad_vertices_sdl(struct float_rect rect, struct render_node *node);
#endif
