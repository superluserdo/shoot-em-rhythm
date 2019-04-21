#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "main.h"
#include "level.h"
#include "music.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h"
#include "structure.h"
#include "dict.h"

/* For debugging */

int node_ptr_count = 0;
int rule_node_count = 0;
int transform_node_count = 0;

int print_object_list_stack(struct std_list *object_list_stack) {
	struct std_list *object = object_list_stack;
	while (object) {
		printf("%s:		rendernode=", object->std->name);
		printf("%p\n", object->std->animation->render_node);
		object = object->prev;
	}
}

int print_render_list(struct render_node *render_node_head) {
	struct render_node *node = render_node_head;
	printf("render_node_head:	%p\n", render_node_head);
	while (node) {
		printf("%s->", node->animation->object->name);
		node = node->next;
	}
	printf("\n");

}

void test_render_list_robustness(struct std_list *object_list_stack, struct graphics_struct *graphics) {
	/* TEST that render node list is robust against deletions by deleting whole list every frame */
	printf("BEFORE list_rm:\n");
	print_object_list_stack(object_list_stack);
	print_render_list(graphics->render_node_head);

	render_list_rm(&graphics->render_node_head);
	graphics->render_node_tail = NULL;

	printf("AFTER list_rm:\n");
	print_object_list_stack(object_list_stack);
	print_render_list(graphics->render_node_head);
}

/* RENDER */

void render_process(struct std_list *object_list_stack, struct graphics_struct *graphics, struct time_struct *timing) {
	advance_frames_and_create_render_list(object_list_stack, graphics, timing->currentbeat);
	renderlist(graphics->render_node_head, graphics);

	if (*graphics->debug_test_render_list_robustness) {
		test_render_list_robustness(object_list_stack, graphics);
	}

	SDL_SetRenderTarget(graphics->renderer, NULL);
	SDL_RenderClear(graphics->renderer);

	if (graphics->rendercopyex_data) {
		struct rendercopyex_struct *data = graphics->rendercopyex_data;
		SDL_RenderCopyEx(data->renderer, data->texture, data->srcrect, 
						data->dstrect, data->angle, data->center, data->flip);
	}
	else {
		SDL_RenderCopy(graphics->renderer, graphics->texTarget, NULL, NULL);
	}
	SDL_Delay(wait_to_present(timing));
	SDL_RenderPresent(graphics->renderer);
	update_time(timing);

	SDL_SetRenderTarget(graphics->renderer, graphics->texTarget);
}

