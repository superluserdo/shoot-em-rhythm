/* ANIMATION */

	/* Structs */

struct frame {
	SDL_Rect rect;
	float duration;
};

struct clip {
	SDL_Texture *img;
	int num_frames;
	struct frame *frames;
};

struct animate_generic {
	int num_clips;
	struct clip **clips;
	struct animate_specific *default_specific;
};

struct animate_specific {
	struct animate_generic *generic;

	struct rule_node *rules_list;
	int clip;
	int frame;
	float speed;
	int loops;
	int return_clip;
	float lastFrameBeat;
	struct xy_struct native_offset;
	struct std *parent;
	//struct xy_struct *parent_pos;	/*	Adjust object position and size ratio in these two structs.	*/
	//struct size_ratio_struct *parent_size_ratio;	/*	They're put into rect_out each frame.		*/
	//struct animate_specific *list_head;
	struct func_node *transform_list;

	struct render_node *render_node;
	SDL_Rect rect_out;
	struct animate_specific *next;
};

struct rule_node {
	void (*rule)(void *);
	void *data;
	struct rule_node *next;
};

struct func_node {
	void *data;
	void (*func)(void *rect_trans, void *data);
	struct func_node *next;
};

	/* Funcs */

int advanceFrames(struct render_node *render_node_head);

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


struct render_node *render_node_head;
struct render_node *render_node_tail;

int renderlist(struct render_node *node_ptr);

int node_insert_over(struct render_node *node_src, struct render_node *node_dest);
int node_insert_under(struct render_node *node_src, struct render_node *node_dest);
int node_insert_z_over(struct render_node *node_src, float z);
int node_insert_z_under(struct render_node *node_src, float z);
struct render_node *create_render_node();

int node_rm(struct render_node *node_ptr);
int list_rm(struct render_node *node_ptr);

int image_bank_populate(SDL_Texture **image_bank, SDL_Renderer *renderer);
int generic_bank_populate(struct animate_generic ***generic_bank_ptr, SDL_Texture **image_bank);
int render_node_populate(struct render_node **render_node_head_ptr, struct render_node *r_node, SDL_Texture **imgList, SDL_Renderer *renderer, struct player_struct *playerptr);
struct animate_specific *render_node_populate2(struct render_node **render_node_head_ptr, SDL_Texture **imgList, SDL_Renderer *renderer, struct animate_generic **generic_bank);

struct animate_specific *generate_specific_anim(struct std *std, struct animate_generic **generic_bank, int index);
int generate_render_node(struct animate_specific *specific, SDL_Renderer *renderer);
int graphic_spawn(struct std *std, struct animate_generic **generic_bank, SDL_Renderer *renderer, int *index_array, int num_index);
		
int generate_default_specific_template();
struct animate_specific *generate_default_specific(int index);

void rules_player(void *playervoid);
void rules_ui(void *data);
void rules_ui_bar(void *data);

int transform_add_check(struct animate_specific *animation, void *data, void (*func)());
int transform_rm(struct animate_specific *animation, void (*func)());
