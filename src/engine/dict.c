#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "structdef.h"
#include "dict.h"
#define DICT_RESIZE_MULT 2

/* Implementation of a (currently append-only) dictionary
   with char *keys. Does a dumb O(n) lookup. */

/*		str -> int *	*/

int dict_int_get_index(struct dict_int *dict, const char *key) {

	int index = -1; /* "Not found" */
	for (int i = 0; i < dict->num_entries; i++) {
		struct keyval_str_int entry = dict->entries[i];
		if (strcmp(key, entry.key) == 0) {
			index = i;
			break;
		}
	}

	return index;
}

int dict_int_get_val(struct dict_int *dict, const char *key, int *val) {

	val = NULL; /* "Not found" */
	for (int i = 0; i < dict->num_entries; i++) {
		struct keyval_str_int entry = dict->entries[i];
		if (strcmp(key, entry.key) == 0) {
			*val = entry.val;
			return R_SUCCESS;
		}
	}

	return R_FAILURE;
}

int dict_int_add_keyval(struct dict_int *dict, const char *key, int val) {
	if (dict->num_entries >= dict->max_entries) {
		if (dict->max_entries < 1) {
			dict->max_entries = 1;
		} else {
			dict->max_entries *= DICT_RESIZE_MULT;
		}
		dict->entries = realloc(dict->entries, dict->max_entries * sizeof(dict->entries[0]));
		if (!dict->entries) {
			abort();
		}
	}
		int index = dict->num_entries;
		dict->entries[index] = (struct keyval_str_int) {key, val};
		dict->num_entries++;
		return index;
}

int dict_int_set_val(struct dict_int *dict, const char *key, int val) {
	
	int index = dict_int_get_index(dict, key);
	if (index == -1) {
		dict_int_add_keyval(dict, key, val);
	} else {
		dict->entries[index].val = val;
	}
}
/*		str -> void *	*/

int dict_void_get_index(struct dict_void *dict, const char *key) {

	int index = -1; /* "Not found" */
	for (int i = 0; i < dict->num_entries; i++) {
		struct keyval_str_void entry = dict->entries[i];
		if (strcmp(key, entry.key) == 0) {
			index = i;
			break;
		}
	}

	return index;
}

void *dict_void_get_val(struct dict_void *dict, const char *key) {

	void *val = NULL; /* "Not found" */
	for (int i = 0; i < dict->num_entries; i++) {
		struct keyval_str_void entry = dict->entries[i];
		if (strcmp(key, entry.key) == 0) {
			val = entry.val;
			break;
		}
	}

	return val;
}

int dict_void_add_keyval(struct dict_void *dict, const char *key, void *val) {
	if (dict->num_entries >= dict->max_entries) {
		if (dict->max_entries < 1) {
			dict->max_entries = 1;
		} else {
			dict->max_entries *= DICT_RESIZE_MULT;
		}
		dict->entries = realloc(dict->entries, dict->max_entries * sizeof(dict->entries[0]));
		if (!dict->entries) {
			abort();
		}
	}
		int index = dict->num_entries;
		dict->entries[index] = (struct keyval_str_void) {key, val};
		dict->num_entries++;
		return index;
}

int dict_void_set_val(struct dict_void *dict, const char *key, void *val) {
	
	int index = dict_void_get_index(dict, key);
	if (index == -1) {
		dict_void_add_keyval(dict, key, val);
	} else {
		dict->entries[index].val = val;
	}
}
