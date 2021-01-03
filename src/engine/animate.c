#include "animate.h"
#include "backend_types.h"
#include "backend_funcs.h"

extern struct shaders_struct shaders;

/* For debugging */

void print_object_list_stack(struct std_list *object_list_stack) {
	struct std_list *object = object_list_stack;
	while (object) {
		printf("%s:		animation=", object->std->name);
		printf("%p\n", object->std->animation);
		object = object->prev;
	}
}

void print_render_list(struct animation_struct *render_node_head) {
	struct animation_struct *node = render_node_head;
	printf("animation render_node_head:	%p\n", render_node_head);
	while (node) {
		printf("%s->", node->object->name);
		node = node->render_next;
	}
	printf("\n");

}

//void test_render_list_robustness(struct std_list *object_list_stack, struct graphical_stage_struct *graphics) {
//	/* TEST that render node list is robust against deletions by deleting whole list every frame */
//	printf("BEFORE list_rm:\n");
//	print_object_list_stack(object_list_stack);
//	print_render_list(graphics->render_node_head);
//
//	render_list_rm(&graphics->render_node_head);
//	graphics->render_node_tail = NULL;
//
//	printf("AFTER list_rm:\n");
//	print_object_list_stack(object_list_stack);
//	print_render_list(graphics->render_node_head);
//}

/* RENDER */

void render_process(struct std_list *object_list_stack, struct graphical_stage_struct *graphics, struct graphics_struct *master_graphics, float currentbeat) {
	query_resize(master_graphics);

	//if (*master_graphics->debug_test_render_list_robustness) {
	//	test_render_list_robustness(object_list_stack, graphics);
	//}

	advance_frames_and_create_render_list(object_list_stack, graphics, currentbeat);

	renderlist(graphics->render_node_head, graphics);

}

int renderlist(struct animation_struct *node_ptr, struct graphical_stage_struct *graphics) {

	//printf("---------------- Calling renderlist() again -------------------\n");
	change_renderer(graphics->renderer);

	int animation_count = 0;
	while (node_ptr != NULL) {
		if (node_ptr->customRenderFunc == NULL){
			//int rc = 0;
			//if (graphics->rendercopyex_data) {
			//	struct rendercopyex_struct *data = graphics->rendercopyex_data;
			//	rc = SDL_RenderCopyEx(master_graphics->graphics.renderer, node_ptr->img, node_ptr->rect_in,
			//						data->dstrect, data->angle, data->center, data->flip);
			//}
			//else {
				render_copy(node_ptr, graphics->renderer);
			//}
			//if (rc != 0) {
			//	printf("%s\n", SDL_GetError());
			//	FILEINFO
			//}
		}
		else {
			(*node_ptr->customRenderFunc)(node_ptr->customRenderArgs);
		}

		if (*graphics->master_graphics->debug_anchors) {
			//int rc;
			//int anchor_width = 6;
			float anchor_width = 0.01;
			//SDL_SetRenderDrawColor(master_graphics->graphics.renderer, 0, 0, 255, 255);
			float colour_blue[4] = {0, 0, 1, 1};
			if (node_ptr->container.anchors_exposed) {
				struct float_rect anchor_exposed_rect = {
					.x = node_ptr->rect_out.x + node_ptr->container.anchors_exposed[0].w * node_ptr->rect_out.w - anchor_width/2,
					.y = node_ptr->rect_out.y + node_ptr->container.anchors_exposed[0].h * node_ptr->rect_out.h - anchor_width/2,
					.w = anchor_width,
					.h = anchor_width,
				};
				draw_box_solid_colour(anchor_exposed_rect, colour_blue);
				//rc = SDL_RenderFillRect(master_graphics->graphics.renderer, &anchor_exposed_rect);
			}
			//SDL_SetRenderDrawColor(master_graphics->graphics.renderer, 255, 0, 0, 255);
			float colour_red[4] = {1, 0, 0, 1};
			struct float_rect anchor_hook_rect = {
				.x = node_ptr->rect_out.x + node_ptr->container.anchor_hook.w * node_ptr->rect_out.w - anchor_width/2,
				.y = node_ptr->rect_out.y + node_ptr->container.anchor_hook.h * node_ptr->rect_out.h - anchor_width/2,
				.w = anchor_width,
				.h = anchor_width,
			};
			//rc = SDL_RenderFillRect(master_graphics->graphics.renderer, &anchor_hook_rect);
			draw_box_solid_colour(anchor_hook_rect, colour_red);
			//if (rc != 0) {
			//	printf("%s\n", SDL_GetError());
			//	FILEINFO
			//}
			//SDL_SetRenderDrawColor(master_graphics->graphics.renderer, 0, 0, 0, 255);
		}

		if (*graphics->master_graphics->debug_containers) {
			//SDL_SetRenderDrawColor(master_graphics->graphics.renderer, 0, 0, 255, 255);
			struct visual_container_struct *container = &node_ptr->container;

			while (container) {
				struct float_rect float_rect = decascade_visual_container(container,
						(struct xy_struct){graphics->master_graphics->width, graphics->master_graphics->height});
				//SDL_Rect abs_container = visual_container_to_pixels(container,
				//		(struct xy_struct){master_graphics->width, master_graphics->height});
				draw_box_lines(float_rect);
										
				//SDL_Point points[5] = {
				//	{abs_container.x, abs_container.y},
				//	{abs_container.x + abs_container.w, abs_container.y},
				//	{abs_container.x + abs_container.w, abs_container.y + abs_container.h},
				//	{abs_container.x, abs_container.y + abs_container.h},
				//	{abs_container.x, abs_container.y},
				//};
				//int rc = SDL_RenderDrawLines(master_graphics->graphics.renderer, points, 5);
				//if (rc != 0) {
				//	printf("%s\n", SDL_GetError());
				//	FILEINFO
				//}
				container = container->inherit;
			}
			//SDL_SetRenderDrawColor(master_graphics->graphics.renderer, 0, 0, 0, 255);
		}


		printf("Rendered %d animation %p\n", animation_count, node_ptr); 
		node_ptr = node_ptr->render_next;
		animation_count++;
	}

	return 0;
}