int renderlist(struct render_node *node_ptr, struct graphics_struct *graphics) {
	while (node_ptr != NULL) {
		if (node_ptr->customRenderFunc == NULL){
			int rc = SDL_RenderCopy(node_ptr->renderer, node_ptr->img, node_ptr->rect_in, &node_ptr->rect_out);
			if (rc != 0) {
				printf("%s\n", SDL_GetError());
				FILEINFO
			}
		}
		else {
			(*node_ptr->customRenderFunc)(node_ptr->customRenderArgs);
		}
		if (*graphics->debug_anchors) {
			int rc;
			int anchor_width = 6;
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 255, 255);
			if (node_ptr->animation->container.anchors_exposed) {
				SDL_Rect anchor_exposed_rect = {
					.x = node_ptr->rect_out.x + node_ptr->animation->container.anchors_exposed[0].w * node_ptr->rect_out.w - anchor_width/2,
					.y = node_ptr->rect_out.y + node_ptr->animation->container.anchors_exposed[0].h * node_ptr->rect_out.h - anchor_width/2,
					.w = anchor_width,
					.h = anchor_width,
				};
				rc = SDL_RenderFillRect(node_ptr->renderer, &anchor_exposed_rect);
			}
			SDL_SetRenderDrawColor(node_ptr->renderer, 255, 0, 0, 255);
			SDL_Rect anchor_hook_rect = {
				.x = node_ptr->rect_out.x + node_ptr->animation->container.anchor_hook.w * node_ptr->rect_out.w - anchor_width/2,
				.y = node_ptr->rect_out.y + node_ptr->animation->container.anchor_hook.h * node_ptr->rect_out.h - anchor_width/2,
				.w = anchor_width,
				.h = anchor_width,
			};
			rc = SDL_RenderFillRect(node_ptr->renderer, &anchor_hook_rect);
			if (rc != 0) {
				printf("%s\n", SDL_GetError());
			}
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 0, 255);
		}
		if (*graphics->debug_containers) {
			SDL_SetRenderDrawColor(node_ptr->renderer, 0, 0, 255, 255);
			int line_width = 6;
			struct visual_container_struct *container = &node_ptr->animation->container;

			while (container) {
				SDL_Rect abs_container = visual_container_to_pixels(container,
										(struct xy_struct){graphics->width, graphics->height});
				SDL_Point points[5] = {
					{abs_container.x, abs_container.y},
					{abs_container.x + abs_container.w, abs_container.y},
					{abs_container.x + abs_container.w, abs_container.y + abs_container.h},
					{abs_container.x, abs_container.y + abs_container.h},
					{abs_container.x, abs_container.y},
				};
				int rc = SDL_RenderDrawLines(node_ptr->renderer, points, 5);
				if (rc != 0) {
					printf("%s\n", SDL_GetError());
				}
				container = container->inherit;
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
				if (node->prev) {
					node->prev->next = node_src;
				}
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

int render_node_rm(struct graphics_struct *graphics, struct render_node *node_ptr) {
	if (node_ptr->prev) {
		node_ptr->prev->next = node_ptr->next;
	}
	else {
		graphics->render_node_head = node_ptr->next;
	}

	if (node_ptr->next) {
		node_ptr->next->prev = node_ptr->prev;
	}
	else {
		graphics->render_node_tail = node_ptr->prev;
	}
	free(node_ptr);
	return 0;
}

int render_list_rm(struct render_node **node_ptr_ptr) {
	struct render_node *node_ptr = *node_ptr_ptr;

	if (node_ptr) {
		
		struct render_node *node_ptr_back = node_ptr->prev;
		struct render_node *node_ptr_fwd = node_ptr->next;
		while (node_ptr_back) {
			struct render_node *ptr_tmp = node_ptr_back;
			node_ptr_back = node_ptr_back->prev;
			if (ptr_tmp->animation) {
				ptr_tmp->animation->render_node = NULL;
			}
			free(ptr_tmp);
		}
	
		while (node_ptr_fwd) {
			struct render_node *ptr_tmp = node_ptr_fwd;
			node_ptr_fwd = node_ptr_fwd->next;
			if (ptr_tmp->animation) {
				ptr_tmp->animation->render_node = NULL;
			}
			free(ptr_tmp);
		}
		node_ptr->animation->render_node = NULL;
		free(node_ptr);
	}
	*node_ptr_ptr = NULL;
	return 0;
}

struct render_node *create_render_node() {
	struct render_node *new_node = malloc(sizeof(struct render_node));
	if (new_node == NULL)
		printf("Error creating new rendering node :(\n");
	new_node->prev = NULL;
	new_node->next = NULL;
	new_node->rect_in = NULL;
	new_node->rect_out = (struct SDL_Rect) {0};
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

void advance_frames_and_create_render_list(struct std_list *object_list_stack, struct graphics_struct *graphics, float currentbeat) {

	struct std_list *std_list_node = object_list_stack;
	int std_list_node_count = 0;
	int render_node_count = 0;
	while (std_list_node) {
		struct std *object = std_list_node->std;

		struct animate_specific *animation = object->animation;
		
		while (animation) {
			struct animate_generic *generic = animation->generic;

			if (generic) { /* (Animation is not a dummy) */
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
			struct frame frame = generic->clips[animation->clip]->frames[animation->frame];

			}

			struct visual_container_struct *container = &animation->container;

			struct func_node *transform_node = animation->transform_list;

			/* Tranformations only take effect within the context of this function.
			   Make a backup of the container's coordinates in the container for after the transformation */
			struct float_rect rect_out_parent_scale_pretransform = container->rect_out_parent_scale;

			while (transform_node) {
				transform_node->func((void *)&container->rect_out_parent_scale, transform_node->data);
				transform_node = transform_node->next;
				transform_node_count++;
			}

			struct rule_node *rule_node = animation->rules_list;

			while (rule_node) {
				(rule_node->rule)(rule_node->data);
				rule_node = rule_node->next;
				rule_node_count++;
			}

			/* Convert from container-scale relative width to global scale absolute width */
			SDL_Rect abs_rect = visual_container_to_pixels(container, (struct xy_struct) {graphics->width, graphics->height});

			/*	Revert the container's rect_out_parent_scale back to the pretransform one for the next frame: */
			container->rect_out_parent_scale = rect_out_parent_scale_pretransform;

			if (generic) {
				struct render_node *render_node = animation->render_node;
				if (!render_node) {
					generate_render_node(animation, graphics);
					render_node = animation->render_node;
				}
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

				render_node->rect_out = abs_rect;
				//render_node = render_node->next;
				render_node_count++;
			}

			animation = animation->next;
		}
		std_list_node = std_list_node->prev;
		std_list_node_count++;
	}		

	de_update_containers(object_list_stack);
}

void de_update_containers(struct std_list *std_list_node) {
	while (std_list_node) {
		struct animate_specific *animation = std_list_node->std->animation;
		
		while (animation) {
			struct visual_container_struct *container = &animation->container;
			while (container) {
				if (!container->screen_scale_uptodate) {
					break;
				}
				container->screen_scale_uptodate = 0;
				container = container->inherit;
			}
			animation = animation->next;
		}
		std_list_node = std_list_node->prev;
	}
}

struct animate_specific *new_specific_anim(struct std *std, struct animate_generic *generic) {
		
	struct animate_specific *specific = malloc(sizeof(struct animate_specific));

	*specific = (struct animate_specific) {0};

	/*	Fetch object-type default values for the specific-animation struct.	*/
	specific->object = std;

	make_anchors_exposed(specific, 1);
	specific->generic = generic;

	/* Generic animation (provided generic is not NULL (ie this isn't a dummy animation) */
	if (generic) {
		specific->container.aspctr_lock = generic->clips[specific->clip]->aspctr_lock;
		specific->container.rect_out_parent_scale.w = generic->clips[specific->clip]->container_scale_factor;
		specific->container.rect_out_parent_scale.h = generic->clips[specific->clip]->container_scale_factor;
		
		
		/* By default, make the animation's position in the container be determined
		 * not by an anchor but by its std object's "pos" */
		specific->container.anchor_grabbed = NULL;

		/* By default, make the animation's internal relative anchor hook
		 * the same as the one specified by the generic animation */
		struct frame frame = generic->clips[specific->clip]->frames[specific->frame];

		struct size_ratio_struct anchor_hook_pos_internal = {
			.w = (float)frame.anchor_hook.x / (float)frame.rect.w,
			.h = (float)frame.anchor_hook.y / (float)frame.rect.h,
		};
		specific->container.anchor_hook = anchor_hook_pos_internal;
	}

	specific->z = 0; //TODO Not sure where this should actually be set but not having this line causes valgrind to complain because this result gets passed to generate_render_node which uses z for node_insert_z_over
	return specific;
}

int generate_render_node(struct animate_specific *specific, struct graphics_struct *graphics) {
	/* Creates, populates, and inserts a rendering node	*/

	struct animate_generic *generic = specific->generic;

	struct render_node *r_node = malloc(sizeof(struct render_node));
	*r_node = (struct render_node) {0};
	r_node->rect_in = &generic->clips[specific->clip]->frames[specific->frame].rect;
	r_node->renderer = graphics->renderer;
	r_node->img = generic->clips[specific->clip]->img;
	r_node->animation = specific;
	r_node->customRenderFunc = NULL;
	node_insert_z_over(graphics, r_node, specific->z);
	specific->render_node = r_node;

	return 0;
}

int graphic_spawn(struct std *std, struct std_list **object_list_stack_ptr, struct dict_str_void *generic_anim_dict, struct graphics_struct *graphics, const char* specific_type_array[], int num_specific_anims) {

	std_stack_push(object_list_stack_ptr, std, &std->object_stack_location);

	struct animate_specific **a_ptr = &std->animation;
	struct animate_specific *anim = *a_ptr;
	for (int i = 0; i < num_specific_anims; i++) {
		struct animate_generic *generic = NULL;
		/* Only get generic anim and make render node if
		    animation type is not "none" (glorified container) */
		if (strcmp(specific_type_array[i], "none") != 0) {
			generic = dict_get_val(generic_anim_dict, specific_type_array[i]);
			if (!generic) {
				fprintf(stderr, "Could not find generic animation for \"%s\". Aborting...\n", specific_type_array[i]);
				abort();
			}
			//anim = generate_specific_anim(std, generic);
		}
		anim = new_specific_anim(std, generic);
		*a_ptr = anim;
		if (generic) {
			generate_render_node(anim, graphics);
		}
		a_ptr = (&(*a_ptr)->next);
	}
	// Make final "next" ptr NULL to prevent segfaults
	*a_ptr = NULL;

	/* If the object didn't specify its own container, just the first
	   animation's container as the object's container */
	if (!std->container) {
		std->container = &std->animation->container;
	} else {
	/* Otherwise, have the animation inherit from the object: */
		anim->container.inherit = std->container;
	}

	return 0;
}

int dicts_populate(struct dict_str_void **generic_anim_dict_ptr, struct dict_str_void **image_dict_ptr, struct status_struct *status, SDL_Renderer *renderer) {
	/* Allocates and initialises dicts for generic animations
	   and image textures */

	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "animations.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return R_FAILURE;
	}

	config_setting_t *generics_setting = config_lookup(&cfg, "generics");
	if (generics_setting == NULL) {
		printf("Error looking up setting for 'generics'\n");
		return R_FAILURE;
	}
	int num_generics = config_setting_length(generics_setting); 

	*generic_anim_dict_ptr = malloc(num_generics * sizeof(struct dict_str_void));
	struct dict_str_void *generic_anim_dict = *generic_anim_dict_ptr;

	*image_dict_ptr = malloc(num_generics * sizeof(struct dict_str_void));
	struct dict_str_void *image_dict = *image_dict_ptr;

	*generic_anim_dict = (struct dict_str_void) {0};
	*image_dict = (struct dict_str_void) {0};

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
	int aspctr_lock;
	int num_frames;

	struct frame *frame_array;
	config_setting_t *frames_setting;
	config_setting_t *frame_setting;
	config_setting_t *rect_setting;

	for (int i = 0; i < num_generics; i++) {
		generic_setting = config_setting_get_elem(generics_setting, i);
		if (generic_setting == NULL) {
			printf("Error looking up setting for 'generic'\n");
			return R_FAILURE;
		}

		generic_ptr = malloc(sizeof(struct animate_generic));

		const char *graphic_type_dummy;
		if (config_setting_lookup_string(generic_setting, "name", &graphic_type_dummy) == CONFIG_FALSE) {
			printf("Error looking up value for 'graphic_category'\n");
			return R_FAILURE;
		}
		int len = strlen(graphic_type_dummy);
		char *graphic_type = malloc(len+1);
		strncpy(graphic_type, graphic_type_dummy, len+1);

		int dict_index = dict_add_keyval(generic_anim_dict, graphic_type, generic_ptr);

		const char *graphic_category;
		if (config_setting_lookup_string(generic_setting, "graphic_category", &graphic_category) == CONFIG_FALSE) {
			printf("Error looking up value for 'graphic_category'\n");
			return R_FAILURE;
		}

		clips_setting = config_setting_lookup(generic_setting, "clips");
		if (clips_setting == NULL) {
			printf("Error looking up setting for 'clips' %d\n", i);
			return R_FAILURE;
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
				return R_FAILURE;
			}

			if (config_setting_lookup_float(clip_setting, "container_scale_factor", &container_scale_factor) == CONFIG_FALSE) {
				clip_ptr->container_scale_factor = 1.0;
			} else {
				clip_ptr->container_scale_factor = container_scale_factor;
			}
			if (config_setting_lookup_int(clip_setting, "aspctr_lock", &aspctr_lock) == CONFIG_FALSE) {
				clip_ptr->aspctr_lock = H_DOMINANT;
			} else {
				clip_ptr->aspctr_lock = aspctr_lock;
			}

			const char *img_dummy;
			if (config_setting_lookup_string(clip_setting, "img", &img_dummy) == CONFIG_FALSE) {
				printf("Error looking up value for 'img'\n");
				return R_FAILURE;
			}
			int len = strlen(img_dummy);
			char *img_name = malloc(len+1);
			strncpy(img_name, img_dummy, len+1);

			SDL_Texture *texture = NULL;
			char *path_prefix = "../art/";
			int maxnamelen = 50;
			char *path_prefix_long = malloc(sizeof(path_prefix) + maxnamelen);
			strncpy(path_prefix_long, path_prefix, maxnamelen);
			char *path = strncat(path_prefix_long, img_name, maxnamelen);
			if (path) {
				texture = IMG_LoadTexture(renderer, path);
			}
			else {
				printf("Could not find valid image file \"%s\"\n", path);
				FILEINFO
				return R_FAILURE;
			}
			if (!texture) {
				printf("Error loading image file: \"%s\"\n", path);
				FILEINFO
				return R_FAILURE;
			}

			int dict_index = dict_add_keyval(image_dict, img_name, texture);

			clip_ptr->img = texture;

			frames_setting = config_setting_lookup(clip_setting, "frames");
			if (frames_setting == NULL) {
				printf("Error looking up setting for 'frames'\n");
				return R_FAILURE;
			}
			num_frames = config_setting_length(frames_setting);
			clip_ptr->num_frames = num_frames;

			frame_array = malloc(num_frames * sizeof(struct frame));
			clip_ptr->frames = frame_array;

			for (int k = 0; k < num_frames; k++) {
				frame_setting = config_setting_get_elem(frames_setting, k);
				if (frame_setting == NULL) {
					printf("Error looking up setting for 'frame'\n");
					return R_FAILURE;
				}
				rect_setting = config_setting_lookup(frame_setting, "rect");
				if (rect_setting == NULL) {
					printf("Error looking up setting for 'rect'\n");
					return R_FAILURE;
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

	config_destroy(&cfg);
	return R_SUCCESS;
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
			data->framecount = &status->timing->framecount;
			transform_add_check(player->animation, data, &tr_blink);
		}
		else {
			transform_rm(player->animation, &tr_blink);
		}
	}
}
void rules_ui(void *data) {
	struct tr_bump_data *str = (struct tr_bump_data *)data;
	if (*str->score > 300)
		str->ampl = 1.5;
	else
		str->ampl = 1.2 + (float)(*str->score)/1000.0;
}
void rules_ui_bar(void *animvoid) {
	struct animate_specific *animation = (struct animate_specific *)animvoid;
	//struct ui_bar *bar = animation->object->self_ui_bar;
	struct ui_bar *bar = animation->object->self;
	float ratio = (float) *bar->amount / *bar->max;
	animation->container.rect_out_parent_scale.w = ratio;
	if ( ratio <= 0.5 ) {
		if ( ratio <= 0.2 ) {
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
	//struct ui_counter *counter = animation->object->self_ui_counter;
	struct ui_counter *counter = animation->object->self;
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

struct anchor_struct *make_anchors_exposed(struct animate_specific *anim, int n) {

	struct size_ratio_struct *anchors_exposed = malloc(n * sizeof(*anchors_exposed));

	/* By default, make the animation's exposed anchors in the middle */
	   
	for (int i = 0; i < n; i++) {
		anchors_exposed[i] = (struct size_ratio_struct) { 0.5, 0.5 };
	}

	anim->container.anchors_exposed = anchors_exposed;
	anim->container.num_anchors_exposed = n;
}

