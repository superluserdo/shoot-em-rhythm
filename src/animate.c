#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"

/* RENDER */

int renderlist(struct render_node *node_ptr) {
	while (node_ptr != NULL) {
		if (node_ptr->customRenderFunc == NULL){
			SDL_RenderCopy(node_ptr->renderer, node_ptr->img, node_ptr->rect_in, node_ptr->rect_out);
		}
		else {
			(*node_ptr->customRenderFunc)(node_ptr->customRenderArgs);
		}
		node_ptr = node_ptr->next;
	}
}

int node_insert_over(struct render_node *node_src, struct render_node *node_dest) {
	if (node_dest == NULL) {
		render_node_head = node_src;
		render_node_tail = node_src;
		node_src->z = 0;
	}
	else {
		node_src->next = node_dest->next;
		node_src->prev = node_dest;
		node_dest->next = node_src;	
		node_src->z = node_dest->z;
	}
	if (node_dest == render_node_tail) {
		render_node_tail = node_src;
	}
}


int node_insert_under(struct render_node *node_src, struct render_node *node_dest) {
	if (node_dest == NULL) {
		render_node_head = node_src;
		render_node_tail = node_src;
		node_src->z = 0;
	}
	else {
		node_src->next = node_dest;
		node_src->prev = node_dest->prev;
		node_dest->prev = node_src;	
		node_src->z = node_dest->z;
	}
	if (node_dest == render_node_head) {
		render_node_head = node_src;
	}
}

int node_insert_z_under(struct render_node *node_src, float z) {
	if (render_node_head == NULL) {
		render_node_head = node_src;
		render_node_tail = node_src;
		node_src->z = z;
	}
	else {
		struct render_node *node = render_node_head;
		while (node) {
			if ( node_src->z <= node->z) {
				node_src->next = node;
				node_src->prev = node->prev;
				node->prev = node_src;	
				if (node == render_node_head) {
					render_node_head = node_src;
				}
				break;
			}		
			if (node == render_node_tail) {
				node_src->prev = render_node_tail;
				node_src->next = NULL;
				render_node_tail->next = node_src;
				render_node_tail = node_src;
			}
		}
	}
}

int node_insert_z_over(struct render_node *node_src, float z) {
	if (render_node_head == NULL) {
		render_node_head = node_src;
		render_node_tail = node_src;
		node_src->z = z;
	}
	else {
		struct render_node *node = render_node_head;
		while (node) {
			if ( node_src->z < node->z) {
				node_src->next = node;
				node_src->prev = node->prev;
				node->prev = node_src;	
				if (node == render_node_head) {
					render_node_head = node_src;
				}
				break;
			}		
			if (node == render_node_tail) {
				node_src->prev = render_node_tail;
				node_src->next = NULL;
				render_node_tail->next = node_src;
				render_node_tail = node_src;
			}
		}
	}
}

int node_rm(struct render_node *node_ptr) {
	if (node_ptr->prev) {
		node_ptr->prev->next = node_ptr->next;
	}
	else {
		render_node_head = node_ptr;
	}

	if (node_ptr->next) {
		node_ptr->next->prev = node_ptr->prev;
	}
	else {
		render_node_tail = node_ptr;
	}
	free(node_ptr);
}

int list_rm(struct render_node *node_ptr) {
	if (node_ptr) {
		
		struct render_node *node_ptr_back = node_ptr->prev;
		struct render_node *node_ptr_fwd = node_ptr->next;
		while (node_ptr_back) {
			struct render_node *ptr_tmp = node_ptr_back;
			node_ptr_back = node_ptr_back->prev;
			free(ptr_tmp);
		}
	
		while (node_ptr_fwd) {
			struct render_node *ptr_tmp = node_ptr_fwd;
			node_ptr_fwd = node_ptr_back->next;
			free(ptr_tmp);
		}
		free(node_ptr);
	}
}

