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

void *vector(int elem_size, int len) {
	int *ptr = malloc((sizeof(int)) * 3 + elem_size*len);
	ptr = &ptr[3];
	ptr[ELEM_SIZE] = elem_size;
	ptr[LEN] = len;
	ptr[USED] = 0;
	memset(&ptr[DATA], 0, elem_size*len);
	return (void *)ptr;
}

void append_vec(void **vecptr) {
	int *vec = (int *)*vecptr;
	int elem_size = vec[-3];
	int len = vec[-2];
	int used = vec[-1];
	if (used == len) {
		int *newvec = realloc(&vec[-3], elem_size*len*2 + 3*sizeof(int));
		if (!newvec) {
			fprintf(stderr, "Can't realloc vector\n");
			FILEINFO
			abort();
		}
		newvec = &newvec[3];
		*vecptr = (void *)newvec;
		newvec[-2] *= 2; // Expand listed size
		newvec[-1]++; // Increase "used" count
	}
}

void free_vec(void *vector) {
	int *int_vec = (int *)vector;
	int_vec = &int_vec[-3];
	free(int_vec);
}


/*	Example of usage:
 *	struct test *testarray = (struct test *)vector((sizeof(struct test)), 5);
 */
