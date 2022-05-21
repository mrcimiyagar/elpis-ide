
#include "../api/IO/ConsolePrinter.h"
#include "../structures/list/List.h"

struct Stack* datStack;

void initGlobalRefs(struct Stack* ds) {
    datStack = ds;
}

void* notifyFirstUsage(void* pointer) {
    ((struct Code*)pointer)->alive = 1;
    ((struct Code*)pointer)->owner = malloc(sizeof(struct List));
    ((struct List*)((struct Code*)pointer)->owner)->base.type = NULL;
    ((struct List*)((struct Code*)pointer)->owner)->base.alive = 0;
    ((struct List*)((struct Code*)pointer)->owner)->base.owner = NULL;
    ((struct List*)((struct Code*)pointer)->owner)->size = 0;
    ((struct List*)((struct Code*)pointer)->owner)->startPointer = NULL;
    ((struct List*)((struct Code*)pointer)->owner)->endPointer = NULL;
    ((struct List*)((struct Code*)pointer)->owner)->append = NULL;
    ((struct List*)((struct Code*)pointer)->owner)->delete = NULL;
    return pointer;
}

void notifyNewUsage(void* parent, void* pointer) {
    if (pointer != NULL) {
        struct Code* cd = (struct Code*) pointer;
        struct List *list = ((struct List *) ((struct Code *) pointer)->owner);
        if (list != NULL) {
            struct ListDataItem *pItem = malloc(sizeof(struct ListDataItem));
            pItem->data = parent;
            pItem->next = NULL;
            pItem->prev = NULL;
            if (list->endPointer != NULL) {
                pItem->prev = list->endPointer;
                list->endPointer->next = pItem;
            }
            if (list->size == 0)
                list->startPointer = pItem;
            list->endPointer = pItem;
            list->size++;
        }
    }
}

void notifyParentDestroyed(void* parent, void* child) {
    if (child == NULL) return;
    if (parent == NULL) return;
    if (((struct Code*)child)->owner == NULL) return;
    struct ListDataItem *po = ((struct List *) ((struct Code *) child)->owner)->endPointer;
    while (po != NULL) {
        if (po->data == parent) {
            if (po == ((struct List *) ((struct Code *) child)->owner)->endPointer)
                ((struct List *) ((struct Code *) child)->owner)->endPointer = po->prev;
            if (((struct List *) ((struct Code *) child)->owner)->startPointer == po)
                ((struct List *) ((struct Code *) child)->owner)->startPointer = po->next;
            if (po->prev != NULL) po->prev->next = po->next;
            if (po->next != NULL) po->next->prev = po->prev;
            ((struct List *) ((struct Code *) child)->owner)->size--;
            struct ListDataItem* temp = po->prev;
            po->data = NULL;
            po->prev = NULL;
            po->next = NULL;
            free(po);
            po = temp;
        } else {
            po = po->prev;
        }
    }
}

void free_header(struct Header *h) {
    if (h) {
        free(h->name);
        free(h->value);
        free_header(h->next);
        free(h);
    }
}

void free_request(struct Request *req) {
    free(req->url);
    free(req->version);
    free_header(req->headers);
    free(req->body);
    free(req);
}

void freeList(struct List *list) {
    while (list->endPointer != NULL) {
        struct ListDataItem* temp = list->endPointer;
        list->endPointer = list->endPointer->prev;
        notifyParentDestroyed(list, temp->data);
        notifyUsageEnded(temp->data);
        temp->data = NULL;
        free(temp);
    }
    list->size = 0;
    list->startPointer = NULL;
    list->endPointer = NULL;
    free(list);
}

void dict_free(struct Dictionary* d)
{
    int i;
    struct elt *e;
    struct elt *next;
    for (i = 0; i < d->size; i++) {
        for (e = d->table[i]; e != NULL; e = next) {
            next = e->next;
            notifyParentDestroyed(d, e->value);
            notifyUsageEnded(e->value);
            e->value = NULL;
            notifyParentDestroyed(d, e->key);
            notifyUsageEnded(e->key);
            e->key = NULL;
            free(e);
        }
    }
    free(d->id);
    free(d->table);
    free(d);
}

void freeStack(struct Stack *stack) {
    struct StackDataItem* item = stack->item;
    void* temp = NULL;
    while (item != NULL) {
        temp = item->prev;
        notifyParentDestroyed(stack, item->data);
        notifyUsageEnded(item->data);
        item->data = NULL;
        free(item);
        item = temp;
        stack->stackSize--;
    }
    free(stack->stackName);
    free(stack);
}

