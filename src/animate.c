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
#include "transform.h"

/* RENDER */
extern SDL_Texture **image_bank;
struct animate_specific *default_specific_template = NULL;

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
		}
		*node_ptr->rect_out = rect_trans;
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
	teststruct->ampl_pix = 10.0;
	teststruct->offset = 0.0;
	testfunc->data = (void *)teststruct;
	testfunc->func = &tr_sine;
	testfunc->next = malloc(sizeof(struct func_node));

	struct tr_sine_data *teststruct2 = malloc(sizeof(struct tr_sine_data));
	teststruct2->rect_bitmask = 2;
	teststruct2->freq_perbeat = 1;
	teststruct2->ampl_pix = 10.0;
	teststruct2->offset = 0.25;
	testfunc->next->data = (void *)teststruct2;
	testfunc->next->func = &tr_sine;
	testfunc->next->next = NULL;


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

int generate_default_specific_template() {
	
	default_specific_template = malloc(sizeof(struct animate_specific));

	default_specific_template->generic = NULL;
	default_specific_template->animate_rules = NULL;
	default_specific_template->clip = 0;
	default_specific_template->frame = 0;
	default_specific_template->speed = 1;
	default_specific_template->loops = -1;
	default_specific_template->return_clip = 0;
	default_specific_template->lastFrameBeat = 0.0;

	default_specific_template->size_ratio.w = 1.0;
	default_specific_template->size_ratio.h = 1.0;
	default_specific_template->transform_list = NULL;//malloc(sizeof(struct func_node));

	default_specific_template->render_node = NULL;
}

struct animate_specific *generate_default_specific(int index) {

	if (!default_specific_template) {
			printf("hi\n");
		generate_default_specific_template();
	}

	struct animate_specific *default_specific = malloc(sizeof(struct animate_specific));

	*default_specific = *default_specific_template;

	if (index == 0) {
		default_specific->transform_list = malloc(sizeof(struct func_node));

		struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
		teststruct->rect_bitmask = 1;
		teststruct->freq_perbeat = 1;
		teststruct->ampl_pix = 10.0;
		teststruct->offset = 0.0;
		default_specific->transform_list->data = (void *)teststruct;
		default_specific->transform_list->func = &tr_sine;
		default_specific->transform_list->next = NULL;
	}
	return default_specific;
}

struct animate_specific *generate_specific_anim(struct animate_generic **generic_bank) {
		
	struct animate_specific *specific = malloc(sizeof(struct animate_specific));

	*specific = *generic_bank[0]->default_specific;
	specific->generic = generic_bank[0];
	struct animate_generic *generic = specific->generic;
	
	/*	Fetch object-type default values for the specific-animation struct.	*/

	/* set sprite position */
	specific->rect_out.w = generic->clips[specific->clip]->frames[specific->frame].rect.w
							*specific->size_ratio.w*ZOOM_MULT*2;
	specific->rect_out.h = generic->clips[specific->clip]->frames[specific->frame].rect.h
							*specific->size_ratio.h*ZOOM_MULT*2;
	/*	You will need to set x and y manually after this.	*/
	
//	struct func_node *testfunc = specific->transform_list;
//
//	struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
//	teststruct->rect_bitmask = 1;
//	teststruct->freq_perbeat = 1;
//	teststruct->ampl_pix = 10.0;
//	teststruct->offset = 0.0;
//	testfunc->data = (void *)teststruct;
//	testfunc->func = &tr_sine;
//	testfunc->next = malloc(sizeof(struct func_node));
//
//	struct tr_sine_data *teststruct2 = malloc(sizeof(struct tr_sine_data));
//	teststruct2->rect_bitmask = 2;
//	teststruct2->freq_perbeat = 1;
//	teststruct2->ampl_pix = 10.0;
//	teststruct2->offset = 0.25;
//	testfunc->next->data = (void *)teststruct2;
//	testfunc->next->func = &tr_sine;
//	testfunc->next->next = NULL;
	return specific;
}

