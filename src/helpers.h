/* Helper Functions */

struct std_list *std_stack_push(struct std_list **stack_ptr, struct std *std, struct std_list **stack_ptr_from_std);

void std_stack_rm(struct std_list **stack_ptr, struct std_list *stack_pos, struct std *std);

void int2array(int number, int *array, int array_digits);

void deleteList(struct monster_node** head_ref);

void *vector(int elem_size, int len);

void append_vec(void **vecptr);

void free_vec(void *vector);
