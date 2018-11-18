#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "newstruct.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h"
#include "structure.h"

/* For debugging */

int node_ptr_count = 0;
int rule_node_count = 0;
int transform_node_count = 0;

/* RENDER */
extern SDL_Texture **image_bank;

void render_process(struct std_list *object_list_stack, struct graphics_struct *graphics, struct time_struct *timing) {
	advance_frames_and_create_render_list(object_list_stack, graphics, timing->currentbeat);
	renderlist(graphics->render_node_head, graphics);

	SDL_SetRenderTarget(graphics->renderer, NULL);
	SDL_RenderClear(graphics->renderer);

	if (graphics->rendercopyex_data) {
		struct rendercopyex_struct *data = graphics->rendercopyex_data;
		SDL_RenderCopyEx(data->renderer, data->texture, data->srcrect, 
						data->dstrect, data->angle, data->center, data->flip);
	}
	else {
		SDL_RenderCopy(graphics->renderer, graphics->imgs->texTarget, NULL, NULL);
	}
	SDL_Delay(wait_to_present(timing));
	SDL_RenderPresent(graphics->renderer);
	update_time(timing);

	SDL_SetRenderTarget(graphics->renderer, graphics->imgs->texTarget);
}

int renderlist(struct render_node *node_ptr, struct graphics_struct *graphics) {
	graphics->debug_anchors = 1;
	graphics->debug_containers = 1;
	while (node_ptr != NULL) {
		if (node_ptr->customRenderFunc == NULL){
			int rc = SDL_RenderCopy(node_ptr->renderer, node_ptr->img, node_ptr->rect_in, &node_ptr->rect_out);
			if (rc != 0) {
				printf("%s\n", SDL_GetError());
			}
		}
		else {
			(*node_ptr->customRenderFunc)(node_ptr->customRenderArgs);
		}
		if (graphics->debug_anchors) {
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 255, 255);
			int anchor_width = 6;
			SDL_Rect anchor_rect = {
				.x = node_ptr->rect_out.x + node_ptr->animation->anchors_exposed[0].pos_anim_internal.w * node_ptr->rect_out.w - anchor_width/2,
				.y = node_ptr->rect_out.y + node_ptr->animation->anchors_exposed[0].pos_anim_internal.h * node_ptr->rect_out.h - anchor_width/2,
				.w = anchor_width,
				.h = anchor_width,
			};
			//printf("{%d, %d, %d, %d}\n", anchor_rect.x, anchor_rect.y, anchor_rect.w, anchor_rect.h); 
			int rc = SDL_RenderFillRect(node_ptr->renderer, &anchor_rect);
			if (rc != 0) {
				printf("%s\n", SDL_GetError());
			}
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 0, 255);
		}
		if (graphics->debug_containers) {
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 255, 255);
			int line_width = 6;
			SDL_Rect abs_container = visual_container_to_pixels(node_ptr->animation->parent->container,
									(struct xy_struct){graphics->width, graphics->height});
			SDL_Point points[5] = {
				{abs_container.x, abs_container.y},
				{abs_container.x + abs_container.w, abs_container.y},
				{abs_container.x + abs_container.w, abs_container.y + abs_container.h},
				{abs_container.x, abs_container.y + abs_container.h},
				{abs_container.x, abs_container.y},
			};
			//printf("{%d, %d, %d, %d}\n", anchor_rect.x, anchor_rect.y, anchor_rect.w, anchor_rect.h); 
			int rc = SDL_RenderDrawLines(node_ptr->renderer, points, 5);
			if (rc != 0) {
				printf("%s\n", SDL_GetError());
			}
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 0, 255);
		}
		node_ptr = node_ptr->next;

	}
	return 0;
}

int node_insert_over(struct graphics_struct *graphics, struct render_node *node_src, struct render_node *node_dest) {
	struct render_node *render_node_head = graphics->render_node_head;
	struct render_node *render_node_tail = graphics->render_node_tail;
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
	return 0;
}

int node_insert_under(struct graphics_struct *graphics, struct render_node *node_src, struct render_node *node_dest) {
	struct render_node *render_node_head = graphics->render_node_head;
	struct render_node *render_node_tail = graphics->render_node_tail;
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
	return 0;
}