int generate_render_node(struct animate_specific *specific, SDL_Renderer *renderer) {
	/* Creates, populates, and inserts a rendering node	*/

	struct animate_generic *generic = specific->generic;

	struct render_node *r_node = create_render_node();
	r_node->rect_in = &generic->clips[specific->clip]->frames[specific->frame].rect;
	r_node->rect_out = &specific->rect_out;
	r_node->renderer = renderer;
	r_node->img = generic->clips[specific->clip]->img;
	r_node->animation = specific;
	r_node->customRenderFunc = NULL;
	node_insert_z_over(r_node, 0);
	specific->render_node = r_node;

}

int graphic_spawn(struct animate_specific **specific_ptr, struct animate_generic **generic_bank, SDL_Renderer *renderer) {

	*specific_ptr = generate_specific_anim(generic_bank);
	generate_render_node(*specific_ptr, renderer);

}

int generic_bank_populate(struct animate_generic ***generic_bank_ptr, SDL_Texture **image_bank) {

	struct animate_generic **generic_bank;

	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "animations.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}

	config_setting_t *generics_setting = config_lookup(&cfg, "generics");
	if (generics_setting == NULL) {
		printf("Error looking up setting for 'generics'\n");
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
	int num_frames;

	struct frame *frame_array;
	config_setting_t *frames_setting;
	config_setting_t *frame_setting;
	config_setting_t *rect_setting;

	for (int i = 0; i < num_generics; i++) {
		generic_setting = config_setting_get_elem(generics_setting, i);
		if (generic_setting == NULL) {
			printf("Error looking up setting for 'generic'\n");
		}
		generic_ptr = malloc(sizeof(struct animate_generic));

		generic_bank[i] = generic_ptr;

		generic_ptr->default_specific = generate_default_specific(i);

		clips_setting = config_setting_lookup(generic_setting, "clips");
		if (clips_setting == NULL) {
			printf("Error looking up setting for 'clips' %d\n", i);
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
			}
			if (config_setting_lookup_int(clip_setting, "img", &img_index) == CONFIG_FALSE) {
				printf("Error looking up value for 'img'\n");
			}
			clip_ptr->img = image_bank[img_index];
			frames_setting = config_setting_lookup(clip_setting, "frames");
			if (frames_setting == NULL) {
				printf("Error looking up setting for 'frames'\n");
			}
			num_frames = config_setting_length(frames_setting);
			clip_ptr->num_frames = num_frames;

			frame_array = malloc(num_frames * sizeof(struct frame));
			clip_ptr->frames = frame_array;

			for (int k = 0; k < num_frames; k++) {
				frame_setting = config_setting_get_elem(frames_setting, k);
				if (frame_setting == NULL) {
					printf("Error looking up setting for 'frame'\n");
				}
				rect_setting = config_setting_lookup(frame_setting, "rect");
				if (rect_setting == NULL) {
					printf("Error looking up setting for 'rect'\n");
				}
				frame_array[k].rect.x = config_setting_get_int_elem(rect_setting, 0);
				frame_array[k].rect.y = config_setting_get_int_elem(rect_setting, 1);
				frame_array[k].rect.w = config_setting_get_int_elem(rect_setting, 2);
				frame_array[k].rect.h = config_setting_get_int_elem(rect_setting, 3);
				double dub;
				config_setting_lookup_float(frame_setting, "duration", &dub);
				frame_array[k].duration = (float)dub;
			}	
		}
	}

	*generic_bank_ptr = generic_bank;

//	config_setting_t *objects_setting = config_lookup(&cfg, "objects");
//	int objects_count = config_setting_length(objects_setting); 
//	config_setting_t *object_setting = config_setting_get_elem(all_setting, 0);
//	config_setting_t *timing_setting = config_setting_lookup(thislevel_setting, "timing");
//	if (timing_setting == NULL) {
//		printf("No settings found under 'timing' in the config file\n");
//		return -1;
//	}
}
