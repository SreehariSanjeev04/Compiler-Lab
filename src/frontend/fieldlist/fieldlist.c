#include "fieldlist.h"

struct FieldList* head = NULL;
int fieldIndexCounter = 0;

/**
 * Appends a new field to the FieldList. Returns a pointer to the newly created FieldList entry, or NULL if memory allocation fails.
 * @param name The name of the field to be added.
 * @param type A pointer to the TypeTable entry representing the type of the field.
 * @return A pointer to the newly created FieldList entry, or NULL if memory allocation fails
 */
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

/**
 * Returns a pointer to the head of the FieldList.
 * @return A pointer to the head of the FieldList.
 */
struct FieldList* FieldListReturnHead() {
    return head;
}

/**
 * Sets the head of the FieldList to a new head. Returns a pointer to the new head of the FieldList.
 * @param newHead A pointer to the new head of the FieldList.
 * @return A pointer to the new head of the FieldList.
 */
struct FieldList* FieldListSetHead(struct FieldList* newHead) {
    head = newHead;
    return head;
}

