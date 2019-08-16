#ifndef _ENGINE_HELPERS_H
#define _ENGINE_HELPERS_H
#include <SDL2/SDL.h>
#include "structdef_game.h"

/* Helper Functions */

void std_stack_push(struct std_list **stack_ptr, struct std *std, struct std_list **stack_ptr_from_std);

void std_stack_rm(struct std_list **stack_ptr, struct std_list *stack_pos);

void int2array(int number, int *array, int array_digits);

void *vector(int elem_size, int len);

void append_vec(void **vecptr);

void free_vec(void *vector);
#endif