int node_insert_z_under(struct graphics_struct *graphics, struct render_node *node_src, float z) {
	struct render_node *render_node_head = graphics->render_node_head;
	struct render_node *render_node_tail = graphics->render_node_tail;
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
				//if (node == render_node_tail) {
				//	render_node_tail = node_src;
				//}
				break;
			}		
			if (node == render_node_tail) {
				node_src->prev = render_node_tail;
				node_src->next = NULL;
				render_node_tail->next = node_src;
				render_node_tail = node_src;
				break;
			}
			node = node->next;
		}
	}
	return 0;
}

int node_insert_z_over(struct graphics_struct *graphics, struct render_node *node_src, float z) {
	struct render_node *render_node_head = graphics->render_node_head;
	struct render_node *render_node_tail = graphics->render_node_tail;
	node_src->z = z;
	if (render_node_head == NULL) {
		graphics->render_node_head = node_src;
		graphics->render_node_tail = node_src;
	}
	else {
		struct render_node *node = render_node_head;
		while (node) {
			if ( node_src->z < node->z) {
				node_src->next = node;
				node_src->prev = node->prev;
				node->prev = node_src;	
				if (node == render_node_head) {
					graphics->render_node_head = node_src;
				}
				//if (node == render_node_tail) {
				//	graphics->render_node_tail = node_src;
				//}
				break;
			}		
			if (node == render_node_tail) {
				node_src->prev = render_node_tail;
				node_src->next = NULL;
				graphics->render_node_tail->next = node_src;
				graphics->render_node_tail = node_src;
				break;
			}
			node = node->next;
		}
	}
	return 0;
}

int node_rm(struct graphics_struct *graphics, struct render_node *node_ptr) {
	struct render_node *render_node_head = graphics->render_node_head;
	struct render_node *render_node_tail = graphics->render_node_tail;
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
	return 0;
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
			node_ptr_fwd = node_ptr_fwd->next;
			free(ptr_tmp);
		}
		free(node_ptr);
	}
	return 0;
}

struct render_node *create_render_node() {
	struct render_node *new_node = malloc(sizeof(struct render_node));
	if (new_node == NULL)
		printf("Error creating new rendering node :(\n");
	new_node->prev = NULL;
	new_node->next = NULL;
	new_node->rect_in = NULL;
	new_node->rect_out.x = 0;
	new_node->rect_out.y = 0;
	new_node->rect_out.w = 0;
	new_node->rect_out.h = 0;
	new_node->img = NULL;
	new_node->customRenderFunc = NULL;
	new_node->customRenderArgs = NULL;
	new_node->renderer = NULL;
	new_node->animation = NULL;
	new_node->transform_list = NULL;
	new_node->z = 0;
	return new_node;
}

/* ANIMATE */


