/* Funcs */

int advanceFrames(struct render_node *render_node_head, float currentbeat);
void advance_frames_and_create_render_list(struct std_list *object_list_stack, struct graphical_stage_struct *graphics, float currentbeat);

//void render_process(struct graphics_struct *graphics, struct time_struct *timing);
void render_process(struct std_list *object_list_stack, struct graphical_stage_struct *graphics, struct graphics_struct *master_graphics, SDL_Texture **tex_target_ptr, struct time_struct *timing);
int renderlist(struct render_node *node_ptr, struct graphical_stage_struct *graphics);
void query_resize(struct graphics_struct *master_graphics, SDL_Texture **tex_target_ptr);

int node_insert_over(struct graphical_stage_struct *graphics, struct render_node *node_src, struct render_node *node_dest);
int node_insert_under(struct graphical_stage_struct *graphics, struct render_node *node_src, struct render_node *node_dest);
int node_insert_z_over(struct graphical_stage_struct *graphics, struct render_node *node_src, float z);
int node_insert_z_under(struct graphical_stage_struct *graphics, struct render_node *node_src, float z);
struct render_node *create_render_node();

int render_node_rm(struct graphical_stage_struct *graphics, struct render_node *node_ptr);
int render_list_rm(struct render_node **node_ptr_ptr);

struct dict_void *generic_anim_dict_populate(struct dict_void *image_dict, struct status_struct *status);
int dicts_populate(struct dict_void **generic_anim_dict_ptr, struct dict_void **image_dict_ptr, struct status_struct *status, SDL_Renderer *renderer);

struct animate_specific *new_specific_anim(struct std *std, struct animate_generic *generic);
int generate_render_node(struct animate_specific *specific, struct graphical_stage_struct *graphics);
int graphic_spawn(struct std *std, struct std_list **object_list_stack_ptr, struct dict_void *generic_anim_dict, struct graphical_stage_struct *graphics, const char* specific_type_array[], int num_specific_anims);
		
void rules_player(void *playervoid);
void rules_sword(void *sword_void);
void rules_ui(void *data);
void rules_explosion(void *data);
void rules_ui_bar(void *data);
void rules_ui_counter(void *animvoid);

struct anchor_struct *make_anchors_exposed(struct animate_specific *anim, int n);
void de_update_containers(struct std_list *std_list_node);