void freeArray(struct Array *a) {
    for (int i = 0; i < (int)a->size; i++) {
        notifyParentDestroyed(a, a->array[i]);
        notifyUsageEnded(a->array[i]);
        a->array[i] = NULL;
    }
    free(a->array);
    free(a);
}

void freeData(void* data, bool force) {

    if (data == NULL || ((struct Code*)data)->alive == 0) {
        return;
    }

    ((struct Code*)data)->alive = 0;

    if (((struct Code*)data)->owner != NULL) {
        struct List* refList = ((struct List*)((struct Code*)data)->owner);
        if (refList->size > 0) {
            struct ListDataItem* refListIterator = refList->startPointer;
            while (refListIterator != NULL) {
                void *parent = refListIterator->data;
                if (parent != NULL && ((struct Code *) parent)->alive == 1) {
                    if (strcmp(((struct Code *) parent)->type, "Dictionary") == 0) {
                        struct Dictionary *d = (struct Dictionary *) parent;
                        for (int i = 0; i < d->size; i++) {
                            for (struct elt* e = d->table[i]; e != NULL; e = e->next) {
                                if (e->value != NULL && e->value == data) {
                                    notifyUsageEnded(e->key);
                                    e->key = NULL;
                                    e->value = NULL;
                                }
                            }
                        }
                    } else if (strcmp(((struct Code *) parent)->type, "Array") == 0) {
                        struct Array *d = (struct Array *) parent;
                        for (unsigned long i = 0; i < d->used; i++) {
                            if (d->array[i] == data) {
                                d->array[i] = NULL;
                            }
                        }
                    } else if (strcmp(((struct Code *) parent)->type, "Stack") == 0) {
                        struct Stack *d = (struct Stack *) parent;
                        struct StackDataItem* iterator = d->item;
                        struct StackDataItem* temp = NULL;
                        while (iterator != NULL) {
                            if (iterator->data == data) {
                                iterator->data = NULL;
                                if (temp != NULL) {
                                    temp->prev = iterator->prev;
                                }
                                else {
                                    d->item = iterator->prev;
                                }
                                free(iterator);
                                break;
                            }
                            temp = iterator;
                            iterator = iterator->prev;
                        }
                    }
                }
                refListIterator = refListIterator->next;
            }
        }
        while (refList->endPointer != NULL) {
            struct ListDataItem* temp = refList->endPointer;
            refList->endPointer = refList->endPointer->prev;
            free(temp);
        }

        free(((struct Code*)data)->owner);
        ((struct Code*)data)->owner = NULL;
    }

    if (strcmp(((struct Code*)data)->type, "HttpRequest") == 0) {
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free_request((Request*)data);
    }
    else if (strcmp(((struct Code*)data)->type, "Object") == 0) {
        notifyParentDestroyed(data, ((struct Object*)data)->value);
        notifyUsageEnded(((struct Object*)data)->value);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Array") == 0) {
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        freeArray((struct Array *) data);
    }
    else if (strcmp(((struct Code*)data)->type, "List") == 0) {
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        freeList((struct List *) data);
    }
    else if (strcmp(((struct Code*)data)->type, "Stack") == 0) {
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        freeStack((struct Stack *) data);
    }
    else if (strcmp(((struct Code*)data)->type, "Reference") == 0) {
        notifyParentDestroyed(data, ((struct Reference*)data)->currentChain);
        notifyUsageEnded(((struct Reference*)data)->currentChain);
        notifyParentDestroyed(data, ((struct Reference*)data)->restOfTheChain);
        notifyUsageEnded(((struct Reference*)data)->restOfTheChain);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Identifier") == 0) {
        freeData(((struct Identifier*)data)->id, force);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Index") == 0) {
        notifyParentDestroyed(data, ((struct Index*)data)->var);
        notifyUsageEnded(((struct Index*)data)->var);
        notifyParentDestroyed(data, ((struct Index*)data)->restOfTheChain);
        notifyUsageEnded(((struct Index*)data)->restOfTheChain);
        notifyParentDestroyed(data, ((struct Index*)data)->index);
        notifyUsageEnded(((struct Index*)data)->index);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Dictionary") == 0) {
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        dict_free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "CodeBlock") == 0) {
        free(((struct CodeBlock*)data)->base.type);
        ((struct CodeBlock*)data)->base.type = NULL;
        free(((struct CodeBlock*)data)->data);
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "CodePack") == 0) {
        free(((struct CodePack*)data)->code->base.type);
        free(((struct CodePack*)data)->code->base.owner);
        free(((struct CodePack*)data)->code);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Value") == 0) {
        if (((struct Value*)data)->valueType == 0x01)
            free(((struct StringValue *) data)->value);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "String") == 0) {
        free(((struct String*)data)->value);
        ((struct String*)data)->value = NULL;
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Pair") == 0) {
        notifyParentDestroyed(data, ((struct Pair*)data)->first);
        notifyUsageEnded(((struct Pair*)data)->first);
        notifyParentDestroyed(data, ((struct Pair*)data)->second);
        notifyUsageEnded(((struct Pair*)data)->second);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Class") == 0) {
        notifyParentDestroyed(data, ((struct Class*)data)->className);
        notifyUsageEnded(((struct Class*)data)->className);
        notifyUsageEnded(((struct Class *) data)->inheritance);
        notifyUsageEnded(((struct Class *) data)->behavior);
        notifyUsageEnded(((struct Class *) data)->properties);
        notifyUsageEnded(((struct Class*)data)->functions);
        freeData(((struct Class*)data)->constructor->body, force);
        freeData(((struct Class*)data)->constructor->params, force);
        freeList(((struct Class *) data)->constructor->base.owner);
        free(((struct Class*)data)->constructor->base.type);
        free(((struct Class*)data)->constructor);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Function") == 0) {
        freeData(((struct Function*)data)->params, force);
        freeData(((struct Function*)data)->codes, force);
        freeData(((struct Function*)data)->funcName, force);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Prop") == 0) {
        freeData(((struct Prop*)data)->value, force);
        notifyParentDestroyed(data, ((struct Prop*)data)->id);
        notifyUsageEnded(((struct Prop*)data)->id);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Empty") == 0) {
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
    else if (strcmp(((struct Code*)data)->type, "Period") == 0) {
        notifyParentDestroyed(data, ((struct Period*)data)->start);
        notifyUsageEnded(((struct Period*)data)->start);
        notifyParentDestroyed(data, ((struct Period*)data)->end);
        notifyUsageEnded(((struct Period*)data)->end);
        free(((struct Code*)data)->type);
        ((struct Code*)data)->type = NULL;
        free(data);
    }
}

