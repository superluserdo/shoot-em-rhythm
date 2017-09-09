/* ANIMATION */

	/* Structs */

struct frame {
	SDL_Rect rect;
	float duration;
};

struct clip {
	SDL_Texture *img;
	int numFrames;
	struct frame *frames;
};

struct animate_generic {
	int numAnimations;
	struct clip **clips;
};

struct animate_specific {
	void (*animate_rules)(void *);
	struct animate_generic *generic;
	int clip;
	int frame;
	float speed;
	int loops;
	int return_clip;
	struct render_node *render_node;
	float lastFrameBeat;
	struct func_node *transform_list;
	SDL_Rect rect_out;
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
	SDL_Rect *rect_out;
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

int render_node_populate(struct render_node **render_node_head_ptr, struct render_node *r_node, SDL_Texture **imgList, SDL_Renderer *renderer, struct player_struct *playerptr);
