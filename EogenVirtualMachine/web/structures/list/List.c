
#include "List.h"
#include "string.h"

void initList(struct List* list) {
    list->base.type = malloc((strlen("List") + 1) * sizeof(char));
    strcpy(list->base.type, "List");
    list->size = 0;
    list->endPointer = NULL;
    list->startPointer = NULL;
    list->append = listAdd;
    list->delete = listDelete;
}

void listDelete(struct List* list, void* item) {
    struct ListDataItem* po = list->endPointer;
    while (po != NULL) {
        if (po->data == item) {
            notifyParentDestroyed(list, po->data);
            notifyUsageEnded(po->data);
            po->data = NULL;
            if (po->prev != NULL) po->prev->next = po->next;
            if (po->next != NULL) po->next->prev = po->prev;
            po->data = NULL;
            po->next = NULL;
            po->prev = NULL;
            list->size--;
            free(po);
            break;
        }
        po = po->prev;
    }
}

void listAdd(struct List* list, void* item) {
    notifyNewUsage(list, item);
    struct ListDataItem* pItem = malloc(sizeof(struct ListDataItem));
    pItem->data = item;
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