int node_insert_over(struct graphical_stage_struct *graphics, struct animation_struct *node_src, struct animation_struct *node_dest) {
	if (node_dest == NULL) {
		graphics->render_node_head = node_src;
		graphics->render_node_tail = node_src;
		node_src->z = 0;
	}
	else {
		node_src->render_next = node_dest->render_next;
		node_src->render_prev = node_dest;
		node_dest->render_next = node_src;	
		node_src->z = node_dest->z;
	}
	if (node_dest == graphics->render_node_tail) {
		graphics->render_node_tail = node_src;
	}
	return 0;
}

int node_insert_under(struct graphical_stage_struct *graphics, struct animation_struct *node_src, struct animation_struct *node_dest) {
	if (node_dest == NULL) {
		graphics->render_node_head = node_src;
		graphics->render_node_tail = node_src;
		node_src->z = 0;
	}
	else {
		node_src->render_next = node_dest;
		node_src->render_prev = node_dest->render_prev;
		node_dest->render_prev = node_src;	
		node_src->z = node_dest->z;
	}
	if (node_dest == graphics->render_node_head) {
		graphics->render_node_head = node_src;
	}
	return 0;
}

int node_insert_z_under(struct graphical_stage_struct *graphics, struct animation_struct *node_src) {
	if (graphics->render_node_head == NULL) {
		graphics->render_node_head = node_src;
		graphics->render_node_tail = node_src;
	}
	else {
		struct animation_struct *node = graphics->render_node_head;
		while (node) {
			if ( node_src->z <= node->z) {
				node_src->render_next = node;
				node_src->render_prev = node->render_prev;
				node->render_prev = node_src;	
				if (node == graphics->render_node_head) {
					graphics->render_node_head = node_src;
				}
				//if (node == render_node_tail) {
				//	render_node_tail = node_src;
				//}
				break;
			}		
			if (node == graphics->render_node_tail) {
				node_src->render_prev = graphics->render_node_tail;
				node_src->render_next = NULL;
				graphics->render_node_tail->render_next = node_src;
				graphics->render_node_tail = node_src;
				break;
			}
			node = node->render_next;
		}
	}
	return 0;
}
int node_insert_z_over(struct graphical_stage_struct *graphics, struct animation_struct *node_src) {

	if (graphics->render_node_head == NULL) {
		graphics->render_node_head = node_src;
		graphics->render_node_tail = node_src;
		return 0;
	}

	struct animation_struct *node = graphics->render_node_head;
	int node_count = 0;
	while (node) {
		node_count++;
		if ( node_src->z < node->z) {
			node_src->render_next = node;
			if (node->render_prev) {
				node->render_prev->render_next = node_src;
			}
			node_src->render_prev = node->render_prev;
			node->render_prev = node_src;	
			if (node == graphics->render_node_head) {
				graphics->render_node_head = node_src;
			}
			//if (node == render_node_tail) {
			//	graphics->render_node_tail = node_src;
			//}
			break;
		}		
		if (node == graphics->render_node_tail) {
			node_src->render_prev = graphics->render_node_tail;
			node_src->render_next = NULL;
			graphics->render_node_tail->render_next = node_src;
			graphics->render_node_tail = node_src;
			break;
		}
		node = node->render_next;
	}
	return node_count;
}

