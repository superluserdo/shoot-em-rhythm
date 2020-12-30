#ifndef _ENGINE_ANIMATE_H
#define _ENGINE_ANIMATE_H
#include <stdio.h>
#include <libconfig.h>
#include "structdef_game.h"
#include "helpers.h"
#include "transform.h"
#include "structure.h"
#include "dict.h"

/* Funcs */

void print_object_list_stack(struct std_list *object_list_stack);
void print_render_list(struct render_node *render_node_head);
int advanceFrames(struct render_node *render_node_head, float currentbeat);
void advance_frames_and_create_render_list(struct std_list *object_list_stack, struct graphical_stage_struct *graphics, float currentbeat);

//void render_process(struct graphics_struct *graphics, struct time_struct *timing);
void render_process(struct std_list *object_list_stack, struct graphical_stage_struct *graphics, struct graphics_struct *master_graphics, float currentbeat);
int renderlist(struct render_node *node_ptr, struct graphical_stage_struct *graphics);

int node_insert_over(struct graphical_stage_struct *graphics, struct render_node *node_src, struct render_node *node_dest);
int node_insert_under(struct graphical_stage_struct *graphics, struct render_node *node_src, struct render_node *node_dest);
int node_insert_z_over(struct graphical_stage_struct *graphics, struct render_node *node_src, float z);
int node_insert_z_under(struct graphical_stage_struct *graphics, struct render_node *node_src, float z);
struct render_node *create_render_node();

int render_node_rm(struct graphical_stage_struct *graphics, struct render_node *node_ptr);
int render_list_rm(struct render_node **node_ptr_ptr);

struct dict_void *generic_anim_dict_populate(struct dict_void *image_dict, struct status_struct *status);
int dicts_populate(struct dict_void **generic_anim_dict_ptr, struct dict_void **image_dict_ptr, struct status_struct *status, renderer_t renderer);

struct animation_struct *new_animation(struct std *std, enum animate_mode_e animate_mode, struct animate_generic *generic);
int generate_render_node(struct animation_struct *specific, struct graphical_stage_struct *graphics);
void update_render_node(struct animation_struct *specific, struct render_node *r_node);

int graphic_spawn(struct std *std, struct std_list **object_list_stack_ptr, struct dict_void *generic_anim_dict, struct graphical_stage_struct *graphics, const char* animation_type_array[], int num_anims);
		
void rules_player(void *playervoid);
void rules_sword(void *sword_void);
void rules_ui(void *data);
void rules_explosion(void *data);
void rules_ui_bar(void *data);
void rules_ui_counter(void *animvoid);

void make_anchors_exposed(struct animation_struct *anim, int n);
void de_update_containers(struct std_list *std_list_node);
#endif
