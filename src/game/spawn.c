#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef_game.h"
#include "main.h"
#include "level.h"
#include "clock.h"
#include "helpers.h"
#include "animate.h"
#include "transform.h" // Can hopefully get rid of this soon
#include "structure.h"
#include "spawn.h"
#include "object_logic.h"
#include "dict.h"
#include "backend_funcs.h"

void prepare_anim_character(struct animation_struct *anim, struct status_struct *status) {
	anim->rules_list = malloc(sizeof(struct rule_node));
	anim->rules_list->rule = &rules_player;
	anim->rules_list->data = (void *)status;
	anim->rules_list->next = NULL;

	struct func_node *tr_node = malloc(sizeof(struct func_node));
	anim->transform_list = tr_node;
	struct tr_sine_data *sine_data = malloc(sizeof(struct tr_sine_data));
	sine_data->currentbeat = &status->timing->currentbeat;
	sine_data->rect_bitmask = 2;
	sine_data->freq_perbeat = 0.5;
	sine_data->ampl = 0.2;
	sine_data->offset = 0.0;
	tr_node->data = (void *)sine_data;
	tr_node->func = &tr_sine_abs;
	tr_node->next = NULL;

}

void prepare_anim_ui_bar(struct animation_struct *anim, struct status_struct *status) {

	prepare_anim_ui(anim, status);

	anim->anim_next->anim_next->rules_list = malloc(sizeof(struct rule_node));
	anim->anim_next->anim_next->rules_list->rule = &rules_ui_bar;
	anim->anim_next->anim_next->rules_list->data = anim->anim_next->anim_next;
	anim->anim_next->anim_next->rules_list->next = NULL;

}

void prepare_anim_ui(struct animation_struct *anim, struct status_struct *status) {

	anim->rules_list = malloc(sizeof(struct rule_node));
	anim->rules_list->rule = &rules_ui;
	anim->rules_list->data = NULL;
	anim->rules_list->next = NULL;
	struct func_node *tr_node = malloc(sizeof(struct func_node));

	struct tr_bump_data *bumpdata = malloc(sizeof(struct tr_bump_data));
	bumpdata->score = &status->level->score;
	bumpdata->currentbeat = &status->timing->currentbeat;
	bumpdata->freq_perbeat = 1;
	bumpdata->ampl = 2;
	bumpdata->peak_offset = 0.0;
	bumpdata->bump_width = 0.25;
	bumpdata->centre = NULL;
	tr_node->data = (void *)bumpdata;
	tr_node->func = &tr_bump;
	tr_node->next = NULL;
	anim->transform_list = tr_node;
	anim->rules_list->data = (void *)bumpdata;
}

void spawn_player(struct std_list **object_list_stack_ptr,
					struct dict_void *generic_anim_dict,
					struct graphical_stage_struct *graphics,
					struct player_struct *player,
					struct visual_container_struct *container, 
					struct status_struct *status, 
					config_setting_t *player_setting,
				   	const char *name) {

	player->sword = 1;
	player->flydir = 1;
	player->self = player;

	player->name = "player";
	graphic_spawn(&player->std, object_list_stack_ptr, generic_anim_dict, graphics, (const char *[]){"player"}, 1);

	*player->container = *container;
	player->container->anchors_exposed = malloc(1 * sizeof(container->anchors_exposed[0]));
	player->container->anchors_exposed[0] = (struct size_ratio_struct) {.w = 1.0, .h = 0.4}; //Hand holding the weapon

	prepare_anim_character(player->animation, status);

	config_setting_lookup_int(player_setting, "max_HP", &player->living.max_HP);
	player->living.HP = player->living.max_HP;
	player->living.iframes_duration = 2.0;

	config_setting_lookup_int(player_setting, "max_PP", &player->living.max_PP);
	player->living.power = player->living.max_PP;

	player->object_logic = object_logic_player;
	player->object_data = status;

}