int render_node_rm(struct graphical_stage_struct *graphics, struct animation_struct *node_ptr) {
	if (node_ptr->render_prev) {
		node_ptr->render_prev->render_next = node_ptr->render_next;
	} else {
		graphics->render_node_head = node_ptr->render_next;
	}

	if (node_ptr->render_next) {
		node_ptr->render_next->render_prev = node_ptr->render_prev;
	} else {
		graphics->render_node_tail = node_ptr->render_prev;
	}

	node_ptr->render_prev = NULL;
	node_ptr->render_next = NULL;
	//free(node_ptr);
	return 0;
}

//int render_list_rm(struct animation_struct **node_ptr_ptr) {
//	struct animation_struct *node_ptr = *node_ptr_ptr;
//
//	if (node_ptr) {
//		
//		struct render_node *node_ptr_back = node_ptr->render_prev;
//		struct render_node *node_ptr_fwd = node_ptr->render_next;
//		while (node_ptr_back) {
//			struct render_node *ptr_tmp = node_ptr_back;
//			node_ptr_back = node_ptr_back->render_prev;
//			if (ptr_tmp->animation) {
//				ptr_tmp->animation->render_node = NULL;
//			}
//			free(ptr_tmp);
//		}
//	
//		while (node_ptr_fwd) {
//			struct render_node *ptr_tmp = node_ptr_fwd;
//			node_ptr_fwd = node_ptr_fwd->render_next;
//			if (ptr_tmp->animation) {
//				ptr_tmp->animation->render_node = NULL;
//			}
//			free(ptr_tmp);
//		}
//		//node_ptr->animation->render_node = NULL;
//		free(node_ptr);
//	}
//	*node_ptr_ptr = NULL;
//	return 0;
//}

/* ANIMATE */

