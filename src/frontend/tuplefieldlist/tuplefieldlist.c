#include "logger.h"
#include "tuplefieldlist.h"

struct TupleFieldList* TupleFieldListHead = NULL;
struct TupleFieldList* TupleFieldListTail = NULL;

int fieldIndexCounter = 0;

struct TupleFieldList* TupleFieldListGetHead() {
    return TupleFieldListHead;
}

void TupleFieldListAppend(char* name, int type) {
    TupleFieldList* node = (TupleFieldList*)malloc(sizeof(TupleFieldList));
    if(!node) {
        LOG_ERROR("TupleFieldList node allocation failed\n");
        exit(1);
    }

    node->name = strdup(name);
    node->fieldIndex = fieldIndexCounter++;
    node->type = type;
    node->size = DEFAULT_VAR_SIZE; // only scalar values at this point
    node->offset = (TupleFieldListTail == NULL) ? 0 : (TupleFieldListTail->offset + TupleFieldListTail->size);
    node->next = NULL;

    if(TupleFieldListHead == NULL) {
        // the list is empty
        TupleFieldListHead = node;
        TupleFieldListTail = node;
    } else {
        TupleFieldListTail->next = node;
        TupleFieldListTail = node;
    }
}

void TupleFieldListReset() {
    TupleFieldList* current = TupleFieldListHead;
    while(current != NULL) {
        TupleFieldList* temp = current;
        current = current->next;
        free(temp);
    }

    TupleFieldListHead = NULL;
    TupleFieldListTail = NULL;
    fieldIndexCounter = 0;
}