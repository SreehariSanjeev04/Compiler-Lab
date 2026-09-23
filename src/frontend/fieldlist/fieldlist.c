#include "fieldlist.h"

struct FieldList* head = NULL;
int fieldIndexCounter = 0;

struct FieldList* FieldListAppend(char* name, struct TypeTable* type) {
    struct FieldList* newField = (struct FieldList*)malloc(sizeof(struct FieldList));
    if(!newField) {
        fprintf(stderr, "Memory allocation failed for FieldList\n");
        return NULL;
    }
    newField->name = name;
    newField->fieldIndex = fieldIndexCounter++;
    newField->type = type;
    newField->next = NULL;

    if(head == NULL) {
        head = newField;
    } else {
        struct FieldList* current = head;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newField;
    }
    return newField;
}

struct FieldList* FieldListReturnHead() {
    return head;
}

struct FieldList* FieldListSetHead(struct FieldList* newHead) {
    head = newHead;
    return head;
}

