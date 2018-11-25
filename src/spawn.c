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
#include "transform.h" // Can hopefully get rid of this soon
#include "spawn.h"

//int spawn_hp(struct ui_bar *hp_ptr, struct animate_generic **generic_bank, SDL_Renderer *renderer) {
//
//	struct ui_bar hp = *hp_ptr;
//	graphic_spawn(&hp.std, generic_bank, renderer, (int[]){2,4}, 2);
//
//	hp.animation->next->native_offset.x = 29;
//	hp.animation->next->native_offset.y = 6;
//	
//	hp.animation->next->rules_list->data = hp.animation->next;
//	struct tr_bump_data *tmp_data = malloc(sizeof(struct tr_bump_data));
//	*tmp_data = *(struct tr_bump_data *)hp.animation->next->rules_list->next->data;
//	tmp_data->centre = malloc(sizeof(struct xy_struct));
//	tmp_data->centre->x = hp.animation->rect_out.x + hp.animation->rect_out.w/2;
//	tmp_data->centre->y = hp.animation->rect_out.y + hp.animation->rect_out.h/2;
//	hp.animation->next->rules_list->next->data = tmp_data;
//	hp.animation->next->transform_list->data = tmp_data;
//	*hp_ptr = hp;
//}

int spawn_sword(struct status_struct *status) {
	struct level_struct *level = status->level;
	struct sword_struct *sword = &level->sword;
	//graphic_spawn(&sword->std, generic_bank, graphics, (enum graphic_type_e[]){OBJECT}, 5);
	sword->count = 0;
	sword->down = 0;
	sword->swing = 0;
	sword->rect_in.w = SWORD_WIDTH;
	sword->rect_in.h = SWORD_HEIGHT;
	sword->rect_in.x = 0;
	sword->rect_in.y = 0;

	sword->rect_out.x = player->animation->rect_out.x + player->animation->rect_out.w - 2 * ZOOM_MULT; //set preliminary values for the sword's dimensions (otherwise the beat predictor gets it wrong the first time)
	sword->rect_out.y = level->lanes->laneheight[level->lanes->currentlane] - ( SWORD_HEIGHT + 18 ) * ZOOM_MULT;
	sword->rect_out.w = sword->rect_in.w * ZOOM_MULT * 2;
	sword->rect_out.h = sword->rect_in.h * ZOOM_MULT * 2;

	Swordimg = IMG_LoadTexture(renderer, SWORD_PATH);
	SDL_QueryTexture(Swordimg, NULL, NULL, &w, &h); // get the width and height of the texture
	//SDL_Rect rcSword, rcSwordSrc;
}

int spawn_flying_hamster(struct status_struct *status) {
	struct graphics_struct *graphics = status->graphics;
	struct level_struct *level = status->level;

	struct monster_new *new_flyinghamster = malloc(sizeof(struct monster_new));
	*new_flyinghamster = (struct monster_new) {0};
	new_flyinghamster->std.self = new_flyinghamster;
	struct lane_struct *lanes = &level->lanes;
	struct std_list **object_list_stack_ptr = &level->object_list_stack;
	struct animate_generic **generic_bank = level->generic_bank;

	//new_flyinghamster->container = malloc(sizeof(struct visual_container_struct));
	//*new_flyinghamster->container = (struct visual_container_struct) {
	//	.inherit = &lanes->containers[lanes->currentlane],
	//	.rect = (struct float_rect) { .x = 0.4, .y = 0, .w = 0.1, .h = 1},
	//	.aspctr_lock = WH_INDEPENDENT,
	//};
	new_flyinghamster->container = &lanes->containers[lanes->currentlane];

	new_flyinghamster->living.HP = 1;
	new_flyinghamster->living.power = 10;
	new_flyinghamster->living.defence = 10;

	new_flyinghamster->name = "new_flyinghamster";
	new_flyinghamster->container_pos.w = 1;
	new_flyinghamster->container_pos.h = 0.5;

	graphic_spawn(&new_flyinghamster->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){FLYING_HAMSTER, SMILEY}, 2);

	struct animate_specific *animation = new_flyinghamster->animation;
	//animation->anchor_hook = (struct size_ratio_struct) {0, 0.5};
	set_anchor_hook(&new_flyinghamster->std, 0, 0.5);
	new_flyinghamster->container_pos.w = 0.1;
	
	animation->rules_list = malloc(sizeof(struct rule_node));
	animation->rules_list->rule = &rules_player;
	animation->rules_list->data = NULL;
	animation->rules_list->next = NULL;

	struct func_node *tr_node = malloc(sizeof(struct func_node));
	animation->transform_list = tr_node;
	struct tr_sine_data *teststruct = malloc(sizeof(struct tr_sine_data));
	teststruct->status = status;
	teststruct->rect_bitmask = 2;
	teststruct->freq_perbeat = 0.5;
	teststruct->ampl = 0.2;
	teststruct->offset = 0.0;
	tr_node->data = (void *)teststruct;
	tr_node->func = &tr_sine_abs;
	tr_node->next = NULL;

	new_flyinghamster->animation->rules_list->data = (void *)status;
	//new_flyinghamster->animation->next->rules_list->data = (void *)status;

	struct tr_orbit_xyz_data *orbit_data = malloc(sizeof(*orbit_data));
	*orbit_data = (struct tr_orbit_xyz_data) {
		//.x = (struct cycle_struct) {.freq = 0.5, .ampl = 110, .phase = 0},
		//.y = (struct cycle_struct) {.freq = 0.5, .ampl = 110, .phase = 0},
		//.z = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0.5*PI},
		.x = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0},
		.y = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0},
		.z = (struct cycle_struct) {.freq = 0.5, .ampl = 0.6, .phase = 0.5*PI},
		.z_eqm = new_flyinghamster->animation->z, /* z value at equilibrium */
		.z_layer_ampl = 0.1, /* Only for layer ordering */
		.z_set = &new_flyinghamster->animation->next->z,
		.currentbeat = &status->timing->currentbeat,
	};
	struct tr_blink_data *blink_data = malloc(sizeof(*blink_data));
	*blink_data = (struct tr_blink_data) {
		.status = status,
		.frames_on = 10,
		.frames_off = 10,
	};


	//new_flyinghamster->animation->next->anchor_grabbed = &new_flyinghamster->animation->anchors_exposed[0]; //Lock smiley's hook to hamster main anchor
	//new_flyinghamster->animation->next->transform_list->next = malloc(sizeof(struct rule_node));
	//
	//new_flyinghamster->animation->next->transform_list->next->func = (void *)tr_orbit_xyz;
	//new_flyinghamster->animation->next->transform_list->next->data = (void *)orbit_data;

	new_flyinghamster->animation->next->anchor_grabbed = &new_flyinghamster->animation->anchors_exposed[0]; //Lock smiley's hook to hamster main anchor
	new_flyinghamster->animation->next->transform_list = malloc(sizeof(struct rule_node));
	
	new_flyinghamster->animation->next->transform_list->func = (void *)tr_orbit_xyz;
	new_flyinghamster->animation->next->transform_list->data = (void *)orbit_data;
	new_flyinghamster->animation->next->transform_list->next = NULL;

	//new_flyinghamster->animation->next->transform_list->next->func = (void *)tr_blink;
	//new_flyinghamster->animation->next->transform_list->next->data = (void *)blink_data;
	//new_flyinghamster->animation->next->transform_list->next->next = NULL;

	//graphic_spawn(&new_flyinghamster->std, object_list_stack_ptr, generic_bank, graphics, (enum graphic_type_e[]){FLYING_HAMSTER}, 1);
	//new_flyinghamster->animation->rules_list->data = (void *)&status;
	
	new_flyinghamster->object_logic = object_logic_monster;
	new_flyinghamster->object_data = status;
}

