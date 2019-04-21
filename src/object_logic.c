#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <libconfig.h>
#include "structdef.h"
#include "main.h"
#include "structure.h"
#include "spawn.h"
#include "object_logic.h"

int object_logic_player(struct std *player_std, void *data) {
	struct level_struct *level = data;
	//struct object_spawn_array_struct *object_spawn_array = 
	//		&level->object_spawn_arrays[level->lanes.currentlane];

	/* Get hurt by monster */
	struct player_struct *player = player_std->self;
	if (player->living.invincibility == 0) {
		struct std_list *active_monster_list = 
			level->monster_list_stacks[level->lanes.currentlane];
		struct std_list *monster_node = active_monster_list;
		int count = 0;
		while (monster_node) {
			struct monster_new *monster = monster_node->std->self;
			if (monster->living.alive) {
				int collision = container_test_overlap(player_std->container, monster_node->std->container);
				if (collision) {
					player->living.invincibility_toggle = 1;
					player->living.HP -= 10;
					printf("HIT! HP remaining: %d\n", player->living.HP);
				}
			}
			count++;
			monster_node = monster_node->prev;
		}
		printf("Count: %d\r", count);
	}

}

int object_logic_monster(struct std *monster_std, void *data) {
	struct status_struct *status = (struct status_struct *)data;
	monster_std->container->rect_out_parent_scale.x -= 0.001;
	struct monster_new *monster = monster_std->self;

	if (monster->living.alive == 123123123 /*start-dying code or something */ ) {
		// TODO: Append animation to animation list of explosion or something.
		// This animation has rule where it calls function to destroy whole object at the end.
		// Destroying object involves freeing everything, and taking the 
		// object out of the std_list, removing the render node, etc.
	}


	if (pos_at_custom_anchor_hook(monster_std->container, 1, 0.5).w < 0) {
		// TODO: Destroy monster
		printf("Destroyed\n");
		monster_new_rm((struct monster_new *)monster_std->self, status);
	}

	return 0;
}

