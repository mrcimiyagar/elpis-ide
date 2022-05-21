
#include <uuid/uuid.h>
#include "../list/List.h"
#include "../../api/IO/ConsolePrinter.h"

#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (1)
#define MULTIPLIER (97)

struct Dictionary* internalDictionaryCreate(int size)
{
    struct Dictionary* d = custom_malloc(sizeof(struct Dictionary));
    d->base.type = malloc((strlen("Dictionary") + 1) * sizeof(char));
    strcpy(d->base.type, "Dictionary");
    uuid_t binuuid;
    uuid_generate(binuuid);
    char *uuid = malloc(37);
#ifdef capitaluuid
    uuid_unparse_upper(binuuid, uuid);
#elif lowercaseuuid
    uuid_unparse_lower(binuuid, uuid);
#else
    uuid_unparse(binuuid, uuid);
#endif
    d->id = uuid;
    d->size = size;
    d->n = 0;
    d->table = malloc(sizeof(struct elt*) * size);
    for(int i = 0; i < d->size; i++) d->table[i] = NULL;
    return d;
}
struct Dictionary* dict_new()
{
    return internalDictionaryCreate(512);
}
struct Dictionary* buffer_new(struct Stack* ds) {
    struct Dictionary* dict = dict_new();
    dict_add_str(ds->top(ds), dict->id, dict);
    return dict;
}
void buffer_free(struct Stack* ds, struct Dictionary* buffer) {
    dict_delete((struct Dictionary*)ds->top(ds), buffer->id, true);
}
static unsigned long hash_function(const char *s)
{
    unsigned const char *us;
    unsigned long h;
    h = 0;
    for(us = (unsigned const char *) s; *us; us++)
        h = h * MULTIPLIER + *us;
    return h;
}
static void grow( struct Dictionary* d)
{
    d->table = realloc(d->table, sizeof(struct elt*) * d->size * GROWTH_FACTOR);
    for (int counter = d->size; counter < d->size * GROWTH_FACTOR; counter++)
        d->table[counter] = NULL;
    d->size *= GROWTH_FACTOR;
}
void display_dict(struct Dictionary* dict) {
    struct List* iteratorList = toList(dict);
    struct ListDataItem* iterator = iteratorList->startPointer;
    printf("Log : Dictionary content : { ");
    while (iterator != NULL) {
        printf("%s : %s", ((struct String*)((struct Pair*)iterator->data)->first)->value, ((struct Pair*)iterator->data)->second);
        if (iterator->next != NULL)
            printf(", ");
        iterator = iterator->next;
    }
    printf(" }\n");
}
void dict_add(struct Dictionary *d, struct String *k, void *value)
{
    struct elt *e;
    notifyNewUsage(d, k);
    notifyNewUsage(d, value);
    e = malloc(sizeof(struct elt));
    unsigned long h;
    e->key = k;
    e->value = value;
    h = hash_function(k->value) % d->size;
    e->next = d->table[h];
    d->table[h] = e;
    d->n++;
    if(d->n >= d->size * MAX_LOAD_FACTOR) {
        grow(d);
    }
}
void dict_add_raw(struct Dictionary *d, char *k, void *value)
{
    struct String* str = custom_malloc(sizeof(struct String));
    initString(str, strlen(k) + 1);
    strcpy(str->value, k);
    str->value[strlen(k)] = '\0';
    struct elt *e;
    notifyNewUsage(d, str);
    e = malloc(sizeof(struct elt));
    unsigned long h;
    e->key = str;
    e->value = value;
    h = hash_function(k) % d->size;
    e->next = d->table[h];
    d->table[h] = e;
    d->n++;
    if(d->n >= d->size * MAX_LOAD_FACTOR) {
        grow(d);
    }
}
void dict_add_str(struct Dictionary *d, char *k, void *value)
{
    struct String* str = custom_malloc(sizeof(struct String));
    initString(str, strlen(k) + 1);
    strcpy(str->value, k);
    str->value[strlen(k)] = '\0';
    dict_add(d, str, value);
}
void * dict_get(struct Dictionary* d, char *key)
{
    if (d != NULL) {
        struct elt *e;
        for (e = d->table[hash_function(key) % d->size]; e != NULL; e = e->next) {
            if (e->key != NULL && !strcmp(e->key->value, key)) {
                return e->value;
            }
        }
    }
    return NULL;
}
struct List* toList(struct Dictionary* d) {
    struct List* result = custom_malloc(sizeof(struct List));
    initList(result);
    for(int i = 0; i < d->size; i++) {
        struct elt *e = d->table[i];
        while (e != NULL) {
            if (e->key != NULL) {
                struct Pair *pair = custom_malloc(sizeof(struct Pair));
                pair->base.type = malloc((strlen("Pair") + 1) * sizeof(char));
                strcpy(pair->base.type, "Pair");
                pair->first = e->key;
                pair->second = e->value;
                result->append(result, pair);
            }
            e = e->next;
        }
    }
    return result;
}
void dict_delete(struct Dictionary* d, const char *key, bool destroy)
{
    if (d != NULL) {
        struct elt **prev;
        struct elt *e;
        for (prev = &(d->table[hash_function(key) % d->size]); *prev != 0; prev = &((*prev)->next)) {
            if ((*prev)->key != NULL) {
                if (!strcmp((*prev)->key->value, key)) {
                    e = *prev;
                    *prev = e->next;
                    if (e->value != NULL) {
                    }
                    notifyParentDestroyed(d, e->key);
                    notifyUsageEnded(e->key);
                    e->key = NULL;
                    notifyParentDestroyed(d, e->value);
                    if (destroy)
                        notifyUsageEnded(e->value);
                    e->value = NULL;
                    free(e);
                    d->n--;
                    return;
                }
            }
        }
    }
}