//TODO	Write recursive destructors for each struct type

int object_logic_monster(struct std *monster_std, void *data) {
	struct status_struct *status = (struct status_struct *)data;
	monster_std->container_pos.w -= 0.001;
	struct monster_new *monster = monster_std->self;

	if (monster->living.alive == 123123123 /*start-dying code or something */ ) {
		// TODO: Append animation to animation list of explosion or something.
		// This animation has rule where it calls function to destroy whole object at the end.
		// Destroying object involves freeing everything, and taking the 
		// object out of the std_list, removing the render node, etc.
	}


	if (pos_at_custom_anchor_hook(monster_std, 0, 0.5).w < 0) {
		// TODO: Destroy monster
		monster_new_rm((struct monster_new *)monster_std->self, status);
	}

	return 0;
}

void set_anchor_hook(struct std *std, float x, float y) {
	struct size_ratio_struct new_anchor_hook = {.w = x, .h = y};
	struct size_ratio_struct old_anchor_hook = std->animation->anchor_hook;
	/* Set the new anchor hook: */
	std->animation->anchor_hook = new_anchor_hook;

	/* Preserve the real position: */
	struct size_ratio_struct old_pos = std->container_pos;

	struct float_rect rect_out = std->animation->rect_out_container_scale;
	std->container_pos = (struct size_ratio_struct) {
		.w = old_pos.w + (new_anchor_hook.w - old_anchor_hook.w) * rect_out.w,
		.h = old_pos.h + (new_anchor_hook.h - old_anchor_hook.h) * rect_out.h,
	};
}

struct size_ratio_struct pos_at_custom_anchor_hook(struct std *std, float x, float y) {
	struct size_ratio_struct custom_anchor_hook = {.w = x, .h = y};
	struct size_ratio_struct old_anchor_hook = std->animation->anchor_hook;

	/* Preserve the real position: */
	struct size_ratio_struct old_pos = std->container_pos;

	struct float_rect rect_out = std->animation->rect_out_container_scale;
	struct size_ratio_struct custom_pos = {
		.w = old_pos.w + (custom_anchor_hook.w - old_anchor_hook.w) * rect_out.w,
		.h = old_pos.h + (custom_anchor_hook.h - old_anchor_hook.h) * rect_out.h,
	};

	return custom_pos;
}

//TODO: Freeing functions (still working on these):

void monster_new_rm(struct monster_new *monster, struct status_struct *status) {
	std_rm(&monster->std, status);
	free(monster);
}

void std_rm(struct std *std, struct status_struct *status) {
	animate_specific_rm_recurse(std->animation, status);
	//free(std->object_data); //Need to think about how to do this. Data could be custom mallocd
	//	or pointer to status struct
	
	/* Remove from object_list_stack */
	struct std_list *stack_pos = std->object_stack_location;
	if (stack_pos->prev) {
		stack_pos->prev->next = stack_pos->next;
	}
	if (stack_pos->next) {
		stack_pos->next->prev = stack_pos->prev;
	} else {
		status->level->object_list_stack = stack_pos->prev;
	}
	free(stack_pos);
	std->object_stack_location = NULL;
}

void animate_specific_rm(struct animate_specific *animation, struct status_struct *status) {

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
	node_rm(status->graphics,animation->render_node);

	free(animation->anchors_exposed);

}

void animate_specific_rm_recurse(struct animate_specific *animation, struct status_struct *status) {
	while (animation) {
		struct animate_specific *animation_to_free = animation;
		animate_specific_rm(animation_to_free, status);
		animation = animation->next;
	}
}
