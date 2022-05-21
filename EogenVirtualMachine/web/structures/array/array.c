
#include "array.h"

void initArray(struct Array *a, size_t initialSize) {
    a->base.type = malloc((strlen("Array") + 1) * sizeof(char));
    strcpy(a->base.type, "Array");
    a->used = 0;
    a->size = initialSize;
    a->array = (void**)malloc(initialSize * sizeof(void*));
    for (int counter = 0; counter < a->size; counter++)
        a->array[counter] = NULL;
}

void insertArray(struct Array *a, void *element) {
    notifyNewUsage(a, element);
    if (a->used == a->size) {
        a->size += 100;
        a->array = (void**)realloc(a->array, a->size * sizeof(void*));
        for (int counter = a->used; counter < a->size; counter++)
            a->array[counter] = NULL;
    }
    a->array[a->used] = element;
    a->used++;
}