void forceFree(void* pointer, struct Stack* ds) {
    datStack = ds;
    if (pointer != NULL)
        freeData(pointer, true);
}

bool isDeeperThanStackTop(void* pointer) {
    if (pointer != NULL) {
        struct List *refList = ((struct List *) ((struct Code *) pointer)->owner);
        if (refList != NULL && refList->size > 0) {
            struct ListDataItem *iterator = refList->startPointer;
            while (iterator != NULL) {
                if (iterator->data != NULL && ((struct Code *) iterator->data)->alive == 1) {
                    if (strcmp(((struct Code *) iterator->data)->type, "Stack") == 0) {
                        struct Stack *stack = (struct Stack *) iterator->data;
                        if (stack->stackName != NULL && strcmp(stack->stackName, "Exp") == 0) {
                            return false;
                        }
                    }
                }
                iterator = iterator->next;
            }
            iterator = refList->startPointer;
            while (iterator != NULL) {
                if (iterator->data != NULL && ((struct Code *) iterator->data)->alive == 1) {
                    if (strcmp(((struct Code *) iterator->data)->type, "Stack") == 0) {
                        struct Stack *stack = (struct Stack *) iterator->data;
                        if (stack->stackName != NULL && strcmp(stack->stackName, "Data") == 0) {
                            struct StackDataItem *stackIterator = stack->item;
                            while (stackIterator != NULL) {
                                if (stackIterator->data == pointer) {
                                    if (stackIterator->level <= datStack->stackSize - 1) {
                                        return false;
                                    }
                                }
                                stackIterator = stackIterator->prev;
                            }
                            return true;
                        } else {
                            if (!isDeeperThanStackTop(stack)) {
                                return false;
                            }
                        }
                    } else {
                        if (!isDeeperThanStackTop(iterator->data)) {
                            return false;
                        }
                    }
                }
                iterator = iterator->next;
            }
            return true;
        }
        return true;
    }
    return false;
}

bool notifyUsageEnded(void* pointer) {
    if (pointer != NULL && ((struct Code*)pointer)->alive == 1) {
        if (isDeeperThanStackTop(pointer)) {
            freeData(pointer, false);
            return true;
        } else {
            return false;
        }
    }
    else {
        return true;
    }
}

void* custom_malloc(size_t size) {
    return notifyFirstUsage(malloc(size));
}

void initString(void* str, size_t strLength) {
    ((struct String*)str)->base.type = malloc((strlen("String") + 1) * sizeof(char));
    strcpy(((struct String*)str)->base.type, "String");
    ((struct String*)str)->value = malloc(strLength * sizeof(char));
}