int advanceFrames(struct render_node *render_node_head, float currentbeat) {
	struct render_node *node_ptr = render_node_head;
	node_ptr_count = 0;
	while (node_ptr) {
		rule_node_count = 0;
		transform_node_count = 0;
		struct animate_specific *animation = node_ptr->animation;
		struct animate_generic *generic = animation->generic;

		/* set sprite position from parent object again in case of manual changes during program:	*/
		animation->rect_out.x = animation->parent->pos.x + animation->native_offset.x * ZOOM_MULT * 2;
		animation->rect_out.y = animation->parent->pos.y + animation->native_offset.y * ZOOM_MULT * 2;
	
		animation->rect_out.w = generic->clips[animation->clip]->frames[animation->frame].rect.w
								*animation->parent->size_ratio.w*ZOOM_MULT*2;
		animation->rect_out.h = generic->clips[animation->clip]->frames[animation->frame].rect.h
								*animation->parent->size_ratio.h*ZOOM_MULT*2;

		struct rule_node *rule_node = animation->rules_list;
		while (rule_node) {
			(rule_node->rule)(rule_node->data);
			rule_node = rule_node->next;
			rule_node_count++;
		}

		if (generic->clips[animation->clip]->frames[animation->frame].duration > 0.0) {
			if (currentbeat - animation->lastFrameBeat >= generic->clips[animation->clip]->frames[animation->frame].duration) {
				animation->lastFrameBeat += generic->clips[animation->clip]->frames[animation->frame].duration;
				animation->frame++;
			}
		}
		if (animation->frame >= generic->clips[animation->clip]->num_frames) {
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
		SDL_Rect rect_trans = animation->rect_out;
		struct func_node *transform_node = animation->transform_list;
		while (transform_node) {
			transform_node->func((void *)&rect_trans, transform_node->data);
			transform_node = transform_node->next;
			transform_node_count++;
		}
		node_ptr->rect_out = rect_trans;
		node_ptr = node_ptr->next;
		node_ptr_count++;
	}		

	return 0;
}

void advance_frames_and_create_render_list(struct std_list *object_list_stack, struct graphics_struct *graphics, float currentbeat) {

	struct std_list *std_list_node = object_list_stack;
	int std_list_node_count = 0;
	int render_node_count = 0;
	while (std_list_node) {
		struct std *object = std_list_node->std;
		struct visual_container_struct *container = object->container;
		SDL_Rect abs_container = visual_container_to_pixels(container, (struct xy_struct) 
															{graphics->width, graphics->height});
		
		struct animate_specific *animation = object->animation;
		while (animation) {
			struct animate_generic *generic = animation->generic;
			struct rule_node *rule_node = animation->rules_list;

			while (rule_node) {
				(rule_node->rule)(rule_node->data);
				rule_node = rule_node->next;
				rule_node_count++;
			}

			if (generic->clips[animation->clip]->frames[animation->frame].duration > 0.0) {
				if (currentbeat - animation->lastFrameBeat >= generic->clips[animation->clip]->frames[animation->frame].duration) {
					animation->lastFrameBeat += generic->clips[animation->clip]->frames[animation->frame].duration;
					animation->frame++;
				}
			}
			if (animation->frame >= generic->clips[animation->clip]->num_frames) {
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

			struct size_ratio_struct anchor_grabbed_container_scale;
			if (animation->anchor_grabbed) { /* anchor_grabbed_container_scale.w, h are the relative position in the exposing animation */
				struct anchor_struct anchor_grabbed = *animation->anchor_grabbed;
				
				anchor_grabbed_container_scale.w = anchor_grabbed.pos_anim_internal.w * anchor_grabbed.anim->rect_out_container_scale.w
					 + anchor_grabbed.anim->rect_out_container_scale.x;
				anchor_grabbed_container_scale.h = anchor_grabbed.pos_anim_internal.h * anchor_grabbed.anim->rect_out_container_scale.h
					 + anchor_grabbed.anim->rect_out_container_scale.y;
			} else { /* anchor_grabbed_container_scale.w, h are the relative position in the container */
				//anchor_grabbed_container_scale.w = anchor_grabbed.pos_anim_internal.w;
				//anchor_grabbed_container_scale.h = anchor_grabbed.pos_anim_internal.h;
				anchor_grabbed_container_scale.w = object->container_pos.w;
				anchor_grabbed_container_scale.h = object->container_pos.h;
			}

			struct frame frame = generic->clips[animation->clip]->frames[animation->frame];

			struct float_rect anchor_hook_pos_internal = {
				.x = (float)frame.anchor_hook.x / (float)frame.rect.w,
				.y = (float)frame.anchor_hook.y / (float)frame.rect.h,
			};

			struct float_rect size_and_hook_pos_dominant_scale;
		   	/*  x, y = container-scale anchor hook position.
				w, h = container-scale rect size, 
				scaled in units of container size in direction of container_scale_mode */

			float internal_aspect_ratio = (float)frame.rect.w / (float)frame.rect.h;
			float container_size_aspect_ratio = (float)abs_container.w / (float)abs_container.h;

			if (animation->container_scale_mode == WIDTH) {
				size_and_hook_pos_dominant_scale.x = anchor_grabbed_container_scale.w;
				size_and_hook_pos_dominant_scale.y = anchor_grabbed_container_scale.h / container_size_aspect_ratio;
				size_and_hook_pos_dominant_scale.w = animation->container_scale_factor;
				size_and_hook_pos_dominant_scale.h = animation->container_scale_factor / internal_aspect_ratio;
			} else if (animation->container_scale_mode == HEIGHT) {
				size_and_hook_pos_dominant_scale.y = anchor_grabbed_container_scale.h;
				size_and_hook_pos_dominant_scale.x = anchor_grabbed_container_scale.w * container_size_aspect_ratio;
				size_and_hook_pos_dominant_scale.h = animation->container_scale_factor;
				size_and_hook_pos_dominant_scale.w = animation->container_scale_factor * internal_aspect_ratio;
			}

			//SDL_Rect rect_trans = animation->rect_out;
			struct func_node *transform_node = animation->transform_list;
			while (transform_node) {
				//transform_node->func((void *)&abs_rect, transform_node->data);
				transform_node->func((void *)&size_and_hook_pos_dominant_scale, transform_node->data);
				transform_node = transform_node->next;
				transform_node_count++;
			}

			struct float_rect size_and_hook_pos_container_scale;

			/* Convert back into separate container-scaled dimensions (x and y decoupled): */
			if (animation->container_scale_mode == WIDTH) {

				size_and_hook_pos_container_scale.w = size_and_hook_pos_dominant_scale.w;
				size_and_hook_pos_container_scale.h = size_and_hook_pos_dominant_scale.h
													* container_size_aspect_ratio;

				size_and_hook_pos_container_scale.x = size_and_hook_pos_dominant_scale.x;

				size_and_hook_pos_container_scale.y = size_and_hook_pos_dominant_scale.y 
													* container_size_aspect_ratio;

			} else if (animation->container_scale_mode == HEIGHT) {

				size_and_hook_pos_container_scale.h = size_and_hook_pos_dominant_scale.h;
				size_and_hook_pos_container_scale.w = size_and_hook_pos_dominant_scale.w
													/ container_size_aspect_ratio;

				size_and_hook_pos_container_scale.y = size_and_hook_pos_dominant_scale.y;
				size_and_hook_pos_container_scale.x = size_and_hook_pos_dominant_scale.x
													/ container_size_aspect_ratio;

			}

			/* Convert from hook pos to rect pos: */
			animation->rect_out_container_scale.w = size_and_hook_pos_container_scale.w;
			animation->rect_out_container_scale.h = size_and_hook_pos_container_scale.h;
			animation->rect_out_container_scale.x = size_and_hook_pos_container_scale.x
					- animation->rect_out_container_scale.w * anchor_hook_pos_internal.x;
			animation->rect_out_container_scale.y = size_and_hook_pos_container_scale.y
					- animation->rect_out_container_scale.h * anchor_hook_pos_internal.y;

			/* Convert from container-scale relative width to global scale absolute width */
			SDL_Rect abs_rect = {
				.x = abs_container.x + animation->rect_out_container_scale.x * abs_container.w,
				.y = abs_container.y + animation->rect_out_container_scale.y * abs_container.h,
				.w = animation->rect_out_container_scale.w * abs_container.w,
				.h = animation->rect_out_container_scale.h * abs_container.h,
			};

			struct render_node *render_node = animation->render_node;
			if (!render_node) {
				generate_render_node(animation, graphics);
			} else {
				render_node->rect_in = &(generic->clips[animation->clip]->frames[animation->frame].rect);
				if (render_node->z != animation->z) {
					/* Take render node out of the list */
					if (render_node->prev) {
						render_node->prev->next = render_node->next;
					} else {
						graphics->render_node_head = render_node->next;
					}
					if (render_node->next) {
						render_node->next->prev = render_node->prev;
					} else {
						graphics->render_node_tail = render_node->prev;
					}
					/* Put render node back in the list in the right place */
					node_insert_z_over(graphics, render_node, animation->z);
				}
			}

			render_node->rect_out = abs_rect;
			//render_node = render_node->next;
			render_node_count++;

			animation = animation->next;
		}
		std_list_node = std_list_node->prev;
		std_list_node_count++;
	}		
}

/* INITIALISE THE LEVEL (STORED HERE TEMPORARILY) */

int render_node_populate(struct graphics_struct *graphics, struct render_node *r_node, struct player_struct *playerptr) {
		
	SDL_Renderer *renderer = graphics->renderer;
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
		testframes[i].rect.x = testframes[i].rect.w;// * i;
	}

	int testnumanimations = 1;

	struct clip **testclips = malloc(sizeof(struct clip *)*testnumanimations);

	struct clip *testclip = malloc(sizeof(struct clip));

	testclip->img = Spriteimg;
	testclip->num_frames = 4;
	testclip->frames = testframes;

	testclips[0] = testclip;

	struct animate_generic *testgeneric = malloc(sizeof(struct animate_generic));
	
	testgeneric->num_clips = testnumanimations;
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
	testspecific->rect_out.w = POKESPRITE_SIZEX*ZOOM_MULT*2;
	testspecific->rect_out.h = POKESPRITE_SIZEY*ZOOM_MULT*2;
	testspecific->transform_list = malloc(sizeof(struct func_node));
	
	struct func_node *testfunc = testspecific->transform_list;

	struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
	teststruct->rect_bitmask = 1;
	teststruct->freq_perbeat = 1;
	teststruct->ampl = 10.0;
	teststruct->offset = 0.0;
	testfunc->data = (void *)teststruct;
	testfunc->func = &tr_sine;
	testfunc->next = malloc(sizeof(struct func_node));

	struct tr_sine_data *teststruct2 = malloc(sizeof(struct tr_sine_data));
	teststruct2->rect_bitmask = 2;
	teststruct2->freq_perbeat = 1;
	teststruct2->ampl = 10.0;
	teststruct2->offset = 0.25;
	testfunc->next->data = (void *)teststruct2;
	testfunc->next->func = &tr_sine;
	testfunc->next->next = NULL;


	r_node = malloc(sizeof(struct render_node));
	*r_node = new_render_node();
	r_node->rect_in = &rcSrc;
	r_node->rect_out = testspecific->rect_out;
	r_node->renderer = renderer;
	r_node->img = Spriteimg;
	r_node->animation = testspecific;
	r_node->customRenderFunc = NULL;
	playerptr->animation = testspecific;
	node_insert_z_over(graphics, r_node, 0);
	testspecific->render_node = r_node;
	graphics->render_node_head = r_node;
	return 0;
}

struct animate_specific *generate_default_specific_anim(enum graphic_cat_e graphic_category, struct status_struct *status) {

	struct animate_specific *default_specific_anim = malloc(sizeof(struct animate_specific));

	*default_specific_anim = (struct animate_specific) {0};
	default_specific_anim->loops = -1;

	/*	Trying out some transformation defaults here temporarily.
	 *	Will get it into some kind of loadable animation_default config file eventually.
	 */
	if (graphic_category == CHARACTER) {
		default_specific_anim->rules_list = malloc(sizeof(struct rule_node));
		default_specific_anim->rules_list->rule = &rules_player;
		default_specific_anim->rules_list->data = NULL;
		default_specific_anim->rules_list->next = NULL;

		struct func_node *tr_node = malloc(sizeof(struct func_node));
		default_specific_anim->transform_list = tr_node;
		struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
		teststruct->status = status;
		teststruct->rect_bitmask = 2;
		teststruct->freq_perbeat = 0.5;
		teststruct->ampl = 0.2;
		teststruct->offset = 0.0;
		tr_node->data = (void *)teststruct;
		tr_node->func = &tr_sine_abs;
		tr_node->next = NULL;
	}else if (graphic_category == UI) {
		default_specific_anim->rules_list = malloc(sizeof(struct rule_node));
		default_specific_anim->rules_list->rule = &rules_ui;
		default_specific_anim->rules_list->data = NULL;
		default_specific_anim->rules_list->next = NULL;

		struct func_node *tr_node = malloc(sizeof(struct func_node));

		struct tr_bump_data *teststruct2 = malloc(sizeof(struct tr_bump_data));
		teststruct2->status = status;
		teststruct2->freq_perbeat = 1;
		teststruct2->ampl = 2;
		teststruct2->peak_offset = 0.0;
		teststruct2->bump_width = 0.25;
		teststruct2->centre = NULL;
		tr_node->data = (void *)teststruct2;
		tr_node->func = &tr_bump;
		tr_node->next = NULL;
		default_specific_anim->transform_list = tr_node;
		default_specific_anim->rules_list->data = (void *)teststruct2;
	}else if (graphic_category == UI_BAR) {
		default_specific_anim->rules_list = malloc(sizeof(struct rule_node));
		default_specific_anim->rules_list->rule = &rules_ui_bar;
		default_specific_anim->rules_list->data = default_specific_anim;
		default_specific_anim->rules_list->next = malloc(sizeof(struct rule_node));
		default_specific_anim->rules_list->next->rule = &rules_ui;
		default_specific_anim->rules_list->next->data = NULL;
		default_specific_anim->rules_list->next->next = NULL;

		struct func_node *tr_node = malloc(sizeof(struct func_node));

		struct tr_bump_data *teststruct2 = malloc(sizeof(struct tr_bump_data));
		teststruct2->status = status;
		teststruct2->freq_perbeat = 1;
		teststruct2->ampl = 2;
		teststruct2->peak_offset = 0.0;
		teststruct2->bump_width = 0.25;
		teststruct2->centre = NULL;
		tr_node->data = (void *)teststruct2;
		tr_node->func = &tr_bump;
		tr_node->next = NULL;
		default_specific_anim->transform_list = tr_node;
		default_specific_anim->rules_list->next->data = (void *)teststruct2;
	}
	return default_specific_anim;
}

struct animate_specific *generate_specific_anim(struct std *std, struct animate_generic **generic_bank, int index) {
		
	struct animate_specific *specific = malloc(sizeof(struct animate_specific));

	*specific = *generic_bank[index]->default_specific;
	specific->generic = generic_bank[index];
	struct animate_generic *generic = specific->generic;
	specific->container_scale_mode = generic->clips[specific->clip]->container_scale_mode;
	specific->container_scale_factor = generic->clips[specific->clip]->container_scale_factor;
	
	struct rule_node *rule_tmp;
	struct rule_node **rule_ptr = &specific->rules_list;

	struct func_node *tr_tmp;
	struct func_node **tr_ptr = &specific->transform_list;

	while (*rule_ptr) {
		rule_tmp = malloc(sizeof(struct rule_node));
		*rule_tmp = **rule_ptr;
		if (rule_tmp->data) {
			if (rule_tmp->data == generic_bank[index]->default_specific) {
				rule_tmp->data = (void *)specific;
			}
		}
		*rule_ptr = rule_tmp;
		rule_ptr = &(*rule_ptr)->next;
	}
	while (*tr_ptr) {
		tr_tmp = malloc(sizeof(struct rule_node));
		*tr_tmp = **tr_ptr;
		if (tr_tmp->data) {
			if (tr_tmp->data == generic_bank[index]->default_specific) {
				tr_tmp->data = (void *)specific;
			}
		}
		*tr_ptr = tr_tmp;
		tr_ptr = &(*tr_ptr)->next;
	}
	/*	Fetch object-type default values for the specific-animation struct.	*/

	specific->parent = std;
	
	specific->anchors_exposed = malloc(sizeof(struct size_ratio_struct)); //Temporary until I get anchors properly implemented
	/* By default, make the animation's main exposed anchor in the middle */
	specific->anchors_exposed[0] = (struct anchor_struct) {
		.pos_anim_internal = (struct size_ratio_struct) { 0.5, 0.5 },
		.anim = specific,
	};

	/* By default, make the animation's position in the container be determined
	 * not by an anchor but by its std object's "pos" */
	specific->anchor_grabbed = NULL;

	specific->z = 0; //TODO Not sure where this should actually be set but not having this line causes valgrind to complain because this result gets passed to generate_render_node which uses z for node_insert_z_over
	return specific;
}

int generate_render_node(struct animate_specific *specific, struct graphics_struct *graphics) {
	/* Creates, populates, and inserts a rendering node	*/

	struct animate_generic *generic = specific->generic;

	struct render_node *r_node = malloc(sizeof(struct render_node));
	*r_node = new_render_node();
	r_node->rect_in = &generic->clips[specific->clip]->frames[specific->frame].rect;
	r_node->renderer = graphics->renderer;
	r_node->img = generic->clips[specific->clip]->img;
	r_node->animation = specific;
	r_node->customRenderFunc = NULL;
	node_insert_z_over(graphics, r_node, specific->z);
	specific->render_node = r_node;

	return 0;
}

int graphic_spawn(struct std *std, struct std_list **object_list_stack_ptr, struct animate_generic **generic_bank, struct graphics_struct *graphics, enum graphic_type_e *index_array, int num_index) {

	struct std_list *list_node = malloc(sizeof(struct std_list));
	*list_node = (struct std_list) {
		.std = std,
		.next = NULL,
		.prev = *object_list_stack_ptr
	};
	if (*object_list_stack_ptr) {
		(*object_list_stack_ptr)->next = list_node;
	}
	*object_list_stack_ptr = list_node;

	struct animate_specific **a_ptr = &std->animation;
	struct animate_specific *anim = *a_ptr;
	for (int i = 0; i < num_index; i++) {
		anim = generate_specific_anim(std, generic_bank, index_array[i]);
		*a_ptr = anim;
		generate_render_node(anim, graphics);
		a_ptr = (&(*a_ptr)->next);
	}
	// Make final "next" ptr NULL to prevent segfaults
	*a_ptr = NULL;

	return 0;
}

int image_bank_populate(SDL_Texture **image_bank, SDL_Renderer *renderer) {

	const char *cfg_path = "imgs.cfg";
	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, cfg_path))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		FILEINFO
		config_destroy(&cfg);
		return(R_FAILURE);
	}


	config_setting_t *image_list_setting = config_lookup(&cfg, "image_list");
	if (image_list_setting == NULL) {
		printf("Error looking up setting for 'image_list'\n");
		return R_FAILURE;
	}
	int num_images = config_setting_length(image_list_setting); 
	printf("num_images: %d\n", num_images);
	for (int i = 0; i < num_images; i++) {
		const char *path = config_setting_get_string_elem(image_list_setting, i);
		if (path) {
			image_bank[i] = IMG_LoadTexture(renderer, path);
		}
		else {
			printf("Could not find valid image file path in entry %d of file %s\n", i, cfg_path);
			return R_FAILURE;
		}
		if (image_bank[i] == NULL) {
			printf("Error loading image file: %s\n", path);
			return R_FAILURE;
		}
	}
	return R_SUCCESS;
}	

