#ifndef FIELDLIST_H
#define FIELDLIST_H
#include <typetable.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct FieldList {
    char *name;
    int fieldIndex;
    int size;
    struct TypeTable *type;
    struct FieldList *next;
} FieldList;

extern struct FieldList* FieldListHead;
extern struct FieldList* FieldListTail;
extern int fieldIndexCounter;

/**
 * Appends a new field to the list.
 * @return the new entry, or NULL if allocation fails
 */
struct FieldList* FieldListAppend(char* name, struct TypeTable* type);
/**
 * @return the head of the FieldList
 */
struct FieldList* FieldListReturnHead();

void FieldListReset();

void FieldListDestroy();
#endif