void advance_frames_and_create_render_list(struct std_list *object_list_stack, struct graphical_stage_struct *graphics, float currentbeat) {

	struct std_list *std_list_node = object_list_stack;
	float z_prev;
	struct animation_struct *render_z_prev = NULL;
	int animation_count = 0;
	graphics->render_node_head = NULL;
	graphics->render_node_tail = NULL;

	while (std_list_node) {
		struct std *object = std_list_node->std;

		struct animation_struct *animation = object->animation;
		
		while (animation) {

			if (animation->animate_mode == GENERIC) { /* (Specific animation controls a generic animation) */
				struct animate_control *control = animation->control;
				struct animate_generic *generic = control->generic;
				/* Set lastFrameBeat on first time */
				if (control->lastFrameBeat == 0 && currentbeat >= 0) {
					control->lastFrameBeat = currentbeat;
				}

				if (generic->clips[control->clip]->frames[control->frame].duration > 0.0) {
					if (currentbeat - control->lastFrameBeat >= generic->clips[control->clip]->frames[control->frame].duration) {
						//control->lastFrameBeat = currentbeat;
						control->lastFrameBeat += generic->clips[control->clip]->frames[control->frame].duration;
						if (control->backwards) {
							control->frame--;
						} else {
							control->frame++;
						}
					}
				}
				if (!control->backwards) {
					if (control->frame >= generic->clips[control->clip]->num_frames) {
						if (control->loops == 0) {
							if (control->return_clip >= 0) {
								/* If return_clip == -1 then just sit on this frame.
								   Otherwise return and loop until further notice (loops=-1). */
								//control->clip = control->return_clip;	
								control->frame = 0;
								control->loops = -1;
							} else {
							control->frame = generic->clips[control->clip]->num_frames - 1;
							}
						} else {
							control->frame = 0;
							if (control->loops > 0) {
								control->loops--;
							}
						}
					}
				} else if (control->backwards) {
					if (control->frame < 0) {
						if (control->loops == 0) {
							if (control->return_clip >= 0) {
								/* If return_clip == -1 then just sit on this frame.
								   Otherwise return and loop until further notice (loops=-1). */
								//control->clip = control->return_clip;	
								control->frame = generic->clips[control->clip]->num_frames - 1;;
								control->loops = -1;
							} else {
							control->frame = 0;
							}
						} else {
							control->frame = generic->clips[control->clip]->num_frames - 1;
							if (control->loops > 0) {
								control->loops--;
							}
						}
					}
				}

			}

			struct visual_container_struct *container = &animation->container;

			struct func_node *transform_node = animation->transform_list;

			/* Tranformations only take effect within the context of this function.
			   Make a backup of the container's coordinates in the container for after the transformation */
			struct float_rect rect_out_parent_scale_pretransform = container->rect_out_parent_scale;

			while (transform_node) {
				transform_node->func((void *)&container->rect_out_parent_scale, transform_node->data);
				transform_node = transform_node->next;
			}

			struct rule_node *rule_node = animation->rules_list;

			while (rule_node) {
				(rule_node->rule)(rule_node->data);
				rule_node = rule_node->next;
			}

			/* Convert from container-scale relative width to global scale absolute width */
			struct float_rect abs_rect = decascade_visual_container(container, (struct xy_struct) {graphics->master_graphics->width, graphics->master_graphics->height});
			//SDL_Rect abs_rect = visual_container_to_pixels(container, (struct xy_struct) {graphics->master_graphics->width, graphics->master_graphics->height});


			/*	Revert the container's rect_out_parent_scale back to the pretransform one for the next frame: */
			container->rect_out_parent_scale = rect_out_parent_scale_pretransform;
			container->screen_scale_uptodate = 1; // Put this in decascade_visual_container?

			if (animation->animate_mode != CONTAINER) { /* Animation is to be displayed on screen */

				if (animation->animate_mode == GENERIC) {
					/* Put the right texture on the animation */
					struct animate_control *control = animation->control;
					struct animate_generic *generic = control->generic;
					animation->img = generic->clips[control->clip]->img;
				}

				/* Create and insert render node if necessary */
				update_render_node(animation);
				//TODO: Remove this:
				if (graphics != &graphics->master_graphics->graphics) {
					animation->uniforms = shader_glow_uniforms;
					animation->shader = shaders.glow_behind;
				} else {
					animation->shader = shaders.simple;
				}


				//if (!(!animation->render_prev && !animation->render_next && 
				//			graphics->render_node_head != animation)) {
				//	render_node_rm(graphics, animation);
				//}
				animation->render_prev = NULL;
				animation->render_next = NULL;
				/* Put render node back in the list in the right place */
				int node_count = node_insert_z_over(graphics, animation);
				//reorder_list_node_z(graphics, animation, &z_prev, &render_z_prev);

				animation->rect_out = abs_rect;
				if (!animation->rect_in) {
					animation->rect_in = malloc(sizeof(*animation->rect_in));
					*animation->rect_in = (struct float_rect) {0, 0, 1, 1};
				}
				update_quad_vertices(*animation->rect_in, animation->rect_out, animation, animation->img_y_convention);
				
			}


			animation = animation->anim_next;
			animation_count++;
		}
		std_list_node = std_list_node->prev;
	}		

	de_update_containers(object_list_stack);
}

void de_update_containers(struct std_list *std_list_node) {
	while (std_list_node) {
		struct animation_struct *animation = std_list_node->std->animation;
		
		while (animation) {
			struct visual_container_struct *container = &animation->container;
			while (container) {
				if (!container->screen_scale_uptodate) {
					break;
				}
				container->screen_scale_uptodate = 0;
				container = container->inherit;
			}
			animation = animation->anim_next;
		}
		std_list_node = std_list_node->prev;
	}
}