void spawn_sword(struct std_list **object_list_stack_ptr,
					struct dict_void *generic_anim_dict,
					struct graphical_stage_struct *graphics,
					struct player_struct *player,
					struct sword_struct *sword,
					struct visual_container_struct *container, 
					struct status_struct *status, 
				   	const char *name) {

	sword->self = sword;
	sword->power = 10000;
	sword->down = 0;
	sword->swing = 0;
	sword->swing_up_duration = 0.5;
	sword->swing_down_duration = 0.5;

	sword->name = "sword";
	graphic_spawn(&sword->std, object_list_stack_ptr, generic_anim_dict, graphics, (const char *[]){"sword"}, 1);

	sword->animation->control->return_clip = -1;
	sword->animation->control->backwards = 1;
	sword->animation->rules_list = malloc(sizeof(*sword->animation->rules_list));
	sword->animation->rules_list->rule = &rules_sword;
	sword->animation->rules_list->data = (void *) sword;
	sword->animation->rules_list->next = NULL;

	*sword->container = *container;

	sword->object_logic = object_logic_sword;
	sword->object_data = status;
	sword->dist_to_monster_spawn = 0.5;//TODO
}

struct ui_bar *spawn_ui_bar(struct std_list **object_list_stack_ptr,
							struct dict_void *generic_anim_dict,
							struct graphical_stage_struct *graphics,
							int *bar_amount_ptr, int *bar_max_ptr,
							struct visual_container_struct *container, 
							struct status_struct *status, const char *name) {

	struct ui_bar *bar = malloc(sizeof(*bar));
	*bar = (struct ui_bar) {0};
	bar->container = container;
	bar->amount = bar_amount_ptr;
	bar->max = bar_max_ptr;
	bar->name = name;
	bar->self = bar;
	graphic_spawn(&bar->std, object_list_stack_ptr, generic_anim_dict, graphics, (const char *[]){"hp", "ser_container", "coloured bar"}, 3);

	struct animation_struct *anim = bar->animation;

	prepare_anim_ui_bar(anim, status);

	anim->container = *container;
	anim->container.anchors_exposed = malloc(sizeof(*bar->animation->container.anchors_exposed));
	anim->container.num_anchors_exposed = 1;
	anim->container.anchors_exposed[0] = (struct size_ratio_struct) {0.29, 0.52};

	anim->anim_next->container.inherit = &anim->container;
	anim->anim_next->container.anchor_hook = (struct size_ratio_struct) {0, 0.5};
	anim->anim_next->container.anchor_grabbed = &anim->container.anchors_exposed[0];
	anim->anim_next->container.aspctr_lock = WH_INDEPENDENT;
	anim->anim_next->container.rect_out_parent_scale.w = 0.5;
	anim->anim_next->container.rect_out_parent_scale.h = 0.40;
	anim->anim_next->container.anchors_exposed[0] = (struct size_ratio_struct) {0, 0.5};

	anim->anim_next->anim_next->container.inherit = &anim->anim_next->container;
	anim->anim_next->anim_next->container.anchor_grabbed = &anim->anim_next->container.anchors_exposed[0];
	anim->anim_next->anim_next->container.aspctr_lock = WH_INDEPENDENT;
	anim->anim_next->anim_next->container.anchor_hook = (struct size_ratio_struct) {0, 0.5};
	anim->anim_next->anim_next->container.rect_out_parent_scale = 
		(struct float_rect) {.w = 1, .h = 1};

	return bar;
}

