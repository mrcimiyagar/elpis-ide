
#include <stdbool.h>
#include <stdlib.h>
#include "../../utils/GarbageCenter.h"

void initList(struct List* list);
void listDelete(struct List* list, void* item);
void listAdd(struct List* list, void* item);