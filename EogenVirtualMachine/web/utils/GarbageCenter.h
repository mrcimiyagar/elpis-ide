
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../structures/dictionary/Dictionary.h"

void initGlobalRefs(struct Stack* ds);
void* notifyFirstUsage(void* pointer);
void notifyNewUsage(void* parent, void* pointer);
bool notifyUsageEnded(void* pointer);
void* custom_malloc(size_t size);
void initString(void* str, size_t strLength);
void freeStack(struct Stack *stack);
void freeList(struct List* list);
void dict_free(struct Dictionary* d);
void freeData(void* data, bool force);
void forceFree(void* pointer, struct Stack* ds);
bool isDeeperThanStackTop(void* pointer);
void notifyParentDestroyed(void* parent, void* child);
void free_header(struct Header *h);
void free_request(struct Request *req);