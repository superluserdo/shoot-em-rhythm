#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define FILEINFO fprintf(stderr, "In %s, line %d\n", __FILE__, __LINE__);

void *Vector(int elem_size, int len) {
	int *ptr = (int *)malloc((sizeof(int)) * 3 + elem_size*len);
	ptr[0] = elem_size;
	ptr[1] = len;
	ptr[2] = 0; // used
	memset(&ptr[3], 0, elem_size*len);
	return (void *)&ptr[3];
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
 *	struct test *testarray = (struct test *)Vector((sizeof(struct test)), 5);
 */
