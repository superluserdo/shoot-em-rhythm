#include <SDL2/SDL_image.h>
#include "structdef_engine.h"
#include "structdef_game.h"
#include "sdl_types.h"

int graphics_init(struct graphics_struct *master_graphics) {
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		return 1;

	SDL_Window *win = SDL_CreateWindow("TOM'S SUPER COOL GAME", 100, 100, 
			master_graphics->width, master_graphics->height, 
			SDL_WINDOW_RESIZABLE);
	master_graphics->window = win;
	SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr,"SDL_CreateRenderer failed. Error message: '%s\n'", 
				SDL_GetError());
		
		/* Print driver info if renderer creation fails */
		SDL_RendererInfo info;
		int num = SDL_GetNumRenderDrivers();
		printf("There are %d usable render drivers\n", num);
		printf("Driver  SDL_RendererFlags\n");
		for (int i = 0; i < num; i++) {
	   	if (SDL_GetRenderDriverInfo(i,&info) == 0)
	   	printf("%s  %d\n", info.name, info.flags&2);
		}
	}
	master_graphics->graphics.renderer = renderer;
	return 0;

}

int texture_from_path(texture_t *texture, SDL_Renderer *renderer, char *path) {
		if (path) {
			*texture = IMG_LoadTexture(renderer, path);
		}
		else {
			printf("Could not find valid image file \"%s\"\n", path);
			FILEINFO
			return R_FAILURE;
		}
		if (!(*texture)) {
			printf("Error loading image file: \"%s\"\n", path);
			FILEINFO
			return R_FAILURE;
		}
		return R_SUCCESS;
}

void query_resize(struct graphics_struct *master_graphics, texture_t *tex_target_ptr) {

	/* Get (potentially updated) window dimensions */
	int w, h;
	SDL_GetWindowSize(master_graphics->window, &w, &h);

	if ((w != master_graphics->width) || (h != master_graphics->height)) {
		master_graphics->width = w;
		master_graphics->height = h;

		/* If rendering to a target (not screen), resize texTarget to new texture */

		if (*tex_target_ptr) {
			SDL_Texture *texTarget_new = SDL_CreateTexture(master_graphics->graphics.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
			SDL_Texture *target_prev = SDL_GetRenderTarget(master_graphics->graphics.renderer);
			SDL_SetRenderTarget(master_graphics->graphics.renderer, texTarget_new);

			SDL_RenderClear(master_graphics->graphics.renderer);
			SDL_RenderCopy(master_graphics->graphics.renderer, *tex_target_ptr, NULL, NULL);

			SDL_SetRenderTarget(master_graphics->graphics.renderer, target_prev);
			SDL_DestroyTexture(*tex_target_ptr);
			*tex_target_ptr = texTarget_new;
		}
	}
}

void clear_render_target(struct graphics_struct *master_graphics, struct graphical_stage_struct *graphics) {
	SDL_SetRenderTarget(master_graphics->graphics.renderer, *graphics->tex_target_ptr);
	SDL_RenderClear(master_graphics->graphics.renderer);
}

int render_copy(struct render_node *node_ptr, SDL_Renderer *renderer) {
	int rc = SDL_RenderCopy(renderer, node_ptr->img, node_ptr->rect_in, &node_ptr->rect_out);
	if (rc != 0) {
		printf("%s\n", SDL_GetError());
		FILEINFO
	}
	return rc;
}

SDL_Texture *create_texture(renderer_t renderer, int width, int height) {
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET, width, height);
	return texture;
}

void present_screen(struct time_struct timing, struct graphics_struct *master_graphics) {
	SDL_RenderPresent(master_graphics->graphics.renderer);
}

void set_render_target(SDL_Renderer *renderer, texture_t target) {
	
}

void graphics_deinit(struct graphics_struct *master_graphics) {
	/* Release SDL resources. */
	SDL_DestroyRenderer(master_graphics->graphics.renderer);
	SDL_DestroyWindow(master_graphics->window);
}