struct animation_struct *new_animation(struct std *std, enum animate_mode_e animate_mode, struct animate_generic *generic) {
		
	struct animation_struct *animation = malloc(sizeof(struct animation_struct));

	*animation = (struct animation_struct) {0};
	animation->animate_mode = animate_mode;

	/*	Fetch object-type default values for the animation struct.	*/
	animation->object = std;

	make_anchors_exposed(animation, 1);

	/* Generic animation (provided generic is not NULL (ie this isn't a dummy animation) */
	if (animate_mode == GENERIC) {
		if (!generic) {
			fprintf(stderr, "Requested an animation struct to control a generic animation,\
					but no generic animation was supplied?\n");
			abort();
		}
		animation->control = malloc(sizeof(struct animate_control));
		struct animate_control *control = animation->control;
		*control = (struct animate_control) {0};
		control->generic = generic;
		animation->container.aspctr_lock = generic->clips[control->clip]->aspctr_lock;
		animation->container.rect_out_parent_scale.w = generic->clips[control->clip]->container_scale_factor;
		animation->container.rect_out_parent_scale.h = generic->clips[control->clip]->container_scale_factor;

		/* By default, make the animation's internal relative anchor hook
		 * the same as the one specified by the generic animation */
		struct frame frame = generic->clips[control->clip]->frames[control->frame];

		struct size_ratio_struct anchor_hook_pos_internal = {
			.w = (float)frame.anchor_hook.w,
			.h = (float)frame.anchor_hook.h,
		};
		animation->container.anchor_hook = anchor_hook_pos_internal;
	} else {
		animation->container.aspctr_lock = WH_INDEPENDENT;
		animation->container.rect_out_parent_scale.w = 1;
		animation->container.rect_out_parent_scale.h = 1;

		animation->container.anchor_hook = (struct size_ratio_struct) {0, 0};
	}
		
	/* By default, make the animation's position in the container be determined
	 * not by an anchor but by its std object's "pos" */
	animation->container.anchor_grabbed = NULL;

	animation->z = 0; //TODO Not sure where this should actually be set but 
	//not having this line causes valgrind to complain because this result 
	//gets passed to generate_render_node which uses z for node_insert_z_over

	return animation;
}

int generate_render_node(struct animation_struct *animation, struct graphical_stage_struct *graphics) {
	/* Creates, populates, and inserts a rendering node	*/

	//struct render_node *node = malloc(sizeof(struct render_node));
	//*node = (struct render_node) {0};

	update_render_node(animation);

	//node->animation = animation;

	node_insert_z_over(graphics, animation);

	if (graphics != &graphics->master_graphics->graphics) {
		animation->uniforms = shader_glow_uniforms;
		animation->shader = shaders.glow_behind;
		//update_quad_vertices(*animation->rect_in, animation->rect_out, animation, animation->img_y_convention);
		//TODO: Get border FX working
		//struct animation_struct *border = add_border_vertices(animation, graphics,
		//	shaders.glow, 0.5, 0.5, 0.5, 0.5);
		//border->uniforms = shader_glow_uniforms;
		//border->animation = animation;
		//node_insert_over(graphics, animation);
	} else {
		animation->shader = shaders.simple;
	}

	//animation->render_node = node;

	return 0;
}