int generic_bank_populate(struct animate_generic ***generic_bank_ptr, SDL_Texture **image_bank, struct status_struct *status) {

	struct animate_generic **generic_bank;

	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "animations.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(R_FAILURE);
	}

	config_setting_t *generics_setting = config_lookup(&cfg, "generics");
	if (generics_setting == NULL) {
		printf("Error looking up setting for 'generics'\n");
		return(R_FAILURE);
	}
	int num_generics = config_setting_length(generics_setting); 
	generic_bank = malloc(num_generics * sizeof(struct animate_generic *));

	struct animate_generic *generic_ptr;
	config_setting_t *generic_setting;
	int num_clips;
	
	struct clip **clip_ptr_array;
	config_setting_t *clips_setting;

	struct clip *clip_ptr;
	config_setting_t *clip_setting;
	SDL_Texture *img;
	int img_index;
	double container_scale_factor;
	int container_scale_mode;
	int num_frames;

	struct frame *frame_array;
	config_setting_t *frames_setting;
	config_setting_t *frame_setting;
	config_setting_t *rect_setting;

	for (int i = 0; i < num_generics; i++) {
		generic_setting = config_setting_get_elem(generics_setting, i);
		if (generic_setting == NULL) {
			printf("Error looking up setting for 'generic'\n");
			return(R_FAILURE);
		}
		generic_ptr = malloc(sizeof(struct animate_generic));

		generic_bank[i] = generic_ptr;

		int graphic_cat_int;
		if (config_setting_lookup_int(generic_setting, "graphic_category", &graphic_cat_int) == CONFIG_FALSE) {
			printf("Error looking up value for 'graphic_category'\n");
			return(R_FAILURE);
		}
		enum graphic_cat_e graphic_cat = (enum graphic_cat_e)graphic_cat_int;

		generic_ptr->default_specific = generate_default_specific_anim(graphic_cat, status);

		clips_setting = config_setting_lookup(generic_setting, "clips");
		if (clips_setting == NULL) {
			printf("Error looking up setting for 'clips' %d\n", i);
			return(R_FAILURE);
		}
		num_clips = config_setting_length(clips_setting); 
		generic_ptr->num_clips = num_clips;

		clip_ptr_array = malloc(num_clips * sizeof(struct clip *));
		generic_ptr->clips = clip_ptr_array;

		for (int j = 0; j < num_clips; j++) {
			clip_ptr = malloc(sizeof(struct clip));
			clip_ptr_array[j] = clip_ptr;

			clip_setting = config_setting_get_elem(clips_setting, j);
			if (clip_setting == NULL) {
				printf("Error looking up setting for 'clip'\n");
				return(R_FAILURE);
			}
			if (config_setting_lookup_int(clip_setting, "img", &img_index) == CONFIG_FALSE) {
				printf("Error looking up value for 'img'\n");
				return(R_FAILURE);
			}

			if (config_setting_lookup_float(clip_setting, "container_scale_factor", &container_scale_factor) == CONFIG_FALSE) {
				clip_ptr->container_scale_factor = 1.0;
			} else {
				clip_ptr->container_scale_factor = container_scale_factor;
			}
			if (config_setting_lookup_int(clip_setting, "container_scale_mode", &container_scale_mode) == CONFIG_FALSE) {
				clip_ptr->container_scale_mode = HEIGHT;
			} else {
				clip_ptr->container_scale_mode = container_scale_mode;
			}
			clip_ptr->img = image_bank[img_index];
			frames_setting = config_setting_lookup(clip_setting, "frames");
			if (frames_setting == NULL) {
				printf("Error looking up setting for 'frames'\n");
				return(R_FAILURE);
			}
			num_frames = config_setting_length(frames_setting);
			clip_ptr->num_frames = num_frames;

			frame_array = malloc(num_frames * sizeof(struct frame));
			clip_ptr->frames = frame_array;

			for (int k = 0; k < num_frames; k++) {
				frame_setting = config_setting_get_elem(frames_setting, k);
				if (frame_setting == NULL) {
					printf("Error looking up setting for 'frame'\n");
					return(R_FAILURE);
				}
				rect_setting = config_setting_lookup(frame_setting, "rect");
				if (rect_setting == NULL) {
					printf("Error looking up setting for 'rect'\n");
					return(R_FAILURE);
				}
				frame_array[k].rect.x = config_setting_get_int_elem(rect_setting, 0);
				frame_array[k].rect.y = config_setting_get_int_elem(rect_setting, 1);
				frame_array[k].rect.w = config_setting_get_int_elem(rect_setting, 2);
				frame_array[k].rect.h = config_setting_get_int_elem(rect_setting, 3);
				double dub;
				config_setting_lookup_float(frame_setting, "duration", &dub);
				frame_array[k].duration = (float)dub;
				/* By default, make the frame's anchor hook in the centre of the image: */
				frame_array[k].anchor_hook = (struct xy_struct) {.x=frame_array[k].rect.w/2, .y=frame_array[k].rect.h/2}; //TODO: Actually assign this properly, maybe have it in animation.cfg
				//frame_array[k].anchor_hook = (struct xy_struct) { 0, 0 }; //TODO: Actually assign this properly, maybe have it in animation.cfg
			}	
		}
	}

	*generic_bank_ptr = generic_bank;
	config_destroy(&cfg);
	return R_SUCCESS;
}

