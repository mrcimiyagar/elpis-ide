
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../models/Models.h"

void initStack(struct Stack* stack);
void push(struct Stack* stack, void* data);
struct StackDataItem* iterator(struct Stack* stack);
void* top(struct Stack* stack);
void* pop(struct Stack* stack);
int size(struct Stack* stack);
bool isEmpty(struct Stack* stack);