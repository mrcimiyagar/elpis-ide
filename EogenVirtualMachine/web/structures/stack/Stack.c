
#include "../../utils/GarbageCenter.h"

void initStack(struct Stack* stack) {
    stack->base.type = malloc((strlen("Stack") + 1) * sizeof(char));
    strcpy(stack->base.type, "Stack");
    stack->stackName = NULL;
    stack->item = NULL;
    stack->stackSize = 0;
    stack->push = push;
    stack->pop = pop;
    stack->top = top;
    stack->size = size;
    stack->isEmpty = isEmpty;
    stack->iterator = iterator;
}

void push(struct Stack* stack, void* data) {
    notifyNewUsage(stack, data);
    struct StackDataItem* pItem = malloc(sizeof(struct StackDataItem));
    pItem->level = stack->stackSize;
    pItem->data = data;
    pItem->prev = stack->item;
    stack->item = pItem;
    stack->stackSize++;
}

struct StackDataItem* iterator(struct Stack* stack) {
    struct StackDataItem* iterator = stack->item;
    return iterator;
}

void* top(struct Stack* stack) {
    if (stack->item == NULL)
        return NULL;
    return stack->item->data;
}

void* pop(struct Stack* stack) {
    if (stack->item == NULL)
        return NULL;
    void* topData = stack->item->data;
    struct StackDataItem* oldItem = stack->item;
    stack->item = stack->item->prev;
    notifyParentDestroyed(stack, topData);
    free(oldItem);
    stack->stackSize--;
    return topData;
}

int size(struct Stack* stack) {
    return stack->stackSize;
}

bool isEmpty(struct Stack* stack) {
    return stack->stackSize == 0;
}