//int spawn_hp(struct ui_bar *hp_ptr, struct dict_str_void *generic_anim_dict, SDL_Renderer *renderer);
void spawn_player(struct std_list **object_list_stack_ptr, struct dict_str_void *generic_anim_dict, struct graphics_struct *graphics, struct player_struct *player, struct visual_container_struct *container,  struct status_struct *status,  config_setting_t *player_setting, const char *name);
struct ui_bar *spawn_ui_bar(struct std_list **object_list_stack_ptr, struct dict_str_void *generic_anim_dict, struct graphics_struct *graphics, int *bar_amount_ptr, int *bar_max_ptr, struct visual_container_struct *container, struct status_struct *status, const char *name);
struct ui_counter *spawn_ui_counter(struct std_list **object_list_stack_ptr, struct dict_str_void *generic_anim_dict, struct graphics_struct *graphics, int *counter_value_ptr, int digits, struct visual_container_struct *container, struct status_struct *status, const char *name, float *currentbeat_ptr);
int spawn_flying_hamster(struct status_struct *status, struct visual_container_struct *container);
int object_logic_monster(struct std *monster, void *data);
void set_anchor_hook(struct visual_container_struct *container, float x, float y);
struct size_ratio_struct pos_at_custom_anchor_hook(struct visual_container_struct *container, float x, float y);

void monster_new_rm(struct monster_new *monster, struct status_struct *status);
void std_rm(struct std *std, struct status_struct *status);
void animate_specific_rm(struct animate_specific *animation, struct status_struct *status);
void animate_specific_rm_recurse(struct animate_specific *animation, struct status_struct *status);

void prepare_anim_character(struct animate_specific *anim, struct status_struct *status);
void prepare_anim_ui(struct animate_specific *anim, struct status_struct *status);
