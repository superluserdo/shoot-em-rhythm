int dict_get_index(struct dict_str_void *dict, const char *key);

void *dict_get_val(struct dict_str_void *dict, const char *key);

int dict_add_keyval(struct dict_str_void *dict, const char *key, void *val);

int dict_set_val(struct dict_str_void *dict, const char *key, void *val);