int graphic_spawn(struct std *std, struct std_list **object_list_stack_ptr, 
		struct dict_void *generic_anim_dict, 
		struct graphical_stage_struct *graphics, 
		const char* animation_type_array[], int num_anims) {

	std_stack_push(object_list_stack_ptr, std, &std->object_stack_location);

	struct animation_struct **a_ptr = &std->animation;
	struct animation_struct *anim = *a_ptr;
	for (int i = 0; i < num_anims; i++) {
		struct animate_generic *generic = NULL;

		/* Only get generic anim and make render node if
		   animation type is not:
		   - "ser_container" (glorified container), or 
		   - "ser_texture" (animated texture without generic) */

		enum animate_mode_e animate_mode;
		if (strcmp(animation_type_array[i], "ser_container") == 0) {
			animate_mode = CONTAINER;
		} else if (strcmp(animation_type_array[i], "ser_texture") == 0) {
			animate_mode = TEXTURE;
		} else {
			animate_mode = GENERIC;
			generic = dict_void_get_val(generic_anim_dict, animation_type_array[i]);
			if (!generic) {
				fprintf(stderr, "Could not find generic animation for \"%s\". Aborting...\n", animation_type_array[i]);
				FILEINFO
				abort();
			}
		}
		anim = new_animation(std, animate_mode, generic);
		*a_ptr = anim;
		a_ptr = (&(*a_ptr)->anim_next);
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

int dicts_populate(struct dict_void **generic_anim_dict_ptr, struct dict_void **image_dict_ptr, struct status_struct *status, renderer_t renderer) {
	/* Allocates and initialises dicts for generic animations
	   and image textures */

	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "cfg/animations.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		FILEINFO
		config_destroy(&cfg);
		return R_FAILURE;
	}

	config_setting_t *generics_setting = config_lookup(&cfg, "generics");
	if (generics_setting == NULL) {
		printf("Error looking up setting for 'generics'\n");
		FILEINFO
		return R_FAILURE;
	}
	int num_generics = config_setting_length(generics_setting); 

	*generic_anim_dict_ptr = malloc(num_generics * sizeof(struct dict_void));
	struct dict_void *generic_anim_dict = *generic_anim_dict_ptr;

	//*image_dict_ptr = malloc(sizeof(struct dict_void));
	//struct dict_void *image_dict = *image_dict_ptr;

	*generic_anim_dict = (struct dict_void) {0};
	//*image_dict = (struct dict_void) {0};

	struct animate_generic *generic_ptr;
	config_setting_t *generic_setting;
	int num_clips;
	
	struct clip **clip_ptr_array;
	config_setting_t *clips_setting;

	struct clip *clip_ptr;
	config_setting_t *clip_setting;
	
	double container_scale_factor;
	int aspctr_lock;
	int num_frames;

	struct frame *frame_array;
	config_setting_t *frames_setting;
	config_setting_t *frame_setting;
	config_setting_t *rect_setting;
	config_setting_t *anchor_hook_setting = NULL;

	for (int i = 0; i < num_generics; i++) {
		generic_setting = config_setting_get_elem(generics_setting, i);
		if (generic_setting == NULL) {
			printf("Error looking up setting for 'generic'\n");
			FILEINFO
			return R_FAILURE;
		}

		generic_ptr = malloc(sizeof(struct animate_generic));

		const char *graphic_type_dummy;
		if (config_setting_lookup_string(generic_setting, "name", &graphic_type_dummy) == CONFIG_FALSE) {
			printf("Error looking up value for 'graphic_category'\n");
			FILEINFO
			return R_FAILURE;
		}
		int len = strlen(graphic_type_dummy);
		char *graphic_type = malloc(len+1);
		strncpy(graphic_type, graphic_type_dummy, len+1);

		dict_void_add_keyval(generic_anim_dict, graphic_type, generic_ptr);

		const char *graphic_category;
		if (config_setting_lookup_string(generic_setting, "graphic_category", &graphic_category) == CONFIG_FALSE) {
			printf("Error looking up value for 'graphic_category'\n");
			FILEINFO
			return R_FAILURE;
		}

		clips_setting = config_setting_lookup(generic_setting, "clips");
		if (clips_setting == NULL) {
			printf("Error looking up setting for 'clips' %d\n", i);
			FILEINFO
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
				FILEINFO
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
				FILEINFO
				return R_FAILURE;
			}
			int len = strlen(img_dummy);
			char *img_name = malloc(len+1);
			strncpy(img_name, img_dummy, len+1);

			char *path_prefix = "art/";
			int maxnamelen = 50;
			char *path_prefix_long = malloc(sizeof(path_prefix) + maxnamelen);
			strncpy(path_prefix_long, path_prefix, maxnamelen);
			char *path = strncat(path_prefix_long, img_name, maxnamelen);

			texture_t texture;
			int img_w, img_h;
			if (texture_from_path(&texture, &img_w, &img_h, path) != R_SUCCESS) {
				return R_FAILURE;
			}

			//dict_void_add_keyval(image_dict, img_name, texture);

			clip_ptr->img = texture;
			clip_ptr->img_w = img_w;
			clip_ptr->img_h = img_h;

			frames_setting = config_setting_lookup(clip_setting, "frames");
			if (frames_setting == NULL) {
				printf("Error looking up setting for 'frames'\n");
				FILEINFO
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
					FILEINFO
					return R_FAILURE;
				}
				rect_setting = config_setting_lookup(frame_setting, "rect");
				if (rect_setting == NULL) {
					printf("Error looking up setting for 'rect'\n");
					FILEINFO
					return R_FAILURE;
				}
				frame_array[k].rect.x = (float)config_setting_get_int_elem(rect_setting, 0) 
					/ (float)img_w;
				frame_array[k].rect.y = (float)config_setting_get_int_elem(rect_setting, 1) 
					/ (float)img_h;
				frame_array[k].rect.w = (float)config_setting_get_int_elem(rect_setting, 2) 
					/ (float)img_w;
				frame_array[k].rect.h = (float)config_setting_get_int_elem(rect_setting, 3) 
					/ (float)img_h;
				double dub;
				config_setting_lookup_float(frame_setting, "duration", &dub);
				frame_array[k].duration = (float)dub;

				anchor_hook_setting = config_setting_lookup(frame_setting, "anchor_hook");
				if (anchor_hook_setting) {
					frame_array[k].anchor_hook = (struct size_ratio_struct) {
						.w = config_setting_get_float_elem(anchor_hook_setting, 0),
						.h = config_setting_get_float_elem(anchor_hook_setting, 1),
					};
				} else {
					/* Don't worry, it's optional */
					/* By default, make the frame's anchor hook in the centre of the image: */
					//frame_array[k].anchor_hook = (struct size_ratio_struct) {.w=frame_array[k].rect.w/2, .h=frame_array[k].rect.h/2}; //TODO: Actually assign this properly, maybe have it in animation.cfg
					frame_array[k].anchor_hook = (struct size_ratio_struct) {0.5, 0.5}; //TODO: Actually assign this properly, maybe have it in animation.cfg
				//frame_array[k].anchor_hook = (struct size_ratio_struct) { 0, 0 }; //TODO: Actually assign this properly, maybe have it in animation.cfg
				}
			}	
		}
	}

	config_destroy(&cfg);
	return R_SUCCESS;
}


