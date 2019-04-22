#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "main.h"
#include "structure.h"
#include "spawn.h"
#include "transform.h"
#include "dict.h"		//Get rid
#include "animate.h"	// of these?
#include "object_logic.h"

int object_logic_player(struct std *player_std, void *data) {

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
			struct monster_new *monster = monster_node->std->self;
			if (monster->living.alive == 1) {
				int collision = container_test_overlap_x(player_std->container, monster_node->std->container);
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

int object_logic_sword(struct std *sword_std, void *data) {

	struct status_struct *status = data;
	struct level_struct *level = status->level;
	//struct object_spawn_array_struct *object_spawn_array = 
	//		&level->object_spawn_arrays[level->lanes.currentlane];

	/* Get hurt by monster */
	struct sword_struct *sword = sword_std->self;
	if (sword->swing && sword->down) {
		struct std_list *active_monster_list = 
			level->monster_list_stacks[level->lanes.currentlane];
		struct std_list *monster_node = active_monster_list;
		while (monster_node) {
			struct monster_new *monster = monster_node->std->self;
			if (monster->living.alive == 1) {
				int collision = container_test_overlap_x(sword_std->container, monster_node->std->container);
				if (collision) {
					monster->living.HP -= sword->power;
					break;
				}
			}
			monster_node = monster_node->prev;
		}
	}
}

int object_logic_monster(struct std *monster_std, void *data) {
	struct status_struct *status = (struct status_struct *)data;
	monster_std->container->rect_out_parent_scale.x -= 0.001;
	struct monster_new *monster = monster_std->self;

	if (monster->living.alive == 1 && monster->living.HP <= 0) {
		monster->living.alive = 0;
	}
	if (monster->living.alive == 0 /*start-dying code or something */ ) {
		struct animate_generic *generic = dict_get_val(status->level->generic_anim_dict, "explosion");
		if (!generic) {
			fprintf(stderr, "Could not find generic animation for \"%s\". Aborting...\n", "explosion");
			abort();
		}
		struct animate_specific *explosion_anim = new_specific_anim(monster_std, generic);
		struct animate_specific *anim = monster_std->animation;

		/* Append explosion to end of animations */
		while (anim->next) {
			anim = anim->next;
		}
		anim->next = explosion_anim;
		explosion_anim->container.inherit = monster_std->container;
		explosion_anim->container.rect_out_parent_scale.x = 0.5;
		explosion_anim->container.rect_out_parent_scale.y = 0.5;
		explosion_anim->rules_list = malloc(1 * sizeof(*explosion_anim->rules_list));
		*explosion_anim->rules_list = (struct rule_node) {0};
		explosion_anim->rules_list->rule = &rules_explosion;
		explosion_anim->return_clip = -1;
		explosion_anim->loops = 3;
		struct explosion_data_struct *explosion_data = malloc(sizeof(struct explosion_data_struct));
		*explosion_data = (struct explosion_data_struct) {
			.animation = explosion_anim,
			.living = &monster->living
		};
		explosion_anim->rules_list->data = explosion_data;

		monster->living.alive = -1; /* In process of dying */
		return 0;
	}
	if (monster->living.alive == -2 ) /* Finished dying */ {
		monster_new_rm((struct monster_new *)monster_std->self, status);
		return 0;
	}


	if (pos_at_custom_anchor_hook(monster_std->container, 1, 0.5).w < 0) {
		monster_new_rm((struct monster_new *)monster_std->self, status);
		return 0;
	}

	return 0;
}

