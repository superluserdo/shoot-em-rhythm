int spawn_hp(struct ui_bar *hp_ptr, struct animate_generic **generic_bank, SDL_Renderer *renderer);
int spawn_flying_hamster(struct status_struct *status);
int object_logic_monster(struct std *monster, void *data);
void set_anchor_hook(struct std *std, float x, float y);
struct size_ratio_struct pos_at_custom_anchor_hook(struct std *std, float x, float y);