struct ui_counter *spawn_ui_counter(struct std_list **object_list_stack_ptr,
									struct dict_void *generic_anim_dict,
									struct graphical_stage_struct *graphics,
									int *counter_value_ptr, int digits,
									struct visual_container_struct *container, 
									struct status_struct *status, const char *name,
									float *currentbeat_ptr) {

	struct ui_counter *counter = malloc(sizeof(*counter));
	*counter = (struct ui_counter) {0};
	counter->container = container;
	counter->value = counter_value_ptr;
	counter->digits = digits;
	counter->array = calloc(digits+1, sizeof(int));
	counter->name = name;
	counter->self = counter;

	const char *graphic_types[digits+1];
	/* Initialise the image dictionary and generic animation dictionary.
	   Both are out-parameters here. */
	graphic_types[0] = "ser_container";

	for (int i = 0; i < digits; i++) {
		graphic_types[i+1] = "numbers";
	}
	graphic_spawn(&counter->std, object_list_stack_ptr, generic_anim_dict, graphics, graphic_types, digits+1);

	struct animation_struct *dummy_anim = counter->animation;
	dummy_anim->container = (struct visual_container_struct) {0};
	dummy_anim->container.inherit = container;
	dummy_anim->container.rect_out_parent_scale = (struct float_rect) { .x = 0.5, .y = 0.5, .w = 1, .h = 1};
	dummy_anim->container.anchor_hook = (struct size_ratio_struct) {0.5, 0.5};
	
	prepare_anim_ui(dummy_anim, status);

	struct animation_struct *anim = dummy_anim->anim_next;

	for (int i = 0; i < counter->digits; i++) {
		struct visual_container_struct digit_container = (struct visual_container_struct) {
			.inherit = &dummy_anim->container,
			.rect_out_parent_scale = (struct float_rect) { .x = 0.2 * i, .y = 0, .w = 0.2, .h = 1.0},
			.aspctr_lock = WH_INDEPENDENT,
		};
		anim->container = digit_container;
		anim = anim->anim_next;
	}

	struct rule_node *rule_node = dummy_anim->rules_list;
	while(rule_node->next) {
		rule_node = rule_node->next;
	}
	rule_node->next = malloc(sizeof(struct rule_node));
	rule_node->next->rule = &rules_ui_counter;
	rule_node->next->data = (void *)dummy_anim;
	rule_node->next->next = NULL;

	return counter;
}

struct monster_struct *spawn_flying_hamster(struct status_struct *status, struct visual_container_struct *container, int lane, float spawn_beat) {
	struct level_struct *level = status->level;

	struct monster_struct *new_flyinghamster = malloc(sizeof(struct monster_struct));
	*new_flyinghamster = (struct monster_struct) {0};
	new_flyinghamster->std.self = new_flyinghamster;
	struct std_list **object_list_stack_ptr = &level->stage->graphics.object_list_stack;
	struct dict_void *generic_anim_dict = level->stage->graphics.generic_anim_dict;

	new_flyinghamster->name = "new_flyinghamster";

	new_flyinghamster->living.alive = 1;
	new_flyinghamster->living.HP = 1;
	new_flyinghamster->living.power = 10;
	new_flyinghamster->living.defence = 10;
	new_flyinghamster->entrybeat = spawn_beat;

	graphic_spawn(&new_flyinghamster->std, object_list_stack_ptr, generic_anim_dict, &status->level->stage->graphics, (const char *[]){"flying hamster", "smiley"}, 2);

	/* Insert the monster into the lane's list of active monsters: */

	struct std_list **monster_list_stack_ptr = &level->monster_list_stacks[lane];
	new_flyinghamster->monster_list_stack_ptr = monster_list_stack_ptr;
	std_stack_push(monster_list_stack_ptr, &new_flyinghamster->std, &new_flyinghamster->monster_stack_location);

	struct animation_struct *animation = new_flyinghamster->animation;

	new_flyinghamster->container = malloc(sizeof(struct visual_container_struct));
	*new_flyinghamster->container = *container;
	
	struct visual_container_struct hamster_sub_container = {
		.inherit = new_flyinghamster->container,
		.rect_out_parent_scale = (struct float_rect) {.x = 0, .y = 0, .w = 1, .h = 1},
		.aspctr_lock = H_DOMINANT,
	};

	animation->container = hamster_sub_container;
	animation->anim_next->container = hamster_sub_container;
	animation->anim_next->container.rect_out_parent_scale.w = 0.2;
	animation->anim_next->container.rect_out_parent_scale.h = 0.2;
	animation->anim_next->container.anchor_grabbed = &container->anchors_exposed[0]; //Lock smiley's hook to hamster main anchor
	animation->anim_next->container.anchor_hook = (struct size_ratio_struct) {0.5, 0.5};
	set_anchor_hook(new_flyinghamster->std.container, 0, 0.5);

