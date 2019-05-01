/* Dict types for generic_anim_dict (replacement of crappier generic_bank) */

/*		str -> int	*/

struct dict_int {
	int num_entries;
	int max_entries;
	struct keyval_str_int *entries;
};

struct keyval_str_int {
	const char *key;
	int val;
};

int dict_int_get_index(struct dict_int *dict, const char *key);
int dict_int_get_val(struct dict_int *dict, const char *key, int *val);
int dict_int_add_keyval(struct dict_int *dict, const char *key, int val);
void dict_int_set_val(struct dict_int *dict, const char *key, int val);

/*		str -> void *	*/

struct dict_void {
	int num_entries;
	int max_entries;
	struct keyval_str_void *entries;
};

struct keyval_str_void {
	const char *key;
	void *val;
};

int dict_void_get_index(struct dict_void *dict, const char *key);
void *dict_void_get_val(struct dict_void *dict, const char *key);
int dict_void_add_keyval(struct dict_void *dict, const char *key, void *val);
void dict_void_set_val(struct dict_void *dict, const char *key, void *val);

