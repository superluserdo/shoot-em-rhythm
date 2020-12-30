#ifndef _GAME_SPAWN_H
#define _GAME_SPAWN_H
//int spawn_hp(struct ui_bar *hp_ptr, struct dict_str_void *generic_anim_dict, SDL_Renderer *renderer);
void spawn_player(struct std_list **object_list_stack_ptr, struct dict_void *generic_anim_dict, struct graphical_stage_struct *graphics, struct player_struct *player, struct visual_container_struct *container,  struct status_struct *status,  config_setting_t *player_setting, const char *name);
void spawn_sword(struct std_list **object_list_stack_ptr, struct dict_void *generic_anim_dict, struct graphical_stage_struct *graphics, struct player_struct *player, struct sword_struct *sword, struct visual_container_struct *container, struct status_struct *status, const char *name);
struct ui_bar *spawn_ui_bar(struct std_list **object_list_stack_ptr, struct dict_void *generic_anim_dict, struct graphical_stage_struct *graphics, int *bar_amount_ptr, int *bar_max_ptr, struct visual_container_struct *container, struct status_struct *status, const char *name);
struct ui_counter *spawn_ui_counter(struct std_list **object_list_stack_ptr, struct dict_void *generic_anim_dict, struct graphical_stage_struct *graphics, int *counter_value_ptr, int digits, struct visual_container_struct *container, struct status_struct *status, const char *name, float *currentbeat_ptr);
struct monster_struct *spawn_flying_hamster(struct status_struct *status, struct visual_container_struct *container, int lane, float spawn_beat);
//void spawn_graphical_stage(struct std *std, struct graphics_struct *master_graphics, struct graphical_stage_struct *graphics, void *self, const char *name);
void spawn_graphical_stage_child(struct graphical_stage_child_struct *stage, struct graphics_struct *master_graphics, void *self, const char *name);
void spawn_bg(struct graphical_stage_struct *graphics, struct graphics_struct *master_graphics, texture_t bg_texture);
void set_anchor_hook(struct visual_container_struct *container, float x, float y);

void monster_struct_rm(struct monster_struct *monster, struct std_list **stack_ptr, struct graphical_stage_struct *graphics);

void std_rm(struct std *std, struct std_list **stack_ptr, struct graphical_stage_struct *graphics, int free_self);
void std_list_rm(struct std_list **std_list_head, struct graphical_stage_struct *graphics, int free_self);

void animation_struct_rm(struct animation_struct *animation, struct graphical_stage_struct *graphics);

void animation_struct_rm_recurse(struct animation_struct *animation, struct graphical_stage_struct *graphics);

void prepare_anim_character(struct animation_struct *anim, struct status_struct *status);
void prepare_anim_ui(struct animation_struct *anim, struct status_struct *status);
void prepare_anim_ui_bar(struct animation_struct *anim, struct status_struct *status);

void exit_graphical_stage_child(struct graphical_stage_child_struct *stage);
#endif
