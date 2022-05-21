#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../stack/Stack.h"

/* create a new empty dictionary */
struct Dictionary* dict_new();
struct Dictionary* buffer_new(struct Stack* ds);
void buffer_free(struct Stack* ds, struct Dictionary* buffer);
void dict_add(struct Dictionary *d, struct String *k, void *value);
void dict_add_raw(struct Dictionary *d, char *k, void *value);
void dict_add_str(struct Dictionary *d, char *k, void *value);
void display_dict(struct Dictionary* dict);
void *dict_get(struct Dictionary*, char *key);
struct List* toList(struct Dictionary* d);
void dict_delete(struct Dictionary*, const char *key, bool destroy);