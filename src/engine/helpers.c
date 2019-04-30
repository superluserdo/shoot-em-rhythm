#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include "structdef.h"
#include "main.h"
#include "level.h"
#include "helpers.h"

void int2array(int number, int *array, int array_digits) {
        int digits = 1;
        int numchecker = number;
        while (numchecker /= 10)
                digits++;

        for (int i = 0; i < array_digits; i++ )
                array[i] = 0;

        int diff = array_digits - digits;
        for (int i = 0; i < array_digits; i++) {
                if ( i < diff ) {
                        array[i] = 0;
                }
                else {
                        array[array_digits - (i - diff) - 1] = number%10;
                        number /= 10;
                }
        }

}

struct std_list *std_stack_push(struct std_list **stack_ptr, struct std *std, struct std_list **stack_ptr_from_std) {
	struct std_list *list_node = malloc(sizeof(*list_node));
	*list_node = (struct std_list) {
		.std = std,
		.next = NULL,
		.prev = *stack_ptr
	};
	*stack_ptr_from_std = list_node;

	if (*stack_ptr) {
		(*stack_ptr)->next = list_node;
	}
	*stack_ptr = list_node;
}

void std_stack_rm(struct std_list **stack_ptr, struct std_list *stack_pos) {
	if (stack_pos) {
		if (stack_pos->prev) {
			stack_pos->prev->next = stack_pos->next;
		}
		if (stack_pos->next) {
			stack_pos->next->prev = stack_pos->prev;
		} else {
			*stack_ptr = stack_pos->prev;
		}
		free(stack_pos);
	}
}

void *vec(int elem_size, int len) {
	int *ptr = malloc((sizeof(int)) * 3 + elem_size*len);
	ptr = &ptr[3];
	ptr[ELEM_SIZE] = elem_size;
	ptr[LEN] = len;
	ptr[USED] = 0;
	memset(&ptr[DATA], 0, elem_size*len);
	return (void *)ptr;
}

void vec_push(void **vecptr) {

	int *vec = (int *)*vecptr;

	/* Resize vector if we have run out of space */
	if (vec[USED] == vec[LEN]) {
		vec = realloc(&vec[START], vec[ELEM_SIZE]*vec[LEN]*VEC_RESIZE_MULTIPLIER + 3*sizeof(int));
		if (!vec) {
			fprintf(stderr, "Can't realloc vector\n");
			FILEINFO
			abort();
		}
		vec = &vec[3]; // Make vec point to start of data
		vec[LEN] *= VEC_RESIZE_MULTIPLIER; // Expand listed size
	}

		vec[USED]++; // Increase "used" count
		*vecptr = (void *)vec;
}

void vec_pop(void **vecptr) {
	int *vec = (int *)*vecptr;

	/* Free vector if we are popping the only remaining element */
	if (vec[USED] > 0) {
		vec[USED]--; // Reduce "used" count
	}

	/* Reduce vector if we have unecessarily large free space */
	if (vec[LEN] > vec[USED] * VEC_RESIZE_MULTIPLIER) {
		vec = realloc(&vec[START], vec[ELEM_SIZE]*vec[USED] + 3*sizeof(int));
		if (!vec) {
			fprintf(stderr, "Can't realloc vector\n");
			FILEINFO
			abort();
		}
		vec = &vec[3]; // Make vec point to start of data
		vec[LEN] = vec[USED]; // Expand listed size
	}

		*vecptr = (void *)vec;
}

void vec_free(void *vector) {
	int *int_vec = (int *)vector;
	int_vec = &int_vec[START];
	free(int_vec);
}


