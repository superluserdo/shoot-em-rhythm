//int spawn_hp(struct ui_bar *hp_ptr, struct animate_generic **generic_bank, SDL_Renderer *renderer);
struct ui_bar *spawn_ui_bar(struct std_list **object_list_stack_ptr, struct animate_generic **generic_bank, struct graphics_struct *graphics, int *bar_amount_ptr, int *bar_max_ptr, struct visual_container_struct *container, const char *name);
struct ui_counter *spawn_ui_counter(struct std_list **object_list_stack_ptr, struct animate_generic **generic_bank, struct graphics_struct *graphics, int *counter_value_ptr, int digits, struct visual_container_struct *container, const char *name, float *currentbeat_ptr);
int spawn_flying_hamster(struct status_struct *status);
int object_logic_monster(struct std *monster, void *data);
void set_anchor_hook(struct visual_container_struct *container, float x, float y);
struct size_ratio_struct pos_at_custom_anchor_hook(struct visual_container_struct *container, float x, float y);

void monster_new_rm(struct monster_new *monster, struct status_struct *status);
void std_rm(struct std *std, struct status_struct *status);
void animate_specific_rm(struct animate_specific *animation, struct status_struct *status);
void animate_specific_rm_recurse(struct animate_specific *animation, struct status_struct *status);

