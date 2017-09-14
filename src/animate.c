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
extern struct status_struct status;
extern SDL_Texture **image_bank;
struct animate_specific *default_specific_template = NULL;

int renderlist(struct render_node *node_ptr) {
	while (node_ptr != NULL) {
		if (node_ptr->customRenderFunc == NULL){
			SDL_RenderCopy(node_ptr->renderer, node_ptr->img, node_ptr->rect_in, &node_ptr->rect_out);
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
				if (node == render_node_tail) {
					render_node_tail = node_src;
				}
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
}

int node_insert_z_over(struct render_node *node_src, float z) {
	node_src->z = z;
	if (render_node_head == NULL) {
		render_node_head = node_src;
		render_node_tail = node_src;
	}
	else {
		struct render_node *node = render_node_head;
		while (node) {
			if ( node_src->z > node->z) {
				node_src->next = node;
				node_src->prev = node->prev;
				node->prev = node_src;	
				if (node == render_node_head) {
					render_node_head = node_src;
				}
				if (node == render_node_tail) {
					render_node_tail = node_src;
				}
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
			node_ptr_fwd = node_ptr_fwd->next;
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
		}

		if (generic->clips[animation->clip]->frames[animation->frame].duration > 0.0) {
			if (timing.currentbeat - animation->lastFrameBeat >= generic->clips[animation->clip]->frames[animation->frame].duration) {
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
		}
		node_ptr->rect_out = rect_trans;
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
	r_node->rect_out = testspecific->rect_out;
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
	default_specific_template->rules_list = NULL;
	default_specific_template->clip = 0;
	default_specific_template->frame = 0;
	default_specific_template->speed = 1;
	default_specific_template->loops = -1;
	default_specific_template->return_clip = 0;
	default_specific_template->lastFrameBeat = 0.0;
	default_specific_template->native_offset.x = 0;
	default_specific_template->native_offset.y = 0;

	default_specific_template->parent = NULL;
	default_specific_template->transform_list = NULL;//malloc(sizeof(struct func_node));

	default_specific_template->render_node = NULL;
}

struct animate_specific *generate_default_specific(int index) {

	if (!default_specific_template) {
		generate_default_specific_template();
	}

	struct animate_specific *default_specific = malloc(sizeof(struct animate_specific));

	*default_specific = *default_specific_template;

	/*	Trying out some transformation defaults here temporarily.
	 *	Will get it into some kind of loadable animation_default config file eventually.
	 */
	if (index == 0) {
		default_specific->rules_list = malloc(sizeof(struct rule_node));
		default_specific->rules_list->rule = &rules_player;
		default_specific->rules_list->data = NULL;
		default_specific->rules_list->next = NULL;

		struct func_node *tr_node = malloc(sizeof(struct func_node));
		default_specific->transform_list = tr_node;
		struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
		teststruct->rect_bitmask = 1;
		teststruct->freq_perbeat = 1;
		teststruct->ampl_pix = 10.0;
		teststruct->offset = 0.0;
		tr_node->data = (void *)teststruct;
		tr_node->func = &tr_sine;
		tr_node->next = NULL;
	}
	if (index == 2 || index == 3) {
		default_specific->rules_list = malloc(sizeof(struct rule_node));
		default_specific->rules_list->rule = &rules_ui;
		default_specific->rules_list->data = NULL;
		default_specific->rules_list->next = NULL;

		struct func_node *tr_node = malloc(sizeof(struct func_node));

		struct tr_bump_data *teststruct2 = malloc(sizeof(struct tr_bump_data));
		teststruct2->freq_perbeat = 1;
		teststruct2->ampl = 2;
		teststruct2->peak_offset = 0.0;
		teststruct2->bump_width = 0.25;
		teststruct2->centre = NULL;
		tr_node->data = (void *)teststruct2;
		tr_node->func = &tr_bump;
		tr_node->next = NULL;
		default_specific->transform_list = tr_node;
		default_specific->rules_list->data = (void *)teststruct2;
	}
	if (index == 4) {
		default_specific->rules_list = malloc(sizeof(struct rule_node));
		default_specific->rules_list->rule = &rules_ui_bar;
		default_specific->rules_list->data = default_specific;
		default_specific->rules_list->next = malloc(sizeof(struct rule_node));
		default_specific->rules_list->next->rule = &rules_ui;
		default_specific->rules_list->next->data = NULL;
		default_specific->rules_list->next->next = NULL;

		struct func_node *tr_node = malloc(sizeof(struct func_node));

		struct tr_bump_data *teststruct2 = malloc(sizeof(struct tr_bump_data));
		teststruct2->freq_perbeat = 1;
		teststruct2->ampl = 2;
		teststruct2->peak_offset = 0.0;
		teststruct2->bump_width = 0.25;
		teststruct2->centre = NULL;
		tr_node->data = (void *)teststruct2;
		tr_node->func = &tr_bump;
		tr_node->next = NULL;
		default_specific->transform_list = tr_node;
		default_specific->rules_list->next->data = (void *)teststruct2;
	}
	return default_specific;
}

struct animate_specific *generate_specific_anim(struct std *std, struct animate_generic **generic_bank, int index) {
		
	struct animate_specific *specific = malloc(sizeof(struct animate_specific));

	*specific = *generic_bank[index]->default_specific;
	specific->generic = generic_bank[index];
	struct animate_generic *generic = specific->generic;
	
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

	/* set sprite position */
	specific->parent = std;
	specific->rect_out.x = specific->parent->pos.x;
	specific->rect_out.y = specific->parent->pos.y;

	specific->rect_out.w = generic->clips[specific->clip]->frames[specific->frame].rect.w
							*specific->parent->size_ratio.w*ZOOM_MULT*2;
	specific->rect_out.h = generic->clips[specific->clip]->frames[specific->frame].rect.h
							*specific->parent->size_ratio.h*ZOOM_MULT*2;
	/*	You will need to set x and y manually after this.	*/
	
	return specific;
}

int generate_render_node(struct animate_specific *specific, SDL_Renderer *renderer) {
	/* Creates, populates, and inserts a rendering node	*/

	struct animate_generic *generic = specific->generic;

	struct render_node *r_node = create_render_node();
	r_node->rect_in = &generic->clips[specific->clip]->frames[specific->frame].rect;
	r_node->renderer = renderer;
	r_node->img = generic->clips[specific->clip]->img;
	r_node->animation = specific;
	r_node->customRenderFunc = NULL;
	node_insert_z_over(r_node, 0);
	specific->render_node = r_node;

}

int graphic_spawn(struct std *std, struct animate_generic **generic_bank, SDL_Renderer *renderer, int *index_array, int num_index) {

	struct animate_specific **a_ptr = &std->animation;
	struct animate_specific *anim = *a_ptr;
	for (int i = 0; i < num_index; i++) {
		anim = generate_specific_anim(std, generic_bank, index_array[i]);
		*a_ptr = anim;
		generate_render_node(anim, renderer);
		a_ptr = (&(*a_ptr)->next);
	}

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
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}


	config_setting_t *image_list_setting = config_lookup(&cfg, "image_list");
	if (image_list_setting == NULL) {
		printf("Error looking up setting for 'image_list'\n");
	}
	int num_images = config_setting_length(image_list_setting); 
	for (int i = 0; i < num_images; i++) {
		const char *path = config_setting_get_string_elem(image_list_setting, i);
		if (path) {
			image_bank[i] = IMG_LoadTexture(renderer, path);
		}
		else {
			printf("Could not find valid image file path in entry %d of file %s\n", i, cfg_path);
			return -1;
		}
		if (image_bank[i] == NULL) {
			printf("Error loading image file: %s\n", path);
			return -1;
		}
	}
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
	config_destroy(&cfg);
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

}


void rules_player(void *playervoid) {
	struct player_struct *player = (struct player_struct *)playervoid;
	if (player->invincibility_toggle) {
		player->invincibility_toggle = 0;
		if (player->invincibility) {
			struct tr_blink_data *data = malloc(sizeof(struct tr_blink_data));
			data->frames_on = 4;
			data->frames_off = 4;
			transform_add_check(player->animation, data, &tr_blink);
		}
		else {
			transform_rm(player->animation, &tr_blink);
		}
	}
}
void rules_ui(void *data) {
	struct tr_bump_data *str = (struct tr_bump_data *)data;
	if (program.score > 1000)
		str->ampl = 1.2;
	else
		str->ampl = 1 + program.score/5000.0;
}
void rules_ui_bar(void *animvoid) {
	struct animate_specific *animation = (void *)animvoid;
	struct ui_bar *bar = animation->parent->self_ui_bar;
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
