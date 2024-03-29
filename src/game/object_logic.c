#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef_game.h"
#include "main.h"
#include "structure.h"
#include "spawn.h"
#include "transform.h"
#include "dict.h"		//Get rid
#include "animate.h"	// of these?
#include "audio.h"
#include "object_logic.h"

//TODO: Remove
#define FIXME_SWITCH 0

void object_logic_player(struct std *player_std, void *data) {

	struct status_struct *status = data;
	struct level_struct *level = status->level;

	//struct object_spawn_array_struct *object_spawn_array = 
	//		&level->object_spawn_arrays[level->lanes.currentlane];

	/* Put character in correct lane */
	player_std->animation->container.inherit = &level->lanes.containers[level->lanes.currentlane];

	/* Get hurt by monster */
	struct player_struct *player = player_std->self;
	if (player->living.invincibility == 0) {
		struct std_list *active_monster_list = 
			level->monster_list_stacks[level->lanes.currentlane];
		struct std_list *monster_node = active_monster_list;
		int count = 0;
		while (monster_node) {
			struct monster_struct *monster = monster_node->std->self;
			if (monster->living.alive == 1) {
				struct graphics_struct *graphics = status->master_graphics;
				int collision = container_test_overlap_x(player_std->container, monster_node->std->container, (struct xy_struct) {graphics->width, graphics->height});
				if (collision) {
					player->living.HP -= 10;
					if ( player->living.HP <= 0 ) {
						status->audio->soundchecklist[5] = 1;
						level->levelover = 1;
					} else {

						/* Turn on i-frames */
						struct tr_blink_data *blink_data = malloc(sizeof(*blink_data));
						*blink_data = (struct tr_blink_data) {
							.framecount = &status->timing->framecount,
							.frames_on = 5,
							.frames_off = 5,
						};
						transform_add_check(player->std.animation, blink_data, tr_blink);
						status->audio->soundchecklist[4] = 1;

						player->living.invincibility = 1;
						player->living.invincible_since = status->timing->currentbeat;
					}
					break;
				}
			}
			count++;
			monster_node = monster_node->prev;
		}
	}

	if (player->living.invincibility == 1) {

		/* Stop iframes */
		if (status->timing->currentbeat - player->living.invincible_since >= 
			player->living.iframes_duration) {
			transform_rm(player->std.animation, tr_blink);
			player->living.invincibility = 0;
		}
	}


}

void object_logic_sword(struct std *sword_std, void *data) {

	struct status_struct *status = data;
	struct level_struct *level = status->level;
	//struct object_spawn_array_struct *object_spawn_array = 
	//		&level->object_spawn_arrays[level->lanes.currentlane];

	/* Get hurt by monster */
	struct sword_struct *sword = sword_std->self;
	if (sword->swing && sword->down) {
		queue_sound(status->audio, "sounds/whoosh.wav", 0.5);
		struct std_list *active_monster_list = 
			level->monster_list_stacks[level->lanes.currentlane];
		struct std_list *monster_node = active_monster_list;
		while (monster_node) {
			struct monster_struct *monster = monster_node->std->self;
			if (monster->living.alive == 1) {
				int collision = container_test_overlap_x(sword_std->container, monster_node->std->container, (struct xy_struct) {status->master_graphics->width, status->master_graphics->height});
				if (collision) {
					monster->living.HP -= sword->power;
					break;
				}
			}
			monster_node = monster_node->prev;
		}
	}
}

void object_logic_monster(struct std *monster_std, void *data) {
	struct status_struct *status = (struct status_struct *)data;
	struct monster_struct *monster = monster_std->self;

	/* Move the monster */


	struct time_struct *timing = status->timing;
	float speed = timing->container_width_per_beat;

	float Dt = (timing->currentbeat + 1) - monster->entrybeat; /* +1 to align with int */
	monster_std->container->rect_out_parent_scale.x = 1 - speed * Dt;
	if (monster->living.alive == 1) {
		if (monster->living.HP <= 0) {
			monster->living.alive = 0;
			queue_sound(status->audio, "sounds/killsound.wav", 0.5);
			status->level->score += 10;
		}
	}
	if (monster->living.alive == 0 /*start-dying code or something */ ) {
		struct animate_generic *generic = dict_void_get_val(status->level->stage->graphics.generic_anim_dict, "explosion");
		if (!generic) {
			fprintf(stderr, "Could not find generic animation for \"%s\". Aborting...\n", "explosion");
			abort();
		}
		struct animation_struct *explosion_anim = new_animation(monster_std, GENERIC, generic);
		struct animation_struct *anim = monster_std->animation;

		/* Append explosion to end of animations */
		while (anim->anim_next) {
			anim = anim->anim_next;
		}
		anim->anim_next = explosion_anim;
		explosion_anim->container.inherit = monster_std->container;
		explosion_anim->container.rect_out_parent_scale.x = 0.5;
		explosion_anim->container.rect_out_parent_scale.y = 0.5;
		explosion_anim->rules_list = malloc(1 * sizeof(*explosion_anim->rules_list));
		*explosion_anim->rules_list = (struct rule_node) {0};
		explosion_anim->rules_list->rule = &rules_explosion;
		explosion_anim->control->return_clip = -1;
		explosion_anim->control->loops = 3;
		struct explosion_data_struct *explosion_data = malloc(sizeof(struct explosion_data_struct));
		*explosion_data = (struct explosion_data_struct) {
			.animation = explosion_anim,
			.living = &monster->living
		};
		explosion_anim->rules_list->data = explosion_data;

		monster->living.alive = -1; /* In process of dying */
		return;
	}
	if (monster->living.alive == -2 ) /* Finished dying */ {
		monster_struct_rm((struct monster_struct *)monster_std->self, &status->level->stage->graphics.object_list_stack, &status->level->stage->graphics);
		return;
	}


	if (pos_at_custom_anchor_hook(monster_std->container, 1, 0.5, (struct xy_struct) {status->master_graphics->width, status->master_graphics->height}).w < 0) {
		monster_struct_rm((struct monster_struct *)monster_std->self, &status->level->stage->graphics.object_list_stack, &status->level->stage->graphics);
		return;
	}

	return;
}

void object_logic_timeout(struct std *std, void *data) {

	struct timeout_data_struct *timeout_data = data;

	float Dt = *timeout_data->current_time - timeout_data->start_time;
	std->container->rect_out_parent_scale.x = 1 - Dt;
	if (Dt >= timeout_data->duration) {
		std_rm(std, &timeout_data->graphics->object_list_stack, timeout_data->graphics, 1);
	}
}

#if FIXME_SWITCH
void object_logic_fadeout(struct std *std, void *data) {

	struct timeout_data_struct *timeout_data = data;

	float Dt = *timeout_data->current_time - timeout_data->start_time;
	std->animation->container.rect_out_parent_scale.w = 0;//1 - Dt;
	SDL_SetTextureAlphaMod(std->animation->img, 255 * (1 - Dt/timeout_data->duration));
	if (Dt >= timeout_data->duration) {
		std_rm(std, &timeout_data->graphics->object_list_stack, timeout_data->graphics, 1);
	}
}
#endif

void process_object_logics(struct std_list *object_list_stack) {
	struct std_list *current_obj = object_list_stack;
	while (current_obj) {
		struct std_list *new_obj = current_obj->prev;
		if (current_obj->std->object_logic) {
			current_obj->std->object_logic(current_obj->std, current_obj->std->object_data);
		}
		current_obj = new_obj; /* Do this so an object can delete itself and 
								  this loop still works */
	}
}
