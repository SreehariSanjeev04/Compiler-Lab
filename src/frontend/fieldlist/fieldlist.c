#include "logger.h"
#include "fieldlist.h"

struct FieldList* FieldListHead = NULL;
struct FieldList* FieldListTail = NULL;
int fieldIndexCounter = 0;


struct FieldList* FieldListAppend(char* name, struct TypeTable* type) {
    struct FieldList* newField = (struct FieldList*)malloc(sizeof(struct FieldList));
    if(!newField) {
        LOG_ERROR("Memory allocation failed for FieldList\n");
        return NULL;
    }
    newField->name = name;
    newField->fieldIndex = fieldIndexCounter++;
    newField->type = type;
    newField->size = type->size;
    newField->next = NULL;

    if (FieldListHead == NULL) {
        FieldListHead = newField;
        FieldListTail = newField;
    } else {
        FieldListTail->next = newField;
        FieldListTail = newField;
    }
    return newField;
}

struct FieldList* FieldListReturnHead() {
    return FieldListHead;
}

void FieldListReset() {
    FieldListHead = FieldListTail = NULL;
}

void FieldListDestroy() {
    FieldList* current = FieldListHead;
    while (current) {
        FieldList* next = current->next;
        free(current);
        current = next;
    }
    FieldListReset();
}