	animation->rules_list = malloc(sizeof(struct rule_node));
	animation->rules_list->rule = &rules_player;
	animation->rules_list->data = NULL;
	animation->rules_list->next = NULL;

	struct func_node *tr_node = malloc(sizeof(struct func_node));
	animation->transform_list = tr_node;
	struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
	teststruct->currentbeat = &status->timing->currentbeat;
	teststruct->rect_bitmask = 2;
	teststruct->freq_perbeat = 0.5;
	teststruct->ampl = 0.2;
	teststruct->offset = 0.0;
	tr_node->data = (void *)teststruct;
	tr_node->func = &tr_sine_abs;
	tr_node->next = NULL;

	new_flyinghamster->animation->rules_list->data = (void *)status;

	struct tr_orbit_xyz_data *orbit_data = malloc(sizeof(*orbit_data));
	*orbit_data = (struct tr_orbit_xyz_data) {
		.x = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0},
		.y = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0},
		.z = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0.5*PI},
		.z_eqm = new_flyinghamster->animation->z, /* z value at equilibrium */
		.z_layer_ampl = 0.1, /* Only for layer ordering */
		.z_set = &new_flyinghamster->animation->anim_next->z,
		.currentbeat = &status->timing->currentbeat,
	};
	struct tr_blink_data *blink_data = malloc(sizeof(*blink_data));
	*blink_data = (struct tr_blink_data) {
		.framecount = &status->timing->framecount,
		.frames_on = 10,
		.frames_off = 10,
	};

	new_flyinghamster->animation->anim_next->transform_list = malloc(sizeof(struct rule_node));
	
	new_flyinghamster->animation->anim_next->transform_list->func = (void *)tr_orbit_xyz;
	new_flyinghamster->animation->anim_next->transform_list->data = (void *)orbit_data;
	new_flyinghamster->animation->anim_next->transform_list->next = NULL;
	
	new_flyinghamster->object_logic = object_logic_monster;
	new_flyinghamster->object_data = status;

	return new_flyinghamster;
}

void spawn_graphical_stage_child(struct graphical_stage_child_struct *stage, struct graphics_struct *master_graphics, void *self, const char *name) {

	/* Spawns a graphical stage that is itself a member of a graphical stage 
	   (ie it isn't the top-level one in main.c) */

	struct std *std = &stage->std;
	struct graphical_stage_struct *graphics = &stage->graphics;
	/* Give the level its own "object" std struct */
	std->name = name;
	std->container = malloc(sizeof(*std->container));
	*std->container = (struct visual_container_struct) {
		.inherit = &master_graphics->screen,
		.rect_out_parent_scale = (struct float_rect) { .x = 0.1, .y = 0.1, .w = 0.8, .h = 0.8},
		.aspctr_lock = WH_INDEPENDENT,
	};
	std->self = self;

	graphic_spawn(std, &master_graphics->graphics.object_list_stack, 
			master_graphics->graphics.generic_anim_dict, 
			&master_graphics->graphics, (const char *[]){"ser_texture"}, 1);

	std->animation->container = *std->container;

	std->animation->img = master_graphics->graphics.renderer->framebuffer->texture;
	std->animation->img_y_convention = Y_OPENGL;

	struct glrenderer *renderer = make_renderer(master_graphics->graphics.renderer->framebuffer, (float []){0.2f, 0.3f, 0.3f, 1.0f}, 0, (struct int_rect){0});

	if( !renderer)
	{
		printf( "Unable to create a new renderer object!\n" );
		exit(1);
	}
	graphics->renderer = renderer;

	/* Graphical vars of the level only */
	graphics->render_node_head = NULL;
	graphics->render_node_tail = NULL;
	graphics->num_images = 0;
	graphics->master_graphics = master_graphics;
}

//TODO	Write recursive destructors for each struct type

