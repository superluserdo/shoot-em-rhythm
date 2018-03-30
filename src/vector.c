#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define FILEINFO fprintf(stderr, "In %s, line %d\n", __FILE__, __LINE__);
enum vector_e { ELEM_SIZE=-3, LEN=-2, USED=-1, DATA=0};

