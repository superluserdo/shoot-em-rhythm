/* Funcs */

int advanceFrames(struct render_node *render_node_head, float currentbeat);
void advance_frames_and_create_render_list(struct std_list *object_list_stack, struct graphics_struct *graphics, float currentbeat);

//void render_process(struct graphics_struct *graphics, struct time_struct *timing);
void render_process(struct std_list *object_list_stack, struct graphics_struct *graphics, struct time_struct *timing);
int renderlist(struct render_node *node_ptr, struct graphics_struct *graphics);

int node_insert_over(struct graphics_struct *graphics, struct render_node *node_src, struct render_node *node_dest);
int node_insert_under(struct graphics_struct *graphics, struct render_node *node_src, struct render_node *node_dest);
int node_insert_z_over(struct graphics_struct *graphics, struct render_node *node_src, float z);
int node_insert_z_under(struct graphics_struct *graphics, struct render_node *node_src, float z);
struct render_node *create_render_node();

int node_rm(struct graphics_struct *graphics, struct render_node *node_ptr);
int list_rm(struct render_node *node_ptr);

int image_bank_populate(SDL_Texture **image_bank, SDL_Renderer *renderer);
int generic_bank_populate(struct animate_generic ***generic_bank_ptr, SDL_Texture **image_bank, struct status_struct *status);
int render_node_populate(struct graphics_struct *graphics, struct render_node *r_node, struct player_struct *playerptr);
struct animate_specific *render_node_populate2(struct render_node **render_node_head_ptr, SDL_Texture **imgList, SDL_Renderer *renderer, struct animate_generic **generic_bank);

struct animate_specific *generate_specific_anim(struct std *std, struct animate_generic **generic_bank, int index);
int generate_render_node(struct animate_specific *specific, struct graphics_struct *graphics);
//int graphic_spawn(struct std *std, struct animate_generic **generic_bank, struct graphics_struct *graphics, enum graphic_type_e *index_array, int num_index);
int graphic_spawn(struct std *std, struct std_list **object_list_stack_ptr, struct animate_generic **generic_bank, struct graphics_struct *graphics, enum graphic_type_e *index_array, int num_index);
		
int generate_default_specific_template();
struct animate_specific *generate_default_specific(int index, struct status_struct *status);

void rules_player(void *playervoid);
void rules_ui(void *data);
void rules_ui_bar(void *data);
void rules_ui_counter(void *animvoid);

int transform_add_check(struct animate_specific *animation, void *data, void (*func)());
int transform_rm(struct animate_specific *animation, void (*func)());