void spawn_bg(struct graphical_stage_struct *graphics, struct graphics_struct *master_graphics, texture_t bg_texture) {
	/* Spawn a texture as the background of a stage */
	struct std *bg_std = malloc(sizeof(*bg_std));
	*bg_std = (struct std) {0};
	bg_std->name = "background";
	bg_std->container = malloc(sizeof(*bg_std->container));
	*bg_std->container = (struct visual_container_struct) {0};
	
	bg_std->container->inherit = &master_graphics->screen;
	bg_std->container->rect_out_parent_scale = (struct float_rect) { .x = 0, .y = 0, .w = 1, .h = 1};
	bg_std->container->aspctr_lock = WH_INDEPENDENT;

	bg_std->self = NULL;

	graphic_spawn(bg_std, &graphics->object_list_stack, graphics->generic_anim_dict, graphics, (const char *[]){"ser_texture"}, 1);

	bg_std->animation->container = *bg_std->container;
	bg_std->animation->img = bg_texture;
	bg_std->animation->z = -1; /* Background layer */
}

void set_anchor_hook(struct visual_container_struct *container, float x, float y) {
	struct size_ratio_struct new_anchor_hook = {.w = x, .h = y};
	struct size_ratio_struct old_anchor_hook = container->anchor_hook;
	/* Set the new anchor hook: */
	container->anchor_hook = new_anchor_hook;

	/* Preserve the real position: */
	container->rect_out_parent_scale.x += (new_anchor_hook.w - old_anchor_hook.w) * container->rect_out_parent_scale.w;
	container->rect_out_parent_scale.y += (new_anchor_hook.h - old_anchor_hook.h) * container->rect_out_parent_scale.h;
}

//TODO: Freeing functions (still working on these):

void monster_struct_rm(struct monster_struct *monster, struct std_list **stack_ptr, struct graphical_stage_struct *graphics) {

	/* Remove from active monster list */
	std_stack_rm(
		monster->monster_list_stack_ptr,
		monster->monster_stack_location);
	monster->monster_stack_location = NULL;

	std_rm(&monster->std, stack_ptr, graphics, 1);
	/* Also frees std->self, aka monster */
}

void std_list_rm(struct std_list **std_list_head, struct graphical_stage_struct *graphics, int free_self) {

	struct std_list *std_node = *std_list_head;

	while(std_node) {
		struct std_list *std_next = std_node->next;
		std_rm(std_node->std, std_list_head, graphics, free_self);
		// free(std_node); //(Don't need this as it's freed by std_stack_rm)
		std_node = std_next;
	}
}

void std_rm(struct std *std, struct std_list **stack_ptr, struct graphical_stage_struct *graphics, int free_self) {
	animation_struct_rm_recurse(std->animation, graphics);
	
	struct std_list *stack_pos = std->object_stack_location;

	/* Remove from object_list_stack */
	std_stack_rm(stack_ptr, stack_pos);
	std->object_stack_location = NULL;

	if (free_self && std->self) {
		free(std->self);
	}
}

void animation_struct_rm(struct animation_struct *animation, struct graphical_stage_struct *graphics) {

	struct rule_node *rule = animation->rules_list;
	while (rule) {
		struct rule_node *rule_to_free = rule;
		rule = rule->next;
		free(rule_to_free);
	}
	struct func_node *transform = animation->transform_list;
	while (rule) {
		struct func_node *transform_to_free = transform;
		transform = transform->next;
		free(transform_to_free);
	}
	
	render_node_rm(graphics, animation);

	free(animation->container.anchors_exposed);

}

void animation_struct_rm_recurse(struct animation_struct *animation, struct graphical_stage_struct *graphics) {
	while (animation) {
		struct animation_struct *animation_to_free = animation;
		animation_struct_rm(animation_to_free, graphics);
		animation = animation->anim_next;
	}
}

void exit_graphical_stage_child(struct graphical_stage_child_struct *stage) {
	struct graphical_stage_struct *graphics = &stage->graphics;
	std_list_rm(&graphics->object_list_stack, graphics, 0);
	dict_void_rm(graphics->generic_anim_dict);

	/* Lose the stage from the graphical_stage_child_struct 
	   to become just a std object */
	//struct std *std_from_stage = realloc(stage, sizeof(struct std));
	//struct std *std_from_stage = &stage->std;
	//std_from_stage->self = std_from_stage;

}

void destroy_graphical_stage_child(struct graphical_stage_child_struct *stage) {

}
