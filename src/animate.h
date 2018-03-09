	/* Funcs */

int advanceFrames(struct render_node *render_node_head, float currentbeat);

/* RENDERING */

	/* Structs */

struct render_node {

	struct render_node *prev;
	struct render_node *next;
	SDL_Rect *rect_in;
	SDL_Rect rect_out;//*rect_out;
	SDL_Texture *img;
	int (*customRenderFunc)(void*);
	void *customRenderArgs;
	SDL_Renderer *renderer;
	struct animate_specific *animation;
	struct func_node *transform_list;
	float z; // Player defined as z = 0. +z defined as out of screen towards human.
};

	/* z	Reserved for
	 * -1	background
	 *  0	player
	 *  1	enemies
	 */

	/* Funcs */


int renderlist(struct render_node *node_ptr);

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
int graphic_spawn(struct std *std, struct animate_generic **generic_bank, struct graphics_struct *graphics, enum graphic_type_e *index_array, int num_index);
		
int generate_default_specific_template();
struct animate_specific *generate_default_specific(int index, struct status_struct *status);

void rules_player(void *playervoid);
void rules_ui(void *data);
void rules_ui_bar(void *data);
void rules_ui_counter(void *animvoid);

int transform_add_check(struct animate_specific *animation, void *data, void (*func)());
int transform_rm(struct animate_specific *animation, void (*func)());