int transform_add_check(struct animate_specific *animation, void *data, void (*func)()) {
	struct func_node *node = animation->transform_list;
	int do_add = 1;
	while (node) {
		if (node->func == func) {
			do_add = 0;
			break;
		}
		node = node->next;
	}
	if (do_add) {
		node = malloc(sizeof(struct func_node));
		node->data = data;
		node->func = func;
		node->next = animation->transform_list;
		animation->transform_list = node;
	}
	return 0;
}
	

int transform_rm(struct animate_specific *animation, void (*func)()) {
	struct func_node *node = animation->transform_list;
	struct func_node *prev = NULL;
	while (node) {
		
		if (node->func == func) {
			if (prev) {
				prev->next = node->next;
			}
			else {
				animation->transform_list = node->next;
			}
			free(node->data);
			free(node);
			break;
		}
		prev = node;
		node = node->next;
	}

	return 0;
}


void rules_player(void *status_void) {
	struct status_struct *status = (struct status_struct *)status_void;
	struct player_struct *player = status->player;
	if (player->living.invincibility_toggle) {
		player->living.invincibility_toggle = 0;
		if (player->living.invincibility) {
			struct tr_blink_data *data = malloc(sizeof(struct tr_blink_data));
			data->frames_on = 4;
			data->frames_off = 4;
			data->status = status;
			transform_add_check(player->animation, data, &tr_blink);
		}
		else {
			transform_rm(player->animation, &tr_blink);
		}
	}
}
void rules_ui(void *data) {
	struct tr_bump_data *str = (struct tr_bump_data *)data;
	if (str->status->level->score > 300)
		str->ampl = 1.2;
	else
		str->ampl = 1 + str->status->level->score/1000.0;
}
void rules_ui_bar(void *animvoid) {
	struct animate_specific *animation = (struct animate_specific *)animvoid;
	//struct ui_bar *bar = animation->parent->self_ui_bar;
	struct ui_bar *bar = animation->parent->self;
	float HP_ratio = (float) *bar->amount / *bar->max;
	animation->rect_out.w = 50 * ZOOM_MULT * 2 * HP_ratio * animation->parent->size_ratio.w;
	if ( HP_ratio <= 0.5 ) {
		if ( HP_ratio <= 0.2 ) {
			animation->frame = 2;
		}
		else {
			animation->frame = 1;
		}
	}
	else {
		animation->frame = 0;
	}
}
void rules_ui_counter(void *animvoid) {
	struct animate_specific *animation = (struct animate_specific *)animvoid;
	//struct ui_counter *counter = animation->parent->self_ui_counter;
	struct ui_counter *counter = animation->parent->self;
	int2array(*counter->value, counter->array, counter->digits);
	for (int i = 0; i < counter->digits; i++) {
		if (animation) {
			if (counter->array < 0) {
				fprintf(stderr, "A UI counter has en*counter*ed (haha) a negative number. That's bad.\n");
				fprintf(stderr, "In %s, line %d\n", __FILE__, __LINE__);
				abort();
			}
			animation->clip = counter->array[i];
			animation = animation->next;
		}
		else {
			break;
		}
	}
}
