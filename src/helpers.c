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

void deleteList(struct monster_node** head_ref) {
   struct monster_node* current = *head_ref;
   struct monster_node* next;

   while (current != NULL) 
   {
       next = current->next;
       free(current);
       current = next;
   }
   *head_ref = NULL;
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


/*	Example of usage:
 *	struct test *testarray = (struct test *)vector((sizeof(struct test)), 5);
 */
