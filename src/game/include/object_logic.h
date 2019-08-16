#ifndef _GAME_OBJECT_LOGIC_H
#define _GAME_OBJECT_LOGIC_H
void object_logic_monster(struct std *monster, void *data);
void object_logic_player(struct std *player_std, void *data);
void object_logic_sword(struct std *sword_std, void *data);
void object_logic_timeout(struct std *std, void *data);
void object_logic_fadeout(struct std *std, void *data);

void process_object_logics(struct std_list *object_list_stack);
#endif