void rules_sword(void *sword_void) {
	struct sword_struct *sword = (struct sword_struct *)sword_void;
	if (sword->swing) {
		sword->std.animation->control->backwards ^= 1; //Toggle
		sword->swing = 0;
	}
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

void rules_explosion(void *data) {
	struct explosion_data_struct *explosion_data = data;
	if (explosion_data->animation->control->loops <= 0) {
		explosion_data->living->alive = -2;
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
	struct animation_struct *animation = (struct animation_struct *)animvoid;
	//struct ui_bar *bar = animation->object->self_ui_bar;
	struct ui_bar *bar = animation->object->self;
	float ratio = (float) *bar->amount / *bar->max;
	animation->container.rect_out_parent_scale.w = ratio;
	if ( ratio <= 0.5 ) {
		if ( ratio <= 0.2 ) {
			animation->control->frame = 2;
		}
		else {
			animation->control->frame = 1;
		}
	}
	else {
		animation->control->frame = 0;
	}
}
void rules_ui_counter(void *animvoid) {
	struct animation_struct *animation = (struct animation_struct *)animvoid;
	//struct ui_counter *counter = animation->object->self_ui_counter;
	struct ui_counter *counter = animation->object->self;
	if (*counter->value > 0) {
		int2array(*counter->value, counter->array, counter->digits);
	} else {
		int2array(0, counter->array, counter->digits);
	}


	animation = animation->anim_next;
	for (int i = 0; i < counter->digits; i++) {
		if (animation) {
			animation->control->clip = counter->array[i];
			animation = animation->anim_next;
		}
		else {
			break;
		}
	}
}

void make_anchors_exposed(struct animation_struct *anim, int n) {

	struct size_ratio_struct *anchors_exposed = malloc(n * sizeof(*anchors_exposed));

	/* By default, make the animation's exposed anchors in the middle */
	   
	for (int i = 0; i < n; i++) {
		anchors_exposed[i] = (struct size_ratio_struct) { 0.5, 0.5 };
	}

	anim->container.anchors_exposed = anchors_exposed;
	anim->container.num_anchors_exposed = n;
}