struct render_node *create_render_node() {
	struct render_node *new_node = malloc(sizeof(struct render_node));
	if (new_node == NULL)
		printf("Error creating new rendering node :(\n");
	new_node->next = NULL;
	new_node->prev = NULL;
	new_node->img = NULL;
	new_node->customRenderFunc = NULL;
	new_node->customRenderArgs = NULL;
	return new_node;
}

/* ANIMATE */


int advanceFrames(struct render_node *render_node_head) {
	struct render_node *node_ptr = render_node_head;
	while (node_ptr) {
		struct animate_specific *animation = node_ptr->animation;
		struct animate_generic *generic = animation->generic;
		//printf("frame:  %d", animation->frame);
		//printf("lastframbeat:  %f", animation->lastFrameBeat);
		//printf("	time:  %f\n", timing.currentbeat);
		if (timing.currentbeat - animation->lastFrameBeat >= generic->clips[animation->clip]->frames[animation->frame].duration) {
			animation->lastFrameBeat += generic->clips[animation->clip]->frames[animation->frame].duration;
			animation->frame++;
		}
		if (animation->frame >= generic->clips[animation->clip]->numFrames) {
			if (animation->loops == 0) {
				animation->clip = animation->return_clip;	
				animation->frame = 0;
				animation->loops = -1;
			}
			else {
				animation->frame = 0;
				if (animation->loops > 0)
					animation->loops--;
			}
		}
		node_ptr->rect_in = &(generic->clips[animation->clip]->frames[animation->frame].rect);
		node_ptr = node_ptr->next;
	}		

}

/* INITIALISE THE LEVEL (STORED HERE TEMPORARILY) */

int render_node_populate(struct render_node **render_node_head_ptr, struct render_node *r_node, SDL_Texture **imgList, SDL_Renderer *renderer, struct player_struct *playerptr) {
		
	int w,h;
	SDL_Texture *Spriteimg = IMG_LoadTexture(renderer, SPRITE_PATH);

	SDL_QueryTexture(Spriteimg, NULL, NULL, &w, &h); // get the width and height of the texture
	SDL_Rect rcSrc;

	/* set animation frame */
	rcSrc.x = 0;
	rcSrc.y = 0;
	rcSrc.w = POKESPRITE_SIZEX;
	rcSrc.h = POKESPRITE_SIZEY;

	struct frame *testframes = malloc(4 * sizeof(struct frame));
	for (int i = 0; i < 4; i++) {
		testframes[i].rect = rcSrc;
		testframes[i].duration = 0.25;
		testframes[i].rect.x = testframes[i].rect.w * i;
	}

	int testnumanimations = 1;

	struct clip **testclips = malloc(sizeof(struct clip *)*testnumanimations);

	struct clip *testclip = malloc(sizeof(struct clip));

	testclip->img = Spriteimg;
	testclip->numFrames = 4;
	testclip->frames = testframes;

	testclips[0] = testclip;

	struct animate_generic *testgeneric = malloc(sizeof(struct animate_generic));
	
	testgeneric->numAnimations = testnumanimations;
	testgeneric->clips = testclips;


	struct animate_specific *testspecific = malloc(sizeof(struct animate_specific));
	
	
		testspecific->generic = testgeneric;
		testspecific->clip = 0;
		testspecific->frame = 0;
		testspecific->speed = 1;
		testspecific->loops = -1;
		testspecific->return_clip = 0;
		testspecific->lastFrameBeat = 0.0;
		/* set sprite position */
		testspecific->rect_out.x = playerptr->pos.x;
		testspecific->rect_out.y = playerptr->pos.y;
		testspecific->rect_out.w = POKESPRITE_SIZEX*ZOOM_MULT*2;
		testspecific->rect_out.h = POKESPRITE_SIZEY*ZOOM_MULT*2;


	r_node = create_render_node();
	r_node->rect_in = &rcSrc;
	r_node->rect_out = &testspecific->rect_out;
	r_node->renderer = renderer;
	r_node->img = Spriteimg;
	r_node->animation = testspecific;
	r_node->customRenderFunc = NULL;
	playerptr->animation = testspecific;
	node_insert_z_over(r_node, 0);
	testspecific->render_node = r_node;
	*render_node_head_ptr = r_node;
}
